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
    - USB Serial (115200 baud)
    - Bluetooth UART / Serial1 (Pins 0 RX, 1 TX at 9600 baud)
  Commands: ADD <text>, LIST, CLEAR, HELP

  Reminders are stored in RAM and are cleared on reset/power loss.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

bool lastBtConnectedState = false;
bool btSerialConnected = false;
unsigned long lastBtTrafficMs = 0;

String usbBuffer;
String btBuffer;

// Bluetooth connection logic:
// 1. Hardware detection via STATE pin on Pin D4 (digital HIGH) or Pin A0 (analog > 2V)
// 2. Active serial traffic detection on Serial1 (within last 15 seconds)
bool isBluetoothConnected() {
  if (digitalRead(BT_STATE_PIN) == HIGH) {
    return true;
  }
  if (analogRead(A0) > 400) {
    return true;
  }
  if (btSerialConnected && (millis() - lastBtTrafficMs < 15000)) {
    return true;
  }
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
  broadcastPrintln(F("  ADD <text> - add a reminder"));
  broadcastPrintln(F("  LIST       - list saved reminders"));
  broadcastPrintln(F("  CLEAR      - remove all reminders"));
  broadcastPrintln(F("  HELP       - show this help"));
}

void showIdleScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("RoomTask"));
  lcd.setCursor(0, 1);
  if (isBluetoothConnected()) {
    lcd.print(F("Connected"));
  } else {
    lcd.print(F("Disconnected"));
  }
}

void showCurrentTask() {
  if (taskCount == 0) {
    showIdleScreen();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Reminder "));
  lcd.print(displayedTask + 1);
  lcd.print('/');
  lcd.print(taskCount);

  lcd.setCursor(0, 1);
  // LCD displays the first 16 characters; longer tasks are stored in full
  lcd.print(tasks[displayedTask]);
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

  if (fromBt) {
    btSerialConnected = true;
    lastBtTrafficMs = millis();
    if (!lastBtConnectedState) {
      lastBtConnectedState = true;
      if (!warmupActive && !showingTasks) showIdleScreen();
    }
  }

  String upper = command;
  upper.toUpperCase();

  if (upper == "HELP") {
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

  // Instant connection state check
  bool currentBt = isBluetoothConnected();
  if (currentBt != lastBtConnectedState) {
    lastBtConnectedState = currentBt;
    if (!showingTasks) {
      showIdleScreen();
    }
    if (currentBt) {
      broadcastPrintln(F("Bluetooth: Connected"));
    } else {
      broadcastPrintln(F("Bluetooth: Disconnected"));
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
