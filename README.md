# HAMSTER FITNESS TRACKER

## Introduction
My hamster, Quyt, is 1.5+ years old and I noticed she seems less active at night than she used to be (I often found her sleeping in the wheel LOL).
<img width="240" height="320" alt="IMG_3295 Small" src="https://github.com/user-attachments/assets/a07578ff-1bdd-4016-8be9-9b7f7adfdada" />


This project tracks her nightly running to make sure she is still getting enough exercise. A healthy senior hamster is expected to run 2–4 km per night, so having real data helps me spot any changes in her activity early.

## Project Overview
Built a hamster wheel rotation tracker using an Arduino and a unipolar Hall-effect magnetic switch sensor. The system detects each rotation of the wheel using a magnet attached to the wheel and a Hall-effect sensor mounted on the cage frame. Rotation data is processed in real time to calculate RPM and total distance traveled, then streamed over UART serial communication and validated using a logic analyzer.

## Material Used

- Arduino Uno R4
- Magnetic switch unipolar 3SIP (Hall-effect sensor, Package UA pinout)
- Coin magnet
- Digilent Analog Discovery 3 logic analyzer
- SunFounder starter kit (breadboard, jumper wires, resistors)
- USB-C cable (2x)

## Wiring connection

| Component | Pin | Arduino / Rail |
|---|---|---|
| Hall sensor Pin 1 (VCC) | Left | Arduino 5V |
| Hall sensor Pin 2 (GND) | Middle | GND rail |
| Hall sensor Pin 3 (VOUT) | Right | Arduino pin 2 |
| Arduino GND | — | Breadboard GND rail |
| AD3 DIO 0 (pink wire) | — | Same row as Arduino pin 2 |
| AD3 DIO 1 (green wire) | — | Arduino pin 8 (SoftwareSerial TX) |
| AD3 GND (black wire) | — | Breadboard GND rail |

## How It Works

### Hall-Effect Sensor
The sensor detects magnetic fields. One coin magnet is taped to the hamster wheel. Every time the wheel spins and the magnet passes the sensor, the sensor output pulls LOW. The Arduino detects this falling edge and counts it as one rotation.

### GPIO Interrupt
Instead of checking the sensor continuously in `loop()`, `attachInterrupt()` is used so the CPU immediately responds the instant the sensor fires.

### Debounce Logic
When the magnet passes the sensor edge, the signal can flicker briefly before settling, causing false counts. The debounce window (5ms) ignores any pulse that arrives too soon after the last one. Since the hamster wheel can spin at most a few rotations per second, any pulse arriving within 5ms of the last is physically impossible and is treated as noise.

### RPM and DistanceCalculation
```
RPM = (pulses in window × 60000ms) / elapsed time in ms

Distance (m) = total pulse count × circumference (0.942m per rotation)
```

### UART Streaming
The Arduino sends RPM data over two serial channels:
- `Serial` (pin 1 / USB) -> readable in Arduino IDE Serial Monitor at 9600 baud
- `mySerial` (pin 8 / SoftwareSerial) -> captured by the logic analyzer at 2400 baud

## Logic Analyzer Validation

**Tool:** Digilent Analog Discovery 3 with WaveForms software

**What was validated:**

| Test | Result |
|---|---|
| Hall sensor signal shape | Clean HIGH/LOW transitions confirmed on DIO 0 |
| Sensor triggers on magnet pass | DIO 0 dips LOW each time magnet approaches |
| UART transmission active | Bursts of square wave pulses captured on DIO 1 |
| UART data decodes to ASCII | RPM value decoded in Events tab |
| RPM changes with magnet activity | Different values captured with/without magnet |

## Issues Encountered and How They Were Fixed

### 1. Sensor always read 0 (LOW)
**Problem:** The Hall sensor output was always LOW regardless of the magnet.

**Cause:** The sensor's active face was pointing away. The unipolar Hall switch only responds to one polarity of magnetic field on the correct face.

**Fix:** Rotated the sensor 180° so the labeled face pointed outward. Output immediately read HIGH with no magnet and LOW when the magnet was brought close (Pin 1 = VCC, Pin 2 = GND, Pin 3 = VOUT.)

### 2. Missing `pinMode(HALL_PIN, INPUT_PULLUP)` in setup
**Problem:** Code was missing the pinMode call, meaning the internal pull-up resistor was never activated. The pin floated and produced unreliable readings.

**Fix:** Added `pinMode(HALL_PIN, INPUT_PULLUP)` in `setup()` before `attachInterrupt()`.

### 3. Logic analyzer showed no signal on DIO 1 (UART)
**Problem:** DIO 1 connected to Arduino pin 1 (TX) showed no activity even though the Serial Monitor was displaying data normally.

**Cause:** Arduino pin 1 (TX) is shared with the onboard USB-Serial chip (ATmega16U2). This chip's circuitry loads the signal, weakening it below the AD3's detection threshold.

**Fix:** Used `SoftwareSerial` to send UART data on pin 8 instead — a clean, unloaded GPIO pin. Connected DIO 1 to pin 8 and got a clean signal.

### 4. UART framing errors in logic analyzer decoder
**Problem:** Protocol decoder showed framing errors and garbled characters even with correct baud rate setting.

**Cause 1:** Sample rate too low at wider time base settings — the AD3 couldn't sample fast enough to resolve individual UART bits.

**Cause 2:** SoftwareSerial timing inaccuracy. At 9600 baud, software bit-banging introduces small timing errors that accumulate and cause the stop bit to be misread.

**Fix:** Reduced SoftwareSerial baud rate from 9600 -> 4800 -> 2400. At 2400 baud each bit is wider, giving both SoftwareSerial and the AD3 more margin. Most characters decoded correctly at 2400 baud, with only the first byte of each transmission garbled (a known SoftwareSerial limitation).

---

## What's Next

- Adding LED to blink when detect magnet
- Log overnight data to PC using Python (pyserial) to plot activity graphs
