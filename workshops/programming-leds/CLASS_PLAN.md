# Programming LEDs — 60 Minute Class Plan

Instructor guide for the [Programming LEDs](README.md) workshop. Attendees get
the [README](README.md); the kit list is in [parts.md](parts.md).

**Audience:** beginners. No ESP32, no PlatformIO, no LED, no soldering experience assumed.
**Outcome:** every student flashes an ESP32-C3, lights a WS2812B strip, and changes
the pattern by editing one constant and re-uploading.

The single most important design decision in this plan: **students get working
lights by minute 34.** Everything conceptual comes *after* the strip is lit. A
room full of dark strips at minute 45 is a failed class.

---

## Before class — instructor prep

### Kit per student (or per pair)

| Item | Notes |
|---|---|
| ESP32-C3 dev board | `esp32-c3-devkitm-1` or compatible (Seeed XIAO ESP32C3, etc.) |
| WS2812B strip, ~10 LEDs | Pre-cut, with 3 wires already tinned or terminated |
| USB-C **data** cable | The #1 class killer is a charge-only cable. Test every one. |
| 3 × jumper wires or a screw terminal | If not soldering |
| Laptop | VS Code + PlatformIO **installed before class** |

### Pre-class homework email (send 3+ days ahead)

1. Install [VS Code](https://code.visualstudio.com/).
2. Install the **PlatformIO IDE** extension from the Extensions marketplace.
3. Open VS Code once after install and let PlatformIO finish its first-run setup
   (it downloads a Python core — several minutes, needs internet).
4. Clone or download this repo, open `workshops/programming-leds/firmware/` in
   VS Code, and run **Build** once. First build downloads the Espressif toolchain and FastLED: ~5 minutes
   and several hundred MB. **Doing this live in class will eat 20 minutes.**

If a student shows up without this done, pair them with someone who did it.

### Instructor prep

- Build and run one board yourself the morning of. Confirm the strip lights.
- Have 2 spare boards, 2 spare strips, 2 spare cables. Something will be DOA.
- Bring a USB power meter if you have one — it makes the current discussion real.
- **Check `NUM_LEDS` matches the strips you actually hand out.** It is currently
  `11` in `firmware/src/main.cpp` while the README says 10. Set it to the real count
  before class, or make discovering that mismatch the first exercise.
- Draw the wiring diagram on the whiteboard before students arrive:

```
   ESP32-C3                      WS2812B strip
  ┌─────────┐                   ┌──────────────────┐
  │      5V ├───────────────────┤ +5V              │
  │     GND ├───────────────────┤ GND              │
  │  GPIO 4 ├───────────────────┤ DIN  →  (arrows) │
  └─────────┘                   └──────────────────┘
                                 follow the arrows!
```

---

## Timeline

| Time | Segment | Mode |
|---|---|---|
| 0:00 – 0:04 | Hook + what we're building | Demo |
| 0:04 – 0:11 | Hardware tour + electronics basics | Talk |
| 0:11 – 0:17 | Serial: one wire, one thing at a time | Talk |
| 0:17 – 0:24 | Wire it up | Hands-on |
| 0:24 – 0:34 | PlatformIO: build and upload | Hands-on |
| 0:34 – 0:39 | Serial monitor + baud rate | Hands-on |
| 0:39 – 0:50 | Change the code (3 exercises) | Hands-on |
| 0:50 – 0:57 | How it works + why no `delay()` | Talk |
| 0:57 – 1:00 | Where to go next | Talk |

Buffer is deliberately thin. If you fall behind, **cut the "How it works"
segment**, not the hands-on time. Students remember what they did, not what
they heard.

---

## 0:00 – 0:04 · Hook

Have your board already running on the bench with the strip lit and blending.
Don't explain it yet. Ask: *"What's making that happen?"*

Land these three sentences:

- That strip has no pattern built into it. A program on that little chip is
  calculating the color of **every single LED, 60 times a second**.
- Each LED is doing the same thing as its neighbor, just slightly behind — that's
  the only reason it looks like the color is moving.
- In an hour you will each have that running, and you'll have changed what it
  does.

Then hold up the parts: *"This is a computer. This is a strip of tiny computers.
Three wires connect them."*

---

## 0:04 – 0:11 · Hardware tour and electronics basics

### The board

- An **ESP32-C3** is a full computer: 160 MHz CPU, 400 KB RAM, WiFi, Bluetooth,
  ~20 GPIO pins. Costs a few dollars.
- **GPIO** = General Purpose Input/Output. A pin the program can turn on and off,
  or read. We use GPIO 4 as an output.
- It runs **one program**, forever, starting the instant it gets power. There is
  no OS, no desktop, no "launching an app." Power on → your code runs.

### The strip

- Each WS2812B is a tiny controller chip with a red, a green and a blue LED
  bonded onto it. They're often called "NeoPixels" (Adafruit's brand name).
