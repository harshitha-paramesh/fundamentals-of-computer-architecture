#include <Arduino.h>
#include <string.h>
#include <math.h>

#define TRACKER_GLITCHES 0
#include "official_tracker.h"

struct GroundStationState
{
    uint32_t readingCount;
    uint32_t worstReadingCycles;

    float descentN;
    float descentSx;
    float descentSh;
    float descentSxx;
    float descentSxh;

    double coastN;
    double coastSx;
    double coastSx2;
    double coastSx3;
    double coastSx4;
    double coastSh;
    double coastSxh;
    double coastSx2h;

    char command[32];
    uint8_t commandLength;
};

GroundStationState state;

void initializeState()
{
    memset(&state, 0, sizeof(state));
}

void receiveReadings()
{
    for (uint32_t i = 0; i < 100000UL; i++)
    {
        float altitude = nextAltitude();

        uint32_t before = ESP.getCycleCount();

        float t = (float)i * 0.001f;

        if (t >= 45.0f)
        {
            float x = t - 72.5f;

            state.descentN += 1.0f;
            state.descentSx += x;
            state.descentSh += altitude;
            state.descentSxx += x * x;
            state.descentSxh += x * altitude;
        }

        if (i < 8000UL)
        {
            double x = (double)t - 4.0;
            double h = (double)altitude;
            double x2 = x * x;

            state.coastN += 1.0;
            state.coastSx += x;
            state.coastSx2 += x2;
            state.coastSx3 += x2 * x;
            state.coastSx4 += x2 * x2;
            state.coastSh += h;
            state.coastSxh += x * h;
            state.coastSx2h += x2 * h;
        }

        state.readingCount++;

        uint32_t after = ESP.getCycleCount();

        uint32_t cycles = (uint32_t)(after - before);

        if (cycles > state.worstReadingCycles)
        {
            state.worstReadingCycles = cycles;
        }
    }
}

float calculateDescentVelocity()
{
    float numerator =
        state.descentN * state.descentSxh -
        state.descentSx * state.descentSh;

    float denominator =
        state.descentN * state.descentSxx -
        state.descentSx * state.descentSx;

    return numerator / denominator;
}

float calculateDescentIntercept(float b)
{
    return
        (state.descentSh - b * state.descentSx) /
        state.descentN;
}

float calculateLand()
{
    float b = calculateDescentVelocity();
    float a = calculateDescentIntercept(b);

    return 72.5f - a / b;
}

bool solveCoast(double &A, double &B, double &C)
{
    double m[3][4];

    m[0][0] = state.coastN;
    m[0][1] = state.coastSx;
    m[0][2] = state.coastSx2;
    m[0][3] = state.coastSh;

    m[1][0] = state.coastSx;
    m[1][1] = state.coastSx2;
    m[1][2] = state.coastSx3;
    m[1][3] = state.coastSxh;

    m[2][0] = state.coastSx2;
    m[2][1] = state.coastSx3;
    m[2][2] = state.coastSx4;
    m[2][3] = state.coastSx2h;

    for (int col = 0; col < 3; col++)
    {
        int pivot = col;
        double largest = fabs(m[col][col]);

        for (int row = col + 1; row < 3; row++)
        {
            double value = fabs(m[row][col]);

            if (value > largest)
            {
                largest = value;
                pivot = row;
            }
        }

        if (largest < 1e-12)
        {
            return false;
        }

        if (pivot != col)
        {
            for (int j = col; j < 4; j++)
            {
                double temp = m[col][j];
                m[col][j] = m[pivot][j];
                m[pivot][j] = temp;
            }
        }

        for (int row = col + 1; row < 3; row++)
        {
            double factor =
                m[row][col] / m[col][col];

            for (int j = col; j < 4; j++)
            {
                m[row][j] -=
                    factor * m[col][j];
            }
        }
    }

    C =
        m[2][3] /
        m[2][2];

    B =
        (m[1][3] -
         m[1][2] * C) /
        m[1][1];

    A =
        (m[0][3] -
         m[0][2] * C -
         m[0][1] * B) /
        m[0][0];

    return true;
}

double calculateGravity()
{
    double A, B, C;

    if (!solveCoast(A, B, C))
    {
        return NAN;
    }

    return -2.0 * C;
}

bool calculateApogee(double &time, double &height)
{
    double A, B, C;

    if (!solveCoast(A, B, C))
    {
        return false;
    }

    double xApogee =
        -B / (2.0 * C);

    time =
        4.0 + xApogee;

    height =
        A +
        B * xApogee +
        C * xApogee * xApogee;

    return true;
}

