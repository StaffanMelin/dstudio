#pragma once

//#include <string>

#include "dstudio.h"
#include "dsound.h"
#include "dsynth.h"

#include <sndfile.h>

// base freq
#define DHITS_BASE_FREQ 440.0f
// max sample time in seconds
#define DHITS_SAMPLE_TIME_MAX 5
#define DHITS_SAMPLE_BUFFER_MAX (DSTUDIO_SAMPLE_RATE * DHITS_SAMPLE_TIME_MAX)
#define DHITS_HITS_MAX 8

class DHits : public DSynth
{

public:

    DHits()
    {
        for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
        {
            sample_buffer_[i] = new (std::nothrow) float[DHITS_SAMPLE_BUFFER_MAX];
        }

    }

    ~DHits()
    {
        for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
        {
        delete sample_buffer_[i];
    }
}

    struct Config
    {
        float sample_rate;
        float level[DHITS_HITS_MAX];
        float pan[DHITS_HITS_MAX];
        float tune[DHITS_HITS_MAX];
        float eg_a_attack[DHITS_HITS_MAX];
        float eg_a_decay[DHITS_HITS_MAX];
        float eg_a_sustain[DHITS_HITS_MAX];
        float eg_a_release[DHITS_HITS_MAX];
        float delay_delay;
        float delay_feedback;
        float overdrive_gain;
        float overdrive_drive;
        std::string sample_file_name[DHITS_HITS_MAX];
        uint32_t sample_phase_start[DHITS_HITS_MAX];
        uint32_t sample_phase_end[DHITS_HITS_MAX];
        uint32_t sample_length[DHITS_HITS_MAX];
    };

    void Init();
    void Set(const Config&);
    void Process(float *, float *);
    void MidiIn(uint8_t, uint8_t, uint8_t);
    void NoteOn(uint8_t midi_note, uint8_t midi_velocity = MIDI_VELOCITY_MAX);
    void NoteOff(uint8_t midi_note);
    void Silence();
    void SetLevel(uint8_t hit, float level);
    void SetPan(uint8_t hit, float pan);
    void SetTune(uint8_t tune);
    void SetFreq(uint8_t hit, float freq);
    void SetEG(uint8_t hit, float, float, float, float);
    void SetDelay(float, float);
    void SetOverdrive(float, float);
    bool Load(uint8_t hit, std::string, bool reset = true);
    void GetPhase(uint8_t hit, uint32_t *, uint32_t *);
    void SetPhase(uint8_t hit, uint32_t, uint32_t);
    uint32_t GetLength(uint8_t hit);

private:

    float sample_rate_;
    float level_[DHITS_HITS_MAX];
    float pan_[DHITS_HITS_MAX];
    float tune_[DHITS_HITS_MAX];
    float eg_a_attack_[DHITS_HITS_MAX];
    float eg_a_decay_[DHITS_HITS_MAX];
    float eg_a_sustain_[DHITS_HITS_MAX];
    float eg_a_release_[DHITS_HITS_MAX];
    float delay_delay_;
    float delay_feedback_;
    float overdrive_gain_;
    float overdrive_drive_;
    std::string sample_file_name_[DHITS_HITS_MAX];
    uint32_t sample_phase_start_[DHITS_HITS_MAX];
    uint32_t sample_phase_end_[DHITS_HITS_MAX]; // if 0, set to sample_length_ - 1 when loading sample
    uint32_t sample_length_[DHITS_HITS_MAX]; // set when loading sample; length of sample, < BUFFER_MAX

    Config base_config_;

    // MIDI
    float note_freq_[DHITS_HITS_MAX];
    float note_velocity_[DHITS_HITS_MAX];

    // sampler
    float *sample_buffer_[DHITS_HITS_MAX];

    // sample info
    // sssssssssssssssssssss
    // start lstart lend end
    // gate--<--------->
    // sample - runtime
    float sample_index_[DHITS_HITS_MAX]; // index into buffer
    float sample_index_factor_[DHITS_HITS_MAX]; // how much to advance index for a new sample

    // objects
    daisysp::Adsr eg_a_[DHITS_HITS_MAX];

    // "global" fx
    daisysp::DelayLine<float, DSYNTH_DELAY_MAX> delay_l_;
    daisysp::DelayLine<float, DSYNTH_DELAY_MAX> delay_r_;
    daisysp::Overdrive overdrive_;
};
