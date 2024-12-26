#include "main.h"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <getopt.h>

// standard
#include "../rtaudio/RtAudio.h"
#include "../rtDaisySP/src/daisysp.h"
#include "../rtDStudio/src/dstudio.h"

// application - DStudio
#include "../rtDStudio/src/dmixer.h"
#include "../rtDStudio/src/dsynthsub.h"
#include "../rtDStudio/src/dhaxo.h"
#include "../rtDStudio/src/dsettings.h"



//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;
DSound *dout_;

// rtAudio data buffer
double *rt_data_ = NULL;
// DAC
RtAudio rt_dac_(RtAudio::LINUX_ALSA);

// application - DStudio
DSynthSub dsynthmelody;
DMixer dmixer;
DHaxo dhaxo;



//////////////////////////////////////////////////
// util
//////////////////////////////////////////////////

// Interrupt handler function
static void finish(int /*ignore*/)
{
	done_ = true;
}



//////////////////////////////////////////////////
// Audio
//////////////////////////////////////////////////

// Audio callback, interleaved
int AudioCallback(
	void *output_buffer,
	void * /*inputBuffer*/,
	unsigned int frame_count,
	double stream_time,
	RtAudioStreamStatus status,
	void *data_)
{
	MY_TYPE *buffer = (MY_TYPE *)output_buffer;

	if (status)
		std::cout << "Stream underflow detected!" << std::endl;

	for (size_t i = 0; i < frame_count; i++)
	{
		MY_TYPE sigL, sigR;
		dout_->Process(&sigL, &sigR);
		*buffer = sigL;
		buffer++;
		*buffer = sigR;
		buffer++;
	}

	return 0;
}

bool InitRtAudio(bool arg_devices_list,
				bool arg_devices_set,
			    char* arg_device)
{
	bool retval = true;

	float sample_rate = DSTUDIO_SAMPLE_RATE;
	unsigned int rt_device = 0;
	unsigned int rt_channels = 2;
	unsigned int rt_buffer_frames = DSTUDIO_BUFFER_SIZE;

	RtAudio::StreamParameters rt_params;
	RtAudio::StreamOptions rt_options;
	RtAudio::DeviceInfo rt_info;

	// output all messages
	rt_dac_.showWarnings(true);

	// setup device
	std::vector<unsigned int> deviceIds = rt_dac_.getDeviceIds();
	if (deviceIds.size() < 1)
	{
		std::cout << "RTAUDIO: ERROR - No audio devices found!\n";
		retval = false;
	}

	if (retval)
	{
		std::cout << "RTAUDIO: Found " << deviceIds.size() << " device(s).\n";
		std::cout << "RTAUDIO: API: " << RtAudio::getApiDisplayName(rt_dac_.getCurrentApi()) << "\n";

		for (unsigned int i = 0; i < deviceIds.size(); i++)
		{
			rt_info = rt_dac_.getDeviceInfo(deviceIds[i]);

			if (arg_devices_set)
			{
				if (rt_info.name.find(arg_device) != std::string::npos)
				{
					rt_device = deviceIds[i];
				}
			}
			// list device information
			if (arg_devices_list)
			{
				std::cout << "RTAUDIO: Device Name " << rt_info.name << "\n";
				std::cout << "RTAUDIO: Device ID " << deviceIds[i] << "\n";
			}
		}
		// set output device
		if (rt_device == 0)
		{
			rt_device = rt_dac_.getDefaultOutputDevice();
			std::cout << "RTAUDIO: Get Device " << rt_device << "\n";
		}
		std::cout << "RTAUDIO: Device set to " << rt_device << "\n";
		rt_params.deviceId = rt_device;
		rt_params.nChannels = rt_channels;
		//rt_params.firstChannel = rt_offset;
		rt_params.firstChannel = 0;
		rt_options.flags = RTAUDIO_SCHEDULE_REALTIME;
		rt_options.numberOfBuffers = DSTUDIO_NUM_BUFFERS;
		rt_options.priority = 1;
		rt_data_ = (double *)calloc(rt_channels * rt_buffer_frames, sizeof(double));
		// open stream
		if (rt_dac_.openStream(&rt_params, // output
							   NULL,	   // input
							   FORMAT,	   // sample data format
							   sample_rate,
							   &rt_buffer_frames, // buffer size in frames
							   &AudioCallback,
							   (void *)rt_data_,
							   &rt_options)) // flags and number of buffers
		{
			retval = false;
		}

		std::cout << "Stream open." << std::endl;
		std::cout << "Stream latency = " << rt_dac_.getStreamLatency() << "\n";

		// start stream
		if (rt_dac_.startStream())
		{
			retval = false;
		}
	}
	return (retval);
}



//////////////////////////////////////////////////
// DStudio
//////////////////////////////////////////////////

// init synths
bool InitSynths()
{
	bool retval = true;

	// synth melody
	DSynthSub::Config dsynthsub_config;
	dsynthmelody.Init();
	DSettings::LoadSetting(DSettings::DSYNTHSUB, DSettings::NONE, "data/sub_melody.xml", &dsynthsub_config);
	dsynthmelody.Set(dsynthsub_config);

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
	dmix_level[0] = 0.6;
	dmix_pan[0] = 0.5f;
	dmix_chorus_level[0] = 0.2f;
	dmix_reverb_level[0] = 0.5f;
	dmix_mono[0] = true;
	dmix_group[0] = 0;
	dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
	dmix_config.channels = 1;
	dmix_config.amp = 1.0f;
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
	dhaxo_config.controller = false;
	dhaxo_config.controller_targets = 3;
	dhaxo_config.controller_target[0] = DSynth::DSYNTH_PARAM_TUNE; 
	dhaxo_config.controller_target[1] = DSynth::DSYNTH_PARAM_FILTER_CUTOFF; 
	dhaxo_config.controller_target[2] = DSynth::DSYNTH_PARAM_LFO_FREQ;
	dhaxo_config.synth = &dsynthmelody;
	dhaxo.Init();
	dhaxo.Set(dhaxo_config);

	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
    dhaxo.Process();
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
	if (rt_dac_.isStreamRunning())
	{
		rt_dac_.stopStream();
	}
	if (rt_dac_.isStreamOpen())
	{
		rt_dac_.closeStream();
	}

	if (rt_data_ != NULL)
		free(rt_data_);

	return 0;
}
