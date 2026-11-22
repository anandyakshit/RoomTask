# Serial & Bluetooth Command Reference

RoomTask features dual-stream command processing. You can manage reminders over **USB Serial** (when tethered to a computer) or **Bluetooth** (wirelessly via your smartphone) simultaneously.

---

## Connection Settings

| Interface | Hardware Port | Baud Rate | Line Ending | Recommended Software |
|---|---|---|---|---|
| **USB Serial** | USB-C (`Serial`) | **115200** | **Newline (`\n`)** | Arduino IDE Serial Monitor, Arduino CLI, PuTTY |
| **Bluetooth UART** | Pins 0 & 1 (`Serial1`) | **9600** *(default)* | **Newline (`\n`)** | Android: *Serial Bluetooth Terminal* (Kai Morich)<br>iOS/Web: *Web Bluetooth Terminal*, *DSD TECH Bluetooth* |

---

## Supported Commands

Commands are case-insensitive and processed in real time:

| Command | Action | Example Input | Example Output |
|---|---|---|---|
| `ADD <text>` | Add a reminder (up to 40 characters) | `ADD Buy milk` | `Added: Buy milk` |
| `LIST` | List all saved reminders in sequence | `LIST` | `1. Buy milk`<br>`2. Check desk` |
| `CLEAR` | Remove all saved reminders from memory | `CLEAR` | `All reminders cleared.` |
| `HELP` | Display available commands | `HELP` | Displays command usage |

---

## Example Wireless Session (Smartphone via Bluetooth)

1. Pair your phone with your Bluetooth module (e.g., `HC-05`, PIN is typically `1234` or `0000`).
2. Open **Serial Bluetooth Terminal** on your phone.
3. Tap **Connect** to establish the RFCOMM serial link.
4. Enter commands directly:

```text
ADD Water the indoor plants
Added: Water the indoor plants

ADD Submit assignment
Added: Submit assignment

LIST
Saved reminders:
1. Water the indoor plants
2. Submit assignment
```

5. When you walk past the PIR motion sensor, the display will illuminate and cycle through your tasks!

---

## Storage & Limits

- Holds up to **8 simultaneous reminders**.
- Up to **40 characters** per reminder.
- In **v0.2**, reminders are held in volatile SRAM for fast, non-wearing operations during the active power session.
