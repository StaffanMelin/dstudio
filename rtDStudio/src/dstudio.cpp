#include "dstudio.h"
#include <stdlib.h>     /* srand, rand */

#include <chrono>
uint64_t dGetElapsedTimeMicros(){
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

float dRandom(float max) {
    return (max * rand() / float(RAND_MAX)) * (1.0f - std::numeric_limits<float>::epsilon());
}

// class that measure interval in microseconds
void DInterval::Init(uint64_t interval_us)
{
    interval_us_ = interval_us;
    uint64_t start_ = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

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