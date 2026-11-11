#include <Arduino.h>
#include <math.h>

const int N = 1000;
const int TRIALS = 3;

const int ADC_PIN = 34;
const int ADC_MAX = 4095;
const int ADC_MV = 3300;

const int WINDOW = 8;

int samples[N];

uint32_t measureBaseline() {
  uint32_t t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    asm volatile("");
  }

  uint32_t t1 = ESP.getCycleCount();
  return t1 - t0;
}

uint32_t measureAnalogRead() {
  uint32_t t0 = ESP.getCycleCount();

  volatile int value;

  for (int i = 0; i < N; i++) {
    value = analogRead(ADC_PIN);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0);
}

uint32_t measureDoublePipeline() {
  uint32_t t0 = ESP.getCycleCount();

  volatile double result = 0.0;
  double sum = 0.0;

  for (int i = 0; i < N; i++) {
    int raw = samples[i];

    double mv = (double)raw * 3300.0 / 4095.0;

    sum += mv;

    if (i >= WINDOW) {
      sum -= (double)samples[i - WINDOW] * 3300.0 / 4095.0;
    }

    result = sum / (double)((i + 1 < WINDOW) ? (i + 1) : WINDOW);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0);
}

uint32_t measureFloatPipeline() {
  uint32_t t0 = ESP.getCycleCount();

  volatile float result = 0.0f;
  float sum = 0.0f;

  for (int i = 0; i < N; i++) {
    int raw = samples[i];

    float mv = (float)raw * 3300.0f / 4095.0f;

    sum += mv;

    if (i >= WINDOW) {
      sum -= (float)samples[i - WINDOW] * 3300.0f / 4095.0f;
    }

    result = sum / (float)((i + 1 < WINDOW) ? (i + 1) : WINDOW);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0);
}

uint32_t measureIntegerPipeline() {
  uint32_t t0 = ESP.getCycleCount();

  volatile int32_t result = 0;
  int32_t sum = 0;

  for (int i = 0; i < N; i++) {
    int32_t raw = samples[i];

    int32_t mv = (raw * 3300L) / 4095L;

    sum += mv;

    if (i >= WINDOW) {
      sum -= (samples[i - WINDOW] * 3300L) / 4095L;
    }

    result = sum / ((i + 1 < WINDOW) ? (i + 1) : WINDOW);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0);
}

double getDoubleAverage() {
  double sum = 0.0;
  int count = (N < WINDOW) ? N : WINDOW;

  for (int i = N - count; i < N; i++) {
    sum += (double)samples[i] * 3300.0 / 4095.0;
  }

  return sum / count;
}

float getFloatAverage() {
  float sum = 0.0f;
  int count = (N < WINDOW) ? N : WINDOW;

  for (int i = N - count; i < N; i++) {
    sum += (float)samples[i] * 3300.0f / 4095.0f;
  }

  return sum / count;
}

