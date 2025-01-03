#pragma once

#include "../rtaudio/RtAudio.h"
#include "../rtDStudio/src/dsound.h"

// rtaudio

// DaisySP works with 32 bit floats
typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
// rtAudio data buffer
double *rt_data_ = NULL;
// rtAudio DAC
RtAudio rt_dac_(RtAudio::LINUX_ALSA);
// rtAudio output object called in AudioCallback()
DSound *dout_;

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

		std::cout << "Stream open. Latency: " << rt_dac_.getStreamLatency() << "\n";

		// start stream
		if (rt_dac_.startStream())
		{
			retval = false;
		}
	}
	return (retval);
}



void ExitRtAudio()
{
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
}
