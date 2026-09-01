# Open Circuit SF — Workshops

<https://github.com/OpenCircuitSF/Workshops>

Workshop material for [Open Circuit SF](https://www.opencircuitsf.com): the
written guides attendees follow, and the firmware they flash while following
them.

Open Circuit SF runs hands-on electronics workshops in San Francisco — ESP32
microcontrollers, Raspberry Pi, Home Assistant, addressable LEDs, sensors, IR,
buttons, and soldering. Absolute beginners are welcome, tools and parts are
provided, and the group is venue-independent. This repository is the source of
truth for what gets taught and what gets built.

The public site, workshop listings, and mailing list live in the separate
[`Website`](../Website) repository. No site code belongs here.

## Layout

Each workshop is one self-contained directory. The directory name is the slug
used on the website.

```
workshops/
  programming-leds/
    README.md            # the guide attendees follow, start to finish
    CLASS_PLAN.md        # instructor timeline and teaching notes
    parts.md             # bill of materials, with sources and rough cost
    firmware/            # PlatformIO project (platformio.ini, src/, lib/)
      platformio.ini
      src/main.cpp
    assets/              # wiring diagrams, photos, pinout images
```

## Workshops

| Workshop | Length | Level | Builds |
|---|---|---|---|
| [Programming LEDs](workshops/programming-leds/) | 60 min | Beginner | ESP32-C3 driving a WS2812B strip through a blending color palette |

Shared material lives at the top level:

| Path | Contents |
|---|---|
| `workshops/` | One directory per workshop, named by slug |
| `docs/` | Cross-workshop guides — toolchain setup, driver installs, safety |
| `lib/` | C++ code shared by more than one workshop's firmware |

A workshop's `README.md` is the deliverable. The firmware exists to make that
README work; if the two disagree, the README is wrong until it is fixed.

## Writing a workshop guide

- **Assume no prior experience.** Someone who has never opened a terminal
  should reach a blinking LED without asking a question out loud.
- **Number the steps.** Attendees are following along on a laptop in a noisy
  room, and they will lose their place.
- **State the expected result after every step** ("the LED blinks once per
  second"), so a person who is stuck knows exactly which step failed.
- **List the parts before the steps**, with the exact board and variant. "An
  ESP32" is not enough; a DevKit v1 and a C3 have different pinouts.
- **Put the troubleshooting at the end**, covering at minimum: board not
  appearing as a serial port, upload timeouts, and garbled serial output.

## Firmware conventions

Firmware is C++ built with [PlatformIO](https://platformio.org/), Arduino
framework, targeting ESP32 unless a workshop says otherwise.

- One PlatformIO project per workshop, in that workshop's `firmware/`.
- Pin assignments go in named `constexpr` at the top of `main.cpp`, never as
  bare numbers in the body — attendees rewire, and they need one place to edit.
- Keep the main loop non-blocking: use `millis()` deltas, not `delay()`, for
  anything an attendee will later extend.
- No secrets in source. WiFi credentials come from a git-ignored
  `firmware/include/secrets.h`, alongside a committed `secrets.h.example`.

Build and flash from a workshop's `firmware/` directory:

```bash
pio run                 # compile
pio run -t upload       # compile and flash the attached board
pio device monitor      # serial output
```

## Contributing

Run the workshop yourself, end to end, on a clean machine before publishing
the guide. Every guide in here is meant to have been tested by someone
following it literally, not by its author from memory.

## License

See [`LICENSE`](LICENSE).