int32_t getIntegerAverage() {
  int32_t sum = 0;
  int count = (N < WINDOW) ? N : WINDOW;

  for (int i = N - count; i < N; i++) {
    sum += (samples[i] * 3300L) / 4095L;
  }

  return sum / count;
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
      (raw * 3300L) / 4095L;

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

  Serial.print("Maximum float error (mV) ");
  Serial.println(maxFloatError, 9);

  Serial.print("Raw value at maximum float error ");
  Serial.println(maxFloatRaw);

  Serial.print("Maximum integer error (mV) ");
  Serial.println(maxIntegerError, 9);

  Serial.print("Raw value at maximum integer error ");
  Serial.println(maxIntegerRaw);
}

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("FINAL CHALLENGE - CORRECTED TIMING");

  Serial.print("CPU Frequency ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  for (int i = 0; i < N; i++) {
    samples[i] = i % 4096;
  }

  double cyclesPerSecond = 240000000.0;

  Serial.println();
  Serial.println("SAMPLE RATE BUDGET");

  Serial.print("Cycles available at 10 kHz ");
  Serial.println(cyclesPerSecond / 10000.0, 2);

  Serial.print("Cycles available at 50 kHz ");
  Serial.println(cyclesPerSecond / 50000.0, 2);

  Serial.print("Cycles available at 100 kHz ");
  Serial.println(cyclesPerSecond / 100000.0, 2);

  Serial.println();
  Serial.println("CORRECTED TIMING");

  Serial.print("Repetitions per trial ");
  Serial.println(N);

  Serial.print("Trials ");
  Serial.println(TRIALS);

  uint32_t baseline[TRIALS];
  uint32_t analogReadCycles[TRIALS];
  uint32_t doubleCycles[TRIALS];
  uint32_t floatCycles[TRIALS];
  uint32_t integerCycles[TRIALS];

  for (int t = 0; t < TRIALS; t++) {
    baseline[t] = measureBaseline();
  }

  Serial.println();
  Serial.println("EMPTY LOOP BASELINE");

  for (int t = 0; t < TRIALS; t++) {
    Serial.print("Trial ");
    Serial.print(t + 1);
    Serial.print(" ");
    Serial.println(baseline[t]);
  }

  for (int t = 0; t < TRIALS; t++) {
    uint32_t rawCycles = measureAnalogRead();
    analogReadCycles[t] = rawCycles - baseline[t];
  }

  Serial.println();
  Serial.println("ANALOG READ CYCLES/SAMPLE");

  for (int t = 0; t < TRIALS; t++) {
    Serial.print("Trial ");
    Serial.print(t + 1);
    Serial.print(" ");
    Serial.println((float)analogReadCycles[t] / N, 0);
  }

  for (int t = 0; t < TRIALS; t++) {
    uint32_t rawCycles = measureDoublePipeline();
    doubleCycles[t] = rawCycles - baseline[t];
  }

  Serial.println();
  Serial.println("DOUBLE PIPELINE CYCLES/SAMPLE");

  for (int t = 0; t < TRIALS; t++) {
    Serial.print("Trial ");
    Serial.print(t + 1);
    Serial.print(" ");
    Serial.println((float)doubleCycles[t] / N, 0);
  }

  for (int t = 0; t < TRIALS; t++) {
    uint32_t rawCycles = measureFloatPipeline();
    floatCycles[t] = rawCycles - baseline[t];
  }

  Serial.println();
  Serial.println("FLOAT PIPELINE CYCLES/SAMPLE");

  for (int t = 0; t < TRIALS; t++) {
    Serial.print("Trial ");
    Serial.print(t + 1);
    Serial.print(" ");
    Serial.println((float)floatCycles[t] / N, 0);
  }

  for (int t = 0; t < TRIALS; t++) {
    uint32_t rawCycles = measureIntegerPipeline();
    integerCycles[t] = rawCycles - baseline[t];
  }

  Serial.println();
  Serial.println("INTEGER PIPELINE CYCLES/SAMPLE");

  for (int t = 0; t < TRIALS; t++) {
    Serial.print("Trial ");
    Serial.print(t + 1);
    Serial.print(" ");
    Serial.println((float)integerCycles[t] / N, 0);
  }

  double avgAnalog = 0.0;
  double avgDouble = 0.0;
  double avgFloat = 0.0;
  double avgInteger = 0.0;

  for (int t = 0; t < TRIALS; t++) {
    avgAnalog += (double)analogReadCycles[t] / N;
    avgDouble += (double)doubleCycles[t] / N;
    avgFloat += (double)floatCycles[t] / N;
    avgInteger += (double)integerCycles[t] / N;
  }

  avgAnalog /= TRIALS;
  avgDouble /= TRIALS;
  avgFloat /= TRIALS;
  avgInteger /= TRIALS;

  Serial.println();
  Serial.println("AVERAGE CORRECTED TIMING");

  Serial.print("Average analogRead cycles/sample ");
  Serial.println(avgAnalog, 2);

  Serial.print("Average double pipeline cycles/sample ");
  Serial.println(avgDouble, 2);

  Serial.print("Average float pipeline cycles/sample ");
  Serial.println(avgFloat, 2);

  Serial.print("Average integer pipeline cycles/sample ");
  Serial.println(avgInteger, 2);

  Serial.println();
  Serial.println("FINAL OUTPUT CHECK");

  Serial.print("Double final moving average ");
  Serial.println(getDoubleAverage(), 9);

  Serial.print("Float final moving average ");
  Serial.println(getFloatAverage(), 9);

  Serial.print("Integer final moving average ");
  Serial.println(getIntegerAverage());

  exhaustiveErrorTest();

  double doubleTotal = avgAnalog + avgDouble;
  double floatTotal = avgAnalog + avgFloat;
  double integerTotal = avgAnalog + avgInteger;

  Serial.println();
  Serial.println("COMPLETE SENSOR WORKLOAD");

  Serial.print("Double total cycles/sample ");
  Serial.println(doubleTotal, 2);

  Serial.print("Float total cycles/sample ");
  Serial.println(floatTotal, 2);

  Serial.print("Integer total cycles/sample ");
  Serial.println(integerTotal, 2);

  Serial.println();
  Serial.println("LATENCY CHECK - COMPLETE WORKLOAD");

  Serial.print("Double fits 10 kHz ");
  Serial.println(doubleTotal <= 24000.0 ? "YES" : "NO");

  Serial.print("Float fits 10 kHz ");
  Serial.println(floatTotal <= 24000.0 ? "YES" : "NO");

  Serial.print("Integer fits 10 kHz ");
  Serial.println(integerTotal <= 24000.0 ? "YES" : "NO");

  Serial.print("Double fits 50 kHz ");
  Serial.println(doubleTotal <= 4800.0 ? "YES" : "NO");

  Serial.print("Float fits 50 kHz ");
  Serial.println(floatTotal <= 4800.0 ? "YES" : "NO");

  Serial.print("Integer fits 50 kHz ");
  Serial.println(integerTotal <= 4800.0 ? "YES" : "NO");

  Serial.print("Double fits 100 kHz ");
  Serial.println(doubleTotal <= 2400.0 ? "YES" : "NO");

  Serial.print("Float fits 100 kHz ");
  Serial.println(floatTotal <= 2400.0 ? "YES" : "NO");

  Serial.print("Integer fits 100 kHz ");
  Serial.println(integerTotal <= 2400.0 ? "YES" : "NO");

  Serial.println();
  Serial.println("END FINAL CHALLENGE");
}

void loop() {}