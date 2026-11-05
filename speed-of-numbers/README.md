# The Speed of Numbers — Microarchitectural Profiling & Cycle Benchmarking on ESP32

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20Dev%20Board-blue.svg)](https://www.espressif.com/)
[![Environment: Arduino IDE](https://img.shields.io/badge/IDE-Arduino%20IDE-00979D.svg)](https://www.arduino.cc/en/software)
[![Clock: 240 MHz](https://img.shields.io/badge/Clock%20Frequency-240%20MHz-orange.svg)]()
[![Hardware FPU: Single-Precision](https://img.shields.io/badge/FPU-Single--Precision%20Hardware-green.svg)]()

> **Individual Project Implementation**  
> **Course:** Fundamentals of Computer Architecture (FCA)  
> **Author:** Harshitha Paramesh  
> **Hardware:** ESP32 Dev Module (Xtensa Dual-Core 32-bit LX6 Microprocessor @ 240 MHz)  

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Experimental Methodology: Predict → Measure → Explain](#experimental-methodology-predict--measure--explain)
3. [Hardware & Microarchitectural Context](#hardware--microarchitectural-context)
4. [Experiment Breakdown & Key Findings](#experiment-breakdown--key-findings)
   - [Experiment 1: 32-Bit Integer Addition & Stopwatch Calibration](#experiment-1-32-bit-integer-addition--stopwatch-calibration)
   - [Experiment 2: Data Type Widths — Is Smaller Faster?](#experiment-2-data-type-widths--is-smaller-faster)
   - [Experiment 3: Floating-Point Arithmetic — Hardware FPU vs Software Emulation](#experiment-3-floating-point-arithmetic--hardware-fpu-vs-software-emulation)
   - [Experiment 4: Bit Tricks vs Arithmetic Operators](#experiment-4-bit-tricks-vs-arithmetic-operators)
   - [Experiment 5: Fixed-Point Arithmetic (Q16.16)](#experiment-5-fixed-point-arithmetic-q1616)
   - [Experiment 6: Hidden Costs of Formatting & Serial Transmission](#experiment-6-hidden-costs-of-formatting--serial-transmission)
   - [Final Challenge: Real-Time Sensor Processing Pipeline](#final-challenge-real-time-sensor-processing-pipeline)
5. [Summary of Measured Benchmarks](#summary-of-measured-benchmarks)
6. [Repository & Code Structure](#repository--code-structure)
7. [How to Open, Compile, and Run](#how-to-open-compile-and-run)
8. [Core Architectural Lessons](#core-architectural-lessons)
9. [Individual Attribution](#individual-attribution)

---

## Project Overview

**The Speed of Numbers** is an empirical computer architecture research project investigating how data types, numerical representations, compiler optimizations, and peripheral communications impact instruction latency and throughput on the **ESP32 microcontroller**.

By using high-resolution hardware cycle counters (`ESP.getCycleCount()`), this project rigorously quantifies:
- The clock cycle cost of integer arithmetic across various bit widths (`int8_t` through `int64_t`).
- The performance disparity between **hardware-accelerated single-precision floating-point** (`float`) and **software-emulated double-precision** (`double`).
- When and why bitwise manipulations outperform standard arithmetic operators.
- The precision-speed trade-off of **Q16.16 fixed-point arithmetic**.
- The microarchitectural bottlenecks of string formatting and UART serial streaming in real-time deadlines.
- Real-time sensor pipeline performance under strict sampling deadlines ($10\text{ kHz}$, $50\text{ kHz}$, $100\text{ kHz}$).

---

## Experimental Methodology: Predict → Measure → Explain

Each experiment adheres to a scientific evaluation framework:
1. **Predict:** Formulate a hypothesis based on processor architecture, instruction set reference, and compiler heuristics.
2. **Measure:** Collect empirical hardware timing using cycle-accurate hardware counters on the ESP32 Dev Module. Empty-loop baselines are subtracted to isolate instruction costs.
3. **Explain:** Analyze deviations between prediction and reality using compiler assembly output, pipeline characteristics, and hardware register architectures.

```mermaid
flowchart LR
    A["Predict\nHypothesis from ISA / Architecture"] 
    --> B["Measure\nHardware Cycle Counting (ESP32)"] 
    --> C["Explain\nAssembly & Microarchitectural Analysis"]
```

---

## Hardware & Microarchitectural Context

- **Processor:** Espressif ESP32 Dev Module
- **Core:** Dual-Core 32-bit Xtensa LX6 @ 240 MHz (1 cycle = $4.167\text{ ns}$)
- **Floating-Point Support:** Single-Precision Hardware Floating-Point Unit (FPU) compliant with IEEE 754-2008.
- **Double-Precision:** No 64-bit hardware FPU; operations are linked to software emulation routines (`__adddf3`, `__muldf3`, `__divdf3`).
- **Register Architecture:** 32-bit general-purpose registers (`a0`–`a15`).

---

## Experiment Breakdown & Key Findings

### Experiment 1: 32-Bit Integer Addition & Stopwatch Calibration
- **Aim:** Build a trustworthy cycle-counting benchmark and determine the cost of a 32-bit integer addition.
- **Method:** Evaluated 1,000 repetitions vs. 10 repetitions, with volatile vs. non-volatile qualifiers. Empty-loop baseline (4,999 cycles) was subtracted.
- **Results:**
  - Volatile addition cost: **8.28 cycles / addition** across 3 repeatable trials.
  - 10 repetitions test: Higher variance due to small loop sample size.
  - Non-volatile test: Measured **0.00 cycles** because the compiler detected dead code and removed the calculation entirely.
- **Insight:** Proves that microbenchmarking requires `volatile` variables to prevent compiler elimination and sufficient iterations to dilute loop overhead.

---

### Experiment 2: Data Type Widths — Is Smaller Faster?
- **Aim:** Determine whether smaller integer types (`int8_t`, `int16_t`) execute faster than native `int32_t`.
- **Results:**
  - `int32_t`: **8.28 cycles / op** (Fastest)
  - `int8_t`: **9.28 cycles / op** (+1.0 cycle penalty)
  - `int16_t`: **10.18 cycles / op** (+1.9 cycle penalty)
- **Insight:** On a 32-bit RISC core, smaller types are **slower**, not faster. The ALU operates natively on 32-bit words; operations on 8-bit and 16-bit variables require explicit sign-extension (`sext`) or mask instructions to maintain standard C integer promotion semantics.

---

### Experiment 3: Floating-Point Arithmetic — Hardware FPU vs Software Emulation
- **Aim:** Quantify the execution penalty of double-precision (`double`) over hardware-accelerated single-precision (`float`).
- **Results:**
  - **Addition:** Float = **7.01 cycles** vs. Double = **61.00 cycles** (**8.7× slower**)
  - **Multiplication:** Float = **8.01 cycles** vs. Double = **116.90 cycles** (**14.6× slower**)
  - **Division:** Float = **52.00 cycles** vs. Double = **642.82 cycles** (**12.4× slower**)
  - **Square Root:** Float = **88.90 cycles** vs. Double = **1,094.92 cycles** (**12.3× slower**)
  - **Literal Test:** `x * 0.5f` (single-precision) runs at 6.01 cycles; unadorned `x * 0.5` causes implicit double promotion.
- **Insight:** The hardware FPU processes single-precision numbers in single-digit cycles. Double-precision arithmetic triggers software emulation libraries that take hundreds to thousands of cycles.

---

### Experiment 4: Bit Tricks vs Arithmetic Operators
- **Aim:** Test whether replacing division and modulo with shifts and bitwise masks improves throughput.
- **Results:**
  - `x / 8` (**8.00 cycles**) vs. `x >> 3` (**6.00 cycles**) $\rightarrow$ **25% performance improvement**.
  - `x % 16` (**10.00 cycles**) vs. `x & 15` (**6.00 cycles**) $\rightarrow$ **40% performance improvement**.
  - Variable divisor: Non-constant division consumes **11.29 cycles**.
- **Insight:** Signed division in C rounds toward zero, which requires extra conditional adjustments for negative dividends in software. Bitwise shifts avoid this overhead when dealing with non-negative numbers.

---

### Experiment 5: Fixed-Point Arithmetic (Q16.16)
- **Aim:** Implement and benchmark Q16.16 fixed-point arithmetic (16-bit integer, 16-bit fractional component) as an alternative to floating point.
- **Results:**
  - Addition and subtraction run at raw integer speeds.
  - Scale factor: $2^{16} = 65,536$.
  - Dynamic range: $[-32768.0, +32767.99998]$.
  - Addition accuracy test ($2.33 + 1.2501 = 3.5801$): Absolute error was only $6.6 \times 10^{-7}$.
- **Insight:** Fixed-point representation provides near-float precision with integer instruction latencies, ideal for DSP loops and processors without dedicated FPUs.

---

### Experiment 6: Hidden Costs of Formatting & Serial Transmission
- **Aim:** Measure the microarchitectural cost of number-to-string conversion (`itoa`, `dtostrf`) and UART transmission.
- **Results:**
  - Integer conversion to ASCII (`itoa`): **1,934 cycles** ($8.06\ \mu\text{s}$).
  - Float conversion to ASCII (`dtostrf`): **6,030 cycles** ($25.13\ \mu\text{s}$).
  - Transmitting 1 character over UART @ 115,200 baud: **~20,833 cycles** ($86.8\ \mu\text{s}$).
- **Insight:** Printing numbers is up to **1,000× more expensive** than computing them. A single print statement in a high-frequency control loop will cause missed deadlines.

---

### Final Challenge: Real-Time Sensor Processing Pipeline
- **Aim:** Evaluate a full multi-stage sensor filter pipeline across Double, Float, and Integer implementations against strict sample-rate budgets:
  - **10 kHz Target:** 24,000 cycles budget ($100\ \mu\text{s}$)
  - **50 kHz Target:** 4,800 cycles budget ($20\ \mu\text{s}$)
  - **100 kHz Target:** 2,400 cycles budget ($10\ \mu\text{s}$)
- **Results:**
  - **Integer/Fixed-Point Pipeline:** **~220 cycles** $\rightarrow$ Meets all deadlines with >90% headroom.
  - **Single-Precision Float Pipeline:** **~380 cycles** $\rightarrow$ Meets all deadlines with >84% headroom.
  - **Double-Precision Pipeline:** **~3,450 cycles** $\rightarrow$ Meets 10 kHz, but **fails 50 kHz and 100 kHz deadlines**.

---

## Summary of Measured Benchmarks

| Experiment | Target Operation | Measured Metric | Comparison / Baseline | Microarchitectural Verdict |
| :--- | :--- | :--- | :--- | :--- |
| **Exp 1** | 32-bit Integer Add | **8.28 cycles** | 0.00 cycles (non-volatile) | Volatile prevents compiler removal |
| **Exp 2** | `int8` vs `int16` vs `int32` | **8.28 cycles (`int32`)** | 9.28 (`int8`), 10.18 (`int16`) | Native 32-bit ALU is fastest |
| **Exp 3** | Float vs Double Add | **7.01 cycles (`float`)** | 61.00 cycles (`double`) | Hardware FPU is 8.7× faster |
| **Exp 3** | Float vs Double Mult | **8.01 cycles (`float`)** | 116.90 cycles (`double`) | Hardware FPU is 14.6× faster |
| **Exp 3** | Float vs Double Div | **52.00 cycles (`float`)** | 642.82 cycles (`double`) | Hardware FPU is 12.4× faster |
| **Exp 3** | Float vs Double Sqrt | **88.90 cycles (`float`)** | 1,094.92 cycles (`double`) | Hardware FPU is 12.3× faster |
| **Exp 4** | Divide vs Shift | **6.00 cycles (`>> 3`)** | 8.00 cycles (`/ 8`) | Bit shift is 25% faster |
| **Exp 4** | Modulo vs Mask | **6.00 cycles (`& 15`)** | 10.00 cycles (`% 16`) | Bit mask is 40% faster |
| **Exp 5** | Q16.16 Fixed-Point | **Integer latency** | Error: $6.6 \times 10^{-7}$ | Float-grade precision at integer speed |
| **Exp 6** | Float to String | **6,030 cycles** | 1,934 cycles (`int` to string) | Text formatting is up to 1,000× cost |
| **Exp 6** | UART 1 Byte (115.2k) | **20,833 cycles** | 86.8 µs transmission time | Never print in high-frequency loops |
| **Final** | 100 kHz Pipeline | **~380 cycles (`float`)** | 3,450 cycles (`double`) | Double fails real-time deadline |

---

## Repository & Code Structure

```text
speed-of-numbers/
├── README.md                                          # Master documentation for Speed of Numbers
│
├── code/                                              # Runnable Arduino sketches
│   ├── exp_1/
│   │   ├── Experiment_01/                             # Benchmark stopwatch & 32-bit addition
│   │   │   └── Experiment_01.ino
│   │   └── test_versions/                             # 10 repetitions, volatile & non-volatile tests
│   │       ├── Experiment_01_10_Repetitions_Test/
│   │       ├── Experiment_01_NonVolatile_Test/
│   │       └── Experiment_01_Volatile_Test/
│   ├── exp_2/
│   │   └── Experiment_02/                             # Data type width scaling (int8, int16, int32, int64)
│   │       └── Experiment_02.ino
│   ├── exp_3/                                         # Float vs Double operations
│   │   ├── Experiment_03_Addition/
│   │   ├── Experiment_03_Division/
│   │   ├── Experiment_03_Literal/
│   │   ├── Experiment_03_Multiplication/
│   │   └── Experiment_03_Sqrt/
│   ├── exp_4/                                         # Bit tricks vs arithmetic operators
│   │   ├── Experiment_04_Bit_Tricks/
│   │   └── assembly.txt                               # Xtensa LX6 disassembly analysis
│   ├── exp_5/
│   │   └── Experiment_05/                             # Q16.16 fixed-point implementation
│   │       └── Experiment_05.ino
│   ├── exp_6/
│   │   └── Experiment_06/                             # Hidden cost of formatting & Serial UART
│   │       └── Experiment_06.ino
│   └── final_challenge/                               # Real-time sensor processing pipeline
│       ├── Final_challenge/
│       └── final_challenge_corrected/
│
├── raw_output/                                        # Raw Serial Monitor captures from ESP32
│   ├── Experiment_01_Text Document.txt
│   ├── Experiment_02_Text Document.txt
│   ├── Experiment_04_Text Document.txt
│   ├── Experiment_05_Text Document.txt
│   ├── Experiment_06_Text Document.txt
│   ├── final_challenge_Text Document.txt
│   ├── final_challenge_corrected_Text Document.txt
│   └── exp_3_floating_point/
│       ├── EXPERIMENT 3 - ADDITION_Text Document.txt
│       ├── Experiment_03_division_Text Document.txt
│       ├── Experiment_03_lineral_Text Document.txt
│       ├── Experiment_03_multiplication_Text Document.txt
│       └── Experiment_03_sqrt_Text Document.txt
│
├── reports/                                           # Academic reports for all experiments
│   ├── Master_Speed_of_Numbers_Report.docx
│   ├── Experiment_01_Report.docx
│   ├── Experiment_02_Report.docx
│   ├── Experiment_03_Report.docx
│   ├── Experiment_04_Report.docx
│   ├── Experiment_05_Report.docx
│   ├── Experiment_06_Report.docx
│   └── Final_Challenge_Report.docx
│
└── results/                                           # Consolidated benchmark findings
    └── benchmarks_summary.md
```

---

## How to Open, Compile, and Run

1. **Hardware Connection:** Connect the ESP32 Dev Board to your PC via micro-USB.
2. **Open Sketch:** In Arduino IDE, open any experiment sketch (e.g., `code/exp_3/Experiment_03_Addition/Experiment_03_Addition.ino`).
3. **Configure Board:**
   - **Board:** `ESP32 Dev Module`
   - **CPU Frequency:** `240MHz (WiFi/BT)`
   - **Flash Frequency:** `80MHz`
   - **Upload Speed:** `921600` (or `115200`)
   - **Port:** Select the active COM port.
4. **Upload:** Click **Upload** ($\rightarrow$).
5. **Read Serial Output:** Open Serial Monitor at **`115200 baud`** and press the **EN / RST** button on the ESP32 to view cycle benchmarks.

---

## Core Architectural Lessons

1. **Hardware FPU Priority:** Always prefer single-precision `float` on microcontrollers with 32-bit FPUs. Double-precision arithmetic incurs an 8× to 15× latency penalty due to software emulation.
2. **Native Word Width Advantage:** Small types (`int8_t`, `int16_t`) do not save execution time on 32-bit architectures; they often introduce sign-extension overhead.
3. **Bit Manipulation Efficacy:** Bitwise shifts and masks remain significantly faster than signed division and modulo, avoiding compiler-generated branch logic.
4. **Zero-Overhead Fixed Point:** Q16.16 fixed-point arithmetic offers an effective alternative when processors lack FPUs, preserving real-time performance.
5. **Decouple I/O from Real-Time Loops:** ASCII string formatting (`sprintf`) and UART printing must be moved outside time-critical interrupt routines or handled asynchronously.

---

## Individual Attribution

This assignment is an **individual academic implementation** researched, coded, and benchmarked by:

**Harshitha Paramesh**  
Course: *Fundamentals of Computer Architecture (FCA)*  
Platform: ESP32 Dev Board | Arduino IDE
