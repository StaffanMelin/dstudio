#include "main.h"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <getopt.h>

// standard
#include "../rtDaisySP/src/daisysp.h"
#include "../rtDStudio/src/dstudio.h"

// application - DStudio
#include "../rtDStudio/src/dmixer.h"
#include "../rtDStudio/src/dsynthsub.h"

#include "rt.h"



//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
#define DRONES 16
/*
In dstudio.h we have #define MIXER_CHANNELS_MAX 16.
So if you raise DRONES > MIXER_CHANNELS_MAX rememeber to change MIXER_CHANNELS_MAX too.
Note! Don't go higher than what fits in a uint8_t, ie 255!
*/
DSynthSub ddrone[DRONES];
float tune[DRONES];
float detune[DRONES];
float cutoff[DRONES];

DMixer dmixer;
DInterval clocker;




//////////////////////////////////////////////////
// util
//////////////////////////////////////////////////

// Interrupt handler function
static void finish(int /*ignore*/)
{
	done_ = true;
}



//////////////////////////////////////////////////
// DStudio
//////////////////////////////////////////////////

// init synths
bool InitSynths()
{
	bool retval = true;

    // synths subtractive
    DSynthSub::Config dsynth_config;

    // synths
    for (uint8_t d = 0; d < DRONES; d++)
    {
        dsynth_config.sample_rate = DSTUDIO_SAMPLE_RATE;
        dsynth_config.voices = 1;
        dsynth_config.waveform0 = DSynthSub::WAVE_SAW;
        dsynth_config.waveform1 = DSynthSub::WAVE_TRI;
        tune[d] = dRandom(5.0f);
        dsynth_config.tune = tune[d];
        detune[d] = dRandom(10.0f);
        dsynth_config.detune = detune[d];
        dsynth_config.transpose = 0;
        dsynth_config.osc0_level = 0.9f;
        dsynth_config.osc1_level = 0.9f;
        dsynth_config.noise_level = 0.0f;
        dsynth_config.filter_type = DSynthSub::LOW;
        cutoff[d] = 400.0f + dRandom(400.0f);
        dsynth_config.filter_cutoff = cutoff[d];
        dsynth_config.filter_res = 0.1f;
        dsynth_config.eg_p_level = 0.0f;
        dsynth_config.eg_p_attack = 0.0f;
        dsynth_config.eg_p_decay = 0.0f;
        dsynth_config.eg_p_sustain = 0.0f;
        dsynth_config.eg_p_release = 0.0f;
        dsynth_config.eg_f_level = 1.0f;
        dsynth_config.eg_f_attack = 0.0f;
        dsynth_config.eg_f_decay = 0.0f;
        dsynth_config.eg_f_sustain = 1.0f;
        dsynth_config.eg_f_release = 0.2f;
        dsynth_config.eg_a_attack = 0.3f;
        dsynth_config.eg_a_decay = 0.01f;
        dsynth_config.eg_a_sustain = 1.0f;
        dsynth_config.eg_a_release = 0.4f;
        dsynth_config.lfo_waveform = DSynthSub::WAVE_TRI;
        dsynth_config.lfo_freq = 0;//dRandom(0.5f);
        dsynth_config.lfo_amp = 0;//dRandom(1.0f);
        dsynth_config.lfo_p_level = 0.0f;
        dsynth_config.lfo_f_level = 0.0f;
        dsynth_config.lfo_a_level = 0.0f;
        dsynth_config.portamento = 3.0f;
        dsynth_config.delay_delay = dRandom(1.0f);
        dsynth_config.delay_feedback = dRandom(1.0f);
        dsynth_config.overdrive_gain = 0.0f;
        dsynth_config.overdrive_drive = 0.0f; //dRandom(0.2f);
        ddrone[d].Init();
        ddrone[d].Set(dsynth_config);
    }

    // main mixer
    DSound *dmix_synth[MIXER_CHANNELS_MAX];
    float dmix_pan[MIXER_CHANNELS_MAX];
    float dmix_level[MIXER_CHANNELS_MAX];
    float dmix_chorus_level[MIXER_CHANNELS_MAX];
    float dmix_reverb_level[MIXER_CHANNELS_MAX];
    bool dmix_mono[MIXER_CHANNELS_MAX];
    uint8_t dmix_group[MIXER_CHANNELS_MAX];
    DMixer::Config dmix_config;

    for (uint8_t d = 0; d < DRONES; d++)
    {
        dmix_synth[d] = &ddrone[d];
        dmix_pan[d] = dRandom(1.0f);
        dmix_level[d] = 0.95f / (float)DRONES;
        dmix_chorus_level[d] = dRandom(0.8f);
        dmix_reverb_level[d] = 0.5f + dRandom(0.5f);
        dmix_mono[d] = true;
        dmix_group[d] = d;
    }
    dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dmix_config.channels = DRONES;
    dmix_config.amp = 1.2f;
    dmix_config.synth = dmix_synth;
    dmix_config.pan = dmix_pan;
    dmix_config.level = dmix_level;
    dmix_config.chorus_level = dmix_chorus_level;
    dmix_config.reverb_level = dmix_reverb_level;
    dmix_config.mono = dmix_mono;
    dmix_config.group = dmix_group;
    dmix_config.chorus_return = 0.5;
    dmix_config.reverb_return = 0.8f;
    dmix_config.mix_dry = 0.2;
    dmixer.Init();
    dmixer.Set(dmix_config);

    // demo start
    dmixer.SetReverb(0.4f, 6000.0f);
    for (uint8_t d = 0; d < DRONES; d++)
    {
        ddrone[d].NoteOn(18 + 6 * (int)(dRandom(10.0f)));
    }

    clocker.Init(1000000);

	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
	if (clocker.Process())
    {
		for (uint8_t d = 0; d < DRONES; d++)
		{
			// make tuning of synths slowly drift
			if (dRandom(1.0f) < 0.02)
			{
				tune[d] = dRandom(15.0f); // 5.0f
				ddrone[d].SetTuning(tune[d], detune[d]);
			}
			// make cutoff of synths drift
			if (dRandom(1.0f) < 0.01)
			{
				cutoff[d] = 100.0f + dRandom(2500.0f); // 500.0f
				ddrone[d].SetFilter(DSynthSub::LOW, cutoff[d], 0.1f);
			}
			// pitch drift w portamento
			if (dRandom(1.0f) < 0.002)
			{
				ddrone[d].NoteOn(18 + 10 * (int)(dRandom(10.0f)));
			}
		}
	}
}



// main

// -l - list audio devices
// -d <dev> - use given audio device
int main(int argc, char* argv[])
{
	int c;
	bool arg_devices_set = false;
	bool arg_devices_list = false;
    char* arg_device = nullptr;

	opterr = 0;

	while ((c = getopt(argc, argv, "d:l")) != -1)
	{
		switch (c)
		{
			case 'd':
				arg_devices_set = true;
				arg_device = optarg;
				break;
			case 'l':
				arg_devices_list = true;
				break;
		}
	}

	bool retval = true;

	std::cout << "INFO init synths\n";
	retval = InitSynths();
	dout_ = &dmixer;

	std::cout << "INFO init audio\n";
	retval = InitRtAudio(arg_devices_list, arg_devices_set, arg_device);

	if (retval)
	{
		// run until interrupt with CTRL+C
		done_ = false;
		(void)signal(SIGINT, finish);
		std::cout << "\nPlaying - quit with Ctrl-C.\n";

		// application
		SLEEP(1000);

		// main application loop
		while (!done_ && rt_dac_.isStreamRunning())
		{
			ProcessControl();
			SLEEP(10); // 10 ms
		}
	}

	// rtAudio cleanup
	ExitRtAudio();
	
	return 0;
}
