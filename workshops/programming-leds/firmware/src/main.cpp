// ProgrammingLEDs -- 10 WS2812B LEDs on an ESP32-C3, for an intro class on
// programming LEDs.
//
// The strip continuously blends through a palette of colors. Every LED is
// showing the same journey through the palette, just started a little later
// than the one before it, so the colors appear to travel down the strip.
// That traveling gradient is the point: it is obvious at a glance that a
// program on the ESP32-C3 is computing every pixel, not a fixed wiring.
//
// Timing note: there is no delay() anywhere, including the delay(500) that
// usually follows Serial.begin() on this chip. USB CDC still needs roughly
// half a second before the host enumerates it, so the startup banner is
// gated by an elapsed-time check in loop() instead of blocking setup().
//
// Every timer here compares a *difference* (now - start >= interval), never
// two absolute millis() values -- an absolute comparison strands the timer
// for 49.7 days if the loop is ever blocked across the millis() rollover.

#include <Arduino.h>
#include <FastLED.h>
#include <esp_log.h>

static const char *TAG = "LEDS";

#define NUM_LEDS   11
#define DATA_PIN   4
#define BRIGHTNESS 128

// USB CDC enumeration window. Serial output written before this elapses is
// dropped by the host, so the banner waits it out without blocking.
static const uint32_t SERIAL_READY_MS = 500;

// How long one color takes to blend into the next. The strip never holds a
// solid color -- it is always on its way somewhere.
static const uint32_t STEP_MS = 2000;

// Render tick, ~60fps. This is smoothness, not speed: STEP_MS alone decides
// how fast the colors change. Raising FRAME_MS makes the fade look chunkier
// without making it any slower.
static const uint32_t FRAME_MS = 16;

// The blend is measured in "units", where 256 units is one full color step.
// A whole trip around the palette is therefore PALETTE_LEN * 256 units.
static const uint16_t UNITS_PER_STEP = 256;

// How far apart neighboring LEDs sit in that blend. 0 makes the whole strip
// one solid color that fades as a block; 26 (about 256 / NUM_LEDS) spreads
// roughly one color step across the strip, which is the traveling look.
static const uint16_t LED_SPREAD = 26;

CRGB leds[NUM_LEDS];

struct NamedColor {
    CRGB        color;
    const char *name;
};

// The sequence the strip blends through, in order, then back to the start.
// Add, remove, or reorder entries here to change the show.
//
// CRGB::Red and friends are enum constants, so the CRGB(...) conversion is
// spelled out to make this a constant-initialized array.
static const NamedColor PALETTE[] = {
    { CRGB(CRGB::Red),     "red"     },
    { CRGB(CRGB::Orange),  "orange"  },
    { CRGB(CRGB::Green),   "green"   },
    { CRGB(CRGB::Blue),    "blue"    },
    { CRGB(CRGB::Magenta), "magenta" },
};
static const uint8_t PALETTE_LEN = sizeof(PALETTE) / sizeof(PALETTE[0]);

// One full trip through the palette and back around to the first color.
static const uint32_t CYCLE_MS = (uint32_t)PALETTE_LEN * STEP_MS;

static uint32_t bootMs = 0;

// Turns a position along the palette into a color.
//
// The low 8 bits say how far we are between two palette entries (0 = fully
// on the first, 255 = almost entirely on the second); the bits above that
// pick which pair we are between. blend() does the actual mixing.
static CRGB colorAt(uint16_t position) {
    uint16_t wrapped = position % (UNITS_PER_STEP * PALETTE_LEN);
    uint8_t  index   = wrapped / UNITS_PER_STEP;
    fract8   amount  = wrapped % UNITS_PER_STEP;

    uint8_t nextIndex = (index + 1) % PALETTE_LEN;
    return blend(PALETTE[index].color, PALETTE[nextIndex].color, amount);
}

// Paints one frame: every LED gets the same blend, offset a little further
// along the palette than its neighbor.
static void renderFrame() {
    // Elapsed form, so this is safe if the loop is ever blocked. Taking the
    // remainder against CYCLE_MS keeps the multiply below well inside 32 bits.
    // (2^32 is not an exact multiple of CYCLE_MS, so the pattern does jump
    // once at the 49.7-day millis() rollover. One skipped frame, then it
    // carries on -- nothing stalls.)
    uint32_t elapsed  = (millis() - bootMs) % CYCLE_MS;
    uint16_t position = (elapsed * UNITS_PER_STEP) / STEP_MS;

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = colorAt(position + i * LED_SPREAD);
    }
    FastLED.show();
}

void setup() {
    Serial.begin(115200);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    bootMs = millis();

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    // 5V / 500mA cap: 10 LEDs at full white would draw ~600mA, more than a
    // USB-powered board is meant to supply. FastLED scales brightness down to
    // stay inside this budget rather than browning out the regulator.
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    FastLED.setBrightness(BRIGHTNESS);

    // Start dark. The first frame in loop() lights the strip a few ms later.
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void loop() {
    // One-shot banner, elapsed form. Fires once USB CDC has had time to come
    // up; a rollover during the first 500ms after boot is not reachable, and
    // the elapsed comparison would survive it anyway.
    static bool bannerPrinted = false;
    if (!bannerPrinted && millis() - bootMs >= SERIAL_READY_MS) {
        bannerPrinted = true;
        Serial.println("\n=== ProgrammingLEDs ===");
        Serial.printf("%d LEDs on GPIO %d, blending through %d colors, "
                      "%lums per color\n",
                      NUM_LEDS, DATA_PIN, PALETTE_LEN,
                      (unsigned long)STEP_MS);
    }

    // FastLED's EVERY_N_MILLISECONDS is a rollover-safe elapsed-form block
    // macro (lib8tion.h). Preferred over a hand-rolled timer here because
    // FastLED is already linked. A loop blocked across the millis() wrap
    // costs one late frame, never a permanent stall.
    EVERY_N_MILLISECONDS(FRAME_MS) {
        renderFrame();
    }

    // Name the color the first LED is heading toward, so the serial log lines
    // up with what the strip is doing.
    EVERY_N_MILLISECONDS(STEP_MS) {
        static uint8_t step = 0;
        ESP_LOGI(TAG, "blending toward %s", PALETTE[step].name);
        step = (step + 1) % PALETTE_LEN;
    }

    // yield() feeds the watchdog and lets the idle task run.
    yield();
}
