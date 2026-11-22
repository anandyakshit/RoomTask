/*
  RoomTask: Arduino Motion-Activated Task Reminder
  Board: Arduino UNO R4 Minima
  Inputs: PIR motion sensor (Pin D2)
  Output: 16x2 I2C LCD (SDA / SCL)
  Optional: Passive Buzzer / Audio Chime (Pin D3)
  Communication: 
    - USB Serial (115200 baud)
    - Bluetooth UART / Serial1 (Pins 0 RX, 1 TX at 9600 baud)
  Commands: ADD <text>, LIST, CLEAR, HELP

  Reminders are stored in RAM and are cleared on reset/power loss.
  Check hardware/README.md before wiring.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte PIR_PIN = 2;                 // Digital pin for PIR motion sensor
const byte BUZZER_PIN = 3;              // Optional: connect a passive buzzer on Pin D3
const bool ENABLE_BUZZER = false;       // Set to true to enable motion audio chime

const byte LCD_ADDRESS = 0x27;          // Default I2C address (change to 0x3F if needed)
const byte LCD_COLUMNS = 16;
const byte LCD_ROWS = 2;

const unsigned long USB_BAUD_RATE = 115200;
const unsigned long BT_BAUD_RATE  = 9600;   // Default baud for HC-05 / HC-06 / HM-10

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

String usbBuffer;
String btBuffer;

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
    tone(BUZZER_PIN, 1047, 80); // C6 subtle chime
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
  lcd.print(F("RoomTask ready"));
  lcd.setCursor(0, 1);
  lcd.print(taskCount);
  lcd.print(F(" reminder(s)"));
}

void showCurrentTask() {
  if (taskCount == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("No reminders"));
    lcd.setCursor(0, 1);
    lcd.print(F("ADD via BT / USB"));
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
  showIdleScreen();
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

void handleCommand(String command) {
  command.trim();
  if (command.length() == 0) return;

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
    showIdleScreen();
  } else if (upper.startsWith("ADD ")) {
    addTask(command.substring(4));
  } else {
    broadcastPrintln(F("Unknown command. Enter HELP."));
  }
}

// Read commands non-blockingly from an input stream
void readStream(Stream &stream, String &buffer) {
  while (stream.available() > 0) {
    char incoming = (char)stream.read();

    if (incoming == '\n' || incoming == '\r') {
      if (buffer.length() > 0) {
        handleCommand(buffer);
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
  readStream(Serial, usbBuffer);
  readStream(Serial1, btBuffer);
}

void startReminderCycle() {
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
  if (ENABLE_BUZZER) {
    pinMode(BUZZER_PIN, OUTPUT);
  }

  // USB CDC Serial
  Serial.begin(USB_BAUD_RATE);
  // Bluetooth UART Serial1 (Pins 0 RX, 1 TX)
  Serial1.begin(BT_BAUD_RATE);

  lcd.init();
  lcd.backlight();
  showIdleScreen();

  broadcastPrintln(F("\nRoomTask ready. Dual Serial (USB 115200 & Bluetooth 9600) active."));
  printHelp();
}

void loop() {
  readSerialCommands();

  // Expire the cooldown without blocking Serial or LCD updates
  if (cooldownActive && millis() - cooldownStartedAt >= REMINDER_COOLDOWN_MS) {
    cooldownActive = false;
    broadcastPrintln(F("Motion trigger cooldown ended."));
  }

  bool pirState = digitalRead(PIR_PIN) == HIGH;
  if (pirState && !previousPirState && !showingTasks && !cooldownActive) {
    startReminderCycle();
  }
  previousPirState = pirState;

  updateReminderDisplay();
}
