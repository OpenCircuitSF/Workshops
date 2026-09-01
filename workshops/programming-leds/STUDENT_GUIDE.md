# Programming LEDs — Student Guide

Welcome. In the next hour you will flash a program onto a small computer, light
up an LED strip, and then change what it does and flash it again.

**No experience needed.** If you have never opened a terminal, written code, or
touched a circuit board, you are exactly who this workshop is for.

---

## What to bring

Two things:

- **A laptop** — macOS, Windows, or Linux. You need permission to install
  software on it, so a locked-down work laptop may fight you.
- **A USB-C cable that carries data** — the one that came with your phone or
  laptop is usually fine. See the warning below.

> ### About that cable
>
> Many USB-C cables are **charge-only**: they have power wires but no data
> wires, and they look identical to a normal cable. Plugged into the board, a
> charge-only cable gives you a lit power LED and nothing else — your laptop
> never sees the board at all.
>
> This is the single most common reason someone can't get started. If your
> cable has ever successfully moved a file, transferred photos, or connected a
> phone to a computer, it's a data cable. If you only ever plug it into a wall
> charger, bring a second one. We'll have spares, but there won't be many.

**Everything else is provided.** You don't need to buy or prepare any hardware.

---

## What we provide

![The kit: an ESP32-C3 on a breadboard, wired to a WS2812B LED strip mounted on a wood board](assets/kit-wired.jpg)

Your kit arrives already wired, exactly as pictured:

| Part | What it is |
|---|---|
| **ESP32-C3 board** | The computer. A full CPU with WiFi and Bluetooth, about the size of a stick of gum. Already seated in the breadboard. |
| **Breadboard** | A solderless board for making connections. It just holds things and joins them together — no soldering today. |
| **LED strip on a wood backing** | A row of WS2812B LEDs. Each one is its own tiny controller chip with red, green, and blue LEDs inside. |
| **Three jumper wires** | Already connecting the board to the strip: power, ground, and data. |

The strip's three wires end in a header that plugs straight into the
breadboard, so the wiring is done for you. You will not have to strip a wire or
touch a soldering iron.

### The three wires

Everything the strip needs travels on exactly three wires:

| Wire | Strip label | Goes to | Carries |
|---|---|---|---|
| Power | `+5V` | The board's 5V pin | Electricity to light the LEDs |
| Ground | `GND` | Any GND pin on the board | The return path — the other half of every circuit |
| Data | `Din` | **GPIO 4** on the board | The colors, sent one bit at a time |

**Trust the labels, not the colors.** The tiny text is printed right on the
strip next to the pads — `+5V`, `Din`, `GND`. Wire colors vary between kits and
are not a reliable guide.

`Din` means **data in**. The strip is a one-way chain: each LED keeps the first
color it receives and passes the rest along to the next one. That's why the
strip has a specific input end, and why your kit comes pre-wired to it — feed
the wrong end and *nothing lights up, with no error message at all*.

---

## Before you arrive

**Please do this at home.** It downloads several hundred megabytes and takes
15–20 minutes on a good connection. Doing it live in the room will eat a third
of the workshop, and venue WiFi with several laptops all downloading at once
might be slow.

