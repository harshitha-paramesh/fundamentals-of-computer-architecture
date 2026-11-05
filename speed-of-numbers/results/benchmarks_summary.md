# The Speed of Numbers — Comprehensive Hardware Benchmarks Summary

All measurements were performed on an physical **ESP32 Dev Module** running at **240 MHz**, using `ESP.getCycleCount()` with baseline loop overhead subtraction.

---

## 1. Experiment 1: 32-Bit Integer Addition & Timing Methodology

| Configuration | Baseline Cycles (1,000 iters) | Total Cycles | Net Operation Cycles | Cycles / Operation | Observation |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Volatile (1,000 iters) — Trial 1** | 4,999 | 13,284 | 8,285 | **8.28 cycles** | Highly repeatable |
| **Volatile (1,000 iters) — Trial 2** | 4,999 | 13,284 | 8,285 | **8.28 cycles** | Zero jitter |
| **Volatile (1,000 iters) — Trial 3** | 4,999 | 13,284 | 8,285 | **8.28 cycles** | Zero jitter |
| **10 Repetitions Test** | 50 | 135 | 85 | **8.50 cycles** | Measurement overhead has higher variance |
| **Non-Volatile Test** | 4,999 | 4,999 | 0 | **0.00 cycles** | Compiler optimizes away dead code |

**Architectural Takeaway:** Benchmarking microsecond operations requires `volatile` variables to prevent dead-code elimination, coupled with baseline empty-loop subtraction to isolate the instruction cost from loop branching.

---

## 2. Experiment 2: Data Type Widths — Is Smaller Faster?

Target: Testing 1,000 arithmetic additions across different integer widths on the 32-bit Xtensa LX6 architecture.

| Data Type | Bit Width | Total Cycles | Net Cycles | Cycles / Operation | Speed Rank |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `int32_t` | 32-bit | 13,284 | 8,285 | **8.28 cycles** | **Fastest (Native ALU width)** |
| `int8_t` | 8-bit | 14,284 | 9,285 | **9.28 cycles** | Slower (+1.0 cycle sign-extension) |
| `int16_t` | 16-bit | 15,179 | 10,180 | **10.18 cycles** | Slower (+1.9 cycles sign-extension) |
| `int64_t` | 64-bit | — | — | > 15 cycles | Multi-word register pairing required |

**Architectural Takeaway:** On a 32-bit RISC architecture (Xtensa LX6), 8-bit and 16-bit variables are **not faster** than 32-bit integers. They require additional sign-extension (`sext`) or bit-masking operations before ALU manipulation. Native 32-bit integers achieve peak efficiency.

---

## 3. Experiment 3: Floating-Point Operations — Hardware FPU vs Software Emulation

Comparison of single-precision `float` (supported by the Xtensa hardware FPU) versus double-precision `double` (emulated in software).

| Arithmetic Operation | Single-Precision `float` (cycles/op) | Double-Precision `double` (cycles/op) | 32-Bit Integer (cycles/op) | Slowdown Factor (`double` vs `float`) |
| :--- | :--- | :--- | :--- | :--- |
| **Addition ($+$)** | **7.01 cycles** | 61.00 cycles | 7.01 cycles | **8.7× Slower** |
| **Multiplication ($\times$)** | **8.01 cycles** | 116.90 cycles | 8.00 cycles | **14.6× Slower** |
| **Division ($\div$)** | **52.00 cycles** | 642.82 cycles | 10.01 cycles | **12.4× Slower** |
| **Square Root ($\sqrt{x}$)** | **88.90 cycles** | 1,094.92 cycles | — | **12.3× Slower** |
| **Literal ($x \times 0.5\text{f}$ vs $0.5$)** | **6.01 cycles** | 6.01 – 6.91 cycles | — | Implicit promotion risk |

**Architectural Takeaway:** The ESP32 Xtensa LX6 core features a single-precision floating-point hardware unit. `float` operations run in single-digit cycles, comparable to integers. `double` requires multi-word software emulation, suffering an **8× to 15× penalty**. Avoid `double` in real-time embedded systems unless high precision is strictly mandatory.

---

## 4. Experiment 4: Bit Tricks vs Arithmetic

Evaluating whether bit-level tricks (shifts and masks) outperform standard division and modulo operators.

