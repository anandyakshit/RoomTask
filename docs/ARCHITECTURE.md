# RoomTask v0.1 architecture

## Goal

Show a short list of reminders when the room's PIR sensor detects a new motion event.

## Data flow

1. The user enters commands through the USB serial connection.
2. The Arduino parses the command and updates the in-memory reminder list.
3. The PIR input is checked continuously.
4. A LOW-to-HIGH transition starts a reminder display cycle.
5. The LCD displays each saved reminder in sequence.

## Main components

- **Arduino UNO R4 Minima:** runs the application.
- **PIR sensor:** supplies a digital motion signal.
- **16×2 I²C LCD:** displays status and reminders.
- **USB Serial:** provides the initial configuration interface.

## Storage

The v0.1 task list is held in a fixed-size RAM array:

- Maximum reminders: 8
- Maximum stored text per reminder: 40 characters
- Lifetime: until reset or power loss

There is no EEPROM/flash persistence in this version.

## Display behavior

A rising edge on the PIR input begins the reminder cycle. Each reminder is displayed for approximately 2.5 seconds. After the last reminder, the idle screen returns. If there are no reminders, the LCD displays an empty-list message.

## Deliberate exclusions

- Bluetooth or Wi-Fi control
- RTC or calendar-based scheduling
- Persistent task storage
- Mobile or browser interface
- Multiple rooms or sensors

The first configuration path is USB Serial Monitor, as agreed for the initial prototype.

## Known limitations

- The PIR sensor reports motion, not a guaranteed human room-entry event.
- A person already in the room may trigger additional motion events depending on sensor behavior.
- LCD address and physical pin assignments must be checked against the actual modules.
- The sketch is an initial scaffold and still needs compile and hardware testing.
