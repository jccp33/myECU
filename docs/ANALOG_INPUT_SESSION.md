# STM32 Analog Input Bring-Up — Potentiometer Session

**Date:** 2026-09-09  
**Platform:** STM32F103C8T6 (Blue Pill)  
**Project:** myECU  
**Status:** Passive characterization completed

---

## 1. Objective

Begin the first physical input integration for `myECU` using a potentiometer as a controlled analog source.

The final intended signal path is:

```text
Potentiometer
      ↓
STM32 ADC
      ↓
SignalSample
      ↓
SignalStore
      ↓
FaultManager
      ↓
Control
      ↓
EcuState
```

This session intentionally stopped **before powering the potentiometer or connecting it to the STM32 ADC**.

The objective of this first stage was to understand and electrically characterize the physical component and breadboard before introducing power or firmware.

---

## 2. Safety methodology

The hardware bring-up process being followed is:

1. Identify the component.
2. Verify the component electrically while unpowered.
3. Verify breadboard connectivity.
4. Mount the component.
5. Identify each electrical terminal experimentally.
6. Wire the circuit while unpowered.
7. Verify that no unintended short circuit exists.
8. Apply power.
9. Measure the electrical signal with the multimeter.
10. Connect the signal to the MCU.
11. Implement and test the firmware.

During the measurements documented in this session:

```text
STM32 power       : NOT CONNECTED
ST-LINK           : NOT CONNECTED
USB power         : NOT CONNECTED
ADC input         : NOT CONNECTED
External supply   : NOT CONNECTED
```

All resistance and continuity measurements were therefore performed on an **unpowered circuit**.

---

## 3. Equipment

Hardware used during this session:

- B103 rotary potentiometer
- REMATE DT-9205A digital multimeter
- Solderless breadboard
- Dupont male-to-male wires
- Digital microscope

The STM32F103C8T6 Blue Pill was intentionally not connected during this stage.

---

## 4. Multimeter configuration

For resistance measurements:

```text
Black probe → COM
Red probe   → VΩ
Selector    → 20 kΩ
```

The potentiometer was measured while completely unpowered.

---

## 5. Potentiometer identification

The potentiometer marking was visually inspected using the digital microscope.

The component is marked:

```text
B103
```

The expected nominal resistance is approximately:

```text
10 kΩ
```

Rather than relying only on the component marking, the resistance was verified experimentally.

Measured resistance between the two endpoints:

```text
≈ 10.5 kΩ
```

This is consistent with a nominal 10 kΩ potentiometer.

---

## 6. Potentiometer operating principle

A potentiometer contains a resistive track and a movable contact called the **wiper**.

Conceptually:

```text
Endpoint A                     Endpoint B
    ●──────────────────────────────●
             resistive track
                    ▲
                    │
                  Wiper
```

Rotating the shaft moves the wiper along the resistive track.

Therefore, the resistance between the wiper and each endpoint changes in opposite directions.

---

## 7. Wiper identification

The potentiometer was mounted on the breadboard and its terminals were initially connected to rows:

```text
Row 59
Row 60
Row 61
```

Resistance measurements were then performed while rotating the potentiometer.

### Measurement 1

Between rows 59 and 60:

```text
0 kΩ → 10.45 kΩ
```

### Measurement 2

Between rows 60 and 61:

```text
10.5 kΩ → 0 kΩ
```

The complementary behavior confirms that:

```text
Row 60 = Wiper
```

Therefore:

```text
Row 59 = Endpoint A
Row 60 = Wiper
Row 61 = Endpoint B
```

This experimentally verifies the electrical function of all three potentiometer terminals.

---

## 8. Breadboard characterization

Before connecting the potentiometer to the STM32, the breadboard itself was tested using the multimeter continuity function.

### Central terminal strips

Continuity was detected between holes belonging to the same five-hole strip.

Conceptually:

```text
●──●──●──●──●
```

All five points belong to the same electrical node.

Adjacent numbered rows were verified to be electrically independent.

For example:

```text
Row 5

●──●──●──●──●
      ✓ continuity


Row 6

●──●──●──●──●
      separate node
```

This confirms the expected breadboard internal topology.

---

## 9. Power rail validation

The red and blue breadboard power rails were also tested using continuity mode.

Both rails demonstrated the expected longitudinal electrical connectivity.

Conceptually:

```text
+  ●──●──●──●──●──●──●

-  ●──●──●──●──●──●──●
```

These rails may later be used to distribute:

```text
Red rail  → 3.3 V
Blue rail → GND
```

However, no power was applied during this session.

---

## 10. Breadboard reliability observation

Some holes around approximately rows:

```text
1–20
```

showed intermittent or unreliable electrical contact.

For this reason, that region will not be used for the initial ADC experiment.

The potentiometer was instead mounted around:

```text
59–61
```

where the physical connections were more reliable.

This observation is important because a poor breadboard connection can produce symptoms such as:

- unstable voltage;
- noisy ADC readings;
- apparently missing sensor samples;
- intermittent behavior;
- false assumptions about firmware defects.

Therefore, physical connectivity must be eliminated as a possible fault before debugging the ADC firmware.

---

## 11. Dupont wiring

Male-to-male Dupont wires were connected to the potentiometer rows so that measurements no longer require directly holding the multimeter probes against the potentiometer terminals.

