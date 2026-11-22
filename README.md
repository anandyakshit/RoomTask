# RoomTask: Smart Motion-Activated Arduino Task Reminder

> **Difficulty:** Beginner &bull; **Time:** ~45 Minutes &bull; **Target Board:** Arduino UNO R4 Minima &bull; **Status:** v0.3 (Fully Hardware Bench-Verified &bull; Dual USB & Bluetooth &bull; Web App & Calendar Sync)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Arduino CLI](https://img.shields.io/badge/Arduino%20CLI-renesas__uno-00979C.svg)](https://github.com/arduino/arduino-cli)
[![Target: UNO R4 Minima](https://img.shields.io/badge/Target-UNO%20R4%20Minima-blue.svg)](https://store.arduino.cc/products/uno-r4-minima)
[![Hardware: Bench-Verified](https://img.shields.io/badge/Hardware-Bench--Verified-brightgreen.svg)](docs/DEVLOG.md)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

**RoomTask** is a beginner-friendly **Arduino PIR motion sensor LCD reminder project** designed for the **Arduino UNO R4 Minima**. It detects whenever you enter your room or approach your desk and automatically triggers a 16×2 character LCD to cycle through your daily to-do tasks.

Reminders can be managed wirelessly from your **smartphone over Bluetooth**, via **USB Serial** on your computer, or directly through the included **Web Dashboard with Google/Outlook Calendar sync**.

```text
    +-------------------------------------------------------------------------+
    |                                ROOMTASK                                 |
    |                                                                         |
    |  [ Walk into Room ] --> (PIR Motion Sensor)                             |
    |                                |                                        |
    |                                v                                        |
    |                     (Arduino UNO R4 Minima)                             |
    |                        |                 |                              |
    |                        v                 v                              |
    |                [ 16x2 I2C LCD ]   [ Tri-Way Ingestion ]                 |
    |                "Reminder 1/3"      - Web Dashboard (.ICS Calendar Sync) |
    |                "Water plants"      - Bluetooth (Smartphone App)         |
    |                                    - USB CDC (Computer Terminal)        |
    +-------------------------------------------------------------------------+
```

---

## Why RoomTask Is Ranked Among the Best Beginner Arduino Projects

Most beginner Arduino tutorials stop at blinking an onboard LED or printing numbers to a serial monitor. **RoomTask** provides a practical, real-world desktop device while teaching foundational embedded systems engineering principles:

1. **Zero Soldering Required:** Built entirely with modular components on a standard breadboard using jumper wires.
2. **Real-World Utility:** Solves a real problem (forgetting desk chores and daily reminders) upon entering your room.
3. **Non-Blocking Architecture:** Teaches beginners how to write responsive embedded C++ code using `millis()` state checks instead of CPU-blocking `delay()` calls.
4. **I²C Bus Mastery:** Demonstrates how 2-wire serial communication (`SDA` / `SCL`) drives a 16-pin character LCD with only 4 wires.
5. **Dual-Interface Serial Communication:** Concurrent command processing across USB CDC serial and hardware UART (`Serial1`) Bluetooth.
6. **Smart Calendar Integration:** Import events directly from Google Calendar, Apple Calendar, or Outlook via `.ics` file parsing into your microcontroller.
7. **PIR Stabilization State Machine:** Prevents startup false triggers by running a live 60-second initialization countdown while remaining fully responsive to serial commands.

---

## Hardware Requirements & Parts Table

| Component | Recommended Model | Function | Pin on UNO R4 Minima |
|---|---|---|---|
| **Microcontroller** | Arduino UNO R4 Minima (Renesas RA4M1 32-bit ARM) | Core controller, timers, and dual serial parser | — |
| **Motion Sensor** | PIR Sensor (HC-SR501, AM312) | Detects human motion upon room entry | `Pin D2` (Digital In) |
| **Display** | 16×2 Character LCD with PCF8574 I²C Backpack | Cycles through saved reminder tasks | Dedicated `SDA` & `SCL` |
| **Wireless Module** | Bluetooth UART Module (HC-05, HC-06, HM-10, JDY-31) | Wireless smartphone task management | `Pin 0 (RX1)` & `Pin 1 (TX1)` |
| **Connection State** | Bluetooth Module `STATE` Pin | Real-time paired/disconnected detection | `Pin D4` *(or Analog A0)* |
| **Audio Chime (Optional)**| 5V Passive Piezo Buzzer | Subtle audio chime upon room entry | `Pin D3` (Digital Out) |
| **Prototyping** | Half-size Breadboard & Male-to-Female Jumper Wires | Power distribution (5V & GND) & wiring | — |
| **Power Supply** | 5V USB-C cable & phone charger or power bank | Untethered standalone room operation | USB-C Port |

---

## Complete Wiring Guide

Connect the Arduino's **5V** and **GND** pins to the breadboard power rails, then wire each module:

```text
               +---------------------------------------------------+
               |              Arduino UNO R4 Minima                |
               |                                                   |
               | [AREF] [SDA] [SCL] [D13] ... [D4] [D2] [D1] [D0]  |
               +---|------|-----|--------------|----|---|---|---+--+
                   |      |     |              |    |   |   |
                   |      |     |              |    |   |   +--[Pin 0/RX1] <--- TXD (Bluetooth)
                   |      |     |              |    |   +------[Pin 1/TX1] ---> RXD (Bluetooth)*
                   |      |     |              |    +----------[Pin D2]    <--- OUT (PIR Sensor)
                   |      |     |              +---------------[Pin D4/A0] <--- STATE (Bluetooth)
                   |      |     |
               +---|------|-----+  (Power Pins)
               |  5V     GND
               +---|-------|----+
                   |       |
                   v       v
         ===========================  (Breadboard Rails)
            (+) 5V Rail     (-) GND Rail
         ===========================
            |        |          |        |          |        |
         +--+--+  +--+--+    +--+--+  +--+--+    +--+--+  +--+--+
         | VCC |  | GND |    | VCC |  | GND |    | VCC |  | GND |
         +-----+  +-----+    +-----+  +-----+    +-----+  +-----+
          16x2 I2C LCD         PIR Sensor         Bluetooth Module
           [SDA]   [SCL]         [OUT]             [TX] [RX] [STATE]
             |       |             |                 |    |     |
             +-------+-------------+-----------------+----+-----+ (To Arduino)
```

> [!TIP]
> * **Bluetooth Logic Note:** The UNO R4 operates at 5V logic. For long-term reliability with 3.3V Bluetooth RX pins, use a simple 1kΩ / 2kΩ resistor divider between Arduino Pin 1 (TX1) and Bluetooth RXD. Bluetooth TXD connects directly to Pin 0 (RX1).
> * For full step-by-step assembly diagrams and voltage safety notes, see [hardware/README.md](hardware/README.md).

---

## Startup Sequence & Warm-Up Timer

When powered on, RoomTask displays a live 60-second initialization countdown:

```text
+----------------+
|RoomTask init   |
|60s remain      |
+----------------+
```

* **Why?** PIR motion sensors require a 30–60 second thermal stabilization period upon boot. Ignoring motion triggers during this interval eliminates startup false alarms.
* **Fully Non-Blocking:** While the countdown is running on the screen, the Arduino actively listens for `ADD` commands over Bluetooth and USB.
* **Smart Completion:** When the timer hits `0s`, if reminders were added during the warm-up, it immediately launches into the reminder display cycle! If no reminders were added, it settles cleanly onto:
  ```text
  +----------------+
  |RoomTask        |
  |Connected       | (or Disconnected)
  +----------------+
  ```

---

## Quick Start Guide

### Option A: Arduino CLI (Terminal)

```bash
# 1. Install Renesas UNO R4 platform core
arduino-cli core install arduino:renesas_uno

# 2. Install I2C LCD driver library
arduino-cli lib install "LiquidCrystal I2C"

# 3. Compile sketch
arduino-cli compile --fqbn arduino:renesas_uno:minima firmware/RoomTask

# 4. Upload to board (replace COMx with your port)
arduino-cli upload -p COMx --fqbn arduino:renesas_uno:minima firmware/RoomTask
```

### Option B: Arduino IDE (GUI)

1. Open **Tools > Board > Boards Manager**, search for `Arduino UNO R4 Boards`, and click **Install**.
2. Open **Sketch > Include Library > Manage Libraries**, search for `LiquidCrystal I2C` (by Frank de Brabander / Marco Schwartz), and install it.
3. Open `firmware/RoomTask/RoomTask.ino`.
4. Select **Arduino UNO R4 Minima** and your COM port.
5. Click **Upload** (`Ctrl+U` / `Cmd+U`).

---

## How to Add Reminders

### Method 1: Web Companion Dashboard & Calendar Sync (Browser)
Open [`web/index.html`](web/index.html) in Google Chrome or Edge:
* **Web Bluetooth:** 1-click connect to BLE modules (HM-10, CC2541) on Chrome (Android/Desktop) or Bluefy (iOS).
* **Web Serial:** Connect directly over USB cable or paired Bluetooth serial ports (HC-05).
* **Calendar Sync:** Upload your `.ics` export file from Google Calendar, Apple Calendar, or Outlook to automatically push today's schedule into RoomTask!

### Method 2: Wirelessly from your Smartphone (Bluetooth)
1. Pair your phone with your Bluetooth module (default PIN is usually `1234` or `0000`).
2. Open any free serial terminal app (*Serial Bluetooth Terminal* on Android, *Web Bluetooth Terminal* on iOS).
3. Connect at **9600 baud** and send commands (`ADD <text>`, `LIST`, `CLEAR`).

### Method 3: Tethered from your Computer (USB Serial)
Open the Serial Monitor in the Arduino IDE or via Arduino CLI at **115200 baud** with **Newline** line endings:
```bash
arduino-cli monitor -p COMx -c baudrate=115200
```

---

## Command Reference

| Command | Action | Example Input | Example Output |
|---|---|---|---|
| `ADD <text>` | Add a reminder to memory (up to 40 characters) | `ADD Buy milk` | `Added: Buy milk` |
| `LIST` | List all saved reminders in sequence | `LIST` | `1. Buy milk` |
| `CLEAR` | Wipe all reminders from active memory | `CLEAR` | `All reminders cleared.` |
| `HELP` | Display list of supported commands | `HELP` | Prints command list |

*Commands are case-insensitive and can be sent via Bluetooth or USB interchangeably.*

---

## Frequently Asked Questions (FAQ)

### Can I build this on an Arduino UNO R3 or Nano?
Yes. The sketch code is portable across the Arduino ecosystem. On an UNO R3 or Nano, `Serial` is shared between USB and Pins 0/1. On the UNO R4 Minima, `Serial` (USB) and `Serial1` (Pins 0/1) are independent hardware channels, which is why the UNO R4 offers true concurrent dual-stream operation.

### Why does the screen say "RoomTask Disconnected"?
The screen displays `Connected` whenever your phone is actively paired or transmitting, and `Disconnected` when unpaired. Connecting the module's `STATE` pin to **Pin D4** (or Analog **A0**) provides hardware-level instant detection.

### How do I power RoomTask permanently in my room?
Once programmed, RoomTask requires no computer connection. Plug the Arduino into any 5V USB-A or USB-C wall adapter, phone charger, or power bank.

---

## Project Roadmap

- [x] **v0.1 — Core Motion & LCD Prototype:** PIR sensor loop, 16×2 LCD non-blocking state machine, USB Serial CLI.
- [x] **v0.2 — Wireless Bluetooth UART & Dual Serial:** Concurrently listening on hardware `Serial1` (9600 baud) and USB (115200 baud). Bench-verified on real hardware.
- [x] **v0.3 — Web Dashboard, Calendar Sync & 60s Warm-Up State Machine:** 
  - Non-blocking 60-second startup countdown (`RoomTask init \n 60s remain`) with active command parsing.
  - Automatic task presentation if reminders are added during boot.
  - Instant hardware connection detection (Pins D4 & A0) + traffic fallback.
  - Complete single-file Web Dashboard with Web Bluetooth, Web Serial, and .ICS calendar sync.
- [ ] **v0.4 — Non-Volatile Persistent Storage:** EEPROM / on-board Data Flash emulation so tasks persist across reboots.
- [ ] **v0.5 — Real-Time Clock (RTC) Integration:** Utilize the UNO R4's internal RTC for daily scheduled task alerts.

---

## Contributing & Community

Contributions are warmly welcomed! Check out our [Community Showcase Kit](docs/COMMUNITY_SHOWCASE_KIT.md) or open an issue using our [beginner feature template](.github/ISSUE_TEMPLATE/feature_request.yml).

## License

This project is open-source and licensed under the [MIT License](LICENSE).
