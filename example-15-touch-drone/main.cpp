#include "main.h"

#include <iostream>
#include <cstdlib>
#include <signal.h>

// standard
#include "../rtDaisySP/src/daisysp.h"
#include "../rtDStudio/src/dstudio.h"

// application - DStudio
#include "../rtDStudio/src/dmixer.h"
#include "../rtDStudio/src/dsynthsub.h"
#include "../rtDStudio/src/dsynthvar.h"
#include "../rtDStudio/src/dfx.h"
#include "../rtDStudio/src/dsettings.h"
#include "../rtDStudio/src/dseqmidi.h"

// application - DHaxo
// #include "../rtDStudio/src/dhaxo.h"

// application - DWindow
#include "../rtDStudio/src/dwindow.h"

#include "rt.h"



//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
DSynthSub dsynthd0;
DSynthSub dsynthd1;
DSynthSub dsynthd2;
DSynthVar dsynthd3;
DSynthVar dsynthd4;
DSynthVar dsynthd5;
DMixer dmixer;
DSeqMidi dseqmidi;

// application - DWindow
DWindow dwindow;
DControl dctrl[DWINDOW_CTRLS_MAX];



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

	DSynthSub::Config dsynthsub_config;
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_drone1.xml", &dsynthsub_config);
	dsynthd0.Init();
	dsynthd1.Init();
	dsynthd2.Init();
	DSynthVar::Config dsynthvar_config;
	DSettings::LoadSetting(DSettings::DSYNTHVAR, DSettings::NONE, "data/var_drone1.xml", &dsynthvar_config);
	dsynthd3.Init();
	dsynthd4.Init();
	dsynthd5.Init();
	std::cout << "INFO synth set\n";
	dsynthd0.Set(dsynthsub_config);
	dsynthd1.Set(dsynthsub_config);
	dsynthd2.Set(dsynthsub_config);
	dsynthd3.Set(dsynthvar_config);
	dsynthd4.Set(dsynthvar_config);
	dsynthd5.Set(dsynthvar_config);

	std::cout << "INFO mixer\n";
	// mixer
	DSound *dmix_synth[MIXER_CHANNELS_MAX];
	float dmix_pan[MIXER_CHANNELS_MAX];
	float dmix_level[MIXER_CHANNELS_MAX];
	float dmix_chorus_level[MIXER_CHANNELS_MAX];
	float dmix_reverb_level[MIXER_CHANNELS_MAX];
	bool dmix_mono[MIXER_CHANNELS_MAX];
	uint8_t dmix_group[MIXER_CHANNELS_MAX];
	DMixer::Config dmix_config;

	dmix_synth[0] = &dsynthd0;
	dmix_level[0] = 0.2;
	dmix_pan[0] = 0.5f;
	dmix_chorus_level[0] = 0.0f;
	dmix_reverb_level[0] = 0.2f;
	dmix_mono[0] = true;
	dmix_group[0] = 0;

	dmix_synth[1] = &dsynthd1;
	dmix_level[1] = 0.2;
	dmix_pan[1] = 0.3f;
	dmix_chorus_level[1] = 0.0f;
	dmix_reverb_level[1] = 0.3f;
	dmix_mono[1] = true;
	dmix_group[1] = 1;

	dmix_synth[2] = &dsynthd2;
	dmix_level[2] = 0.2;
	dmix_pan[2] = 0.7f;
	dmix_chorus_level[2] = 0.0f;
	dmix_reverb_level[2] = 0.4f;
	dmix_mono[2] = true;
	dmix_group[2] = 2;

	dmix_synth[3] = &dsynthd3;
	dmix_level[3] = 0.2;
	dmix_pan[3] = 0.2f;
	dmix_chorus_level[3] = 0.0f;
	dmix_reverb_level[3] = 0.4f;
	dmix_mono[3] = true;
	dmix_group[3] = 3;

	dmix_synth[4] = &dsynthd4;
	dmix_level[4] = 0.2;
	dmix_pan[4] = 0.6f;
	dmix_chorus_level[4] = 0.2f;
	dmix_reverb_level[4] = 0.4f;
	dmix_mono[4] = true;
	dmix_group[4] = 4;

	dmix_synth[5] = &dsynthd5;
	dmix_level[5] = 0.2;
	dmix_pan[5] = 0.9f;
	dmix_chorus_level[5] = 0.0f;
	dmix_reverb_level[5] = 0.6f;
	dmix_mono[5] = true;
	dmix_group[5] = 5;

	dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dmix_config.channels = 6;
	dmix_config.amp = 0.1f;
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

	// demo start
	dmixer.SetReverb(0.9f, 2000.0f);

	return retval;
}



