#include <Arduino.h>
#include <math.h>

const int N = 1000;
const int TRIALS = 3;
const int WINDOW = 8;
const int ADC_PIN = 34;

volatile double doubleSink;
volatile float floatSink;
volatile int32_t intSink;
volatile int analogSink;

uint16_t testRaw[N];

uint32_t measureBaseline() {
  uint32_t t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    asm volatile("");
  }

  uint32_t t1 = ESP.getCycleCount();
  return t1 - t0;
}

uint32_t measureAnalogRead(uint32_t baseline) {
  uint32_t t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    analogSink = analogRead(ADC_PIN);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0 - baseline) / N;
}

uint32_t measureDouble(uint32_t baseline) {
  uint32_t t0 = ESP.getCycleCount();

  double sum = 0.0;
  double output = 0.0;

  for (int i = 0; i < N; i++) {
    double mV = (double)testRaw[i] * 3300.0 / 4095.0;
    sum += mV;

    if (i >= WINDOW) {
      double oldmV =
        (double)testRaw[i - WINDOW] * 3300.0 / 4095.0;
      sum -= oldmV;
    }

    int count = (i + 1 < WINDOW) ? (i + 1) : WINDOW;
    output = sum / (double)count;
  }

  doubleSink = output;

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0 - baseline) / N;
}

uint32_t measureFloat(uint32_t baseline) {
  uint32_t t0 = ESP.getCycleCount();

  float sum = 0.0f;
  float output = 0.0f;

  for (int i = 0; i < N; i++) {
    float mV = (float)testRaw[i] * 3300.0f / 4095.0f;
    sum += mV;

    if (i >= WINDOW) {
      float oldmV =
        (float)testRaw[i - WINDOW] * 3300.0f / 4095.0f;
      sum -= oldmV;
    }

    int count = (i + 1 < WINDOW) ? (i + 1) : WINDOW;
    output = sum / (float)count;
  }

  floatSink = output;

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0 - baseline) / N;
}

uint32_t measureInteger(uint32_t baseline) {
  uint32_t t0 = ESP.getCycleCount();

  int32_t sum = 0;
  int32_t output = 0;

  for (int i = 0; i < N; i++) {
    int32_t mV =
      ((int32_t)testRaw[i] * 3300) / 4095;

    sum += mV;

    if (i >= WINDOW) {
      int32_t oldmV =
        ((int32_t)testRaw[i - WINDOW] * 3300) / 4095;
      sum -= oldmV;
    }

    int count = (i + 1 < WINDOW) ? (i + 1) : WINDOW;
    output = sum / count;
  }

  intSink = output;

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0 - baseline) / N;
}

void generateTestData() {
  for (int i = 0; i < N; i++) {
    testRaw[i] = (i * 37 + 123) % 4096;
  }
}

