# Catch the Rocket — Verified Hardware Results

This document records the exact physical test measurements and performance benchmarks obtained on the **ESP32 Dev Module** hardware during the final Level 5 evaluation.

All measurements reflect streaming execution over 100,000 simulated altitude telemetry readings generated at 1 reading/ms (1 kHz rate).

---

## 1. Summary of Flight Telemetry & Performance

| Parameter | Value | Constraint / Budget | Margin / Status |
| :--- | :--- | :--- | :--- |
| **Total Stream Readings** | 100,000 | 100,000 | 100% Processed |
| **Valid Altitude Readings** | 99,998 | — | Clean signal extraction |
| **Detected Glitches** | 2 | $\le 1\%$ ($\le 1,000$) | Exactly filtered online |
| **Coast Phase Window** | 8,000 readings ($0.000\text{ s} - 8.000\text{ s}$) | $\ge 8\text{ s}$ guaranteed coast | Used for quadratic least-squares |
| **Descent Window** | 55,000 readings ($45.000\text{ s} - 99.999\text{ s}$) | Post-deployment steady state | Used for linear least-squares |
| **Persistent State Size** | **248 bytes** | $\le 4,096\text{ bytes}$ (4 KB) | **3,848 bytes margin (94% headroom)** |
| **Worst-Case Reading Latency** | **10,394 cycles** | $\le 12,000\text{ cycles}$ | **1,606 cycles margin (PASS)** |
| **Worst-Case Reading Index** | Index 0 | — | Occurs during startup / state zeroing |
| **Last Command Latency** | 368 cycles | — | Instantaneous response |
| **Worst Command Latency** | 8,051 cycles | — | Sub-millisecond response |

---

## 2. Command Evaluation Results (Adversarial Tracker Run)

The following commands were evaluated via the Serial Monitor interface at 115,200 baud on COM5. Latencies reflect pure algorithmic execution time measured via `ESP.getCycleCount()` excluding serial print overhead.

| Command | Serial Monitor Output | Execution Latency | Physical Meaning |
| :--- | :--- | :--- | :--- |
| `STATS` | `Total readings - 100000`<br>`Valid readings - 99998`<br>`Coast readings - 8000`<br>`Descent readings - 55000`<br>`Glitches - 2`<br>`State size - 248 bytes`<br>`Worst reading cycles - 10394`<br>`Worst reading index - 0`<br>`Cycle budget - 12000` | 1,215 cycles | Overall streaming telemetry statistics |
| `G` | `2.0930 m/s^2` | 1,549 cycles | Calculated local gravitational acceleration ($-2c$) |
| `APOGEE` | `time - 2.3730 s, height - 1005.9510 m` | 706 cycles | Maximum altitude achieved during coast phase |
| `DEPLOY` | `8.1980 s` | 1,972 cycles | Parachute deployment timestamp (residual departure) |
| `DESCENT` | `-2.0005 m/s` | 201 cycles | Terminal velocity during parachute descent |
| `LAND` | `493.3004 s` | 205 cycles | Estimated rocket touchdown time ($h = 0$) |
| `ALT 20.0000` | `946.8334 m` | 4,571 cycles | Reconstructed altitude at $t = 20.0\text{ s}$ |
| `ALT 40.0000` | `906.8236 m` | 1,457 cycles | Reconstructed altitude at $t = 40.0\text{ s}$ |
| `ALT 50.0000` | `886.8187 m` | 1,458 cycles | Reconstructed altitude at $t = 50.0\text{ s}$ |
| `GLITCHES` | `2` | 133 cycles | Count of detected corrupt data samples |

---

## 3. Hardware Test Evidence

The final verification was captured on the Arduino Serial Monitor connected to the physical ESP32 Dev Board.

- **Serial Port:** COM5
- **Baud Rate:** 115200
- **Screenshot Artifact:** `Final_Level5_SerialMonitor.png` (also viewable in this directory)