- **Three wires: 5V, GND, DATA.** There is no clock wire, which is worth a whole
  segment on its own — that's next.
- The LEDs are chained: `DIN → DOUT → DIN → DOUT →`. Save the details for the
  serial segment; here just plant that **it is a chain, not ten parallel wires**.

### Electronics: voltage, current, power

Use the water analogy, it works:

- **Voltage (V)** = water pressure. How hard the electricity is pushed.
- **Current (A)** = flow rate. How much is actually moving.
- **Power (W) = V × A.** The stuff that costs money and makes heat.

### Why 5V and not 3.3V or 12V?

This is the richest teaching moment in the class. Cover it at whatever depth
the room can take.

**Why the strip wants 5V:** the WS2812B's datasheet says 3.5V–5.3V. The blue and
green LED dice inside need roughly 3.0–3.2V across them just to emit light at
all, plus headroom for the driver circuitry. At 3.3V supply, blue and green go
dim and the color balance goes wrong. 5V is the lowest common supply rail that
comfortably clears that.

**Why 3.3V is the board's number:** modern silicon is built with tiny transistors
that would break down at 5V. Lower voltage also means dramatically lower power
(power scales roughly with V²), which is why phones and microcontrollers moved
to 3.3V and below. **ESP32 GPIO pins are not 5V tolerant** — putting 5V into an
input pin can damage it. Data flows *out* of the board here, so we're fine.

**The subtle problem worth naming out loud:** the WS2812B expects a logic "high"
on its data pin of about 0.7 × 5V = **3.5V**. Our ESP32-C3 outputs **3.3V**.
That is *technically out of spec*. It works reliably in practice for short runs
and few LEDs, which is why this class works — but it's the real explanation for
"my strip flickers randomly" in bigger projects. The professional fixes:

- A level shifter (74AHCT125) between GPIO and DIN.
- Power the strip from 4.5V instead of 5V, lowering its threshold to ~3.15V.
- Sacrifice the first LED as a level shifter (its output is a clean 5V).
- Use 3.3V-native pixels: SK6812, or the newer WS2812B-V5.

**Why 12V exists:** for long runs. Since P = V × I, delivering the same power at
12V needs a quarter the current of 5V. Less current means thinner wire, less
heat, and much less **voltage drop** — the far end of a long 5V strip goes dim
and orange because the copper itself has resistance and eats volts. The tradeoff:
12V addressable strips (WS2815, WS2811) usually control LEDs in **groups of
three**, so you get one third the addressable resolution, and the extra 7V is
burned off as heat by onboard regulators or resistors.

**Rule of thumb to give them:** short and dense → 5V. Long installs and
architectural runs → 12V or 24V.

### The current budget (this one is practical)

- One WS2812B at full white = 3 LEDs × ~20 mA = **~60 mA**.
- 10 LEDs at full white = **~600 mA**.
- A USB 2.0 port is specified to supply **500 mA**.

So a strip that looks small can out-draw the port powering it, and the board
browns out and reboots — the classic "why does it keep restarting when I go to
white?" This is exactly what these two lines in `setup()` are for:

```cpp
FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
FastLED.setBrightness(BRIGHTNESS);
```

FastLED estimates the draw of every frame and scales brightness down to stay
under the cap. Point out that at `BRIGHTNESS 128` we're already near half power,
which is also why the strip looks fine.