//////////////////////////////////////////////////
// Haxophone
//////////////////////////////////////////////////

// init haxo
bool InitHaxo()
{
	bool retval = true;

	/*
	DHaxo::Config dhaxo_config;
	dhaxo_config.synth = &dsynthmelody;
	dhaxo_config.hexo_connected = true;

	// order of hexo_target maps to order of values received from hexo controller
	dhaxo_config.hexo_target[0] = DSynth::DSYNTH_PARAM_TUNE;
	dhaxo_config.hexo_target[1] = DSynth::DSYNTH_PARAM_FILTER_CUTOFF;
	dhaxo_config.hexo_target[2] = DSynth::DSYNTH_PARAM_LFO_FREQ;
	dhaxo.Init(dhaxo_config);
	*/

	return retval;
}



//////////////////////////////////////////////////
// Window
//////////////////////////////////////////////////

void InitCtrlConfig(DControl::Config* config, uint8_t id,
						DControl::ControlType type,
						std::string text,
						const SDL_Rect* pos,
						int min,
						int max,
						int value,
						DWindow* window)
{
	config->id = id;
	config->type = type;
	config->text = text;
	config->pos = pos;
	config->min = min;
	config->max = max;
	config->value = value;
	config->window = window;
}



bool InitWin()
{
	bool retval = true;

	SDL_Rect rect;
	DControl::Config dcontrol_config;

	rect = {0, 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D0_PITCH, DControl::VERTICAL, "D0", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D0_PITCH].Init(dcontrol_config);

	rect = {1 * (DWINDOW_LO_HM + DWINDOW_LO_H_), 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D1_PITCH, DControl::VERTICAL, "D1", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D1_PITCH].Init(dcontrol_config);

	rect = {2 * (DWINDOW_LO_HM + DWINDOW_LO_H_), 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D2_PITCH, DControl::VERTICAL, "D2", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D2_PITCH].Init(dcontrol_config);

	rect = {3 * (DWINDOW_LO_HM + DWINDOW_LO_H_), 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D3_PITCH, DControl::VERTICAL, "DVar0", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D3_PITCH].Init(dcontrol_config);

	rect = {4 * (DWINDOW_LO_HM + DWINDOW_LO_H_), 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D4_PITCH, DControl::VERTICAL, "DVar1", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D4_PITCH].Init(dcontrol_config);

	rect = {5 * (DWINDOW_LO_HM + DWINDOW_LO_H_), 0, DWINDOW_LO_HM, DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D5_PITCH, DControl::VERTICAL, "DVar2", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D5_PITCH].Init(dcontrol_config);

	// ctrl: vol
	rect = {0, DWINDOW_LO_VL + DWINDOW_LO_V_, DWINDOW_LO_HM, DWINDOW_LO_VS}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D0_VOL, DControl::HORIZONTAL, "Vol0", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D0_VOL].Init(dcontrol_config);
	
	// ctrl: pan
	rect = {0, DWINDOW_LO_VL + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_, DWINDOW_LO_HM, DWINDOW_LO_VS}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D0_PAN, DControl::HORIZONTAL, "Pan0", &rect, 0, 100, 50, &dwindow);
	dctrl[DCTRL_D0_PAN].Init(dcontrol_config);

	// ctrl: filter
	rect = {0, DWINDOW_LO_VL + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_, DWINDOW_LO_HM, DWINDOW_LO_VS}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_D0_CUTOFF, DControl::HORIZONTAL, "Cutf0", &rect, 0, 100, 20, &dwindow);
	dctrl[DCTRL_D0_CUTOFF].Init(dcontrol_config);

