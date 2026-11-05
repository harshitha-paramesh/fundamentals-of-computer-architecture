#include <Arduino.h>
#include <string.h>
#include <math.h>

#define TRACKER_GLITCHES 1
#include "adversarial_tracker_1.h"

#define TOTAL_READINGS 100000UL
#define COAST_READINGS 8000UL
#define DESCENT_START 45000UL
#define CENTER_TIME 4.0f

#define GLITCH_THRESHOLD 20.0f
#define DEPLOY_THRESHOLD 15.0f
#define DEPLOY_CONFIRM 3

struct GroundStationState
{
    uint32_t readingCount;
    uint32_t validCount;
    uint32_t glitchCount;
    uint32_t worstReadingCycles;
    uint32_t worstReadingIndex;

    float previousAltitude;
    float previousPreviousAltitude;
    uint32_t previousIndex;
    uint32_t previousPreviousIndex;
    uint8_t validHistory;

    double coastN;
    double coastSx;
    double coastSx2;
    double coastSx3;
    double coastSx4;
    double coastSh;
    double coastSxh;
    double coastSx2h;

    double coastA;
    double coastB;
    double coastC;
    bool coastSolved;

    float descentN;
    float descentRefH;
    float descentSx;
    float descentSy;
    float descentSxx;
    float descentSxy;

    bool deploymentDetected;
    uint32_t deploymentCandidateIndex;
    uint8_t deploymentConfirmCount;

    float deploymentTime;
    float apogeeTime;
    float apogeeHeight;
    float gravity;
    float descentVelocity;
    float landTime;

    char command[32];
    uint8_t commandLength;
};

GroundStationState state;

float coastAf;
float coastBf;
float coastCf;

void initializeState()
{
    memset(&state, 0, sizeof(state));
}

bool isGlitch(float altitude, uint32_t index)
{
    if (state.validHistory < 2)
    {
        return false;
    }

    uint32_t gap =
        index - state.previousIndex;

    float predicted;

    if (gap == 1)
    {
        predicted =
            2.0f * state.previousAltitude -
            state.previousPreviousAltitude;
    }
    else
    {
        float delta =
            state.previousAltitude -
            state.previousPreviousAltitude;

        predicted =
            state.previousAltitude +
            delta *
            (
                (float)gap /
                (float)(
                    state.previousIndex -
                    state.previousPreviousIndex
                )
            );
    }

    float difference =
        altitude - predicted;

    return difference > GLITCH_THRESHOLD ||
           difference < -GLITCH_THRESHOLD;
}

void updateValidHistory(
    float altitude,
    uint32_t index
)
{
    if (state.validHistory == 0)
    {
        state.previousAltitude = altitude;
        state.previousIndex = index;
        state.validHistory = 1;
        return;
    }

    state.previousPreviousAltitude =
        state.previousAltitude;

    state.previousPreviousIndex =
        state.previousIndex;

    state.previousAltitude =
        altitude;

    state.previousIndex =
        index;

    state.validHistory = 2;
}

void updateCoast(
    float altitude,
    uint32_t index
)
{
    float t =
        (float)index * 0.001f;

    float x =
        t - CENTER_TIME;

    float x2 =
        x * x;

    state.coastN += 1.0;
    state.coastSx += (double)x;
    state.coastSx2 += (double)x2;
    state.coastSx3 += (double)(x2 * x);
    state.coastSx4 += (double)(x2 * x2);
    state.coastSh += (double)altitude;
    state.coastSxh += (double)(x * altitude);
    state.coastSx2h += (double)(x2 * altitude);
}

bool solveCoast()
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

    for (int i = 0; i < 3; i++)
    {
        int pivot = i;

        for (int j = i + 1; j < 3; j++)
        {
            if (fabs(m[j][i]) >
                fabs(m[pivot][i]))
            {
                pivot = j;
            }
        }

        if (fabs(m[pivot][i]) < 1e-12)
        {
            return false;
        }

        if (pivot != i)
        {
            for (int k = 0; k < 4; k++)
            {
                double temp = m[i][k];
                m[i][k] = m[pivot][k];
                m[pivot][k] = temp;
            }
        }

        double divisor =
            m[i][i];

        for (int k = i; k < 4; k++)
        {
            m[i][k] /= divisor;
        }

        for (int j = 0; j < 3; j++)
        {
            if (j == i)
            {
                continue;
            }

            double factor =
                m[j][i];

            for (int k = i; k < 4; k++)
            {
                m[j][k] -=
                    factor *
                    m[i][k];
            }
        }
    }

    state.coastA = m[0][3];
    state.coastB = m[1][3];
    state.coastC = m[2][3];

    state.coastSolved = true;

    return true;
}

