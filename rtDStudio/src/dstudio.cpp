#include "dstudio.h"
#include <stdlib.h> /* srand, rand */

#include <chrono>

uint64_t dGetElapsedTimeMicros()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

uint32_t dGetElapsedTimeMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void dSleepMs(uint32_t ms)
{
    uint32_t start = dGetElapsedTimeMillis();
    while (start - dGetElapsedTimeMillis() < ms)
    {
        // do nothing, just wait
    }
}

float dRandom(float max)
{
    return (max * rand() / float(RAND_MAX)) * (1.0f - std::numeric_limits<float>::epsilon());
}

// return random number from 0 to max (inclusive)
float dRand(float max)
{
    return (max * rand() / float(RAND_MAX));
}

// return random number from 0 to max (not inclusive)
uint32_t dRandI(uint32_t max)
{
        return static_cast<int>(max * (rand() / float(RAND_MAX)));
}

int dRandWeightedList(float w[], int count)
{
    float sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += w[i];
    }
    float rnd = dRand(sum);
    for (int i = 0; i < count; i++)
    {
        if (rnd < w[i])
        {
            return i;
        }
        rnd -= w[i];
    }
    // should't get here but...
    return 0;
}

// measure interval in microseconds
void DInterval::Init(uint64_t interval_us)
{
    interval_us_ = interval_us;
    uint64_t start_ = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// countdown interval and check if it has passed
bool DInterval::Process()
{
    bool retval = false;

    uint64_t where = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if (where - start_ > interval_us_)
    {
        start_ = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        retval = true;
    }

    return (retval);
}