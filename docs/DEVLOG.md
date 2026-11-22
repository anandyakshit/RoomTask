# Development Log (DEVLOG)

This document tracks changes, toolchain checks, compilation results, and hardware verification notes for the **RoomTask** project.

---

## Log Entry Template

Use this markdown structure for future development entries:

```markdown
### [YYYY-MM-DD] - <Short Summary of Changes or Milestone>

**Author / Contributor:** <Name or GitHub handle>
**Version / Milestone:** <e.g., v0.1-dev, v0.2-alpha>
**Hardware Status:** [Unverified | Compile-Verified | Bench-Tested | Fully Verified]

#### Summary
- Brief bullet points of what was done.

#### Toolchain & Environment
- **Core / Toolchain:** `arduino:renesas_uno` (<version>)
- **Board FQBN:** `arduino:renesas_uno:minima`
- **Libraries Used:** `<LibraryName>` (<version>)
- **Host OS:** <Windows / Linux / macOS>

#### CLI Commands & Compilation
```bash
arduino-cli compile --fqbn <FQBN> <SketchPath>
```
- **Flash Memory:** `<X> bytes (<Y>%)`
- **SRAM Memory:** `<X> bytes (<Y>%)`
- **Compiler Warnings / Notes:** `<Any notes or flags>`

#### Hardware Verification Notes
- Wiring, pin assignments, bench measurements, or known hardware issues observed.

#### Next Steps
- [ ] Task 1
- [ ] Task 2
```

---

## Log Entries

### [2026-11-22] - Physical Hardware Validation & v0.2 Bluetooth Integration

**Author / Contributor:** @anandyakshit  
**Version / Milestone:** v0.2 Dual Serial & Bluetooth  
**Hardware Status:** Fully Bench-Tested & Hardware-Verified (End-to-End Core, LCD, PIR, USB, & Bluetooth)

#### Summary
- **Physical Hardware Verification PASSED:** Physical Arduino UNO R4 Minima was connected on COM6, programmed via DFU, and verified on live test bench. PIR motion triggering (Pin D2), 16×2 I²C LCD (SDA/SCL at `0x27`), and command line parser (`ADD`, `LIST`, `CLEAR`, `HELP`) were confirmed functional on hardware.
- **Added Bluetooth UART Support:** Updated `RoomTask.ino` to listen concurrently on hardware `Serial1` (Pins 0 RX, 1 TX at 9600 baud) and USB `Serial` (115200 baud).
- **Dual-Stream Broadcast & Loopback Validated:** Tested live concurrent communication. Commands sent over USB Serial (`ADD USB Serial Test`) were executed in real time, logged on the 16x2 LCD, and mirrored instantly to the smartphone Bluetooth terminal app.
- Updated documentation across `README.md`, `hardware/README.md`, `docs/ARCHITECTURE.md`, and `docs/SERIAL_COMMANDS.md`.

#### Toolchain & Environment
- **Arduino CLI Version:** Bundled CLI with Arduino IDE 2.x
- **Core Package:** `arduino:renesas_uno` (version 1.6.0)
- **Board FQBN:** `arduino:renesas_uno:minima`
- **External Libraries:** `LiquidCrystal I2C` (by Frank de Brabander / Marco Schwartz)
- **Host OS:** Windows 10/11 x64

#### CLI Commands & Compilation Results
```bash
arduino-cli compile --fqbn arduino:renesas_uno:minima firmware/RoomTask
```
- **Flash Memory:** 49,376 bytes (18% of 262,144 bytes maximum)
- **SRAM Memory:** 5,856 bytes (17% of 32,768 bytes maximum)
- **Compilation Status:** 0 errors.

#### Hardware Verification Notes
- Bench test confirmed: PIR sensor LOW-to-HIGH transitions correctly trigger the 16x2 LCD cycle.
- I2C backpack contrast potentiometer adjusted for clear readability at address `0x27`.
- Bluetooth module (HC-05 / HM-10) wiring added: TXD to Pin 0 (RX1), RXD to Pin 1 (TX1 with recommended resistor divider).

#### Next Steps
- [x] Wire physical Bluetooth module (HC-05 / HM-10) to Pin 0 & Pin 1 and test wireless smartphone pairing. (PASSED)
- [ ] Proceed to v0.3: Non-volatile persistent task storage (EEPROM / Data Flash emulation).

---

### [2026-11-21] - Initial Toolchain Setup & Arduino CLI Compilation Verification

**Author / Contributor:** @anandyakshit  
**Version / Milestone:** v0.1 Scaffold  
**Hardware Status:** Compile-Verified (Physical Hardware Unverified)

#### Summary
- Verified project compilation using Arduino CLI with the Renesas UNO core.
- Confirmed exact core identifiers, FQBN, and external library requirements for Arduino UNO R4 Minima.
- Established baseline memory consumption metrics.

#### Toolchain & Environment
- **Arduino CLI Version:** Bundled CLI with Arduino IDE 2.x
- **Core Package:** `arduino:renesas_uno` (version 1.6.0 installed)
- **Board FQBN:** `arduino:renesas_uno:minima`
- **External Libraries:** `LiquidCrystal I2C` (by Frank de Brabander / Marco Schwartz)
- **Host OS:** Windows 10/11 x64

#### CLI Commands & Verification Results
1. Core Installation:
   ```bash
   arduino-cli core install arduino:renesas_uno
   ```
   *Result:* Successfully installed/verified platform `arduino:renesas_uno@1.6.0`.

2. Library Installation:
   ```bash
   arduino-cli lib install "LiquidCrystal I2C"
   ```
   *Result:* Confirmed library `LiquidCrystal I2C` installed in sketch libraries.

3. Compilation Command:
   ```bash
   arduino-cli compile --fqbn arduino:renesas_uno:minima firmware/RoomTask
   ```
   *Result:* **Compilation SUCCEEDED with 0 errors.**
   - **Flash Usage:** 49,132 bytes (18% of 262,144 bytes maximum)
   - **SRAM Usage:** 5,848 bytes (17% of 32,768 bytes maximum)
   - **Notes:** Warning emitted: `WARNING: library LiquidCrystal_I2C claims to run on all architecture(s) and may be incompatible with your current board which runs on renesas_uno architecture(s).` Despite the architecture header warning in `library.properties`, the library headers and implementation compile cleanly against the Renesas UNO R4 Wire HAL.

#### Hardware Verification Notes
- Initial scaffold compile check.