float coastAltitude(float t)
{
    float x =
        t - CENTER_TIME;

    return
        coastAf +
        coastBf * x +
        coastCf * x * x;
}

void updateDescent(
    float altitude,
    uint32_t index
)
{
    if (index < DESCENT_START)
    {
        return;
    }

    float t =
        (float)index * 0.001f;

    float x =
        t - 50.0f;

    if (state.descentN == 0.0f)
    {
        state.descentRefH =
            altitude;
    }

    float y =
        altitude -
        state.descentRefH;

    state.descentN += 1.0f;
    state.descentSx += x;
    state.descentSy += y;
    state.descentSxx += x * x;
    state.descentSxy += x * y;
}

void updateDeployment(
    float altitude,
    uint32_t index
)
{
    float t =
        (float)index * 0.001f;

    float predicted =
        coastAltitude(t);

    float difference =
        altitude - predicted;

    if (difference * difference >
        DEPLOY_THRESHOLD * DEPLOY_THRESHOLD)
    {
        if (state.deploymentConfirmCount == 0)
        {
            state.deploymentCandidateIndex =
                index;
        }

        state.deploymentConfirmCount++;

        if (state.deploymentConfirmCount >=
            DEPLOY_CONFIRM)
        {
            state.deploymentDetected = true;
        }
    }
    else
    {
        state.deploymentConfirmCount = 0;
    }
}

void processReading(
    uint32_t index,
    float altitude
)
{
    bool glitch = false;

    if (index >= 1000)
    {
        glitch =
            isGlitch(
                altitude,
                index
            );
    }

    if (glitch)
    {
        state.glitchCount++;
        return;
    }

    state.validCount++;

    if (index < COAST_READINGS)
    {
        updateCoast(
            altitude,
            index
        );
    }

    if (index >= DESCENT_START)
    {
        float t =
            (float)index * 0.001f;

        float x =
            t - 50.0f;

        if (state.descentN == 0.0f)
        {
            state.descentRefH =
                altitude;
        }

        float y =
            altitude -
            state.descentRefH;

        state.descentN += 1.0f;
        state.descentSx += x;
        state.descentSy += y;
        state.descentSxx += x * x;
        state.descentSxy += x * y;
    }

    if (index >= COAST_READINGS &&
        index < DESCENT_START &&
        !state.deploymentDetected)
    {
        updateDeployment(
            altitude,
            index
        );
    }

    updateValidHistory(
        altitude,
        index
    );
}

void receiveCoast()
{
    for (uint32_t i = 0;
         i < COAST_READINGS;
         i++)
    {
        uint32_t before =
            ESP.getCycleCount();

        float altitude =
            nextAltitude();

        processReading(
            i,
            altitude
        );

        state.readingCount++;

        uint32_t after =
            ESP.getCycleCount();

        uint32_t cycles =
            after - before;

        if (cycles >
            state.worstReadingCycles)
        {
            state.worstReadingCycles =
                cycles;

            state.worstReadingIndex =
                i;
        }
    }
}

void receiveRemaining()
{
    for (uint32_t i =
             COAST_READINGS;
         i < TOTAL_READINGS;
         i++)
    {
        uint32_t before =
            ESP.getCycleCount();

        float altitude =
            nextAltitude();

        processReading(
            i,
            altitude
        );

        state.readingCount++;

        uint32_t after =
            ESP.getCycleCount();

        uint32_t cycles =
            after - before;

        if (cycles >
            state.worstReadingCycles)
        {
            state.worstReadingCycles =
                cycles;

            state.worstReadingIndex =
                i;
        }
    }
}