---

## 0:11 – 0:17 · Serial: one wire, one thing at a time

Do this **before** wiring. It is the concept that makes the wiring rules obvious
instead of arbitrary, and it sets up the serial monitor later.

### What "serial" means

To send a number to something, you have two choices. Give it **eight wires** and
put all eight bits out at once — *parallel* — or give it **one wire** and send
the bits one after another in time — *serial*.

> Serial trades wires for time. It's slower per wire, but one wire is cheap,
> flexible, and doesn't need eight things to stay perfectly in step.

Almost everything modern picked serial: USB (the S is Serial), Ethernet, SATA,
HDMI. Both links in today's project are serial — the strip, and the log going
back to the laptop.

### What actually leaves GPIO 4

Ask the room: *"If the strip has one data wire and ten LEDs, how does the
tenth LED get its color?"* Let them guess before answering.

FastLED sends **24 bits per LED** — 8 green, 8 red, 8 blue for this chip — as
one continuous burst, in strip order, with no gaps:

```
GPIO 4 ──►  [LED0: 24 bits][LED1: 24 bits][LED2: 24 bits] … [LED9: 24 bits]  ─── silence ───►
            └──────────────── 10 × 24 = 240 bits, ~300 µs ────────────────┘   (>50 µs = "show it")
```

Then it stops for more than 50 µs. That silence is the **reset/latch**: it means
*frame over, everybody display what you're holding.* Which is why the whole strip
updates at once rather than rippling.

### The pass-along — the important bit

The chain works like handing a stack of cards down a line of people, where each
person takes the top card and passes the rest on:

```
      board          LED 0            LED 1            LED 2           LED 9
        │        ┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
 10 vals└───────►│DIN   DOUT│────►│DIN   DOUT│────►│DIN   DOUT│ … ──►│DIN   DOUT│──► (nothing)
                 │ keeps #0 │     │ keeps #1 │     │ keeps #2 │     │ keeps #9 │
                 │ passes 9 │     │ passes 8 │     │ passes 7 │     │ passes 0 │
                 └──────────┘     └──────────┘     └──────────┘     └──────────┘
```

The first chip **keeps the first value for itself** and forwards everything
after it out of DOUT. The second chip sees that shortened stream, keeps the new
first value, forwards the rest. By the tenth LED there is exactly one value left
and nothing to pass on.

Say the consequence out loud, because four separate things fall out of it:

1. **No LED has an address.** Nothing in that stream says "LED 7." *Position in
   the chain is the address.* `leds[0]` in the code is simply the pixel nearest
   the board.
2. **Direction is physical.** The strip has **arrows printed on it**. Data only
   moves that way — a WS2812B cannot pass anything backwards. Feed the DOUT end
   and every chip is waiting for data that never arrives. **Nothing lights, and
   nothing warns you.** This is the single most common "it's broken" of the day.
3. **A dead pixel cuts the chain.** Everything upstream of it works, everything
   downstream goes dark, because the break is in the relay itself.
4. **Count mismatches fail quietly.** Send 10 values to a 12-LED strip and the
   last two never receive anything — they stay dark. Send 12 to a 10-LED strip
   and the extra two are simply passed off the end into nothing.

That last point is a good moment to have them look at `NUM_LEDS` in
`firmware/src/main.cpp` and check it against the strip in their hand.

### Why there's no clock wire

*"If the bits are just going down one wire, how does the chip know where one bit
ends and the next begins?"*

- Two answers exist. Send a **clock** on a second wire that ticks once per bit
  (that's SPI, and that's what APA102/SK9822 "DotStar" strips do), or encode the
  timing **into the data itself**.
- WS2812B does the second. Bits go out at **800 kHz**, and a `1` and a `0` are
  *both* pulses — they differ only in how long the pulse stays high: ~0.8 µs for
  a one, ~0.4 µs for a zero. The chip measures pulse width.
- **Upside:** one less wire, cheaper strip, simpler connectors.
- **Downside:** the timing is brutally strict — a few hundred nanoseconds of slop
  and the frame is garbage. You cannot reliably produce this by hand from a
  general-purpose loop while WiFi interrupts are firing.
