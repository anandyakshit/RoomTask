# RoomTask: Arduino Motion-Activated Task Reminder

> **Difficulty:** Beginner &nbsp;|&nbsp; **Target Board:** Arduino UNO R4 Minima &nbsp;|&nbsp; **Status:** v0.1 Scaffold (Compile-Tested via Arduino CLI &bull; Physical Hardware Not Yet Verified)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Arduino CLI](https://img.shields.io/badge/Arduino%20CLI-renesas__uno-00979C.svg)](https://github.com/arduino/arduino-cli)
[![Target: UNO R4 Minima](https://img.shields.io/badge/Target-UNO%20R4%20Minima-blue.svg)](https://store.arduino.cc/products/uno-r4-minima)

**RoomTask** is a beginner-friendly **Arduino PIR motion sensor LCD reminder project** built around the **Arduino UNO R4 Minima**, a passive infrared (PIR) motion sensor, and a standard 16×2 character LCD with an I²C backpack. 

Whenever you enter your room or approach your desk, the PIR sensor detects motion and triggers the display to cycle through your active to-do list. Tasks are entered and managed directly through the Arduino USB Serial connection at **115200 baud** and are held in memory for the active session.

RoomTask is designed as an accessible, hands-on DIY electronics project demonstrating basic sensor inputs, I²C display output, serial command line interfaces (CLI), and non-blocking timing in embedded C++.

---

## Hardware Requirements & Parts Table

| Component | Specification / Model | Purpose | Provisional Pin |
|---|---|---|---|
| **Microcontroller** | Arduino UNO R4 Minima (Renesas RA4M1) | Main controller & serial interface | — |
| **Motion Sensor** | PIR Sensor (HC-SR501, AM312, or similar) | Detects human motion upon room entry | `D2` (Digital In) |
| **Display** | 16×2 Character LCD with I²C Backpack (PCF8574) | Shows status and cycles task reminders | Dedicated `SDA` / `SCL` |
| **Interconnects** | Jumper Wires (Female-to-Male / Male-to-Male) | Circuit wiring | — |
| **Power & Data** | USB-C to USB-A Cable | USB serial command interface and power | — |

> [!NOTE]
> Review [hardware/README.md](hardware/README.md) for provisional pin assignments, operating voltages, and I²C wiring details before powering your board.

---

## Features & v0.1 Scope

- **Motion-Triggered Cycles:** Senses room motion using a PIR digital input and begins an automated reminder display sequence.
- **16×2 I²C Display Output:** Cycles through each reminder with configurable display duration (default: 10 seconds per task).
- **Cooldown Interval:** Enforces a non-blocking cooldown period (default: 30 seconds) between motion triggers to avoid repeated firing while you remain in the room.
- **USB Serial CLI:** Fast task management over USB serial at 115200 baud using straightforward text commands (`ADD`, `LIST`, `CLEAR`, `HELP`).
- **Zero Solder Breadboard Setup:** Uses standard modular components suitable for beginners.
- **Lightweight Memory Footprint:** Reminders are stored in volatile RAM (up to 8 tasks, 40 characters each) during the active power session.
- *Deliberate v0.1 exclusions:* No Bluetooth, Wi-Fi, Real-Time Clock (RTC), or persistent EEPROM storage yet (see [Roadmap](#project-roadmap)).

---

## Repository Layout

```text
RoomTask/
├── docs/
│   ├── ARCHITECTURE.md     # System architecture, data flow, and timing
│   ├── DEVLOG.md           # Development history, CLI test results, and log template
│   └── SERIAL_COMMANDS.md  # Serial Monitor command reference and session examples
├── firmware/
│   └── RoomTask/
│       └── RoomTask.ino    # Main Arduino sketch
├── hardware/
│   └── README.md           # Wiring table, module notes, and safety checks
├── .gitignore
├── LICENSE                 # MIT License
└── README.md
```

---

## Quick Start Guide

You can build and upload RoomTask using either the **Arduino IDE** graphical interface or the command-line **Arduino CLI**.

### Method A: Arduino IDE (GUI)

1. **Install Board Core:**
   - Open Arduino IDE (v2.x recommended).
   - Go to **Tools > Board > Boards Manager...**
   - Search for **Arduino UNO R4 Boards** and click **Install** (`arduino:renesas_uno`).
2. **Install Required Library:**
   - Go to **Sketch > Include Library > Manage Libraries...**
   - Search for **LiquidCrystal I2C** (by Frank de Brabander / Marco Schwartz) and click **Install**.
3. **Open Sketch:**
   - Open `firmware/RoomTask/RoomTask.ino`.
4. **Select Board & Port:**
   - Select **Arduino UNO R4 Minima** from the board drop-down.
   - Choose the serial communication port assigned to your board.
5. **Upload Sketch:**
   - Click the **Upload** button (`Ctrl+U` or `Cmd+U`).
6. **Open Serial Monitor:**
   - Open **Tools > Serial Monitor** (`Ctrl+Shift+M` or `Cmd+Shift+M`).
   - Set the baud rate to **115200 baud**.
   - Set the line ending dropdown to **Newline** (or *Both NL & CR*).
   - Enter `HELP` to confirm communication.

---

### Method B: Arduino CLI (Command Line)

The project has been compile-tested using the official Arduino CLI.

1. **Install the UNO R4 Core:**
   ```bash
   arduino-cli core install arduino:renesas_uno
   ```

2. **Install the I²C LCD Library:**
   ```bash
   arduino-cli lib install "LiquidCrystal I2C"
   ```

3. **Compile the Sketch:**
   ```bash
   arduino-cli compile --fqbn arduino:renesas_uno:minima firmware/RoomTask
   ```
   *Expected output:* Compiled successfully (~49 KB program storage space, ~5.8 KB dynamic memory).

4. **Upload to the Board:**
   Find your port with `arduino-cli board list`, then upload:
   ```bash
   arduino-cli upload -p <PORT> --fqbn arduino:renesas_uno:minima firmware/RoomTask
   ```
   *(Replace `<PORT>` with your actual port, e.g., `COM3` on Windows or `/dev/ttyACM0` on Linux/macOS).*

5. **Open the Serial Monitor:**
   ```bash
   arduino-cli monitor -p <PORT> -c baudrate=115200
   ```

---

## Serial Command Reference

Connect your board to your computer via USB, open your serial terminal at **115200 baud** with **Newline** line endings, and send commands:

| Command | Description | Example Input | Example Output |
|---|---|---|---|
| `ADD <text>` | Add a new reminder to RAM (up to 40 chars) | `ADD Buy milk` | `Added: Buy milk` |
| `LIST` | List all currently active reminders | `LIST` | `1. Buy milk` |
| `CLEAR` | Wipe all reminders from RAM | `CLEAR` | `All reminders cleared.` |
| `HELP` | Display list of supported commands | `HELP` | Prints help text |

*Commands are case-insensitive. See [docs/SERIAL_COMMANDS.md](docs/SERIAL_COMMANDS.md) for full details.*

---

## Troubleshooting & FAQ

### 1. The LCD screen is backlit, but shows no text or only white blocks
- **Adjust Contrast:** Locate the small blue potentiometer on the back of the I²C backpack and slowly turn it with a small screwdriver until character outlines appear.
- **Check I²C Address:** The default address in `RoomTask.ino` is set to `0x27`. Some backpacks use `0x3F` or another address. If text does not appear, run an I²C scanner sketch to find your backpack's address and change `LCD_ADDRESS` in the sketch.
- **Check SDA/SCL Pins:** Ensure SDA is connected to the UNO R4 Minima's dedicated `SDA` pin and SCL is connected to `SCL`.

### 2. PIR sensor triggers immediately or stays HIGH continuously
- **Warm-Up Delay:** PIR sensors (like HC-SR501) require **30 to 60 seconds** after power-up to calibrate to ambient infrared levels. During this warm-up period, false triggers are expected.
- **Sensitivity & Time Pots:** On HC-SR501 sensors, turn the sensitivity potentiometer counter-clockwise to reduce false triggers, and turn the time delay potentiometer fully counter-clockwise (lowest delay).
- **Trigger Jumper:** Set the sensor jumper to **L** (single trigger mode) rather than **H** (repeatable trigger mode) if you encounter looping triggers.

### 3. Serial Monitor commands receive no response
- **Check Baud Rate:** Ensure the serial monitor is strictly set to **115200 baud**.
- **Line Endings:** Make sure the terminal sends a newline (`\n` or `CR+LF`). The parser waits for a newline character before executing commands.
- **USB Cable:** Confirm you are using a full data USB cable and not a charge-only cable.

### 4. Compiler warning about `LiquidCrystal_I2C` architecture
- During compilation, Arduino CLI may output:
  `WARNING: library LiquidCrystal_I2C claims to run on all architecture(s) and may be incompatible with your current board...`
- This is a benign header warning in the library's metadata properties. The library compiles and links cleanly with the Renesas UNO R4 architecture.

### 5. Why did my reminders disappear?
- In **v0.1**, reminders are held purely in volatile microcontroller RAM for rapid prototyping. They are reset whenever power is disconnected or the reset button is pressed. Persistent storage is scheduled for **v0.2**.

---

## Project Roadmap

- [x] **v0.1 — Scaffold & Core Prototype:**
  - PIR motion detection event loop
  - 16×2 I²C LCD page cycling with non-blocking timing
  - USB Serial command line parser (`ADD`, `LIST`, `CLEAR`, `HELP`)
  - RAM-only storage (up to 8 tasks)
  - Verified Arduino CLI compile flow for Arduino UNO R4 Minima
- [ ] **v0.2 — Persistent Storage:**
  - Save reminders to non-volatile storage (EEPROM / on-board Data Flash) so tasks survive power cycles and board reboots.
- [ ] **v0.3 — Real-Time Clock (RTC) & Scheduled Tasks:**
  - Leverage the UNO R4 Minima's on-chip RTC to add time/date stamping and scheduled reminder alerts.
- [ ] **v0.4 — Wireless Connectivity (Bluetooth LE / Web Bluetooth):**
  - Add BLE or wireless companion interface to manage task reminders wirelessly without USB tethering.

---

## Current Status & Verification Notice

> [!IMPORTANT]
> **RoomTask is currently at v0.1.** The sketch has been **compile-tested and verified with Arduino CLI** against the `arduino:renesas_uno` core platform (version 1.6.0). However, **it is not yet hardware-verified on a live physical test bench**. Users should confirm module voltages, pin wiring, and I²C addresses before deployment. Feedback and hardware test reports are welcome via GitHub Issues.

See [docs/DEVLOG.md](docs/DEVLOG.md) for full compilation metrics, test commands, and ongoing technical development logs.

---

## Contributing

Contributions, bug reports, and hardware test reports are warmly welcome! Whether you are testing with a different I²C display, adjusting PIR sensitivity, or porting to the UNO R4 WiFi, feel free to open an issue or submit a pull request.

---

## License

This project is licensed under the [MIT License](LICENSE).
