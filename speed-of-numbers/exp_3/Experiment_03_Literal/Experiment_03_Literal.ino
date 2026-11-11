const int N = 1000;
const int TRIALS = 3;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 3 - FLOAT LITERAL 0.5f vs 0.5");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.print("Repetitions: ");
  Serial.println(N);

  Serial.println();

  volatile float x = 100.0f;
  volatile float resultFloat = 0.0f;
  volatile float resultDoubleLiteral = 0.0f;

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

    // FLOAT LITERAL: 0.5f
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      resultFloat = x * 0.5f;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x * 0.5f: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    // DOUBLE LITERAL: 0.5
    t0 = ESP.getCycleCount();

    for (int i = 0; i < N; i++) {
      resultDoubleLiteral = x * 0.5;
    }

    t1 = ESP.getCycleCount();
    total = t1 - t0;
    operationCycles = total - baseline;

    Serial.print("Trial ");
    Serial.print(trial);
    Serial.print(" - x * 0.5: ");
    Serial.print((float)operationCycles / N, 2);
    Serial.println(" cycles/op");

    Serial.println();
  }

  Serial.print("Final x * 0.5f result: ");
  Serial.println(resultFloat, 2);

  Serial.print("Final x * 0.5 result: ");
  Serial.println(resultDoubleLiteral, 2);
}

void loop() {
}