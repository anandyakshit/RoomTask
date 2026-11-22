/*
  RoomTask: Arduino Motion-Activated Task Reminder
  Board: Arduino UNO R4 Minima
  Inputs: 
    - PIR motion sensor (Pin D2)
    - Bluetooth STATE detection (Pin D4 or Analog A0)
  Output: 
    - 16x2 I2C LCD (SDA / SCL)
    - Passive Buzzer / Audio Chime (Pin D3, optional)
  Communication: 
    - USB Serial (115200 baud) - Web Serial / CDC
    - Bluetooth UART / Serial1 (Pins 0 RX, 1 TX at 9600 baud) - HC-05 / HM-10
  Commands:
    - ADD <text>      : add a reminder (e.g. "ADD 09:00: Daily Standup" or "ADD 4:30 PM: Team Standup")
    - LIST            : list saved reminders
    - CLEAR           : remove all reminders
    - CONNECT         : signal companion active (Web Serial / Bluetooth)
    - DISCONNECT      : signal companion disconnected
    - HELP            : show available commands

  Reminders are stored in RAM and are cleared on reset/power loss.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ctype.h>

const byte PIR_PIN = 2;                 // Digital pin for PIR motion sensor
const byte BUZZER_PIN = 3;              // Optional: connect a passive buzzer on Pin D3
const bool ENABLE_BUZZER = false;       // Set to true to enable motion audio chime
const byte BT_STATE_PIN = 4;            // Connect to HC-05/HM-10 "STATE" pin (or use A0)

const byte LCD_ADDRESS = 0x27;          // Default I2C address (change to 0x3F if needed)
const byte LCD_COLUMNS = 16;
const byte LCD_ROWS = 2;

const unsigned long USB_BAUD_RATE = 115200;
const unsigned long BT_BAUD_RATE  = 9600;   // Default baud for HC-05 / HC-06 / HM-10

const unsigned long PIR_WARMUP_MS = 60000;  // 60-second PIR sensor warm-up
const byte MAX_TASKS = 8;
const byte TASK_LENGTH = 41;            // 40 characters plus null terminator
const unsigned long PAGE_INTERVAL_MS = 10000;    // 10s per reminder on LCD
const unsigned long REMINDER_COOLDOWN_MS = 30000; // 30s cooldown after display cycle

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

char tasks[MAX_TASKS][TASK_LENGTH];
byte taskCount = 0;

bool previousPirState = false;
bool showingTasks = false;
byte displayedTask = 0;
unsigned long lastPageChange = 0;
unsigned long cooldownStartedAt = 0;
bool cooldownActive = false;

unsigned long bootTime = 0;
bool warmupActive = true;
int lastRemainingSec = -1;

// Connection tracking
bool lastConnectedState = false;
bool btSerialConnected = false;
unsigned long lastBtTrafficMs = 0;
bool usbConnected = false;
unsigned long lastUsbTrafficMs = 0;

String usbBuffer;
String btBuffer;

// Connection detection logic:
// 1. Bluetooth hardware detection via STATE pin (Pin D4 digital HIGH) or Pin A0 (analog > 400 / ~2V)
// 2. Bluetooth active serial traffic / handshake (within last 15 seconds)
// 3. USB Web Serial connection (handshake received AND USB port open)
bool isDeviceConnected() {
  if (digitalRead(BT_STATE_PIN) == HIGH) return true;
  if (analogRead(A0) > 400) return true;
  if (btSerialConnected && (millis() - lastBtTrafficMs < 15000)) return true;
  if (usbConnected && bool(Serial)) return true;
  return false;
}

// Broadcast text to both USB Serial and Bluetooth UART
void broadcastPrint(const __FlashStringHelper *ifsh) {
  Serial.print(ifsh);
  Serial1.print(ifsh);
}

void broadcastPrintln(const __FlashStringHelper *ifsh) {
  Serial.println(ifsh);
  Serial1.println(ifsh);
}

void broadcastPrint(const String &text) {
  Serial.print(text);
  Serial1.print(text);
}

void broadcastPrintln(const String &text = "") {
  Serial.println(text);
  Serial1.println(text);
}

void broadcastPrint(int val) {
  Serial.print(val);
  Serial1.print(val);
}

void broadcastPrintln(int val) {
  Serial.println(val);
  Serial1.println(val);
}

void playChime() {
  if (ENABLE_BUZZER) {
    tone(BUZZER_PIN, 1047, 80); // C6 chime
    delay(100);
    tone(BUZZER_PIN, 1319, 120); // E6 chime
  }
}

void printHelp() {
  broadcastPrintln(F("RoomTask commands (USB 115200 / BT 9600):"));
  broadcastPrintln(F("  ADD <text>   - add a reminder (e.g. ADD 10:00 AM: Meeting)"));
  broadcastPrintln(F("  LIST         - list saved reminders"));
  broadcastPrintln(F("  CLEAR        - remove all reminders"));
  broadcastPrintln(F("  CONNECT      - companion connection handshake"));
  broadcastPrintln(F("  DISCONNECT   - companion disconnect handshake"));
  broadcastPrintln(F("  HELP         - show this help"));
}

void showIdleScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("RoomTask        "));
  lcd.setCursor(0, 1);
  if (isDeviceConnected()) {
    lcd.print(F("Connected       "));
  } else {
    lcd.print(F("Disconnected    "));
  }
}

// Parses a raw reminder string into TIME (row 0) and REMINDER NAME (row 1)
void parseReminder(const char *raw, String &outTime, String &outName) {
  outTime = "";
  outName = "";
  if (!raw || strlen(raw) == 0) return;

  String str = raw;
  str.trim();
  int len = str.length();

  // Pattern 1: Check if starts with a time format, e.g. "H:MM", "HH:MM", "4:30 PM", "09:00"
  int colonPos = -1;
  int searchLimit = (len < 6) ? len : 6;
  for (int i = 0; i < searchLimit; i++) {
    if (str[i] == ':') {
      colonPos = i;
      break;
    }
  }

  bool isTimeAtStart = false;
  int timeEndIdx = -1;

  if (colonPos == 1 || colonPos == 2) {
    // Verify preceding characters are digits
    bool preDigits = isDigit(str[0]) && (colonPos == 1 || isDigit(str[1]));
    // Verify next 2 characters are digits (minutes)
    if (preDigits && (colonPos + 2 < len) && isDigit(str[colonPos + 1]) && isDigit(str[colonPos + 2])) {
      isTimeAtStart = true;
      timeEndIdx = colonPos + 3; // position after minutes

      // Check for optional "AM", "PM", "am", "pm"
      int p = timeEndIdx;
      while (p < len && str[p] == ' ') p++;
      if (p + 1 < len) {
        char c1 = toupper((unsigned char)str[p]);
        char c2 = toupper((unsigned char)str[p + 1]);
        if ((c1 == 'A' || c1 == 'P') && c2 == 'M') {
          timeEndIdx = p + 2;
        }
      }
    }
  }

  if (isTimeAtStart) {
    outTime = str.substring(0, timeEndIdx);
    outTime.trim();

    // Remainder is the reminder name
    String remainder = str.substring(timeEndIdx);
    remainder.trim();
    while (remainder.length() > 0 && (remainder[0] == ':' || remainder[0] == '-' || remainder[0] == ' ')) {
      remainder = remainder.substring(1);
      remainder.trim();
    }
    outName = remainder;
  } else {
    // Pattern 2: Check for " at " or " @ " followed by time: e.g. "Team Standup at 10:00"
    int atPos = str.indexOf(" @ ");
    if (atPos == -1) atPos = str.indexOf(" at ");
    if (atPos == -1) atPos = str.indexOf(" AT ");

    if (atPos != -1) {
      String potentialTime = str.substring(atPos + 3);
      potentialTime.trim();
      if (potentialTime.length() >= 4 && potentialTime.indexOf(':') != -1) {
        outTime = potentialTime;
        outName = str.substring(0, atPos);
        outName.trim();
      }
    }

    if (outTime.length() == 0) {
      outName = str;
    }
  }

  if (outName.length() == 0) {
    outName = str;
  }
}

void showCurrentTask() {
  if (taskCount == 0) {
    showIdleScreen();
    return;
  }

  String timeStr, nameStr;
  parseReminder(tasks[displayedTask], timeStr, nameStr);

  // Line 0: "x/y TIME" (or "x/y" if no time present)
  char rawLine0[32];
  if (timeStr.length() > 0) {
    snprintf(rawLine0, sizeof(rawLine0), "%d/%d %s", displayedTask + 1, taskCount, timeStr.c_str());
  } else {
    snprintf(rawLine0, sizeof(rawLine0), "%d/%d", displayedTask + 1, taskCount);
  }

  // Exactly 16 characters for LCD row 0
  char line0[17];
  snprintf(line0, sizeof(line0), "%-16.16s", rawLine0);

  // Exactly 16 characters for LCD row 1 (reminder name)
  char line1[17];
  snprintf(line1, sizeof(line1), "%-16.16s", nameStr.c_str());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);

  broadcastPrint(F("Displaying: "));
  broadcastPrint(line0);
  broadcastPrint(F(" | "));
  broadcastPrintln(line1);
}

void addTask(const String &taskText) {
  if (taskText.length() == 0) {
    broadcastPrintln(F("Usage: ADD <text>"));
    return;
  }

  if (taskCount >= MAX_TASKS) {
    broadcastPrintln(F("Task list full. Use CLEAR before adding more."));
    return;
  }

  taskText.substring(0, TASK_LENGTH - 1).toCharArray(tasks[taskCount], TASK_LENGTH);
  taskCount++;

  broadcastPrint(F("Added: "));
  broadcastPrintln(tasks[taskCount - 1]);
  if (!warmupActive && !showingTasks) {
    showIdleScreen();
  }
}

void listTasks() {
  if (taskCount == 0) {
    broadcastPrintln(F("No reminders saved."));
    return;
  }

  broadcastPrintln(F("Saved reminders:"));
  for (byte i = 0; i < taskCount; i++) {
    broadcastPrint(i + 1);
    broadcastPrint(F(". "));
    broadcastPrintln(tasks[i]);
  }
}

void handleCommand(String command, bool fromBt) {
  command.trim();
  if (command.length() == 0) return;

  // Track traffic source
  if (fromBt) {
    btSerialConnected = true;
    lastBtTrafficMs = millis();
  } else {
    usbConnected = true;
    lastUsbTrafficMs = millis();
  }

  String upper = command;
  upper.toUpperCase();

  if (upper == "CONNECT") {
    if (fromBt) {
      btSerialConnected = true;
      lastBtTrafficMs = millis();
    } else {
      usbConnected = true;
      lastUsbTrafficMs = millis();
    }
    broadcastPrintln(F("OK: Connected"));
    if (!warmupActive && !showingTasks) {
      showIdleScreen();
    }
  } else if (upper == "DISCONNECT") {
    if (fromBt) {
      btSerialConnected = false;
      lastBtTrafficMs = 0;
    } else {
      usbConnected = false;
      lastUsbTrafficMs = 0;
    }
    broadcastPrintln(F("OK: Disconnected"));
    if (!warmupActive && !showingTasks) {
      showIdleScreen();
    }
  } else if (upper == "HELP") {
    printHelp();
  } else if (upper == "LIST") {
    listTasks();
  } else if (upper == "CLEAR") {
    taskCount = 0;
    showingTasks = false;
    broadcastPrintln(F("All reminders cleared."));
    if (!warmupActive) showIdleScreen();
  } else if (upper.startsWith("ADD ")) {
    addTask(command.substring(4));
  } else {
    broadcastPrintln(F("Unknown command. Enter HELP."));
  }
}

// Read commands non-blockingly from an input stream
void readStream(Stream &stream, String &buffer, bool isBt) {
  while (stream.available() > 0) {
    char incoming = (char)stream.read();

    if (incoming == '\n' || incoming == '\r') {
      if (buffer.length() > 0) {
        handleCommand(buffer, isBt);
        buffer = "";
      }
    } else if (buffer.length() < 100) {
      buffer += incoming;
    } else {
      buffer = "";
      broadcastPrintln(F("Command too long; buffer cleared."));
    }
  }
}

void readSerialCommands() {
  readStream(Serial, usbBuffer, false);
  readStream(Serial1, btBuffer, true);
}

void startReminderCycle() {
  if (taskCount == 0) return; // Do not start cycle if there are no reminders
  showingTasks = true;
  displayedTask = 0;
  lastPageChange = millis();
  showCurrentTask();
  playChime();
  broadcastPrintln(F("Motion detected: showing reminders."));
}

void updateReminderDisplay() {
  if (!showingTasks || taskCount == 0) return;

  if (millis() - lastPageChange >= PAGE_INTERVAL_MS) {
    lastPageChange = millis();
    displayedTask++;

    if (displayedTask >= taskCount) {
      showingTasks = false;
      cooldownStartedAt = millis();
      cooldownActive = true;
      showIdleScreen();
      broadcastPrintln(F("Reminder cycle finished. Motion trigger cooldown started."));
      return;
    }

    showCurrentTask();
  }
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(BT_STATE_PIN, INPUT);

  if (ENABLE_BUZZER) {
    pinMode(BUZZER_PIN, OUTPUT);
  }

  // USB CDC Serial
  Serial.begin(USB_BAUD_RATE);
  // Bluetooth UART Serial1 (Pins 0 RX, 1 TX)
  Serial1.begin(BT_BAUD_RATE);

  lcd.init();
  lcd.backlight();
  
  bootTime = millis();
  warmupActive = true;
  lastRemainingSec = -1;

  // Initialize display with non-blocking warm-up screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("RoomTask init   "));
  lcd.setCursor(0, 1);
  lcd.print(F("60s remain      "));

  broadcastPrintln(F("\nRoomTask ready. Starting 60s PIR warm-up."));
  printHelp();
}

void loop() {
  // Always listen for commands (USB & Bluetooth) without any delay() blocking
  readSerialCommands();

  // If USB cable unplugged or port closed by OS, clear usbConnected
  if (usbConnected && !bool(Serial)) {
    usbConnected = false;
  }

  // Dynamic connection state detection
  bool currentConn = isDeviceConnected();
  if (currentConn != lastConnectedState) {
    lastConnectedState = currentConn;
    if (!showingTasks && !warmupActive) {
      showIdleScreen();
    }
    if (currentConn) {
      broadcastPrintln(F("Status: Connected"));
    } else {
      broadcastPrintln(F("Status: Disconnected"));
    }
  }

  // Non-blocking 60-second PIR warm-up countdown
  if (warmupActive) {
    unsigned long elapsed = millis() - bootTime;
    if (elapsed < PIR_WARMUP_MS) {
      int remaining = (int)((PIR_WARMUP_MS - elapsed + 999) / 1000);
      if (remaining != lastRemainingSec) {
        lastRemainingSec = remaining;
        lcd.setCursor(0, 0);
        lcd.print(F("RoomTask init   "));
        lcd.setCursor(0, 1);
        char buf[17];
        snprintf(buf, sizeof(buf), "%-2ds remain     ", remaining);
        lcd.print(buf);
      }
      return; // Skip PIR motion check while warming up
    } else {
      warmupActive = false;
      broadcastPrintln(F("PIR warm-up complete. Ready for motion detection."));
      if (taskCount > 0) {
        startReminderCycle();
      } else {
        showIdleScreen();
      }
    }
  }

  // Expire the cooldown without blocking Serial or LCD updates
  if (cooldownActive && millis() - cooldownStartedAt >= REMINDER_COOLDOWN_MS) {
    cooldownActive = false;
    broadcastPrintln(F("Motion trigger cooldown ended."));
  }

  bool pirState = digitalRead(PIR_PIN) == HIGH;
  if (pirState && !previousPirState && !showingTasks && !cooldownActive) {
    if (taskCount > 0) {
      startReminderCycle();
    }
  }
  previousPirState = pirState;

  updateReminderDisplay();
}