//	rect = {0, DWINDOW_LO_VL + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_, DWINDOW_LO_HM, DWINDOW_LO_VS}; // x, y, w, h
	rect = {0, 
			DWINDOW_LO_VL + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_ + DWINDOW_LO_VS + DWINDOW_LO_V_ + DWINDOW_LO_VS,
			DWINDOW_LO_HL * 3, 
			DWINDOW_LO_VL}; // x, y, w, h
	InitCtrlConfig(&dcontrol_config, DCTRL_MATRIX0, DControl::MATRIX, "Matrix0", &rect, MATRIX0_LEN, MATRIX0_PITCH, 0, &dwindow);
	dctrl[DCTRL_MATRIX0].Init(dcontrol_config);

	for (int i = 0; i < DCTRLS; i++)
	{
		//if (dctrl[i] != NULL)
		{
			dwindow.AddControl(&dctrl[i]);
		}
	}

	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{

	// dhaxo.ProcessControl();

	// handle changes in DWindow controls

	uint16_t x;

	if (dctrl[DCTRL_D0_PITCH].GetChange(&x))
	{
		dsynthd0.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}
	if (dctrl[DCTRL_D0_VOL].GetChange(&x))
	{
		dsynthd0.ChangeParam(DSynth::DSYNTH_PARAM_AMP, x / 100.0);
	}
	if (dctrl[DCTRL_D0_PAN].GetChange(&x))
	{
		dsynthd0.ChangeParam(DSynth::DSYNTH_PARAM_PAN, x / 100.0);
	}
	if (dctrl[DCTRL_D0_CUTOFF].GetChange(&x))
	{
		dsynthd0.ChangeParam(DSynth::DSYNTH_PARAM_FILTER_CUTOFF, x / 100.0);
	}

	if (dctrl[DCTRL_D1_PITCH].GetChange(&x))
	{
		dsynthd1.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}
	if (dctrl[DCTRL_D2_PITCH].GetChange(&x))
	{
		dsynthd2.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}
	if (dctrl[DCTRL_D3_PITCH].GetChange(&x))
	{
		dsynthd3.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}
	if (dctrl[DCTRL_D4_PITCH].GetChange(&x))
	{
		dsynthd4.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}
	if (dctrl[DCTRL_D5_PITCH].GetChange(&x))
	{
		dsynthd5.ChangeParam(DSynth::DSYNTH_PARAM_FREQ, x);
	}

	if (dctrl[DCTRL_MATRIX0].GetChange(&x))
	{
		// cel x has changed from on to off or the opposite
		// map x to pos and pitch
		int x_pos = x % MATRIX0_LEN;
		int x_pitch = x / MATRIX0_PITCH;
		// and modifiy seq data
		// assume 
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

	// std::cout << "INFO init haxo\n";
	// retval = InitHaxo();

	std::cout << "INFO init ctrl and window\n";
	dwindow.Init();
	retval = InitWin();

	if (retval)
	{
		// run until interrupt with CTRL+C
		done_ = false;
		(void)signal(SIGINT, finish);
		std::cout << "\nPlaying - quit with Ctrl-C.\n";

		// application: start notes
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, 60, 100);
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, 36, 100);
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, 48, 100);
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 3, 60, 100);
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 4, 36, 100);
		dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, 48, 100);

		// main application loop
		while (!done_ && rt_dac_.isStreamRunning())
		{
			done_ |= dwindow.Event();

			ProcessControl();
		}
	}

	// rtAudio cleanup
	ExitRtAudio();
	
	return 0;
}
