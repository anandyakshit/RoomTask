# USB Serial commands

Connect the UNO R4 Minima over USB and open Serial Monitor.

## Serial Monitor settings

- Baud rate: **115200**
- Line ending: **Newline** (or Both NL & CR)

## Commands

| Command | Action |
|---|---|
| `ADD <text>` | Add a reminder |
| `LIST` | Print all saved reminders |
| `CLEAR` | Remove all reminders |
| `HELP` | Print command help |

Commands are case-insensitive.

## Example session

```text
ADD Water the plants
Added: Water the plants
ADD Check the desk
Added: Check the desk
LIST
Saved reminders:
1. Water the plants
2. Check the desk
CLEAR
All reminders cleared.
```

## Limits

- Up to 8 reminders at once.
- Up to 40 characters stored per reminder.
- Reminders are held in RAM and disappear after reset or power loss.
