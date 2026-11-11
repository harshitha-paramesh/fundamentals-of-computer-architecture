#include <stdint.h>

const int N = 1000;
const int TRIALS = 3;

volatile int32_t x = 1000;
volatile int32_t result = 0;

volatile int32_t divisor8 = 8;
volatile int32_t divisor16 = 16;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 4 - ARE BIT TRICKS REALLY FASTER?");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.println();

  for (int trial = 1; trial <= TRIALS; trial++) {

    uint32_t t0, t1;
    uint32_t baseline;
    uint32_t total;
    uint32_t operationCycles;

    // Measure loop overhead
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      asm volatile("");
    }

    t1 = ESP.getCycleCount();
    baseline = t1 - t0;

    // x / 8
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x / 8;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x / 8: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // x >> 3
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x >> 3;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x >> 3: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // x % 16
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x % 16;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x % 16: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // x & 15
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x & 15;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x & 15: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // x / volatile 8
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x / divisor8;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x / volatile 8: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // x % volatile 16
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      result = x % divisor16;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x % volatile 16: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    Serial.println();
  }

  // Negative-number comparison
  int32_t negative = -9;

  Serial.println("NEGATIVE NUMBER CHECK");

  Serial.print("-9 / 8 = ");
  Serial.println(negative / 8);

  Serial.print("-9 >> 3 = ");
  Serial.println(negative >> 3);

  Serial.print("-9 in binary = ");
  Serial.println(negative, BIN);

  Serial.println();

  Serial.print("Final result = ");
  Serial.println(result);
}

void loop() {
}