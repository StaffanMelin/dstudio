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
#include "../rtDStudio/src/dsynthfm.h"
#include "../rtDStudio/src/dsynthvar.h"
#include "../rtDStudio/src/dfx.h"
#include "../rtDStudio/src/dgen.h"
#include "../rtDStudio/src/dsettings.h"

#include "../rtDStudio/src/drt.h"



//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
DSynthSub dsynthbass;
DSynthSub dsynthhi;
DSynthSub dsynthpad;
DSynthVar dsynthmelody;
DSynthSub dsyntharp;
DSynthFm dsynthembellish;
DSynthSub dsynthfill;

DFXFlanger dfxflanger;
DFXFilter dfxfilter;

DMixer dmixer;

DGenDrone dgen;



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

    // synth bass
    dsynthbass.Init();
    DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_bass.xml", &dsynth_config);
    dsynthbass.Set(dsynth_config);

    // synth hi
    dsynthhi.Init();
    DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_hi.xml", &dsynth_config);
    dsynthhi.Set(dsynth_config);

    // synth pad (dsynthsub)
    dsynthpad.Init();
    DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_pad.xml", &dsynth_config);
    dsynthpad.Set(dsynth_config);

    // synth filler (dsynthsub)
    dsynthfill.Init();
    DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_filler.xml", &dsynth_config);
    dsynthfill.Set(dsynth_config);

    // synth melody (dsynthvar)
    DSynthVar::Config dsynthvar_config;
    dsynthmelody.Init();
    DSettings::LoadSetting(DSettings::DSYNTHVAR, DSettings::NONE, "data/var_melody.xml", &dsynthvar_config);
    dsynthmelody.Set(dsynthvar_config);

    // synth arp
    dsyntharp.Init();
    DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_arp.xml", &dsynth_config);
    dsyntharp.Set(dsynth_config);

    // synth embellish
    DSynthFm::Config dsynthfm_config;
    dsynthembellish.Init();
    DSettings::LoadSetting(DSettings::DSYNTHFM, DSettings::NONE, "data/fm_embellish.xml", &dsynthfm_config);
    dsynthembellish.Set(dsynthfm_config);

    // filter on var shape osc to remove DC offset
    DFXFilter::Config dfxfilter_config;
    dfxfilter_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dfxfilter_config.level = 1.0f;
    dfxfilter_config.filter_type = DFXFilter::HIGH;
    dfxfilter_config.filter_cutoff = 200.0f;
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

    dmix_synth[0] = &dsynthbass;
    dmix_synth[1] = &dsynthhi;
    dmix_synth[2] = &dsynthpad;
    dmix_synth[3] = &dfxfilter; // dsynthmelody
    dmix_synth[4] = &dsyntharp;
    dmix_synth[5] = &dsynthembellish;
    dmix_synth[6] = &dsynthfill;
    dmix_pan[0] = 0.5f;
    dmix_pan[1] = 0.12f;
    dmix_pan[2] = 0.8f;
    dmix_pan[3] = 0.35f;
    dmix_pan[4] = 0.75f;
    dmix_pan[5] = 0.3f;
    dmix_pan[6] = 0.7f;
    dmix_level[0] = 0.4;
    dmix_level[1] = 0.22;
    dmix_level[2] = 0.4;
    dmix_level[3] = 0.5;
    dmix_level[4] = 0.65;
    dmix_level[5] = 0.3;//.3
    dmix_level[6] = 0.35;//.35
    dmix_chorus_level[0] = 0.2f;
    dmix_chorus_level[1] = 0.5f;
    dmix_chorus_level[2] = 0.5f;
    dmix_chorus_level[3] = 0.2f;
    dmix_chorus_level[4] = 0.0f;
    dmix_chorus_level[5] = 0.3f;
    dmix_chorus_level[6] = 0.7f;
    dmix_reverb_level[0] = 0.5f;
    dmix_reverb_level[1] = 0.5f;
    dmix_reverb_level[2] = 0.5f;
    dmix_reverb_level[3] = 0.7f;
    dmix_reverb_level[4] = 0.5f;
    dmix_reverb_level[5] = 0.5f;
    dmix_reverb_level[6] = 0.7f;
    dmix_mono[0] = true;
    dmix_mono[1] = true;
    dmix_mono[2] = true;
    dmix_mono[3] = false;
    dmix_mono[4] = true;
    dmix_mono[5] = true;
    dmix_mono[6] = true;
    dmix_group[0] = 0;
    dmix_group[1] = 1;
    dmix_group[2] = 2;
    dmix_group[3] = 3;
    dmix_group[4] = 4;
    dmix_group[5] = 5;
    dmix_group[6] = 6;
    dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dmix_config.channels = 6;
    dmix_config.amp = 1.0f;
    dmix_config.synth = dmix_synth;
    dmix_config.pan = dmix_pan;
    dmix_config.level = dmix_level;
    dmix_config.chorus_level = dmix_chorus_level;
    dmix_config.reverb_level = dmix_reverb_level;
    dmix_config.mono = dmix_mono;
    dmix_config.group = dmix_group;
    dmix_config.chorus_return = 0.5;
    dmix_config.reverb_return = 0.8f;
    dmix_config.mix_dry = 0.15;
    dmixer.Init();
    dmixer.Set(dmix_config);

    // demo start
    dmixer.SetReverb(0.94f, 12000.0f);

    // gen note creation

    DGenDrone::ChannelType dgen_channel_type[MIXER_CHANNELS_MAX];
    dgen_channel_type[0] = DGenDrone::BASS;
    dgen_channel_type[1] = DGenDrone::TREBLE;
    dgen_channel_type[2] = DGenDrone::PAD;
    dgen_channel_type[3] = DGenDrone::MELODY;
    dgen_channel_type[4] = DGenDrone::ARPEGGIO;
    dgen_channel_type[5] = DGenDrone::EMBELLISH;
    dgen_channel_type[6] = DGenDrone::BASS;
    float dgen_drama_fade[MIXER_CHANNELS_MAX];
    dgen_drama_fade[0] = 0.1f;
    dgen_drama_fade[1] = 0.0f;
    dgen_drama_fade[2] = 0.1f;
    dgen_drama_fade[3] = 0.0f;
    dgen_drama_fade[4] = 0.1f;
    dgen_drama_fade[5] = 0.0f;
    dgen_drama_fade[6] = 0.2f;

    dgen_note_t dgen_note_base = {24, 60, 48, 48, 72, 72, 36}; // base MIDI note of each channel; same length as number of channels
    dgen_note_t dgen_note_pad = {0, 4, 7, 10}; // relative to base note
    dgen_note_t dgen_note_arp = {0, 7, 7, 2}; // relative to base note
    dgen_note_t dgen_note_melody = {0, 2, 5, 4, 5, 9, 7, 9}; // relative to base note

    DGenDrone::Config dgen_config;
    dgen_config.bpm = 60;
    dgen_config.channels = 7;
    dgen_config.child = &dmixer;
    dgen_config.channel_type = dgen_channel_type;
    dgen_config.drama_fade = dgen_drama_fade;
    dgen_config.level = dmix_level;
    dgen_config.note_base = dgen_note_base;
    dgen_config.note_pad = dgen_note_pad;
    dgen_config.note_arp = dgen_note_arp;
    dgen_config.note_melody = dgen_note_melody;
    dgen.Init();
    dgen.Set(dgen_config);

	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
    dgen.Process();
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

        dgen.Start(DGen::VERSE);
        
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
