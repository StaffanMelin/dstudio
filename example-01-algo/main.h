#pragma once

#include "../rtaudio/RtAudio.h"

#define DEBUG

// rtaudio

// DaisySP works with 32 bit floats
typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32

#include <unistd.h>
#define SLEEP(milliseconds) usleep((unsigned long)(milliseconds * 1000.0))