The wire colors were initially arbitrary.

After identifying the wiper, the colors were reorganized to follow a clearer embedded-electronics convention.

### Current confirmed mapping

```text
Row 59 → RED   → Endpoint A
Row 60 → BLUE  → Wiper
Row 61 → BLACK → Endpoint B
```

This is the physical configuration at the end of the session.

---

## 12. Planned voltage-divider configuration

The next step will convert the potentiometer into a voltage divider.

The intended connection is:

```text
             3.3 V
               │
               │
              RED
            Row 59
               │
               │
          ┌────┴────┐
          │  B103   │
          │         │
          └────┬────┘
               │
          BLUE / Wiper
            Row 60
               │
               ├────────→ future ADC signal
               │
          resistive track
               │
              BLACK
            Row 61
               │
               │
              GND
```

The expected wiper voltage will therefore be approximately:

```text
0 V ─────────────── 3.3 V
```

depending on shaft position.

Swapping the two endpoint connections would only reverse the direction in which voltage increases when the potentiometer is rotated.

---

## 13. STM32 ADC target

After validating the voltage divider with the multimeter, the wiper signal is planned to be connected to:

```text
STM32F103C8T6
PA0
ADC1_IN0
```

The STM32F103 ADC is nominally 12-bit.

Therefore, the expected raw conversion range will eventually be approximately:

```text
0 V      → ADC ≈ 0
3.3 V    → ADC ≈ 4095
```

with intermediate voltages producing intermediate ADC values.

For example, conceptually:

```text
Voltage          ADC

0.0 V      →       0
~0.825 V   →    ~1024
~1.65 V    →    ~2048
~2.475 V   →    ~3072
~3.3 V     →    ~4095
```

Actual values will depend on the real supply/reference voltage and ADC characteristics.

---

## 14. Logic analyzer limitation

The Saleae-compatible logic analyzer currently used for STM32 timing validation is a **digital logic analyzer**.

It should therefore not be used to characterize the analog voltage coming directly from the potentiometer.

The analog signal will first be verified using the multimeter.

```text
Potentiometer analog output
            ↓
       Multimeter
            ↓
      voltage verified
            ↓
        STM32 ADC
            ↓
     digital ADC value
```

The logic analyzer can still be used later for digital timing instrumentation if required.

---

## 15. Session results

### PASS — Breadboard characterization

Verified:

- connectivity inside a five-hole terminal strip;
- isolation between adjacent rows;
- power rail continuity;
- unreliable breadboard region identified.

### PASS — Potentiometer identification

Verified:

```text
B103
Nominal ≈ 10 kΩ
Measured ≈ 10.5 kΩ
```

### PASS — Wiper identification

Measured:

```text
59 ↔ 60 : 0 → 10.45 kΩ
60 ↔ 61 : 10.5 → 0 kΩ
```

Therefore:

```text
Row 60 = Wiper
```

### PASS — Physical mounting

Current configuration:

```text
59 → RED   → Endpoint
60 → BLUE  → Wiper
61 → BLACK → Endpoint
```

---

## 16. Not yet performed

The following work has intentionally **not** been performed yet:

- 3.3 V has not been applied to the potentiometer.
- GND has not been connected.
- Wiper voltage has not been measured.
- PA0 has not been connected.
- ADC1 has not been configured.
- No ADC conversion has been executed.
- No ADC driver has been added to the STM32 platform layer.
- No physical sensor value has entered `SignalSample`.
- No physical signal has entered `SignalStore`.
- Fault evaluation has not yet consumed a real ADC signal.

This distinction is important because the current milestone validates only the **passive hardware stage**.

---

## 17. Exact resume point

Do not repeat the potentiometer identification measurements unless the physical configuration changes.

Current known configuration:

```text
B103 on breadboard

Row 59 = RED   = Endpoint A
Row 60 = BLUE  = Wiper
Row 61 = BLACK = Endpoint B
```

The next milestone is:

> **Build and validate a 0–3.3 V potentiometer voltage divider before connecting it to the STM32 ADC.**

The next session should continue in this order:

```text
Current passive B103 setup
          ↓
Connect 3.3 V and GND while unpowered
          ↓
Verify there is no unintended short
          ↓
Power circuit
          ↓
Measure BLUE wiper voltage
          ↓
Rotate potentiometer
          ↓
Verify approximately 0–3.3 V
          ↓
Power down
          ↓
Connect BLUE to PA0 / ADC1_IN0
          ↓
Implement minimal STM32 ADC support
          ↓
Read raw ADC values
          ↓
Validate approximately 0–4095 range
          ↓
Create SignalSample
          ↓
Store in SignalStore
          ↓
FaultManager
          ↓
Control
          ↓
EcuState
```

---

## 18. Target milestone

The first physical-input milestone will be considered complete when a real potentiometer movement produces a value that travels through the actual `myECU` pipeline:

```text
Physical rotation
      ↓
Analog voltage
      ↓
STM32 ADC
      ↓
Digital sample
      ↓
SignalSample
      ↓
SignalStore
      ↓
Fault evaluation
      ↓
Control
      ↓
EcuState
```

At that point, `myECU` will have moved from executing its core on real STM32 hardware to **receiving and processing its first real physical input**.
