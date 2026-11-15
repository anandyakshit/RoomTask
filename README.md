# RoomTask

RoomTask is a simple room-entry task reminder built around an **Arduino UNO R4 Minima**, a PIR motion sensor, and a 16×2 I²C LCD.

When the PIR sensor detects a new motion event, RoomTask displays the saved reminders on the LCD. Tasks are entered over the Arduino USB serial connection and are kept in RAM for the current power session.

## v0.1 scope

- Detect a motion event with a PIR sensor.
- Display reminders on a 16×2 I²C LCD.
- Add, list, and clear reminders through the USB Serial Monitor.
- Keep reminders in temporary RAM storage.
- No Bluetooth, RTC, date/time scheduling, or persistent storage in this version.

## Repository layout

- `firmware/RoomTask/RoomTask.ino` — Arduino sketch.
- `hardware/README.md` — wiring notes and provisional pin configuration.
- `docs/ARCHITECTURE.md` — v0.1 design and behavior.
- `docs/SERIAL_COMMANDS.md` — Serial Monitor command reference.

## Requirements

- Arduino UNO R4 Minima
- PIR motion sensor with a digital output
- 16×2 LCD with an I²C backpack
- Arduino IDE or Arduino CLI
- A compatible `LiquidCrystal_I2C` library

## Getting started

1. Review [hardware setup](hardware/README.md) and confirm the pin assignments match your wiring.
2. Install a compatible `LiquidCrystal_I2C` library in the Arduino IDE.
3. Open `firmware/RoomTask/RoomTask.ino`.
4. Select **Arduino UNO R4 Minima** and the correct USB port.
5. Upload the sketch.
6. Open Serial Monitor at **115200 baud** and set the line ending to **Newline**.
7. Enter `HELP` to see the available commands.

Example:

```text
ADD Water the plants
ADD Check the desk
LIST
```

The reminders are lost when the board resets or loses power.

## Current status

This is the initial v0.1 scaffold. Hardware has not been verified and the sketch has not yet been compile-tested on a physical UNO R4 Minima. Confirm the LCD I²C address and wiring before use.

## License

MIT — see [LICENSE](LICENSE).
