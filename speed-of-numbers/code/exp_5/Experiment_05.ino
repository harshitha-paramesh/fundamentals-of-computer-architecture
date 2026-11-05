#include <stdint.h>

const int N = 1000;
const int TRIALS = 3;
const int32_t SCALE = 65536;

int32_t toFixed(double x) {
  return (int32_t)(x * SCALE);
}

double fromFixed(int32_t x) {
  return (double)x / SCALE;
}

int32_t fixedAdd(int32_t a, int32_t b) {
  return a + b;
}

int32_t fixedMultiply(int32_t a, int32_t b) {
  int64_t temp = (int64_t)a * (int64_t)b;
  return (int32_t)(temp >> 16);
}

int32_t fixedDivide(int32_t a, int32_t b) {
  int64_t temp = ((int64_t)a << 16);
  return (int32_t)(temp / b);
}

uint32_t measureFixedAdd(int32_t a, int32_t b) {
  volatile int32_t result = 0;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = fixedAdd(a, b);
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

uint32_t measureFixedMultiply(int32_t a, int32_t b) {
  volatile int32_t result = 0;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = fixedMultiply(a, b);
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

uint32_t measureFixedDivide(int32_t a, int32_t b) {
  volatile int32_t result = 0;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = fixedDivide(a, b);
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

uint32_t measureFloatAdd(float a, float b) {
  volatile float result = 0.0f;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = a + b;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

uint32_t measureFloatMultiply(float a, float b) {
  volatile float result = 0.0f;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = a * b;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

uint32_t measureFloatDivide(float a, float b) {
  volatile float result = 0.0f;
  uint32_t t0, t1;
  uint32_t baseline, total;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    asm volatile("");
  }
  t1 = ESP.getCycleCount();
  baseline = t1 - t0;

  t0 = ESP.getCycleCount();
  for (int i = 0; i < N; i++) {
    result = a / b;
  }
  t1 = ESP.getCycleCount();
  total = t1 - t0;

  return total - baseline;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("EXPERIMENT 5 - FIXED POINT VS FLOAT");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.println();
  Serial.println("Q16.16 REPRESENTATION");

  int32_t fixed375 = toFixed(3.75);
  int32_t fixedNeg125 = toFixed(-1.25);

  Serial.print("3.75 fixed: ");
  Serial.println(fixed375);

  Serial.print("-1.25 fixed: ");
  Serial.println(fixedNeg125);

  Serial.print("3.75 recovered: ");
  Serial.println(fromFixed(fixed375), 8);

  Serial.print("-1.25 recovered: ");
  Serial.println(fromFixed(fixedNeg125), 8);

  Serial.print("Q16.16 minimum: ");
  Serial.println(fromFixed(INT32_MIN), 8);

  Serial.print("Q16.16 maximum: ");
  Serial.println(fromFixed(INT32_MAX), 8);

  double x = 1.2345;
  double y = 2.3456;

  int32_t fx = toFixed(x);
  int32_t fy = toFixed(y);

  float fX = (float)x;
  float fY = (float)y;

  Serial.println();
  Serial.println("ACCURACY TEST");

  double trueAdd = x + y;
  double trueMultiply = x * y;
  double trueDivide = x / y;

  double fixedAddResult = fromFixed(fixedAdd(fx, fy));
  double fixedMultiplyResult = fromFixed(fixedMultiply(fx, fy));
  double fixedDivideResult = fromFixed(fixedDivide(fx, fy));

  double floatAddResult = (double)(fX + fY);
  double floatMultiplyResult = (double)(fX * fY);
  double floatDivideResult = (double)(fX / fY);

  Serial.print("True addition: ");
  Serial.println(trueAdd, 10);

  Serial.print("Fixed addition: ");
  Serial.println(fixedAddResult, 10);

  Serial.print("Float addition: ");
  Serial.println(floatAddResult, 10);

  Serial.print("Fixed addition error: ");
  Serial.println(fabs(trueAdd - fixedAddResult), 10);

  Serial.print("Float addition error: ");
  Serial.println(fabs(trueAdd - floatAddResult), 10);

  Serial.println();

  Serial.print("True multiplication: ");
  Serial.println(trueMultiply, 10);

  Serial.print("Fixed multiplication: ");
  Serial.println(fixedMultiplyResult, 10);

  Serial.print("Float multiplication: ");
  Serial.println(floatMultiplyResult, 10);

  Serial.print("Fixed multiplication error: ");
  Serial.println(fabs(trueMultiply - fixedMultiplyResult), 10);

  Serial.print("Float multiplication error: ");
  Serial.println(fabs(trueMultiply - floatMultiplyResult), 10);

  Serial.println();

  Serial.print("True division: ");
  Serial.println(trueDivide, 10);

  Serial.print("Fixed division: ");
  Serial.println(fixedDivideResult, 10);

  Serial.print("Float division: ");
  Serial.println(floatDivideResult, 10);

  Serial.print("Fixed division error: ");
  Serial.println(fabs(trueDivide - fixedDivideResult), 10);

  Serial.print("Float division error: ");
  Serial.println(fabs(trueDivide - floatDivideResult), 10);

  Serial.println();
  Serial.println("TIMING");
  Serial.print("Repetitions per trial: ");
  Serial.println(N);
  Serial.print("Trials: ");
  Serial.println(TRIALS);

  Serial.println();
  Serial.println("Fixed-point addition cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFixedAdd(fx, fy);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println("Fixed-point multiplication cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFixedMultiply(fx, fy);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println("Fixed-point division cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFixedDivide(fx, fy);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println("Float addition cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFloatAdd(fX, fY);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println("Float multiplication cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFloatMultiply(fX, fY);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println("Float division cycles/op:");

  for (int t = 0; t < TRIALS; t++) {
    uint32_t cycles = measureFloatDivide(fX, fY);
    Serial.print((float)cycles / N, 2);
    Serial.println();
  }

  Serial.println();
  Serial.println("END EXPERIMENT 5");
}

void loop() {
}