- So the ESP32 has a dedicated **RMT** peripheral that generates these pulses in
  hardware, and FastLED drives it. Good moment to say: *libraries exist to hide
  exactly this kind of thing.*

### Baud rate — the other serial link

The strip is one serial connection. There's a second one going the other way:
the ESP32 talking back to the laptop over USB, which is what the serial monitor
shows.

- **Baud rate is just "bits per second."** 115200 baud ≈ 115,200 bits/sec.
- Same problem as the strip, same trade: **no clock wire.** The receiver has no
  tick to follow, so it samples the line at a rate both sides agreed on in
  advance. Nobody negotiates it — it is simply configured identically at both
  ends.
- Get it wrong and you don't get silence, you get **garbage characters**:
  `­¾ýÿ¿þ`. The receiver is sampling a real signal at the wrong moments. That
  distinctive symptom is worth showing them, because it tells you the wiring is
  *fine* and only the speed is wrong.
- It has to match in **two places** in this project:

  ```
  firmware/src/main.cpp      Serial.begin(115200);
  firmware/platformio.ini    monitor_speed = 115200
  ```

- Why 115200 and not faster? It's the long-standing default that every tool
  agrees on, and it's fast enough that printing doesn't slow the program down
  much. Higher rates work; they just have less margin for error.

Optional 60-second demo, if you're on schedule: change `monitor_speed` to `9600`,
reopen the monitor, show the garbage, change it back. Nobody forgets it after
seeing it.

Worth naming the contrast: **both links are serial, with completely different
rules.** The strip runs at 800,000 bits/sec, one direction only, with no
acknowledgment and no error checking of any kind — it just fires and hopes. The
USB log runs at 115,200 bits/sec and can go both ways.

---

## 0:17 – 0:24 · Wire it up

Rules on the whiteboard before anyone touches anything:

1. **Unplug USB before wiring.** Plug in only when the wiring is checked.
2. **5V → +5V, GND → GND, GPIO 4 → DIN.**
3. **Follow the arrows on the strip.** DIN end only — this is the pass-along
   chain from the last segment, so the wrong end means every chip sits waiting
   for data that never arrives.
4. Don't let bare 5V and GND touch each other.

Walk the room and check every single connection before anyone plugs in. This is
worth the four minutes — a shorted board wastes far more.

Two things worth mentioning while you walk around, even though this class
doesn't need them:

- Real installs put a **330–470 Ω resistor** in series with the data line (tames
  reflections on the wire) and a **1000 µF capacitor** across 5V and GND at the
  strip (absorbs the inrush when every LED switches at once).
- **Ground must be shared.** "Voltage" is always a *difference*. If the strip and
  the board don't share a ground, the strip has no reference for what 3.3V even
  means, and the data is meaningless. This is the #1 mistake when the strip gets
  its own power supply later.

---

## 0:24 – 0:34 · PlatformIO: build and upload

Open `workshops/programming-leds/firmware/` in VS Code — PlatformIO needs the
folder holding `platformio.ini`, not the repository root. Give them the mental
model first:

> Your laptop can't run this code — it's a different kind of processor. So we
> **cross-compile**: build on your Mac/PC, produce a binary for the ESP32, and
> copy it over USB into the chip's flash memory. PlatformIO is the thing that
> downloads the right compiler, the right libraries, and the right upload tool
> for whatever board you name.

Tour `platformio.ini` — it's ~15 lines and it's the whole configuration:

```ini
[env:esp32c3]
platform = espressif32          ; which chip family
board = esp32-c3-devkitm-1      ; the exact board — pins, flash size, upload logic
framework = arduino             ; the API style: setup() / loop()
monitor_speed = 115200          ; serial speed, must match Serial.begin()
lib_deps =
    FastLED                     ; downloaded automatically on first build
```

Emphasize `lib_deps`: **you never download a library by hand.** Name it, build,
it appears. That is most of PlatformIO's value.

Then the buttons on the blue status bar at the bottom of VS Code:

