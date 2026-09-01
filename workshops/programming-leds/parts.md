# Programming LEDs — Parts

One kit per student, or per pair if boards are short. Quantities are for a
class of 10 plus the spares you will need.

## Per student

| Item | Qty | Notes |
|---|---|---|
| ESP32-C3 dev board | 1 | `esp32-c3-devkitm-1` or compatible (Seeed XIAO ESP32C3, etc.) |
| WS2812B strip, ~10 LEDs | 1 | Pre-cut, with the 3 wires already tinned or terminated |
| USB-C **data** cable | 1 | Not charge-only — see below |
| Jumper wires or screw terminal | 3 | If the class is not soldering |
| Laptop | 1 | VS Code + PlatformIO installed **before** class |

## Instructor spares

Bring at least 2 of each. Something is always dead on arrival.

| Item | Qty |
|---|---|
| ESP32-C3 dev board | 2 |
| WS2812B strip | 2 |
| USB-C data cable | 2 |
| USB power meter | 1 (optional — makes the current budget discussion real) |

## The cable

**Test every cable before class.** A charge-only USB cable is the single most
common reason a student cannot flash a board, it accounts for roughly 70% of
"no serial port" failures, and it is indistinguishable from a working cable by
sight. Plug each one into a board and confirm a serial port appears.

## Not needed for this workshop

Real installations want these; a 10-LED strip on a bench does not, and adding
them costs class time. Mention them, do not hand them out:

- **330–470 Ω resistor** in series with the data line — tames reflections on
  longer wire runs.
- **1000 µF capacitor** across 5V and GND at the strip — absorbs the inrush
  when every LED switches on at once.
- **74AHCT125 level shifter** — the ESP32-C3 drives 3.3V into a strip that
  wants 3.5V for a logic high. Out of spec, but reliable at this length. This
  is the real cause of "my strip flickers randomly" in bigger projects.
- **External 5V supply** — needed past roughly 10 LEDs at full brightness, and
  it must share a ground with the board.