void printStats()
{
    Serial.print("bytes used: ");
    Serial.print(sizeof(GroundStationState));
    Serial.println(" of 4096");

    Serial.print("worst reading: ");
    Serial.print(state.worstReadingCycles);
    Serial.println(" cycles (budget 12000)");

    Serial.print("readings received: ");
    Serial.println(state.readingCount);

    Serial.print("descent samples: ");
    Serial.println((uint32_t)state.descentN);

    Serial.print("coast samples: ");
    Serial.println((uint32_t)state.coastN);
}

void processCommand()
{
    if (strcmp(state.command, "STATS") == 0)
    {
        printStats();
        return;
    }

    if (strcmp(state.command, "DESCENT") == 0)
    {
        uint32_t before = ESP.getCycleCount();

        float velocity =
            calculateDescentVelocity();

        uint32_t after = ESP.getCycleCount();

        uint32_t cycles =
            (uint32_t)(after - before);

        Serial.print(velocity, 3);
        Serial.print(" m/s (");
        Serial.print(cycles);
        Serial.println(" cycles)");

        return;
    }

    if (strcmp(state.command, "LAND") == 0)
    {
        uint32_t before = ESP.getCycleCount();

        float landingTime =
            calculateLand();

        uint32_t after = ESP.getCycleCount();

        uint32_t cycles =
            (uint32_t)(after - before);

        Serial.print(landingTime, 3);
        Serial.print(" s (");
        Serial.print(cycles);
        Serial.println(" cycles)");

        return;
    }

    if (strcmp(state.command, "G") == 0)
    {
        uint32_t before = ESP.getCycleCount();

        double gravity =
            calculateGravity();

        uint32_t after = ESP.getCycleCount();

        uint32_t cycles =
            (uint32_t)(after - before);

        if (isnan(gravity))
        {
            Serial.println("ERR");
            return;
        }

        Serial.print(gravity, 3);
        Serial.print(" m/s^2 (");
        Serial.print(cycles);
        Serial.println(" cycles)");

        return;
    }

    if (strcmp(state.command, "APOGEE") == 0)
    {
        uint32_t before = ESP.getCycleCount();

        double time;
        double height;

        bool valid =
            calculateApogee(time, height);

        uint32_t after = ESP.getCycleCount();

        uint32_t cycles =
            (uint32_t)(after - before);

        if (!valid)
        {
            Serial.println("ERR");
            return;
        }

        Serial.print(time, 3);
        Serial.print(" s ");
        Serial.print(height, 2);
        Serial.print(" m (");
        Serial.print(cycles);
        Serial.println(" cycles)");

        return;
    }

    if (strcmp(state.command, "DEPLOY") == 0)
    {
        Serial.println("TODO");
        return;
    }

    if (strcmp(state.command, "GLITCHES") == 0)
    {
        Serial.println("TODO");
        return;
    }

    if (strncmp(state.command, "ALT ", 4) == 0)
    {
        Serial.println("TODO");
        return;
    }

    Serial.println("ERR");
}

void readCommand()
{
    while (Serial.available() > 0)
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (state.commandLength == 0)
            {
                continue;
            }

            state.command[state.commandLength] = '\0';

            processCommand();

            state.commandLength = 0;
            state.command[0] = '\0';

            Serial.print("> ");

            return;
        }

        if (state.commandLength == 0 && c == ' ')
        {
            continue;
        }

        if (state.commandLength <
            sizeof(state.command) - 1)
        {
            state.command[state.commandLength] = c;
            state.commandLength++;

            state.command[state.commandLength] = '\0';
        }
        else
        {
            state.commandLength = 0;
            state.command[0] = '\0';

            Serial.println("ERR");
            Serial.print("> ");

            return;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    delay(500);

    initializeState();

    Serial.println();
    Serial.println("==============================");
    Serial.println("FCA CATCH THE ROCKET");
    Serial.println("LEVEL 0 + LEVEL 1 + LEVEL 2");
    Serial.println("==============================");

    Serial.println("Receiving 100000 readings...");

    receiveReadings();

    Serial.println("Receiving complete.");

    Serial.print("Readings received: ");
    Serial.println(state.readingCount);

    Serial.print("Descent samples: ");
    Serial.println((uint32_t)state.descentN);

    Serial.print("Coast samples: ");
    Serial.println((uint32_t)state.coastN);

    Serial.println();

    printStats();

    Serial.println();
    Serial.println("Ready for commands.");
    Serial.print("> ");
}

void loop()
{
    readCommand();
}