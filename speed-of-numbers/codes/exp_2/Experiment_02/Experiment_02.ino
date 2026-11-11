#include <stdint.h>

// EXPERIMENT 2: IS SMALLER FASTER?
const int N = 1000;
const int TRIALS = 3;

void printResult(const char* name, uint32_t baseline, uint32_t total) {
  uint32_t operationCycles = total - baseline;

  Serial.println(name);
  Serial.print("Baseline cycles: ");
  Serial.println(baseline);
  Serial.print("Total cycles: ");
  Serial.println(total);
  Serial.print("Operation cycles: ");
  Serial.println(operationCycles);
  Serial.print("Cycles per operation: ");
  Serial.println((float)operationCycles / N);
  Serial.println();
}

void testAddition() {
  volatile int8_t a8 = 10, b8 = 20, r8;
  volatile int16_t a16 = 10, b16 = 20, r16;
  volatile int32_t a32 = 10, b32 = 20, r32;
  volatile int64_t a64 = 10, b64 = 20, r64;

  uint32_t t0, t1, baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r8 = a8 + b8;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("ADD int8_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r16 = a16 + b16;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("ADD int16_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r32 = a32 + b32;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("ADD int32_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r64 = a64 + b64;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("ADD int64_t", baseline, total);
}

void testMultiplication() {
  volatile int8_t a8 = 3, b8 = 4, r8;
  volatile int16_t a16 = 3, b16 = 4, r16;
  volatile int32_t a32 = 3, b32 = 4, r32;
  volatile int64_t a64 = 3, b64 = 4, r64;

  uint32_t t0, t1, baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r8 = a8 * b8;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("MUL int8_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r16 = a16 * b16;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("MUL int16_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r32 = a32 * b32;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("MUL int32_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r64 = a64 * b64;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("MUL int64_t", baseline, total);
}

void testDivision() {
  volatile int8_t a8 = 100, b8 = 3, r8;
  volatile int16_t a16 = 100, b16 = 3, r16;
  volatile int32_t a32 = 100, b32 = 3, r32;
  volatile int64_t a64 = 100, b64 = 3, r64;
  volatile uint32_t au32 = 100, bu32 = 3, ru32;

  uint32_t t0, t1, baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r8 = a8 / b8;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("DIV int8_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r16 = a16 / b16;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("DIV int16_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r32 = a32 / b32;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("DIV int32_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    r64 = a64 / b64;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("DIV int64_t", baseline, total);

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    ru32 = au32 / bu32;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;
  printResult("DIV uint32_t", baseline, total);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("========================================");
  Serial.println("EXPERIMENT 2 - IS SMALLER FASTER?");
  Serial.println("========================================");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.print("Trials: ");
  Serial.println(TRIALS);

  Serial.println();

  for (int trial = 1; trial <= TRIALS; trial++) {
    Serial.println("----------------------------------------");
    Serial.print("TRIAL ");
    Serial.println(trial);
    Serial.println("----------------------------------------");

    Serial.println("ADDITION");
    testAddition();

    Serial.println("MULTIPLICATION");
    testMultiplication();

    Serial.println("DIVISION");
    testDivision();
  }
}

void loop() {
}