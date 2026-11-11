#include <math.h>

const int N = 1000;
const int TRIALS = 3;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 3 - FLOAT vs DOUBLE - SQRT");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.println();

  volatile float floatA = 123.456f;
  volatile float floatResult = 0.0f;

  volatile double doubleA = 123.456;
  volatile double doubleResult = 0.0;

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

    // FLOAT SQRT
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      floatResult = sqrtf(floatA);
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - Float sqrt: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // DOUBLE SQRT
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      doubleResult = sqrt(doubleA);
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - Double sqrt: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    Serial.println();
  }

  Serial.print("Final float result: ");
  Serial.println(floatResult, 6);

  Serial.print("Final double result: ");
  Serial.println(doubleResult, 6);
}

void loop() {
}