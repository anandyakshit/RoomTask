# RoomTask Architecture (v0.2)

## Goal

Provide a room-entry task reminder system where a PIR motion sensor triggers a 16×2 character LCD to cycle through to-do items, with task management accessible via both USB serial and wireless Bluetooth UART.

---

## Data Flow & System Loop

```text
[ Smartphone (Bluetooth) ] \
                            --> [ Hardware Serial1 (9600) ] \
                                                             --> [ Command Parser & RAM Store ]
                            --> [ USB CDC Serial (115200) ] /
[ Computer (USB Cable)   ] /                                                 |
                                                                             v
[ PIR Motion Sensor (D2) ] ---------------------------------------------> [ Display State Machine ]
                                                                             |
                                                                             v
                                                                 [ 16x2 I2C LCD Display ]
```

1. **Task Ingestion:** Commands (`ADD`, `LIST`, `CLEAR`, `HELP`) are received concurrently from either USB (`Serial`) or Bluetooth (`Serial1`).
2. **Buffer Parsing:** Incoming newline characters (`\n` / `\r`) trigger the command handler.
3. **Task Storage:** Up to 8 tasks (40 chars each) are held in dynamic SRAM.
4. **Motion Detection:** The PIR digital input (Pin D2) is scanned non-blockingly each loop iteration.
5. **Display State Machine:**
   - On a LOW-to-HIGH motion transition, the system enters the active display cycle.
   - Each task is rendered on the 16×2 LCD for 10 seconds.
   - When all tasks have been shown, an automatic 30-second cooldown starts to prevent re-triggering while the user remains in the room.
   - The screen returns to the idle state (`RoomTask ready`).

---

## Main Components

- **Arduino UNO R4 Minima:** Renesas RA4M1 48 MHz 32-bit ARM Cortex-M4 microcontroller.
- **PIR Sensor:** Supplies active-HIGH digital motion trigger on Pin D2.
- **16×2 Character LCD with PCF8574 I²C Backpack:** Displays system status and reminders using 2-wire I²C (`SDA` / `SCL`).
- **Bluetooth UART Module (HC-05 / HM-10):** Connected to hardware `Serial1` (Pins 0 RX and 1 TX) enabling wireless task entry from smartphones.
- **USB Interface:** Maintains tethered serial terminal capabilities at 115200 baud.

---

## Verification Status

- **v0.1 Physical Hardware Test:** PASSED (PIR motion trigger, 16×2 I²C LCD contrast and communication, and Serial CLI verified on physical test bench).
- **v0.2 Firmware Compilation:** PASSED (Verified via Arduino CLI with `arduino:renesas_uno` 1.6.0 core, 0 compilation errors).
