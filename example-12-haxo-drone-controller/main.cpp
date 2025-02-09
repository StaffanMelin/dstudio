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
#include "../rtDStudio/src/dhaxo.h"
#include "../rtDStudio/src/dsettings.h"

#include "../rtDStudio/src/drt.h"



//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
DSynthSub dsynthmelody;
DSynthSub dsynthpad;
DMixer dmixer;
DHaxo dhaxo;
DSettingsD dsetd; // sound files



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

	// synth melody
	dsetd.InitDir(DSettings::DSYNTHSUB, DSettings::NONE, "data/");
	std::string synth_file = dsetd.NextFile();
	DSynthSub::Config dsynthsub_config;
	dsynthmelody.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, synth_file, &dsynthsub_config);
	dsynthmelody.Set(dsynthsub_config);

	// pad
	dsynthpad.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_pad.xml", &dsynthsub_config);
	dsynthpad.Set(dsynthsub_config);

	// mixer
	DSound *dmix_synth[MIXER_CHANNELS_MAX];
	float dmix_pan[MIXER_CHANNELS_MAX];
	float dmix_level[MIXER_CHANNELS_MAX];
	float dmix_chorus_level[MIXER_CHANNELS_MAX];
	float dmix_reverb_level[MIXER_CHANNELS_MAX];
	bool dmix_mono[MIXER_CHANNELS_MAX];
	uint8_t dmix_group[MIXER_CHANNELS_MAX];
	DMixer::Config dmix_config;

	dmix_synth[0] = &dsynthmelody;
	dmix_synth[1] = &dsynthpad;
	dmix_level[0] = 1.0;
	dmix_level[1] = 0.3;
	dmix_pan[0] = 0.5f;
	dmix_pan[1] = 0.2f;
	dmix_chorus_level[0] = 0.2f;
	dmix_chorus_level[1] = 0.0f;
	dmix_reverb_level[0] = 0.5f;
	dmix_reverb_level[1] = 0.6f;
	dmix_mono[0] = true;
	dmix_mono[1] = true;
	dmix_group[0] = 0;
	dmix_group[1] = 1;
	dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dmix_config.channels = 2;
	dmix_config.amp = 0.5f;
	dmix_config.synth = dmix_synth;
	dmix_config.pan = dmix_pan;
	dmix_config.level = dmix_level;
	dmix_config.chorus_level = dmix_chorus_level;
	dmix_config.reverb_level = dmix_reverb_level;
	dmix_config.mono = dmix_mono;
	dmix_config.group = dmix_group;
	dmix_config.chorus_return = 0.5;
	dmix_config.reverb_return = 0.5f;
	dmix_config.mix_dry = 0.3;
	dmixer.Init();
	dmixer.Set(dmix_config);

	// example start
	dmixer.SetReverb(0.9f, 2000.0f);

	// send dmixer obj to be able to send MIDI to mixer
	DHaxo::Config dhaxo_config;
	dhaxo_config.controller = true;
	dhaxo_config.controller_targets = 3;
	dhaxo_config.controller_target[0] = DSynth::DSYNTH_PARAM_TUNE; 
	dhaxo_config.controller_target[1] = DSynth::DSYNTH_PARAM_FILTER_CUTOFF; 
	dhaxo_config.controller_target[2] = DSynth::DSYNTH_PARAM_LFO_AMP;
	dhaxo_config.synth = &dsynthmelody;
	dhaxo.Init();
	dhaxo.Set(dhaxo_config);

	// start drone
	dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, 36, 100);
  
	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
	static uint64_t last_time = dGetElapsedTimeMicros();
	DHaxo::HaxoControl haxo_control = dhaxo.ProcessControl();

	// half a second must pass between control key presses
	if ((dGetElapsedTimeMicros() - last_time) > 1000000)
	{
		last_time = dGetElapsedTimeMicros();

		switch (haxo_control)
		{
		case DHaxo::HAXOCONTROL_PREVSOUND:
			{
				std::string synth_file = dsetd.PrevFile();
				DSynthSub::Config dsynthsub_config;
				DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, synth_file, &dsynthsub_config);
				dsynthmelody.Set(dsynthsub_config);
			}
			break;
		case DHaxo::HAXOCONTROL_NEXTSOUND:
			{
				std::string synth_file = dsetd.NextFile();
				DSynthSub::Config dsynthsub_config;
				DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, synth_file, &dsynthsub_config);
				dsynthmelody.Set(dsynthsub_config);
			}
			break;
		case DHaxo::HAXOCONTROL_TURNOFF:
			{
				#ifdef DEBUG
				std::cout << "Main turnoff \n";
				#endif
				done_ = true;
			}
			break;
		default:
			break;
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
