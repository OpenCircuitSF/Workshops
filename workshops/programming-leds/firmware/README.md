# Programming LEDs — Firmware

PlatformIO project for the [Programming LEDs](../README.md) workshop: an
ESP32-C3 driving a WS2812B strip through a blending color palette.

```bash
pio run                  # build
pio run --target upload  # flash over USB
pio device monitor       # serial log, 115200 baud
```

Everything a student edits is in the constants at the top of
[`src/main.cpp`](src/main.cpp). `platformio.ini` is the entire build config —
board, framework, serial speed, and the one `lib_deps` entry (FastLED) that
PlatformIO downloads on the first build.

Set `NUM_LEDS` to the number of LEDs actually on the strip before class.

See the [workshop README](../README.md) for what the code does and
[`../CLASS_PLAN.md`](../CLASS_PLAN.md) for the troubleshooting table.
