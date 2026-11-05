#include <stdint.h>

const int N = 1000;
const int TRIALS = 3;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 3 - FLOAT vs DOUBLE - ADDITION");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.println();

  volatile float floatA = 10.5f;
  volatile float floatB = 20.5f;
  volatile float floatResult = 0.0f;

  volatile double doubleA = 10.5;
  volatile double doubleB = 20.5;
  volatile double doubleResult = 0.0;

  volatile int32_t intA = 10;
  volatile int32_t intB = 20;
  volatile int32_t intResult = 0;

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

    // FLOAT ADDITION
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      floatResult = floatA + floatB;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - Float addition: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // DOUBLE ADDITION
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      doubleResult = doubleA + doubleB;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - Double addition: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // INT32 ADDITION
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      intResult = intA + intB;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - Int32 addition: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    Serial.println();
  }

  Serial.print("Final float result: ");
  Serial.println(floatResult, 2);

  Serial.print("Final double result: ");
  Serial.println(doubleResult, 2);

  Serial.print("Final int32 result: ");
  Serial.println(intResult);
}

void loop() {
}