void calculateFinalResults()
{
    float descentDenominator =
        state.descentN *
        state.descentSxx -
        state.descentSx *
        state.descentSx;

    float descentNumerator =
        state.descentN *
        state.descentSxy -
        state.descentSx *
        state.descentSy;

    state.descentVelocity =
        descentNumerator /
        descentDenominator;

    float descentIntercept =
        state.descentRefH +
        (
            state.descentSy -
            state.descentVelocity *
            state.descentSx
        ) /
        state.descentN;

    state.landTime =
        50.0f -
        descentIntercept /
        state.descentVelocity;

    state.gravity =
        -2.0f *
        (float)state.coastC;

    float apogeeX =
        -(float)state.coastB /
        (
            2.0f *
            (float)state.coastC
        );

    state.apogeeTime =
        CENTER_TIME +
        apogeeX;

    state.apogeeHeight =
        (float)state.coastA +
        (float)state.coastB *
        apogeeX +
        (float)state.coastC *
        apogeeX *
        apogeeX;

    if (state.descentN <= 1.0f)
    {
        return;
    }

    float qc =
        (float)state.coastA -
        (
            descentIntercept +
            state.descentVelocity *
            (CENTER_TIME - 50.0f)
        );

    float qb =
        (float)state.coastB -
        state.descentVelocity;

    float qa =
        (float)state.coastC;

    float discriminant =
        qb * qb -
        4.0f *
        qa *
        qc;

    if (discriminant < 0.0f)
    {
        return;
    }

    float root =
        sqrtf(discriminant);

    float x1 =
        (-qb + root) /
        (2.0f * qa);

    float x2 =
        (-qb - root) /
        (2.0f * qa);

    float t1 =
        CENTER_TIME + x1;

    float t2 =
        CENTER_TIME + x2;

    float candidate =
        (float)
        state.deploymentCandidateIndex *
        0.001f;

    bool valid1 =
        t1 >= 8.0f &&
        t1 <= 45.0f;

    bool valid2 =
        t2 >= 8.0f &&
        t2 <= 45.0f;

    if (valid1 && valid2)
    {
        state.deploymentTime =
            fabsf(t1 - candidate) <
            fabsf(t2 - candidate)
            ? t1
            : t2;
    }
    else if (valid1)
    {
        state.deploymentTime =
            t1;
    }
    else if (valid2)
    {
        state.deploymentTime =
            t2;
    }
}

float calculateAltitude(float t)
{
    if (t < 0.0f ||
        t > 99.999f)
    {
        return NAN;
    }

    if (t <=
        state.deploymentTime)
    {
        return coastAltitude(t);
    }

    return
        state.descentRefH +
        state.descentVelocity *
        (t - 50.0f) +
        (
            state.descentSy -
            state.descentVelocity *
            state.descentSx
        ) /
        state.descentN;
}

bool parseFloat(
    const char *s,
    float &value
)
{
    if (s == nullptr ||
        *s == '\0')
    {
        return false;
    }

    char *endptr;

    value =
        strtof(
            s,
            &endptr
        );

    if (endptr == s)
    {
        return false;
    }

    while (*endptr == ' ')
    {
        endptr++;
    }

    return
        *endptr == '\0' &&
        !isnan(value) &&
        !isinf(value);
}

