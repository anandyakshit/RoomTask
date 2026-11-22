# RoomTask: Arduino Motion-Activated Task Reminder

> **Difficulty:** Beginner &nbsp;|&nbsp; **Target Board:** Arduino UNO R4 Minima &nbsp;|&nbsp; **Status:** v0.2 (Physical Core Hardware-Verified &bull; Dual USB & Bluetooth UART Enabled)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Arduino CLI](https://img.shields.io/badge/Arduino%20CLI-renesas__uno-00979C.svg)](https://github.com/arduino/arduino-cli)
[![Target: UNO R4 Minima](https://img.shields.io/badge/Target-UNO%20R4%20Minima-blue.svg)](https://store.arduino.cc/products/uno-r4-minima)
[![Hardware: Bench-Tested](https://img.shields.io/badge/Hardware-Bench--Verified-brightgreen.svg)](docs/DEVLOG.md)

**RoomTask** is a beginner-friendly **Arduino PIR motion sensor LCD reminder project** built around the **Arduino UNO R4 Minima**, a passive infrared (PIR) motion sensor, a 16×2 character LCD with an I²C backpack, and a **Bluetooth module** for wireless smartphone control.

Whenever you enter your room or approach your desk, the PIR sensor detects motion and triggers the display to cycle through your active to-do reminders. Tasks can be added and managed either via **USB Serial** on your computer (115200 baud) or **wirelessly over Bluetooth** from your smartphone (9600 baud).

RoomTask is designed as an accessible, hands-on DIY electronics project demonstrating motion sensing, I²C display management, non-blocking embedded C++ timers, and dual-interface serial communication.

---

## Hardware Requirements & Parts Table

| Component | Specification / Model | Purpose | Arduino UNO R4 Pin |
|---|---|---|---|
| **Microcontroller** | Arduino UNO R4 Minima (Renesas RA4M1 32-bit) | Main controller, timers, and dual serial parser | — |
| **Motion Sensor** | PIR Sensor (HC-SR501 or AM312) | Senses room entry motion events | `D2` (Digital In) |
| **Display** | 16×2 Character LCD with PCF8574 I²C Backpack | Cycles through active reminders | Dedicated `SDA` / `SCL` |
| **Wireless Module** | Bluetooth UART Module (HC-05, HC-06, HM-10, JDY-31) | Wireless task management from smartphone | `Pin 0 (RX1)` & `Pin 1 (TX1)` |
| **Prototyping** | Breadboard & Jumper Wires | Power distribution (5V & GND rails) and signals | — |
| **Power Supply** | 5V USB-C cable & phone charger / power bank | Standalone power for untethered room placement | — |

> [!NOTE]
> Detailed wiring tables, voltage safety considerations, and resistor divider recommendations for Bluetooth are documented in [hardware/README.md](hardware/README.md).

---

## Features

- **Motion-Triggered Reminder Cycles:** Detects room entry using a PIR digital input and automatically cycles through saved reminders.
- **16×2 I²C Display Output:** Clean character rendering with non-blocking timing (10 seconds per task).
- **Dual Communication Interfaces:**
  - **USB Serial (115200 baud):** For fast terminal management when connected to a PC.
  - **Bluetooth UART (9600 baud):** For wireless, phone-based reminder management from your couch or desk.
- **Non-Blocking Motion Cooldown:** 30-second cooldown interval prevents repetitive screen flashing while you remain in the room.
- **Instant Dual Broadcast:** Command confirmations and status messages are automatically mirrored to both USB Serial and Bluetooth.
- **Standalone Untethered Operation:** Power the device with any standard 5V USB phone charger once programmed.

---

## Repository Layout

```text
RoomTask/
├── docs/
│   ├── ARCHITECTURE.md     # System architecture, data flow diagram, and state machine
│   ├── DEVLOG.md           # Engineering log, CLI benchmarks, and hardware verification records
│   └── SERIAL_COMMANDS.md  # USB and Bluetooth command reference and terminal app guides
├── firmware/
│   └── RoomTask/
│       └── RoomTask.ino    # Main Arduino sketch (dual serial + I2C + PIR)
├── hardware/
│   └── README.md           # Complete wiring table, pinouts, and calibration guide
├── .gitignore
├── LICENSE                 # MIT License
└── README.md
```

---

## Quick Start Guide

### 1. Compile & Upload via Arduino CLI

The firmware has been compile-tested and verified with the Renesas UNO R4 platform core.

```bash
# 1. Install board platform
arduino-cli core install arduino:renesas_uno

# 2. Install I2C LCD library
arduino-cli lib install "LiquidCrystal I2C"

# 3. Compile sketch
arduino-cli compile --fqbn arduino:renesas_uno:minima firmware/RoomTask

# 4. Upload to Arduino UNO R4 Minima (replace COMx with your port)
arduino-cli upload -p COMx --fqbn arduino:renesas_uno:minima firmware/RoomTask
```

---

### 2. Adding Reminders Wirelessly via Bluetooth (Smartphone)

1. Connect your Bluetooth module:
   - Module `VCC` &rarr; `5V`
   - Module `GND` &rarr; `GND`
   - Module `TXD` &rarr; Arduino `Pin 0 (RX1)`
   - Module `RXD` &rarr; Arduino `Pin 1 (TX1)`
2. On your smartphone, pair with your Bluetooth module (default pairing PIN is usually `1234` or `0000`).
3. Open a serial terminal app:
   - **Android:** *Serial Bluetooth Terminal* (by Kai Morich)
   - **iOS / Web:** *Web Bluetooth Terminal* or *DSD TECH Tools*
4. Connect to the module at **9600 baud** and send commands!

---

### 3. Adding Reminders via USB Serial (Computer)

Open the Serial Monitor in the Arduino IDE or via Arduino CLI at **115200 baud** with **Newline** line endings:

```bash
arduino-cli monitor -p COMx -c baudrate=115200
```

---

## Command Reference

Commands are identical across both USB Serial and Bluetooth:

| Command | Action | Example Input | Example Output |
|---|---|---|---|
| `ADD <text>` | Add a new reminder to memory (up to 40 chars) | `ADD Water the plants` | `Added: Water the plants` |
| `LIST` | List all currently active reminders | `LIST` | `1. Water the plants` |
| `CLEAR` | Wipe all reminders from memory | `CLEAR` | `All reminders cleared.` |
| `HELP` | Print command list and active baud rates | `HELP` | Prints help text |

*Commands are case-insensitive. See [docs/SERIAL_COMMANDS.md](docs/SERIAL_COMMANDS.md) for full details.*

---

## Troubleshooting & FAQ

### 1. LCD is illuminated but shows no text
- **Adjust Contrast:** Turn the small blue potentiometer on the back of the I²C backpack with a screwdriver until text appears.
- **I²C Address:** The default address in `RoomTask.ino` is `0x27`. If characters do not appear after adjusting contrast, change `LCD_ADDRESS` to `0x3F`.

### 2. Bluetooth module does not receive commands
- **TX/RX Crossover:** Ensure Bluetooth `TXD` connects to Arduino `Pin 0 (RX1)`, and Bluetooth `RXD` connects to Arduino `Pin 1 (TX1)`.
- **Baud Rate Mismatch:** Confirm your terminal app is connected at **9600 baud** (factory default for HC-05/06/HM-10).
- **Line Endings:** Make sure your mobile terminal app is configured to send a newline (`\n` or `CR+LF`) when transmitting.

### 3. PIR sensor triggers continuously
- **Warm-Up Calibration:** Allow **30 to 60 seconds** after power-on for the PIR sensor to stabilize to ambient room temperature.
- **HC-SR501 Pots:** Set the time delay pot fully counter-clockwise (lowest delay) and set the jumper to **L** (single trigger).

---

## Project Roadmap

- [x] **v0.1 — Scaffold & Core Prototype:**
  - PIR motion detection event loop & 16×2 I²C LCD page cycling.
  - USB Serial CLI (`ADD`, `LIST`, `CLEAR`, `HELP`) at 115200 baud.
  - Bench hardware verification on Arduino UNO R4 Minima.
- [x] **v0.2 — Wireless Bluetooth Integration:**
  - Added hardware `Serial1` UART support at 9600 baud for HC-05 / HM-10 modules.
  - Dual-stream concurrent command parsing and unified message broadcasting.
  - Untethered standalone operation via USB wall charger.
- [ ] **v0.3 — Persistent Storage:**
  - Store tasks in non-volatile storage (EEPROM / on-board Data Flash) so reminders persist across power resets.
- [ ] **v0.4 — Real-Time Clock (RTC) Integration:**
  - Leverage the UNO R4's built-in RTC for scheduled task alerts and timestamped reminders.

---

## Current Status & Verification Notice

> [!TIP]
> **Core Hardware Verified:** The core PIR motion sensing, 16×2 I²C LCD driver, and Serial command engine have been physically bench-tested and verified functional on an Arduino UNO R4 Minima. The v0.2 dual Bluetooth/Serial firmware has been compile-tested and verified with `arduino-cli`. See [docs/DEVLOG.md](docs/DEVLOG.md) for detailed test records.

---

## Contributing

Contributions, bug reports, and hardware test reports are welcome! Open an issue or submit a pull request on GitHub.

---

## License

This project is licensed under the [MIT License](LICENSE).
