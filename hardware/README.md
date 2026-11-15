# Hardware setup — v0.1

## Parts

- Arduino UNO R4 Minima
- PIR motion sensor with digital OUT
- 16×2 LCD with an I²C backpack
- Jumper wires and a suitable USB cable

## Provisional connections

These are starter defaults, not confirmed wiring. Check the labels and voltage requirements of your exact modules before connecting them.

| Module | Signal | UNO R4 Minima |
|---|---|---|
| PIR sensor | OUT | D2 |
| PIR sensor | VCC / GND | Follow the sensor module's specifications |
| I²C LCD | SDA | SDA |
| I²C LCD | SCL | SCL |
| I²C LCD | VCC / GND | Follow the LCD backpack's specifications |

The UNO R4 Minima exposes dedicated SDA and SCL pins. Use those I²C pins rather than assuming the layout of a different Arduino board.

## LCD address

The sketch currently uses I²C address `0x27`, a common address for LCD backpacks. Some modules use `0x3F` or another address. If the display stays blank, verify the backpack address with an I²C scanner and update `LCD_ADDRESS` in the sketch.

## PIR behavior

Many PIR modules need a warm-up period after power-on. During testing, allow the sensor to stabilize and check whether its output is active HIGH. The starter sketch treats a LOW-to-HIGH transition as a new motion event.

## Before powering up

- Verify the exact voltage requirements of both modules.
- Verify SDA/SCL and ground connections.
- Do not connect a module's power pin based only on wire color.
- Keep the PIR output connected to the configured digital input.
- Change `PIR_PIN` and `LCD_ADDRESS` in the sketch if your setup differs.

No pinout or physical wiring has been confirmed yet.