void exhaustiveErrorTest() {
  double maxFloatError = 0.0;
  double maxIntegerError = 0.0;

  int maxFloatRaw = 0;
  int maxIntegerRaw = 0;

  for (int raw = 0; raw <= 4095; raw++) {

    double trueValue =
      (double)raw * 3300.0 / 4095.0;

    float floatValue =
      (float)raw * 3300.0f / 4095.0f;

    int32_t integerValue =
      ((int32_t)raw * 3300) / 4095;

    double floatError =
      fabs((double)floatValue - trueValue);

    double integerError =
      fabs((double)integerValue - trueValue);

    if (floatError > maxFloatError) {
      maxFloatError = floatError;
      maxFloatRaw = raw;
    }

    if (integerError > maxIntegerError) {
      maxIntegerError = integerError;
      maxIntegerRaw = raw;
    }
  }

  Serial.println();
  Serial.println("EXHAUSTIVE ERROR TEST");

  Serial.print("Maximum float error (mV): ");
  Serial.println(maxFloatError, 9);

  Serial.print("Raw value at maximum float error: ");
  Serial.println(maxFloatRaw);

  Serial.print("Maximum integer error (mV): ");
  Serial.println(maxIntegerError, 9);

  Serial.print("Raw value at maximum integer error: ");
  Serial.println(maxIntegerRaw);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("FINAL CHALLENGE - CORRECTED TIMING");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  double cycles10k =
    240000000.0 / 10000.0;

  double cycles50k =
    240000000.0 / 50000.0;

  double cycles100k =
    240000000.0 / 100000.0;

  Serial.println();
  Serial.println("SAMPLE RATE BUDGET");

  Serial.print("Cycles available at 10 kHz: ");
  Serial.println(cycles10k, 2);

  Serial.print("Cycles available at 50 kHz: ");
  Serial.println(cycles50k, 2);

  Serial.print("Cycles available at 100 kHz: ");
  Serial.println(cycles100k, 2);

  generateTestData();

  Serial.println();
  Serial.println("CORRECTED TIMING");

  Serial.print("Repetitions per trial: ");
  Serial.println(N);

  Serial.print("Trials: ");
  Serial.println(TRIALS);

  uint32_t baseline[TRIALS];
  uint32_t analogCycles[TRIALS];
  uint32_t doubleCycles[TRIALS];
  uint32_t floatCycles[TRIALS];
  uint32_t integerCycles[TRIALS];

  for (int trial = 0; trial < TRIALS; trial++) {

    baseline[trial] = measureBaseline();

    analogCycles[trial] =
      measureAnalogRead(baseline[trial]);

    doubleCycles[trial] =
      measureDouble(baseline[trial]);

    floatCycles[trial] =
      measureFloat(baseline[trial]);

    integerCycles[trial] =
      measureInteger(baseline[trial]);
  }

  Serial.println();
  Serial.println("EMPTY LOOP BASELINE");

  for (int trial = 0; trial < TRIALS; trial++) {
    Serial.print("Trial ");
    Serial.print(trial + 1);
    Serial.print(": ");
    Serial.println(baseline[trial]);
  }

  Serial.println();
  Serial.println("ANALOG READ CYCLES/SAMPLE");

  for (int trial = 0; trial < TRIALS; trial++) {
    Serial.print("Trial ");
    Serial.print(trial + 1);
    Serial.print(": ");
    Serial.println(analogCycles[trial]);
  }

  Serial.println();
  Serial.println("DOUBLE PIPELINE CYCLES/SAMPLE");

  for (int trial = 0; trial < TRIALS; trial++) {
    Serial.print("Trial ");
    Serial.print(trial + 1);
    Serial.print(": ");
    Serial.println(doubleCycles[trial]);
  }

  Serial.println();
  Serial.println("FLOAT PIPELINE CYCLES/SAMPLE");

  for (int trial = 0; trial < TRIALS; trial++) {
    Serial.print("Trial ");
    Serial.print(trial + 1);
    Serial.print(": ");
    Serial.println(floatCycles[trial]);
  }

  Serial.println();
  Serial.println("INTEGER PIPELINE CYCLES/SAMPLE");

  for (int trial = 0; trial < TRIALS; trial++) {
    Serial.print("Trial ");
    Serial.print(trial + 1);
    Serial.print(": ");
    Serial.println(integerCycles[trial]);
  }

  double analogAverage = 0;
  double doubleAverage = 0;
  double floatAverage = 0;
  double integerAverage = 0;

  for (int trial = 0; trial < TRIALS; trial++) {
    analogAverage += analogCycles[trial];
    doubleAverage += doubleCycles[trial];
    floatAverage += floatCycles[trial];
    integerAverage += integerCycles[trial];
  }

  analogAverage /= TRIALS;
  doubleAverage /= TRIALS;
  floatAverage /= TRIALS;
  integerAverage /= TRIALS;

  Serial.println();
  Serial.println("AVERAGE CORRECTED TIMING");

  Serial.print("Average analogRead cycles/sample: ");
  Serial.println(analogAverage, 2);

  Serial.print("Average double pipeline cycles/sample: ");
  Serial.println(doubleAverage, 2);

  Serial.print("Average float pipeline cycles/sample: ");
  Serial.println(floatAverage, 2);

  Serial.print("Average integer pipeline cycles/sample: ");
  Serial.println(integerAverage, 2);

  Serial.println();
  Serial.println("FINAL OUTPUT CHECK");

  Serial.print("Double final moving average: ");
  Serial.println(doubleSink, 9);

  Serial.print("Float final moving average: ");
  Serial.println(floatSink, 9);

  Serial.print("Integer final moving average: ");
  Serial.println(intSink);

  exhaustiveErrorTest();

  double completeDouble =
    analogAverage + doubleAverage;

  double completeFloat =
    analogAverage + floatAverage;

  double completeInteger =
    analogAverage + integerAverage;

  Serial.println();
  Serial.println("COMPLETE SENSOR WORKLOAD");

  Serial.print("Double total cycles/sample: ");
  Serial.println(completeDouble, 2);

  Serial.print("Float total cycles/sample: ");
  Serial.println(completeFloat, 2);

  Serial.print("Integer total cycles/sample: ");
  Serial.println(completeInteger, 2);

  Serial.println();
  Serial.println("LATENCY CHECK - COMPLETE WORKLOAD");

  Serial.print("Double fits 10 kHz: ");
  Serial.println(completeDouble < cycles10k ? "YES" : "NO");

  Serial.print("Float fits 10 kHz: ");
  Serial.println(completeFloat < cycles10k ? "YES" : "NO");

  Serial.print("Integer fits 10 kHz: ");
  Serial.println(completeInteger < cycles10k ? "YES" : "NO");

  Serial.print("Double fits 50 kHz: ");
  Serial.println(completeDouble < cycles50k ? "YES" : "NO");

  Serial.print("Float fits 50 kHz: ");
  Serial.println(completeFloat < cycles50k ? "YES" : "NO");

  Serial.print("Integer fits 50 kHz: ");
  Serial.println(completeInteger < cycles50k ? "YES" : "NO");

  Serial.print("Double fits 100 kHz: ");
  Serial.println(completeDouble < cycles100k ? "YES" : "NO");

  Serial.print("Float fits 100 kHz: ");
  Serial.println(completeFloat < cycles100k ? "YES" : "NO");

  Serial.print("Integer fits 100 kHz: ");
  Serial.println(completeInteger < cycles100k ? "YES" : "NO");

  Serial.println();
  Serial.println("END FINAL CHALLENGE");
}

void loop() {}