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
- **Status: NOT YET HARDWARE-VERIFIED.**
- Testing was performed purely through automated command-line toolchain compilation.
- Physical board was not attached during this test (`arduino-cli board list` returned no connected boards).
- Bench verification required:
  - Validate physical signal integrity on Pin D2 with HC-SR501 / AM312 PIR sensors.
  - Verify I2C communication on dedicated SDA/SCL pins with PCF8574 I2C backpack.
  - Check whether the backpack address is `0x27` or `0x3F`.

#### Next Steps
- [ ] Connect physical Arduino UNO R4 Minima over USB and test upload via `arduino-cli upload`.
- [ ] Test Serial Monitor commands (`ADD`, `LIST`, `CLEAR`, `HELP`) at 115200 baud.
- [ ] Confirm PIR motion trigger response time and LCD contrast trimmer settings.