| Icon | Action | Shortcut |
|---|---|---|
| ✓ | Build | `Cmd/Ctrl + Alt + B` |
| → | Upload (build + flash) | `Cmd/Ctrl + Alt + U` |
| 🔌 | Serial Monitor | `Cmd/Ctrl + Alt + S` |
| 🗑 | Clean | — |

**Do it:** Build first (should be fast if they pre-built), then Upload.
Lights on. Let the room react.

Point out the `firmware/.pio/` folder: it's the build cache, it's gitignored, and
deleting it is the standard fix when the build goes weird.

---

## 0:34 – 0:39 · Serial monitor and baud rate

Open the monitor. They should see:

```
=== ProgrammingLEDs ===
10 LEDs on GPIO 4, blending through 5 colors, 2000ms per color
[I][LEDS] blending toward red
```

Teaching points:

- `Serial.println()` is `print()` debugging for hardware. It's how you find out
  what a device with no screen is thinking.
- **Baud rate, in practice.** This is the same "no clock wire, both sides agree
  on a speed" idea from the serial segment, now with a keyboard in front of them.
  `Serial.begin(115200)` in `firmware/src/main.cpp` and `monitor_speed = 115200` in
  `firmware/platformio.ini` must agree. If the log is garbage characters rather than
  nothing at all, the link is fine and only the speed is wrong.
- If you skipped the 9600-baud demo earlier, this is the second chance to run
  it: break `monitor_speed`, reopen, look at the garbage, put it back.
- **`ESP_LOGI` vs `Serial.println`:** the ESP-IDF logging macros add a level, a
  timestamp and a tag, and can be filtered or compiled out entirely by changing
  `CORE_DEBUG_LEVEL` in `firmware/platformio.ini`. Real firmware uses levels; `println` is
  for quick pokes.
- **Close the monitor before uploading.** It holds the serial port, and the
  upload fails with `Could not exclusively lock port`. This *will* happen to
  someone; put it on the whiteboard now.

---

## 0:39 – 0:50 · Change the code

This is the heart of the class. All the constants are at the top of
`firmware/src/main.cpp`. Run three rounds of edit → upload → watch. Ask them to **predict
out loud before uploading**.

**Exercise 1 — `LED_SPREAD` (understanding the pattern).**
Set it to `0`. The whole strip becomes one solid color that fades as a block —
this is the proof that the travelling look is just an offset. Then try `128`:
the strip spans half the palette at once.

**Exercise 2 — `STEP_MS` (understanding time).**
Drop it to `200` and it races. Raise it to `10000` and it drifts so slowly you
can't catch a color changing. Ask: *what would `1` do?* (Answer: faster than the
60 fps render tick can show — the connection to `FRAME_MS` writes itself.)

**Exercise 3 — `PALETTE[]` (understanding data vs code).**
Add a color. Remove one. Reorder them. Nothing else in the program changes,
because `PALETTE_LEN` is computed from the array. Name that: **the pattern lives
in data, not in the logic.**

Stretch for anyone racing ahead:

- Set `BRIGHTNESS` to `255` and watch the serial log / power draw. Then to `10`.
- Make only the even LEDs light: `if (i % 2) leds[i] = CRGB::Black;`
- Reverse the direction: `position - i * LED_SPREAD`.
- Change `WS2812B, DATA_PIN, GRB` to `RGB` and watch every color come out wrong.
  Then explain: manufacturers wire the three dice in different orders, and the
  library needs to be told. **Red showing as green is a color-order bug, not a
  wiring bug.**

---

## 0:50 – 0:57 · How it works, and the `delay()` lesson

Sketch on the board:

```
  time ──► position ──► color
   (millis)   (0..1279)    (blend of two palette entries)
```

1. `renderFrame()` runs ~60×/sec.
2. It converts "how long has the board been on" into a **position** along the
   palette. 256 units = one color step, so 0 = pure red, 128 = halfway to
   orange, 256 = pure orange.
3. `colorAt()` splits that number: the upper bits pick *which two colors*, the
   lower 8 bits say *how far between them*. `blend()` mixes.
4. Each LED asks for `position + i * LED_SPREAD` — its own slightly-later
   position. That one `+ i *` is the entire animation.

