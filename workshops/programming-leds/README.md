# Programming LEDs

An intro workshop: light up a WS2812B strip from an ESP32-C3 and change what it
does by editing one constant. No WiFi, no MQTT, no OTA — the board starts
animating as soon as it is powered.

**Duration:** 60 minutes · **Level:** absolute beginner · **Prerequisites:** none

| File | For |
|---|---|
| [`CLASS_PLAN.md`](CLASS_PLAN.md) | **Instructors** — minute-by-minute timeline, teaching notes, troubleshooting cheat sheet |
| [`parts.md`](parts.md) | **Instructors** — per-student kit and what to buy |
| This README | **Attendees** — what it does, how to build it, what to change |
| [`firmware/`](firmware/) | The PlatformIO project |

## Behavior

The strip blends red → orange → green → blue → magenta → red, forever. It never
holds a solid color; it is always on its way to the next one, two seconds per
step.

Every LED follows the same journey through the palette, just started slightly
later than the one before it, so the colors appear to travel down the strip.
That traveling gradient is the teaching point: it is obvious at a glance that a
program is computing every pixel, not a fixed wiring.

## Hardware

- **Microcontroller**: ESP32-C3 (`esp32-c3-devkitm-1`)
- **LEDs**: WS2812B on GPIO 4, GRB color order
- **Power**: capped at 5V / 500mA in firmware

```
   ESP32-C3                      WS2812B strip
  ┌─────────┐                   ┌──────────────────┐
  │      5V ├───────────────────┤ +5V              │
  │     GND ├───────────────────┤ GND              │
  │  GPIO 4 ├───────────────────┤ DIN  →  (arrows) │
  └─────────┘                   └──────────────────┘
                                 follow the arrows!
```

Wire the strip's data input to GPIO 4, its ground to a board ground, and its 5V
to the board's 5V pin. Ten LEDs at full white would draw about 600mA, more than
a USB-powered board is meant to supply, so FastLED scales brightness down to
stay inside the 500mA budget. If the strip has its own supply, raise the limit
in `setup()`.

> **Set `NUM_LEDS` to match your strip.** It ships as `11`; the prose here
> assumes 10. Fix it before class, or hand the mismatch to the room as the
> first exercise — a count that is too high leaves the extra values falling off
> the end of the chain, and one too low leaves the last pixels dark.

## Things to change

All of it lives at the top of [`firmware/src/main.cpp`](firmware/src/main.cpp).

| Constant | Default | Controls |
|---|---|---|
| `NUM_LEDS` | 11 | LEDs on the strip |
| `DATA_PIN` | 4 | GPIO driving the strip |
| `BRIGHTNESS` | 128 | Master brightness, 0–255 |
| `STEP_MS` | 2000 | Time to blend from one color to the next |
| `LED_SPREAD` | 26 | How far apart neighboring LEDs sit in the blend |
| `FRAME_MS` | 16 | Render tick (~60fps) — smoothness, not speed |

Good first exercises:

- Set `LED_SPREAD` to `0`. The whole strip becomes one solid color that fades as
  a block. Set it to `128` and the strip spans half the palette at once.
- Drop `STEP_MS` to `200` and watch it race; raise it to `10000` for a slow
  drift where individual color changes are hard to catch.
- Edit `PALETTE[]` — add a color, remove one, reorder them. The rest of the code
  adapts on its own.
- Change `NUM_LEDS` to match however many LEDs are actually wired up.

## How it works

`renderFrame()` runs about 60 times a second. Each time, it works out a single
number — the *position* along the palette — from how long the board has been
running. 256 units of position is one full color step, so position `0` is pure
red, `128` is halfway between red and orange, `256` is pure orange, and so on.

`colorAt()` turns a position into a color: the top bits pick which two palette
entries we are between, the bottom 8 bits say how far between them, and
FastLED's `blend()` mixes them.

Each LED asks for its own position — `position + i * LED_SPREAD` — which is why
LED 3 is always a little ahead of LED 2.

## Timing

There is no `delay()` anywhere, including the `delay(500)` that usually follows
`Serial.begin()` on this chip — the startup banner is gated by an elapsed-time
check in `loop()` instead of blocking `setup()`.

Every timer compares a *difference* (`now - start >= interval`), never two
absolute `millis()` values. An absolute comparison strands a timer for 49.7 days
if the loop is ever blocked across the `millis()` rollover; the elapsed form
costs one late frame at worst.

The reason this matters for a class: `delay()` looks like the easy way to make
lights change, but it stops the whole program. Nothing else — a button, a
sensor, a serial command — can be noticed while the board is sitting in a
`delay()`. Doing the timing with `millis()` leaves the loop free.

## Build

This project uses [PlatformIO](https://platformio.org/). Run these from
[`firmware/`](firmware/):

```bash
pio run                  # build
pio run --target upload  # flash over USB
pio device monitor       # watch the serial log
```

Close the serial monitor before uploading — it holds the port and the upload
will fail with `Could not exclusively lock port`.

Serial output is available at 115200 baud via USB CDC (`ARDUINO_USB_MODE=1`,
`ARDUINO_USB_CDC_ON_BOOT=1`). Logging goes to both `Serial` and ESP-IDF's
`ESP_LOGI` at `CORE_DEBUG_LEVEL=3`.

Stuck? The troubleshooting table at the end of [`CLASS_PLAN.md`](CLASS_PLAN.md)
covers the failures that actually happen in a room: charge-only USB cables,
data wired to the DOUT end, baud mismatches, and brownouts on bright colors.