1. **Install [Visual Studio Code](https://code.visualstudio.com/)** — the free
   code editor from Microsoft.
2. **Install the PlatformIO extension.** Open VS Code, click the Extensions
   icon in the left sidebar (four squares), search for **PlatformIO IDE**, and
   click Install.
3. **Let PlatformIO finish setting itself up.** After installing, leave VS Code
   open. PlatformIO downloads its own Python environment on first run — this
   takes several minutes and needs internet. Wait for it to say it's done and
   restart VS Code when it asks.
4. **Get the project code** from
   **[github.com/OpenCircuitSF/Workshops](https://github.com/OpenCircuitSF/Workshops)**.

   Either way works:

   - **Clone it** (keeps it easy to pull updates later):

     ```bash
     git clone https://github.com/OpenCircuitSF/Workshops.git
     ```

   - **Or download a ZIP** — click the green **Code** button on that page,
     choose **Download ZIP**, and unzip it somewhere you can find again.
5. **Open the firmware folder and build it once.** In VS Code choose
   **File → Open Folder** and select `workshops/programming-leds/firmware`.

   > Open the `firmware` folder itself, **not** the top of the repository.
   > PlatformIO looks for a `platformio.ini` file in the folder you open, and
   > that file lives in `firmware/`.

   Then click the **✓ checkmark** in the blue bar at the bottom of the window
   to Build. The first build downloads the compiler for the ESP32 chip and the
   FastLED library — this is the big download. When it finishes you should see
   a green **SUCCESS**.

If any of this goes sideways, don't stress about it — come anyway, arrive a few
minutes early, and we'll sort it out or pair you with someone.

---

## What happens in the workshop

Roughly:

1. **We look at the hardware** and talk about what a microcontroller actually
   is, why the strip wants 5V, and how much current LEDs draw.
2. **We talk about serial** — how ten LEDs share one data wire, and why the
   strip only works in one direction.
3. **You plug in and upload.** Lights on.
4. **You open the serial monitor** and watch the board talk back to your
   laptop.
5. **You change the code** and upload again. Three rounds of it.
6. **We explain how it works** — how a number derived from the clock becomes a
   color, and why there is no `delay()` anywhere in the program.

You will have working lights well before the halfway mark. Everything
conceptual comes after that.

---

## The three buttons you need

They're all in the blue status bar along the bottom of the VS Code window:

| Icon | What it does | Shortcut |
|---|---|---|
| **✓** | **Build** — compile the code, don't send it anywhere | `Cmd/Ctrl + Alt + B` |
| **→** | **Upload** — compile *and* flash it onto the board | `Cmd/Ctrl + Alt + U` |
| **🔌** | **Serial Monitor** — watch messages from the board | `Cmd/Ctrl + Alt + S` |

> **Close the serial monitor before you upload.** The monitor holds the USB port
> open, and the upload will fail with `Could not exclusively lock port`. This
> will happen to you at least once. Now you know what it means.

---

## What you'll be changing

Everything you edit lives at the top of `firmware/src/main.cpp`, in a block of
named constants. You don't need to understand the rest of the file to change
what the strip does.

| Constant | Default | What it controls |
|---|---|---|
| `NUM_LEDS` | 11 | How many LEDs are on your strip |
| `DATA_PIN` | 4 | Which pin drives the strip |
| `BRIGHTNESS` | 128 | Overall brightness, 0–255 |
| `STEP_MS` | 2000 | Milliseconds to blend from one color to the next |
| `LED_SPREAD` | 26 | How far apart neighboring LEDs sit in the color blend |
| `FRAME_MS` | 16 | How often the strip is redrawn (~60 times a second) |

**Your first job is counting.** Count the LEDs on your strip and make
`NUM_LEDS` match. If the number is too small, the last LEDs stay dark. If it's
too big, the extra colors fall off the end of the chain and vanish. Nothing
warns you either way — this is a great illustration of how hardware fails
*quietly*.

Then we'll try things like setting `LED_SPREAD` to `0` (the whole strip becomes
one solid color) and dropping `STEP_MS` to `200` (everything races).

---

## If something isn't working

Work down this list — the top entry is the answer most of the time.

| What you see | Probably | Try |
|---|---|---|
| No port to upload to | Charge-only USB cable | Swap the cable. Really. |
| `Could not exclusively lock port` | Serial monitor is open | Close it, upload again |
| Upload works, no lights | Wrong pin, or a loose jumper | Check `DATA_PIN` is `4`; reseat all three wires |
| Some LEDs light, the rest don't | `NUM_LEDS` is too low | Count them again |
| Colors are wrong (red shows green) | Color order | Ask — it's a one-word change |
| Board restarts on bright colors | Drawing too much power | Lower `BRIGHTNESS` |
| Serial monitor shows garbage like `­¾ýÿ¿þ` | Speed mismatch | The wiring is fine — ask, it's a settings fix |
| Build fails after edits that "should" work | Stale build cache | Delete `firmware/.pio/` and build again |

Nothing here can hurt you and very little can hurt the hardware. The board runs
on 5V from a USB port — you can touch any of it. **Just unplug the USB before
you move wires around,** and don't let the bare 5V and GND wires touch each
other.

Ask early. A stuck laptop is not a personal failing; it's the most normal thing
in this room.

---

## After the workshop

Take the kit home — it's yours. Some directions to go:

- Add a button that changes the palette.
- Use a longer strip with its own 5V power supply.
- Explore FastLED's built-in effects: `fill_rainbow()`, `beatsin8()`,
  `noise16()`.
- Turn the ESP32's WiFi on and control the strip from your phone, or from Home
  Assistant.
- Install [WLED](https://kno.wled.ge/) — the same hardware, running a finished
  product with a web interface.

The [project README](README.md) has more on how the code works and what else
you can change.

---

**Open Circuit SF** · [opencircuitsf.com](https://www.opencircuitsf.com) ·
[Discord](https://discord.gg/Fq9ug6QXV3)
