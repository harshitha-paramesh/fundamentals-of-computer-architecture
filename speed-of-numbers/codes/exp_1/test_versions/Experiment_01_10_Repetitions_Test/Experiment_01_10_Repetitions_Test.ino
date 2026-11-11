#include <stdint.h>

const int N = 10;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 1 - 32-bit integer addition");
  Serial.println("TEST: 10 REPETITIONS");
  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  volatile int32_t a = 10;
  volatile int32_t b = 20;

  uint32_t t0, t1;
  uint32_t baseline;
  uint32_t total;
  uint32_t operationCycles;

  t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    asm volatile("");
  }

  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    a = a + b;
  }

  t1 = ESP.getCycleCount();
  total = t1 - t0;

  operationCycles = total - baseline;

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.print("Baseline cycles: ");
  Serial.println(baseline);

  Serial.print("Total cycles: ");
  Serial.println(total);

  Serial.print("Operation cycles: ");
  Serial.println(operationCycles);

  Serial.print("Cycles per addition: ");
  Serial.println((float)operationCycles / N);

  Serial.print("Final a: ");
  Serial.println(a);
}

void loop() {
}