| Operation | Arithmetic Operator | Bit-Trick Equivalent | Cycle Cost (Arithmetic) | Cycle Cost (Bit-Trick) | Performance Gain |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Divide by 8** | `x / 8` | `x >> 3` | 8.00 cycles | **6.00 cycles** | **25% Faster** |
| **Modulo 16** | `x % 16` | `x & 15` | 10.00 cycles | **6.00 cycles** | **40% Faster** |
| **Variable Divisor** | `x / divisor` (volatile) | — | 11.29 cycles | — | Full division instruction |

**Architectural Takeaway:** Even with modern optimizing compilers, bit-level shifts and masks avoid signed division edge-case checking (e.g., negative dividend rounding toward zero). When variables are volatile or divisor values are non-constant, bit tricks yield 25%–40% speedups.

---

## 5. Experiment 5: Fixed-Point Arithmetic (Q16.16)

Evaluating a 32-bit signed fixed-point format (16 integer bits, 16 fractional bits, scale factor $2^{16} = 65,536$).

| Property / Operation | Mathematical Definition | Fixed-Point Value | Recovered Float | Absolute Error |
| :--- | :--- | :--- | :--- | :--- |
| **Representation ($+3.75$)** | $3.75 \times 65,536$ | `245760` | `3.75000000` | $0.0$ |
| **Representation ($-1.25$)** | $-1.25 \times 65,536$ | `-81920` | `-1.25000000` | $0.0$ |
| **Dynamic Range** | $[-2^{15}, 2^{15} - 2^{-16}]$ | `[-2147483648, 2147483647]` | `[-32768.0, +32767.99998]` | — |
| **Addition Accuracy** | $2.33 + 1.2501 = 3.5801$ | `234626` | `3.58009934` | $6.6 \times 10^{-7}$ |

**Architectural Takeaway:** Q16.16 delivers near-float precision with pure integer addition and subtraction execution speeds, providing a deterministic alternative for processors lacking hardware FPUs.

---

## 6. Experiment 6: Hidden Cost of Text Conversion & Serial Communication

Evaluating the overhead of converting numeric data to ASCII and transmitting over UART at 115,200 baud.

| Task | Execution Cost (Cycles) | Equivalent Time (@ 240 MHz) | Impact on 10 kHz Loop (Budget: 24,000 cycles) |
| :--- | :--- | :--- | :--- |
| **Integer to Text (`itoa`)** | 1,934 cycles | $8.06\ \mu\text{s}$ | Consumes 8.1% of loop budget |
| **Float to Text (`dtostrf`/`sprintf`)** | **6,030 cycles** | **$25.13\ \mu\text{s}$** | **Consumes 25.1% of loop budget** |
| **UART 1 Byte Transmission (115200 baud)** | **~20,833 cycles** | **$86.8\ \mu\text{s}$** | **Exceeds entire sample period (100 µs)** |

**Architectural Takeaway:** Formatting floating-point numbers to ASCII is hundreds of times slower than arithmetic operations. Direct serial transmission in a high-speed real-time loop will inevitably drop frames and violate hard deadlines unless handled asynchronously via DMA or ring buffers.

---

## 7. Final Challenge: Real-Time Sensor Processing Pipeline

Target: Benchmarking a complete multi-stage sensor filter pipeline across Double, Float, and Integer implementations against strict sample-rate deadlines:
- **10 kHz Target:** 24,000 cycles budget ($100\ \mu\text{s}$)
- **50 kHz Target:** 4,800 cycles budget ($20\ \mu\text{s}$)
- **100 kHz Target:** 2,400 cycles budget ($10\ \mu\text{s}$)

| Implementation | Measured Cycles / Sample | 10 kHz Deadline (24,000 cycles) | 50 kHz Deadline (4,800 cycles) | 100 kHz Deadline (2,400 cycles) |
| :--- | :--- | :--- | :--- | :--- |
| **Integer / Fixed-Point** | **~220 cycles** | ✅ **PASS (99% Headroom)** | ✅ **PASS (95% Headroom)** | ✅ **PASS (91% Headroom)** |
| **Single-Precision `float`** | **~380 cycles** | ✅ **PASS (98% Headroom)** | ✅ **PASS (92% Headroom)** | ✅ **PASS (84% Headroom)** |
| **Double-Precision `double`** | **~3,450 cycles** | ✅ **PASS (85% Headroom)** | ❌ **FAIL (Over budget)** | ❌ **FAIL (Over budget)** |
