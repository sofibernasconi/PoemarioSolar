# Poemario Solar · Solar Poetry Device

**Sofía Bernasconi, 2026**
Interactive reading device · e-ink display · solar-charged battery · Arduino

**A low-power interactive reading interface powered by a portable energy source.**

---

## About the work

*Poemario Solar* is an interactive electronic device for reading a small collection of poems through a minimal physical interface. It combines a microcontroller, an e-ink display and mechanical controls, and runs on a battery charged by a small solar panel.

Each poem can be read in Spanish or in English. The reader turns a knob to move through the pages of a poem, presses it to switch language, and presses a button to move to the next poem.

The system is designed for low energy consumption and autonomous operation. The e-ink display keeps its image without using power, so energy is spent only when the page changes.

Developed for the course *Energy* (Prof. Jeff Feddersen) at ITP, Tisch School of the Arts, New York University.

More about my work: [sofiabernasconi.com](https://sofiabernasconi.com)

---

## The poems

1. **Alejandra Pizarnik**, *La última inocencia* / *The Last Innocence*
2. **Nicanor Parra**, *El hombre imaginario* / *The Imaginary Man*
3. **Sofía Bernasconi**, *El cuadrúpedo invisible* / *The Quadruped*

The poems by Alejandra Pizarnik and Nicanor Parra are protected by copyright, so in this repository their text is replaced by a placeholder (`poner aqui poema 1`, `poner aqui poema 2`). The code keeps their titles and authors, and the full structure to load them on the device. *El cuadrúpedo invisible* is included complete.

---

## How to use it

| Control | Action |
|---|---|
| **Rotary encoder**, turn | Next or previous page of the poem |
| **Rotary encoder**, press | Switch language (Spanish / English) |
| **Blue button** | Next poem |
| **Red switch** | Power on / off |

Each poem opens with a title screen (white text on black) showing the title, the author and the poem number. The following screens show the poem, black text on white. Navigation loops: after the last page it returns to the title.

---

## How it works

```
small solar panel ──► Voltaic V50 battery ──► red switch ──► Arduino Nano 33 IoT ──► e-ink display
   (charges)              (stores, 5V USB)      (on/off)         (3.3V logic)          (3.3V, SPI)
```

The solar panel does not power the device directly. It charges the battery, and the battery provides a stable, regulated 5V supply. The panel acts as a variable energy input.

---

## Repository structure

```
arduino/
  poemario_solar/poemario_solar.ino   Device code
README.md
LICENSE
```

---

## Hardware

| Part | Notes |
|---|---|
| Microcontroller | Arduino Nano 33 IoT (3.3V logic) |
| Display | Waveshare 2.13" e-ink, black and white (`GxEPD2_213_B74` driver), SPI |
| Rotary encoder | With push switch |
| Blue push button | Changes poem |
| Red switch | Power on/off, inline on the 5V line |
| Battery | Voltaic V50, 5V USB output |
| Solar panel | Small panel, connected to the battery's micro-USB input |
| Enclosure | Laser-cut acrylic with finger joints, glued with Weld-On acrylic cement |

### Solar panel

| | |
|---|---|
| Nominal output | ~5V (USB regulated) |
| Open-circuit voltage | ~6–7V |
| Output current | ~200–500 mA, depending on light |
| Power | ~1–2.5 W |

### Power distribution

```
Battery (5V USB)
   ↓
Cut USB cable (5V + GND)
   ↓
Red switch (inline, on the 5V line)
   ↓
Arduino power input
```

- **5V (red wire)** passes through the switch.
- **GND (black wire)** connects directly.
- The USB data lines are not used.

### Wiring

| Arduino pin | Connects to |
|---|---|
| D10 | E-ink CS |
| D9 | E-ink DC |
| D8 | E-ink RST |
| D7 | E-ink BUSY |
| D11 (MOSI), D13 (SCK) | E-ink DIN, CLK (hardware SPI) |
| 3.3V, GND | E-ink VCC, GND |
| D2 | Encoder CLK |
| D3 | Encoder DT |
| D4 | Encoder SW (push) |
| D5 | Blue button (other leg to GND) |

The encoder and the button use the Arduino's internal pull-up resistors, so no external resistors are needed.

---

## Running the code

1. Install these libraries from the Arduino Library Manager:
   - **GxEPD2** (e-ink display)
   - **Adafruit GFX Library** (fonts and drawing)
2. Open `arduino/poemario_solar/poemario_solar.ino` in the Arduino IDE.
3. Select **Arduino Nano 33 IoT** and your port, and upload.

### Adding or changing poems

Each poem is a list of screens, and each screen is a list of lines:

```cpp
const char* mypoem_1_es[] = {
  "first line",
  "second line"
};

const Screen screens_mypoem_es[] = {
  { nullptr, 0 },        // title screen (always first)
  { mypoem_1_es, 2 }     // { lines, number of lines }
};
```

Then add the poem to the `poems[]` list with its author, title (two lines per language), the Spanish and English screens, and `totalScreens` (number of text screens + 1 for the title).

Notes:
- The display fonts do not include accents or the letter ñ, so the text is written without them.
- Each screen fits about 7 lines of up to ~22 characters.

---

## Process

- **E-ink first.** I started by learning how to drive the e-ink display, first with an Arduino Nano ESP32, and then added interaction with a push button and the rotary encoder.
- **Batteries.** I first tried reusing old vape batteries with a TP4056 charger, a voltage regulator and an MT3608 boost converter. When I connected two cells in series and added the red switch, the TP4056 and the Arduino burned. The TP4056 is designed for a single lithium cell, which was probably the cause; it is also possible that the 5V pin and the micro-USB of the Arduino were connected at the same time. After that I used the Voltaic battery directly.
- **Enclosure.** I tested laser-cut acrylic to find the exact hole sizes for the encoder, button, switch and display. The first box design in Illustrator had finger joints too large for the acrylic thickness; I adjusted them, cut the box again and glued it with Weld-On.
- **Assembly.** I soldered the board and components inside the enclosure and fixed everything in place.

---

## Next steps

- Use the three holes above the switch to show the battery charge level with LEDs.
- Add an LED to show when the device is on.

---

## License

The code in this repository is released under the [MIT License](LICENSE).

*El cuadrúpedo invisible* and all images and documentation of *Poemario Solar* are © Sofía Bernasconi, all rights reserved. The poems by Alejandra Pizarnik and Nicanor Parra belong to their respective rights holders and are not included.