**Then the most transferable idea in the class — there is no `delay()` anywhere.**

```cpp
// The tempting way:              // What this project does:
digitalWrite(pin, HIGH);          if (millis() - lastRun >= interval) {
delay(1000);                          lastRun = millis();
digitalWrite(pin, LOW);               doTheThing();
delay(1000);                      }
```

`delay()` doesn't schedule anything — it **stops the entire program**. Nothing
else can happen: no button press noticed, no sensor read, no serial command
answered. Everything after this class (buttons, sensors, WiFi, MQTT) requires
the right-hand pattern.

If there's time, the pro detail: notice the timers compare a **difference**
(`now - start >= interval`), never two absolute `millis()` values. `millis()`
wraps to zero after 49.7 days. The subtraction form is immune to that; the
absolute form can strand a timer for seven weeks. FastLED's
`EVERY_N_MILLISECONDS` macro does the safe thing for you.

---

## 0:57 – 1:00 · Where to go next

- Add a button that changes the palette (and notice you *couldn't* have, with
  `delay()`).
- Longer strips + a proper 5V supply, injecting power at both ends.
- FastLED's built-in `CRGBPalette16`, `fill_rainbow()`, `beatsin8()`, `noise16()`.
- Turn WiFi back on: control the strip from a phone, or from Home Assistant.
- Look at [WLED](https://kno.wled.ge/) — the same hardware, a finished product.

Point them at the README's "Things to change" table as the take-home.

---

## Troubleshooting cheat sheet

Print this. Tape it to the bench.

| Symptom | Cause | Fix |
|---|---|---|
| No serial port in PlatformIO | Charge-only USB cable | Swap cable — this is it ~70% of the time |
| No serial port, cable is fine | Missing USB driver | ESP32-C3 uses native USB; on older boards install CP210x/CH340 drivers |
| `Could not exclusively lock port` | Serial monitor is open | Close the monitor, re-upload |
| Upload fails / chip not detected | Not in bootloader | Hold **BOOT**, tap **RESET**, release BOOT, upload |
| Upload succeeds, no lights | Data on the wrong end | Follow the arrows — DIN, not DOUT |
| Upload succeeds, no lights | Wrong pin | `DATA_PIN` must match the physical GPIO |
| Upload succeeds, no lights | No common ground | GND must be shared, always |
| First LED lights, rest dark | Broken LED or bad joint | Cut before the dead pixel, or reflow it |
| Wrong colors (red shows green) | Color order | Change `GRB` to `RGB` in `addLeds<>` |
| Random flicker | 3.3V data into a 5V strip | Short wires today; level shifter in a real build |
| Board reboots on bright colors | Current draw over budget | Lower `BRIGHTNESS`, or external 5V supply |
| Garbage in serial monitor | Baud mismatch | `monitor_speed` must equal `Serial.begin()` |
| Build fails after weird edits | Stale cache | Delete `firmware/.pio/`, rebuild |

---

## Optional deeper topics (if the room is advanced or you have 90 minutes)

- **Gamma correction.** Human brightness perception is logarithmic, so a linear
  PWM ramp looks wrong. `FastLED.setCorrection()` / gamma tables.
- **Additive color.** Screens and LEDs mix light (RGB, additive); paint and ink
  mix pigment (CMY, subtractive). Why red + green = yellow here but mud in paint.
- **HSV vs RGB.** `CHSV` gives you a hue dial — vastly easier for animation than
  interpolating RGB triples. Show `fill_rainbow()`.
- **Why the frame rate is capped.** 800 kHz × 24 bits × N LEDs = the strip's
  transmit time. 300 LEDs ≈ 9 ms per frame, which puts a hard ceiling near
  100 fps and explains why huge installs get split across multiple pins.
- **What "addressable" really means.** Contrast with dumb analog RGB strips,
  where the whole strip is one color driven by 3 MOSFETs.
- **The bootloader and flash memory.** What actually happens during upload; why
  the code survives a power cycle; the flash-write-cycle limit.
- **Watchdog timers.** Why `yield()` is in `loop()` and what happens if you block
  for seconds on an ESP32.