void processCommand()
{
    if (strcmp(
            state.command,
            "STATS"
        ) == 0)
    {
        Serial.print(
            "Total readings = "
        );

        Serial.println(
            state.readingCount
        );

        Serial.print(
            "Valid readings = "
        );

        Serial.println(
            state.validCount
        );

        Serial.print(
            "Coast readings = "
        );

        Serial.println(
            COAST_READINGS
        );

        Serial.print(
            "Descent readings = "
        );

        Serial.println(
            (uint32_t)
            state.descentN
        );

        Serial.print(
            "Glitches = "
        );

        Serial.println(
            state.glitchCount
        );

        Serial.print(
            "State size = "
        );

        Serial.print(
            sizeof(
                GroundStationState
            )
        );

        Serial.println(
            " bytes"
        );

        Serial.print(
            "Worst reading cycles = "
        );

        Serial.println(
            state.worstReadingCycles
        );

        Serial.print(
            "Worst reading index = "
        );

        Serial.println(
            state.worstReadingIndex
        );

        Serial.println(
            "Cycle budget = 12000"
        );

        return;
    }

    if (strcmp(
            state.command,
            "GLITCHES"
        ) == 0)
    {
        Serial.print(
            "GLITCHES = "
        );

        Serial.println(
            state.glitchCount
        );

        return;
    }

    if (strcmp(
            state.command,
            "G"
        ) == 0)
    {
        Serial.print(
            "G = "
        );

        Serial.print(
            state.gravity,
            4
        );

        Serial.println(
            " m/s^2"
        );

        return;
    }

    if (strcmp(
            state.command,
            "APOGEE"
        ) == 0)
    {
        Serial.print(
            "APOGEE time = "
        );

        Serial.print(
            state.apogeeTime,
            4
        );

        Serial.print(
            " s, height = "
        );

        Serial.print(
            state.apogeeHeight,
            4
        );

        Serial.println(
            " m"
        );

        return;
    }

    if (strcmp(
            state.command,
            "DESCENT"
        ) == 0)
    {
        Serial.print(
            "DESCENT = "
        );

        Serial.print(
            state.descentVelocity,
            4
        );

        Serial.println(
            " m/s"
        );

        return;
    }

    if (strcmp(
            state.command,
            "LAND"
        ) == 0)
    {
        Serial.print(
            "LAND = "
        );

        Serial.print(
            state.landTime,
            4
        );

        Serial.println(
            " s"
        );

        return;
    }

    if (strcmp(
            state.command,
            "DEPLOY"
        ) == 0)
    {
        Serial.print(
            "DEPLOY = "
        );

        Serial.print(
            state.deploymentTime,
            4
        );

        Serial.println(
            " s"
        );

        return;
    }

    if (strncmp(
            state.command,
            "ALT ",
            4
        ) == 0)
    {
        float t;

        if (!parseFloat(
                state.command + 4,
                t
            ))
        {
            Serial.println("ERR");
            return;
        }

        float altitude =
            calculateAltitude(t);

        if (isnan(altitude))
        {
            Serial.println("ERR");
            return;
        }

        Serial.print(
            "ALT "
        );

        Serial.print(
            t,
            4
        );

        Serial.print(
            " = "
        );

        Serial.print(
            altitude,
            4
        );

        Serial.println(
            " m"
        );

        return;
    }

    Serial.println("ERR");
}

void readCommand()
{
    while (
        Serial.available() > 0
    )
    {
        char c =
            Serial.read();

        if (c == '\n' ||
            c == '\r')
        {
            if (
                state.commandLength ==
                0
            )
            {
                continue;
            }

            state.command[
                state.commandLength
            ] = '\0';

            processCommand();

            state.commandLength = 0;
            state.command[0] = '\0';

            Serial.print("> ");

            return;
        }

        if (
            state.commandLength <
            sizeof(state.command) - 1
        )
        {
            state.command[
                state.commandLength
            ] = c;

            state.commandLength++;

            state.command[
                state.commandLength
            ] = '\0';
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

    Serial.println(
        "=============================="
    );

    Serial.println(
        "FCA CATCH THE ROCKET"
    );

    Serial.println(
        "LEVEL 5 - PERFORMANCE"
    );

    Serial.println(
        "=============================="
    );

    Serial.println(
        "Receiving 100000 readings..."
    );

    receiveCoast();

    if (!solveCoast())
    {
        Serial.println("ERR");
        return;
    }

    coastAf =
        (float)state.coastA;

    coastBf =
        (float)state.coastB;

    coastCf =
        (float)state.coastC;

    receiveRemaining();

    calculateFinalResults();

    Serial.println(
        "Receiving complete."
    );

    Serial.print(
        "Total readings = "
    );

    Serial.println(
        state.readingCount
    );

    Serial.print(
        "Valid readings = "
    );

    Serial.println(
        state.validCount
    );

    Serial.print(
        "Glitches = "
    );

    Serial.println(
        state.glitchCount
    );

    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        "Ready for commands."
    );

    Serial.print("> ");
}

void loop()
{
    readCommand();
}