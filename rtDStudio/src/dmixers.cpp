#include "dstudio.h"
#include "dmixers.h"



void DMixerS::Init()
{
	sample_rate_ = DSTUDIO_SAMPLE_RATE;

    reverb_.Init(sample_rate_);

    SetType(DSound::MIXER); // default
}



void DMixerS::Set(const Config& config)
{
	//sample_rate_ = config.sample_rate;
	channels_ = config.channels;

    amp_ = config.amp;

	for (uint8_t i = 0; i < channels_; i++)
	{
		synth_[i] = config.synth[i];
        level_[i] = config.level[i];
		pan_[i] = config.pan[i];
        reverb_level_[i] = config.reverb_level[i];
        mono_[i] = config.mono[i];
        group_[i] = config.group[i];
    }

    reverb_return_ = config.reverb_return;
    mix_dry_ = config.mix_dry;
    
	// reverb
    reverb_feedback_ = 0.4f;
	reverb_lpffreq_ = 6000;
	reverb_.SetFeedback(reverb_feedback_);
	reverb_.SetLpFreq(reverb_lpffreq_);

}



void DMixerS::Process(float *out_left, float *out_right)
{
    float mix_left = 0;
    float mix_right = 0;
    float synth_out;
    float synth_left, synth_right;
    float reverb_send_left = 0;
    float reverb_send_right = 0;
    float reverb_out_left, reverb_out_right;
	
	// voices
	
    for (uint8_t i = 0; i < channels_; i++)
	{
        if (mono_[i])
        {
            // level
            synth_out = synth_[i]->Process() * level_[i];

            // pan
            synth_left = synth_out * (1.0f - pan_[i]);
            synth_right = synth_out * (pan_[i]);
        } else {
            synth_[i]->Process(&synth_left, &synth_right);
            // level and pan
            synth_left = synth_left * level_[i] * (1.0f - pan_[i]);
            synth_right = synth_right * level_[i] * (pan_[i]);
            // chorus needs mono
            synth_out = (synth_left + synth_right) / 2;
        }
        // reverb send
        reverb_send_left += synth_left * reverb_level_[i];
        reverb_send_right += synth_right * reverb_level_[i];
		
		// mix
        mix_left += synth_left;
        mix_right += synth_right;
	}

    // reverb send fx
    reverb_.Process(reverb_send_left, reverb_send_right, &reverb_out_left, &reverb_out_right);

    // add chorus and reverb
    *out_left = (mix_left * mix_dry_+ 
        reverb_out_left * reverb_return_
        ) * amp_;
    *out_right = (mix_right * mix_dry_ 
        + reverb_out_right * reverb_return_ 
        ) * amp_;
}



void DMixerS::MidiIn(uint8_t midi_status, uint8_t midi_data0, uint8_t midi_data1 = 0)
{

    switch (GetType())
    {
    case MIXER:
    {
        // mixer handles pan and level
        // everything else goes to channel
        uint8_t midi_channel = midi_status & MIDI_CHANNEL_MASK;
        if (midi_channel < channels_)
        {
            uint8_t midi_message = midi_status & MIDI_MESSAGE_MASK;
            switch (midi_message)
            {
            case MIDI_MESSAGE_CC:
                switch (midi_data0 & MIDI_DATA_MASK)
                {
                case MIDI_CC_VOLUME:
                    SetLevel(midi_channel, (midi_data1 & MIDI_DATA_MASK) / (float)MIDI_DATA_MAX);
                    break;
                case MIDI_CC_PAN:
                    SetPan(midi_channel, (midi_data1 & MIDI_DATA_MASK) / (float)MIDI_DATA_MAX);
                    break;
                default:
                    synth_[midi_channel]->MidiIn(midi_status, midi_data0, midi_data1);
                    break;
                }
                break;
            default:
                synth_[midi_channel]->MidiIn(midi_status, midi_data0, midi_data1);
                break;
            }
        }

        break;
    }
    case MIXER_SUB:
    {
        // send same message to all channels
        for (uint8_t c = 0; c < channels_; c++)
        {
            synth_[c]->MidiIn(midi_status, midi_data0, midi_data1);
        }
        break;
    }
    case MIXER_PERCUSSION:
    {
        // only deal with note on events
        // send message to channel derived from note value -36
        // ref: https://soundprogramming.net/file-formats/general-midi-drum-note-numbers/
        uint8_t midi_message = midi_status & MIDI_MESSAGE_MASK;
        if (midi_message == MIDI_MESSAGE_NOTEON)
        {
            uint8_t c = (midi_data0 & MIDI_DATA_MASK) - MIDI_PERCUSSION_START;
            if (c < channels_)
            {
                synth_[c]->MidiIn(midi_status, midi_data0, midi_data1);
                // handle sound groups
                // eg hihat, turn off closed when open
                uint8_t group = group_[c];
                for (uint8_t i = 0; i < channels_; i++)
                {
                    if ((i != c) && (group_[i] == group))
                    {
                        synth_[i]->Silence();
                    }
                }
            }
        }
        break;
    }
    default:
        break;
    } // switch GetType()

}



void DMixerS::Silence(uint8_t c)
{
    synth_[c]->Silence();
}



void DMixerS::Silence()
{
    for (uint8_t c = 0; c < channels_; c++)
    {
        synth_[c]->Silence();
    }
}



void DMixerS::SetPan(uint8_t channel, float value)
{
    if (channel < channels_)
    {
        pan_[channel] = value;

    }
}



void DMixerS::SetLevel(uint8_t channel, float value)
{
    if (channel < channels_)
    {
        level_[channel] = value;
    }
}



void DMixerS::SetReverb(float feedback, float lpffreq)
{
    reverb_feedback_ = feedback;
    reverb_lpffreq_ = lpffreq;
    reverb_.SetFeedback(reverb_feedback_);
    reverb_.SetLpFreq(reverb_lpffreq_);
}



void DMixerS::SetReverbLevel(uint8_t channel, float value)
{
    if (channel < channels_)
    {
        reverb_level_[channel] = value;
    }
}



uint8_t DMixerS::GetChannels()
{
    return (channels_);
}



void DMixerS::SetReverbReturn(float reverb_return)
{
    reverb_return_ = reverb_return;
}



void DMixerS::SetMixDry(float mix_dry)
{
    mix_dry_ = mix_dry;
}
