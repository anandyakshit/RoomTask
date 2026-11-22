# Hardware Setup — RoomTask (v0.2)

This document describes the verified physical wiring, pin assignments, and electrical requirements for the **RoomTask** reminder system on the **Arduino UNO R4 Minima**.

---

## Parts List

| Component | Recommended Model / Specification | Purpose |
|---|---|---|
| **Microcontroller** | Arduino UNO R4 Minima (Renesas RA4M1 32-bit) | System controller, timers, and dual serial interface |
| **Motion Sensor** | PIR Sensor (HC-SR501, AM312) | Detects room entry motion event |
| **Display** | 16×2 Character LCD with PCF8574 I²C Backpack | Displays status and cycles reminders |
| **Wireless Module** | Bluetooth UART Module (HC-05, HC-06, HM-10, JDY-31) | Wireless task entry via smartphone |
| **Prototyping** | Half-size or full-size Breadboard & Jumper Wires | Power distribution and signal routing |
| **Power Supply** | Standard 5V USB-C power adapter / wall charger | Standalone room power |

---

## Complete Wiring Table

Use the breadboard power rails to distribute **5V** and **GND** from the Arduino to all three peripheral modules.

| Module | Module Pin | UNO R4 Minima Pin | Signal Description |
|---|---|---|---|
| **PIR Sensor** | **VCC** | **5V Rail** | Sensor power supply (4.5V–12V for HC-SR501) |
| | **GND** | **GND Rail** | Common ground |
| | **OUT** | **Digital Pin D2** | Active HIGH digital motion trigger (`PIR_PIN = 2`) |
| **16×2 I²C LCD** | **VCC** | **5V Rail** | Display power supply (requires 5V for crisp contrast) |
| | **GND** | **GND Rail** | Common ground |
| | **SDA** | **Dedicated SDA** | I²C Serial Data (top header, adjacent to AREF / Pin A4) |
| | **SCL** | **Dedicated SCL** | I²C Serial Clock (top header, adjacent to SDA / Pin A5) |
| **Bluetooth Module** | **VCC** | **5V Rail** | Module power (HC-05 / HM-10 breakout boards accept 3.6V–6V) |
| | **GND** | **GND Rail** | Common ground |
| | **TXD** | **Digital Pin 0 (RX1)** | Bluetooth data transmitted to Arduino hardware `Serial1` RX |
| | **RXD** | **Digital Pin 1 (TX1)** | Arduino data transmitted to Bluetooth `Serial1` TX *(see note below)* |

> [!TIP]
> **Bluetooth RXD Voltage Consideration:**  
> The UNO R4 Minima I/O pins operate at 5V logic. Most HC-05 / HC-06 breakout boards have 3.3V-tolerant RX inputs. While many modules tolerate 5V short-term, adding a simple resistor voltage divider (e.g., 1kΩ from Arduino TX1 to BT RXD, and 2kΩ from BT RXD to GND) provides clean 3.3V logic signaling. BT TXD to Arduino Pin 0 (RX1) connects directly because 3.3V HIGH is reliably detected by the Arduino.

---

## Module Calibration & Adjustments

### 1. 16×2 I²C LCD
- **Contrast Adjustment:** If the screen illuminates with no characters or with solid white rectangles, gently turn the blue potentiometer screw on the back of the I²C backpack until characters are crisp and legible.
- **I²C Address:** The firmware default is `0x27`. If characters do not appear, your backpack might use address `0x3F`.

### 2. PIR Motion Sensor
- **Warm-Up Period:** Allow **30 to 60 seconds** after power-up for the sensor to stabilize ambient infrared levels.
- **HC-SR501 Potentiometers:**
  - **Time Delay:** Turn fully counter-clockwise (shortest hold time, ~3 seconds).
  - **Sensitivity:** Set to approximately the midway position.
  - **Trigger Jumper:** Set to position **L** (single trigger mode) to prevent looping triggers while someone remains in the room.

---

## Status Note

* **Core Sensor & LCD Operation:** Verified on live bench hardware on Arduino UNO R4 Minima.
* **Dual Serial:** `Serial` (USB 115200) and `Serial1` (UART 9600) operate concurrently.
