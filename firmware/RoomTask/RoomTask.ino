/*
  RoomTask v0.1
  Board: Arduino UNO R4 Minima
  Inputs: PIR motion sensor
  Output: 16x2 I2C LCD
  Commands: ADD <text>, LIST, CLEAR, HELP

  Reminders are stored in RAM and are cleared on reset/power loss.
  Check hardware/README.md before wiring or uploading.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte PIR_PIN = 2;                 // Provisional: change to match your wiring.
const byte LCD_ADDRESS = 0x27;          // Common default; scan/change if your module differs.
const byte LCD_COLUMNS = 16;
const byte LCD_ROWS = 2;

const byte MAX_TASKS = 8;
const byte TASK_LENGTH = 41;            // 40 visible characters plus null terminator.
// Each reminder stays on the LCD for at least 10 seconds.
const unsigned long PAGE_INTERVAL_MS = 10000;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

char tasks[MAX_TASKS][TASK_LENGTH];
byte taskCount = 0;

bool previousPirState = false;
bool showingTasks = false;
byte displayedTask = 0;
unsigned long lastPageChange = 0;

String serialBuffer;

void printHelp() {
  Serial.println(F("RoomTask v0.1 commands:"));
  Serial.println(F("  ADD <text> - add a reminder"));
  Serial.println(F("  LIST       - list saved reminders"));
  Serial.println(F("  CLEAR      - remove all reminders"));
  Serial.println(F("  HELP       - show this help"));
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
    lcd.print(F("Use ADD via USB"));
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Reminder "));
  lcd.print(displayedTask + 1);
  lcd.print('/');
  lcd.print(taskCount);

  lcd.setCursor(0, 1);
  // LCD displays the first 16 characters; longer tasks are stored in full.
  lcd.print(tasks[displayedTask]);
}

void addTask(const String &taskText) {
  if (taskText.length() == 0) {
    Serial.println(F("Usage: ADD <text>"));
    return;
  }

  if (taskCount >= MAX_TASKS) {
    Serial.println(F("Task list full. Use CLEAR before adding more."));
    return;
  }

  taskText.substring(0, TASK_LENGTH - 1).toCharArray(tasks[taskCount], TASK_LENGTH);
  taskCount++;

  Serial.print(F("Added: "));
  Serial.println(tasks[taskCount - 1]);
  showIdleScreen();
}

void listTasks() {
  if (taskCount == 0) {
    Serial.println(F("No reminders saved."));
    return;
  }

  Serial.println(F("Saved reminders:"));
  for (byte i = 0; i < taskCount; i++) {
    Serial.print(i + 1);
    Serial.print(F(". "));
    Serial.println(tasks[i]);
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
    Serial.println(F("All reminders cleared."));
    showIdleScreen();
  } else if (upper.startsWith("ADD ")) {
    addTask(command.substring(4));
  } else {
    Serial.println(F("Unknown command. Enter HELP."));
  }
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    char incoming = (char)Serial.read();

    if (incoming == '\n' || incoming == '\r') {
      if (serialBuffer.length() > 0) {
        handleCommand(serialBuffer);
        serialBuffer = "";
      }
    } else if (serialBuffer.length() < 100) {
      serialBuffer += incoming;
    } else {
      serialBuffer = "";
      Serial.println(F("Command too long; buffer cleared."));
    }
  }
}

void startReminderCycle() {
  showingTasks = true;
  displayedTask = 0;
  lastPageChange = millis();
  showCurrentTask();
  Serial.println(F("Motion detected: showing reminders."));
}

void updateReminderDisplay() {
  if (!showingTasks || taskCount == 0) return;

  if (millis() - lastPageChange >= PAGE_INTERVAL_MS) {
    lastPageChange = millis();
    displayedTask++;

    if (displayedTask >= taskCount) {
      showingTasks = false;
      showIdleScreen();
      return;
    }

    showCurrentTask();
  }
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  showIdleScreen();

  Serial.println(F("\nRoomTask v0.1 ready."));
  printHelp();
}

void loop() {
  readSerialCommands();

  bool pirState = digitalRead(PIR_PIN) == HIGH;
  if (pirState && !previousPirState) {
    startReminderCycle();
  }
  previousPirState = pirState;

  updateReminderDisplay();
}
