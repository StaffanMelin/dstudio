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
#include "../rtDStudio/src/dsampler.h"
#include "../rtDStudio/src/dsynthvar.h"
#include "../rtDStudio/src/dsynthfm.h"

#include "../rtDStudio/src/dfx.h"
#include "../rtDStudio/src/dsettings.h"

#include "../rtDStudio/src/drt.h"

//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
DSynthSub dsynthpad;
DSynthSub dsynthkajsa;
DSynthSub dsynthspace;
DSampler dsampler;
DSynthVar dsynthmelody;
DFXFilter dfxfilter;
DSynthFm dsynthbling;
DSynthVar dtexture;
DFXSlicer dfxslicer;

DMixer dmixer;

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

	DSynthVar::Config dsynthvar_config;
	DSynthSub::Config dsynth_config;

	// synth pad (dsynthsub)
	dsynthpad.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_pad.xml", &dsynth_config);
	dsynthpad.Set(dsynth_config);

	dsynthkajsa.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_kajsa.xml", &dsynth_config);
	dsynthkajsa.Set(dsynth_config);

	dsynthspace.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_space.xml", &dsynth_config);
	dsynthspace.Set(dsynth_config);

	DSynthFm::Config dsynthfm_config;
	dsynthbling.Init();
	DSettings::LoadSetting(DSettings::DSYNTHFM, DSettings::NONE, "data/fm_bling.xml", &dsynthfm_config);
	dsynthbling.Set(dsynthfm_config);

	// sampler (sampleplayer)
	DSampler::Config dsampler_config;
	dsampler_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dsampler_config.voices = 1;
	dsampler_config.tune = 0.0f;
	dsampler_config.transpose = 0;
	dsampler_config.osc0_level = 1.0;
	dsampler_config.noise_level = 0.0;
	dsampler_config.filter_type = DSampler::LOW;
	dsampler_config.filter_cutoff = 2000.0f;
	dsampler_config.filter_res = 0.2f;
	dsampler_config.eg_p_level = 0.0f;
	dsampler_config.eg_p_attack = 0.0f;
	dsampler_config.eg_p_decay = 0.01f;
	dsampler_config.eg_p_sustain = 1.0f;
	dsampler_config.eg_p_release = 0.5f;
	dsampler_config.eg_f_level = 1.0f;
	dsampler_config.eg_f_attack = 0.9f;
	dsampler_config.eg_f_decay = 0.01f;
	dsampler_config.eg_f_sustain = 1.0f;
	dsampler_config.eg_f_release = 0.0f;
	dsampler_config.eg_a_attack = 5.4f;
	dsampler_config.eg_a_decay = 0.01f;
	dsampler_config.eg_a_sustain = 0.9f;
	dsampler_config.eg_a_release = 5.0f;
	dsampler_config.lfo_waveform = DSampler::WAVE_TRI;
	dsampler_config.lfo_freq = 1.0f;
	dsampler_config.lfo_amp = 1.0f;
	dsampler_config.lfo_p_level = 0.0f;
	dsampler_config.lfo_f_level = 0.0f;
	dsampler_config.lfo_a_level = 0.0f;
	dsampler_config.portamento = 0.0f;
	dsampler_config.delay_delay = 0.3f;
	dsampler_config.delay_feedback = 0.5f;
	dsampler_config.overdrive_gain = 0.0f;
	dsampler_config.overdrive_drive = 0.0f;
	dsampler_config.loop = true;
	dsampler.Init();
	dsampler.Set(dsampler_config);
	// dsampler.Load("data/test.wav", true);
	dsampler.Load("data/goa_bp.wav", true);

	dtexture.Init();
	DSettings::LoadSetting(DSettings::DSYNTHVAR, DSettings::NONE, "data/var_texture.xml", &dsynthvar_config);
	dtexture.Set(dsynthvar_config);

	// slicer on texture
	DFXSlicer::Config dfxslicer_config;
	dfxslicer_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dfxslicer_config.level = 1.0f;
	dfxslicer_config.child = &dtexture;
	dfxslicer_config.record_samples_max = DSTUDIO_SAMPLE_RATE; // 1 sec
	dfxslicer_config.playback_rep_max = 10;
	dfxslicer_config.trig_mode = false;
	dfxslicer.Init();
	dfxslicer.Set(dfxslicer_config);

	// synth melody (dsynthvar)
	dsynthmelody.Init();
	DSettings::LoadSetting(DSettings::DSYNTHVAR, DSettings::NONE, "data/var_melody.xml", &dsynthvar_config);
	dsynthmelody.Set(dsynthvar_config);

	// filter on var shape osc to remove DC offset
	DFXFilter::Config dfxfilter_config;
	dfxfilter_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dfxfilter_config.level = 1.0f;
	dfxfilter_config.filter_type = DFXFilter::HIGH;
	dfxfilter_config.filter_cutoff = 300.0f;
	dfxfilter_config.filter_res = 0.0f;
	dfxfilter_config.child = &dsynthmelody;
	dfxfilter.Init();
	dfxfilter.Set(dfxfilter_config);

	// mixer
	DSound *dmix_synth[MIXER_CHANNELS_MAX];
	float dmix_pan[MIXER_CHANNELS_MAX];
	float dmix_level[MIXER_CHANNELS_MAX];
	float dmix_chorus_level[MIXER_CHANNELS_MAX];
	float dmix_reverb_level[MIXER_CHANNELS_MAX];
	bool dmix_mono[MIXER_CHANNELS_MAX];
	uint8_t dmix_group[MIXER_CHANNELS_MAX];
	DMixer::Config dmix_config;

	dmix_synth[0] = &dsynthpad;
	dmix_pan[0] = 0.5f;
	dmix_level[0] = 0.22;
	dmix_chorus_level[0] = 0.3f;
	dmix_reverb_level[0] = 0.5f;
	dmix_mono[0] = true;
	dmix_group[0] = 0;

	dmix_synth[1] = &dsampler;
	dmix_pan[1] = 0.5f;
	dmix_level[1] = 0.7; //.4
	dmix_chorus_level[1] = 0.2f;
	dmix_reverb_level[1] = 0.7f;
	dmix_mono[1] = false;
	dmix_group[1] = 1;

	dmix_synth[2] = &dfxfilter; // dsynthmelody
	dmix_pan[2] = 0.3f;
	dmix_level[2] = 0.25; //.4
	dmix_chorus_level[2] = 0.2f;
	dmix_reverb_level[2] = 0.7f;
	dmix_mono[2] = false;
	dmix_group[2] = 1;

	dmix_synth[3] = &dsynthkajsa;
	dmix_pan[3] = 0.8f;
	dmix_level[3] = 0.1;
	dmix_chorus_level[3] = 0.3f;
	dmix_reverb_level[3] = 0.4f;
	dmix_mono[3] = true;
	dmix_group[3] = 3;

	dmix_synth[4] = &dsynthspace;
	dmix_pan[4] = 0.1f;
	dmix_level[4] = 0.10;
	dmix_chorus_level[4] = 0.3f;
	dmix_reverb_level[4] = 0.3f;
	dmix_mono[4] = true;
	dmix_group[4] = 4;

	dmix_synth[5] = &dsynthbling;
	dmix_pan[5] = 0.8f;
	dmix_level[5] = 0.05;
	dmix_chorus_level[5] = 0.3f;
	dmix_reverb_level[5] = 0.4f;
	dmix_mono[5] = true;
	dmix_group[5] = 5;

	dmix_synth[6] = &dtexture;
	dmix_pan[6] = 0.4f;
	dmix_level[6] = 0.02;
	dmix_chorus_level[6] = 0.2f;
	dmix_reverb_level[6] = 0.8f;
	dmix_mono[6] = true;
	dmix_group[6] = 6;

	dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dmix_config.channels = 7;
	dmix_config.amp = 1.0f;
	dmix_config.synth = dmix_synth;
	dmix_config.pan = dmix_pan;
	dmix_config.level = dmix_level;
	dmix_config.chorus_level = dmix_chorus_level;
	dmix_config.reverb_level = dmix_reverb_level;
	dmix_config.mono = dmix_mono;
	dmix_config.group = dmix_group;
	dmix_config.chorus_return = 0.6;
	dmix_config.reverb_return = 0.8f;
	dmix_config.mix_dry = 0.2;
	dmixer.Init();
	dmixer.Set(dmix_config);

	// demo start
	dmixer.SetReverb(0.95f, 14000.0f);

	dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, 36, 100);
	dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, 60, 100);
	dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, 48, 100);
	dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 6, 48, 100);

	return retval;
}

