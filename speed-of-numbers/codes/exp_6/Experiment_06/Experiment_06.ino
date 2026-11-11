#include <Arduino.h>
#include <stdio.h>
#include <string.h>

const int N = 1000;
const int PRINT_TRIALS = 10;

char buffer[64];

uint32_t measureIntegerConversion() {
  uint32_t t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    snprintf(buffer, sizeof(buffer), "%d", 123456);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0) / N;
}

uint32_t measureFloatConversion() {
  uint32_t t0 = ESP.getCycleCount();

  for (int i = 0; i < N; i++) {
    snprintf(buffer, sizeof(buffer), "%.6f", 123.456789f);
  }

  uint32_t t1 = ESP.getCycleCount();

  return (t1 - t0) / N;
}

uint32_t measureOneCharacter() {
  Serial.flush();

  uint32_t t0 = micros();

  Serial.write('A');
  Serial.flush();

  uint32_t t1 = micros();

  return t1 - t0;
}

uint32_t measureDecimalPrint() {
  Serial.flush();

  uint32_t t0 = micros();

  for (int i = 0; i < PRINT_TRIALS; i++) {
    Serial.println(123456);
  }

  Serial.flush();

  uint32_t t1 = micros();

  return t1 - t0;
}

uint32_t measureHexPrint() {
  Serial.flush();

  uint32_t t0 = micros();

  for (int i = 0; i < PRINT_TRIALS; i++) {
    Serial.println(123456, HEX);
  }

  Serial.flush();

  uint32_t t1 = micros();

  return t1 - t0;
}

uint32_t measureBinaryPrint() {
  Serial.flush();

  uint32_t t0 = micros();

  for (int i = 0; i < PRINT_TRIALS; i++) {
    Serial.println(123456, BIN);
  }

  Serial.flush();

  uint32_t t1 = micros();

  return t1 - t0;
}

int decimalCharacters() {
  snprintf(buffer, sizeof(buffer), "%d", 123456);
  return strlen(buffer) + 1;
}

int hexCharacters() {
  snprintf(buffer, sizeof(buffer), "%X", 123456);
  return strlen(buffer) + 1;
}

int binaryCharacters() {
  uint32_t value = 123456;
  int count = 0;

  do {
    count++;
    value >>= 1;
  } while (value != 0);

  return count + 1;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("EXPERIMENT 6 - HIDDEN COST OF PRINTING");

  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  Serial.println();

  Serial.println("CONVERSION TO TEXT");

  Serial.print("Repetitions: ");
  Serial.println(N);

  uint32_t integerCycles = measureIntegerConversion();
  uint32_t floatCycles = measureFloatConversion();

  Serial.print("Integer conversion cycles/op: ");
  Serial.println(integerCycles);

  Serial.print("Float conversion cycles/op: ");
  Serial.println(floatCycles);

  Serial.println();

  Serial.println("SERIAL TRANSMISSION");

  Serial.println("Baud rate: 115200");

  Serial.print("Measured one-character transmission time (us): ");
  Serial.println(measureOneCharacter());

  Serial.println();

  Serial.println("PRINTING 10 VALUES");

  uint32_t decimalTime = measureDecimalPrint();
  uint32_t hexTime = measureHexPrint();
  uint32_t binaryTime = measureBinaryPrint();

  Serial.print("Decimal print time for 10 values (us): ");
  Serial.println(decimalTime);

  Serial.print("Hexadecimal print time for 10 values (us): ");
  Serial.println(hexTime);

  Serial.print("Binary print time for 10 values (us): ");
  Serial.println(binaryTime);

  Serial.println();

  Serial.println("CHARACTER COUNTS");

  Serial.print("Decimal characters per println: ");
  Serial.println(decimalCharacters());

  Serial.print("Hexadecimal characters per println: ");
  Serial.println(hexCharacters());

  Serial.print("Binary characters per println: ");
  Serial.println(binaryCharacters());

  Serial.println();

  Serial.println("THEORETICAL SERIAL TIMING");

  Serial.println("Serial format: 8 data bits, 1 start bit, 1 stop bit");

  double bitsPerCharacter = 10.0;
  double theoreticalCharacterUs =
      (bitsPerCharacter / 115200.0) * 1000000.0;

  Serial.print("Theoretical time per character (us): ");
  Serial.println(theoreticalCharacterUs, 2);

  double additionsDuringCharacter =
      theoreticalCharacterUs / (1000000000.0 / 240000000.0);

  Serial.print("Approximate CPU cycles during one character: ");
  Serial.println(theoreticalCharacterUs * 240.0);

  Serial.print("Approximate 32-bit additions during one character: ");
  Serial.println(additionsDuringCharacter, 2);

  Serial.println();

  Serial.println("END EXPERIMENT 6");
}

void loop() {
}