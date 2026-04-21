# HAMSTER FITNESS TRACKER

## Introduction
My hamster, Quyt, is 1.5+ years old and I noticed she seems less active at night than she used to be. This project tracks her nightly running to make sure she is still getting enough exercise. A healthy senior hamster is expected to run 2–4 km per night, so having real data helps me spot any changes in her activity early.

## Project Overview
Built a hamster wheel rotation tracker using an Arduino and a unipolar Hall-effect magnetic switch sensor. The system detects each rotation of the wheel using a magnet attached to the wheel and a Hall-effect sensor mounted on the cage frame. Rotation data is processed in real time to calculate RPM and total distance traveled, then streamed over UART serial communication and validated using a logic analyzer.

## Hardware Used

- Arduino Uno R4
- Magnetic switch unipolar 3SIP (Hall-effect sensor, Package UA pinout)
- Coin magnet
- Digilent Analog Discovery 3 logic analyzer
- SunFounder starter kit (breadboard, jumper wires, resistors)

## How It Works

The core idea: a magnet taped to the hamster wheel passes a sensor on every rotation. Each pass is detected, counted, and converted into a distance.

1. Magnet on the wheel
A small neodymium magnet is taped to the outer edge of the hamster wheel. Every time the wheel completes one full rotation, the magnet swings past the sensor mounted on the cage frame beside it. The two never touch — the sensor reacts to the magnetic field from about 1–2 cm away.

2. Sensor detects the magnet
When the magnet passes close enough, the Hall Effect sensor activates and sends a voltage signal (5V) out through its output leg. When the magnet moves away, the signal drops back to 0V. One full wheel rotation produces exactly one pulse: low -> high -> low.

3. Arduino catches the pulse
The signal wire runs from the sensor to pin D2 on the Arduino. The Arduino is programmed to use an interrupt on that pin — meaning the moment D2 jumps from 0V to 5V, the Arduino instantly pauses whatever it is doing and runs a tiny function that adds 1 to a counter. This happens in microseconds, so no spin is ever missed even if the hamster is running fast.

4. Counter accumulates overnight
The rotation counter is just a number sitting in the Arduino's memory. It starts at 0 when the Arduino is powered on and keeps climbing all night as the hamster runs. Nothing resets it except unplugging the Arduino or pressing the reset button.

5. Results printed every 60 seconds
Every 60 seconds, the Arduino sends the current rotation count and calculated distance through the USB cable to a laptop. Opening the Serial Monitor in the Arduino IDE shows a live log of updates, so in the morning you can scroll back and see the full overnight activity.

### How Distance Is Calculated
Wheel diameter: 30 cm
Each full rotation of the wheel covers a distance equal to the wheel's circumference. Using the formula:
D = n x (π × d)
Where:
- D = total distance
- n = number of rotations
- d = wheel diameter (30 cm)
- π = 3.14159

### Materials Used
| Component                       | Details                                      | Quantity |
|---------------------------------|----------------------------------------------|----------|
| Arduino Uno R4                  | Microcontroller — the brain of the project   | 1
|                                 |                                              |
| Magnetic switch                 | Hall Effect sensor - detects magnet presence | 1
| (unipolar 3SIP)                 |                                              |
|                                 |                                              |
| Neodymium magnet                | Small disc magnet taped to the wheel         | 1
|                                 |                                              |
| Breadboard                      | For connecting components without soldering. | 1
|                                 |                                              |
| 10 kΩ resistor                  | Pull-down resistor on the signal line        | 1
|                                 |                                              |
| Jumper wires                    | For connecting breadboard to Arduino         | 4
|                                 |                                              |
| USB cable                       | Connects Arduino to laptop for power and data| 1
|---------------------------------|----------------------------------------------|

Wheel diameter: 30 cm
Senior hamster running 2-3 km
Formula to calculate cycle: D = n x (pi x d)
where: d is diameter of the wheel, n is number of rotation, pi is 3.14159

Magnetic switch unipolar 3sip: a type of Hall Effect sensor that detects the presence of a magnetic field. Because it is "unipolar," it only responds to one specific pole of a magnet (usually the South pole) and ignores the other. (Voltage: 3.8V ~ 24V)

How It Functions
1. Detection: When the correct magnetic pole (South) gets close enough to the branded face of the sensor, the device "switches" and pulls the output low.

2. Release: When the magnet is removed, the device switches back to its default state.

3. Polarity: If you flip the magnet to the North pole, the sensor will generally do nothing.

Leg Functions
Look at the branded side (the side with the part number printed on it) and the legs are pointing downward, the pins are numbered 1 to 3 from left to right.

- Pin 1: Power supply. Connect this to positive voltage source
- Pin 2: Output. Detect magnet present then send signal to Arduino
- Pin 3: Ground. Connect this to the common ground of your circuit

(Important) Explain leg 2 on magnetic switch: The chip inside is constantly sensing whether a magnet is nearby. When no magnet is present, leg 2 outputs nothing and the pull-down resistor holds it at 0V (LOW). When the magnet gets close, the chip internally connects leg 1's 5V to leg 2, so leg 2 suddenly outputs 5V (HIGH). That HIGH signal travels through the wire to pin 2 on the Arduino, which triggers the interrupt and adds 1 to the counter. When the magnet moves away, the chip disconnects leg 2 again, the pull-down resistor pulls it back to LOW, and the Arduino sees the signal drop.

Components

- Arduino R4 (1x)
- Magnetic switch (1x)
- Breadboard (1x)
- Jumper wires (3x)

Wiring connection

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

### RPM Calculation
```
RPM = (pulses in window × 60000ms) / elapsed time in ms
```

### Speed and Distance
- Distance (m) = total pulse count × circumference (0.942m per rotation)

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

**Logic Analyzer Settings for UART capture:**
- Mode: Repeated
- Base: 20 ms/div
- Trigger: Normal, falling edge on DIO 1
- Protocol: UART, DIO 1, 2400 baud, 8N1, Standard polarity, ASCII

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