//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
	static int sec = 1;
	static int s_melody = 0;
	static int s_kajsa = 0;
	static int n_kajsa = 0;
	static int s_space = 0;

	static int s_bling = 0;
	static int s_bling_c = 5 + dRandom(25);

	// assume ProcessControl() is called once per second
	sec++;

	// melody
	float dice = dRandom(1);
	if (dice > .95)
	{
		switch (s_melody)
		{
		case 0:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, 48 + 3, 100);
			s_melody = (int)dRandom(5);
			break;
		case 1:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, 48 + 1, 100);
			s_melody = (int)dRandom(5);
			break;
		case 2:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, 48 - 2, 100);
			s_melody = (int)dRandom(5);
			break;
		case 3:
		case 4:
			dmixer.Silence(2);
			s_melody = (int)dRandom(5);
			break;
		}
	}

	// kajsa
	dice = dRandom(1);
	if (dice > .95)
	{
		switch (s_kajsa)
		{
		case 0:
			if (dRandom(1) > 0.5)
			{
				n_kajsa = 60 + 7;
				dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 3, n_kajsa, 100);
			}
			else
			{
				n_kajsa = 60;
				dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 3, n_kajsa, 100);
			}
			s_kajsa = (int)dRandom(5);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			if (n_kajsa != 0)
			{
				dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 3, n_kajsa, 0);
				n_kajsa = 0;
			}
			s_kajsa = (int)dRandom(5);
			break;
		}
	}

	// space
	dice = dRandom(1);
	if (dice > .85)
	{
		switch (s_space)
		{
		case 0:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 4, 60 + 8, 100);
			s_space = 1;
			break;
		case 1:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 4, 60 + 7, 100);
			s_space = (int)dRandom(5);
			break;
		case 2:
		case 3:
		case 4:
			dmixer.Silence(4);
			s_melody = (int)dRandom(5);
			break;
		}
	}

	s_bling_c--;
	switch (s_bling)
	{
	case 0:
		if (s_bling_c == 0)
		{
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, 72 + 3, 100);
			s_bling++;
			s_bling_c = 3 + dRandom(5);
		}
		break;
	case 1:
		if (s_bling_c == 0)
		{
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 5, 72 + 3, 0);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, 72 + 2, 100);
			s_bling++;
			s_bling_c = 3 + dRandom(5);
		}
		break;
	case 2:
		if (s_bling_c == 0)
		{
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 5, 72 + 2, 0);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, 72 + 0, 100);
			s_bling++;
			s_bling_c = 3 + dRandom(5);
		}
		break;
	case 3:
		if (s_bling_c == 0)
		{
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 5, 72 + 0, 0);
			s_bling = 0;
			s_bling_c = 13 + dRandom(25);
		}
		break;

	default:
		break;
	}
}

// main

// -l - list audio devices
// -d <dev> - use given audio device
int main(int argc, char *argv[])
{
	int c;
	bool arg_devices_set = false;
	bool arg_devices_list = false;
	char *arg_device = nullptr;

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

	if (retval)
	{
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
				SLEEP(1000); // 10 ms
			}
		}

		// rtAudio cleanup
		ExitRtAudio();
	}
	return 0;
}
