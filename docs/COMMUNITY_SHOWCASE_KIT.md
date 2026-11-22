# Community Showcase & Launch Kit

This kit provides ready-to-use, formatted submission templates to showcase **RoomTask** across the maker and embedded systems web without spending money. 

Publishing across these high-domain-authority platforms builds backlinks and establishes the project as an authoritative reference for search engines and Google AI Overviews.

---

## 1. Hackster.io Submission Template

* **Title:** RoomTask: Smart Motion-Activated Arduino Task Reminder
* **Tagline:** A beginner-friendly Arduino UNO R4 project that uses a PIR motion sensor to display your daily tasks on a 16x2 LCD whenever you enter your room, with wireless Bluetooth task management.
* **Categories:** Arduino, Home Automation, Productivity, Internet of Things
* **Difficulty:** Beginner
* **Estimated Time:** 1 hour
* **Tags:** `arduino-uno-r4`, `pir-sensor`, `lcd-display`, `bluetooth`, `embedded-systems`, `beginner-friendly`

### Project Description / Story
> Have you ever walked into your room intending to do something important, only to get distracted by your phone or computer? 
>
> **RoomTask** solves this with a simple, practical desk accessory: a motion-activated task reminder. Built around the new 32-bit **Arduino UNO R4 Minima**, a passive infrared (PIR) sensor, a 16×2 I²C LCD, and a Bluetooth module, it detects when you enter your room or sit down at your desk and immediately cycles through your daily to-do reminders.
>
> Unlike basic LED blinking tutorials, RoomTask teaches real embedded systems concepts:
> 1. Interfacing digital sensors with hardware debouncing.
> 2. Controlling an LCD screen over the 2-wire I²C bus.
> 3. Concurrent dual-serial command parsing (USB Serial + Bluetooth UART).
> 4. Non-blocking state-machine timing without freezing the CPU using `delay()`.
>
> The best part? You can manage your tasks directly from your smartphone using any free Bluetooth terminal app, or tethered via your computer.
>
> Full open-source code, schematics, and quick-start guides are available on GitHub: https://github.com/anandyakshit/RoomTask

---

## 2. Instructables / Arduino Project Hub Template

* **Step 1: The Concept & How It Works:** Explain the motion trigger, the LCD cycle, and the 30-second non-blocking cooldown.
* **Step 2: Bill of Materials (No-Solder):**
  * Arduino UNO R4 Minima
  * HC-SR501 or AM312 PIR Motion Sensor
  * 16×2 Character LCD with PCF8574 I²C backpack
  * HC-05 / HM-10 Bluetooth module
  * Half-size breadboard & jumper wires
  * USB-C power source
* **Step 3: Wiring the Circuit:** Embed the ASCII diagram and pinout table from `hardware/README.md`.
* **Step 4: Uploading the Firmware:** Provide both the Arduino IDE and Arduino CLI commands.
* **Step 5: Testing with your Smartphone:** Step-by-step pairing with *Serial Bluetooth Terminal* (Android) or *Web Bluetooth* (iOS/Chrome).

---

## 3. Reddit r/arduino & r/diy Showcase Post

* **Post Title:** [Project] RoomTask: Built a motion-activated task reminder with Arduino UNO R4 Minima, PIR sensor, and 16x2 I2C LCD (with wireless Bluetooth management)
* **Flair:** Project Showcase / Tutorial

### Body:
> Hey r/arduino!
>
> Wanted to share a beginner-friendly project I built and verified on the new Arduino UNO R4 Minima called **RoomTask**.
>
> **The Problem:** I kept forgetting small room chores and desk tasks throughout the day.
> **The Solution:** A desktop reminder unit that sits by your door or desk. When the PIR sensor detects motion as you enter the room, it wakes the 16x2 I2C LCD and cycles through your active tasks (10 seconds per task). It then enters a 30-second cooldown so it doesn't repeatedly flash while you remain in the room.
>
> **Cool features:**
> - **Dual-Serial Ingestion:** You can add tasks over USB Serial (115200 baud) or wirelessly over Bluetooth (9600 baud) using a phone terminal app (`ADD <task>`, `LIST`, `CLEAR`).
> - **Zero delay():** Everything is written with non-blocking timing using `millis()` state checks so serial inputs and sensor scans are never blocked.
> - **Beginner-friendly:** Uses modular breadboard jumper connections (zero soldering).
>
> All code, wiring diagrams, and architecture notes are MIT-licensed on GitHub:
> https://github.com/anandyakshit/RoomTask
>
> Would love your feedback and feature suggestions for v0.3!
