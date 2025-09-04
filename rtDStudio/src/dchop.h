#pragma once

// #include <string>

#include "dstudio.h"
#include "dsound.h"
#include "dsynth.h"

#include <sndfile.h>

// base freq
#define DCHOP_BASE_FREQ 261.6f // MIDI note 60
// max sample time in seconds
#define DCHOP_SAMPLE_TIME_MAX 30
#define DCHOP_SAMPLE_BUFFER_MAX (DSTUDIO_SAMPLE_RATE * DCHOP_SAMPLE_TIME_MAX * 2) // 60 secs; 48k * 2 * 4 = 384k/s
#define DCHOP_CHOPS 8

class DChop : public DSynth
{

public:
    DChop()
    {
        sample_buffer_ = new (std::nothrow) float[DCHOP_SAMPLE_BUFFER_MAX * 2];
    }

    ~DChop()
    {
        delete sample_buffer_;
    }

    struct Config
    {
        char settings_name[DSTUDIO_SETTINGS_NAME_MAX];
        float sample_rate;
        uint8_t chops[DCHOP_CHOPS];
        uint8_t chops_notes[DCHOP_CHOPS];
        float chop_gate;    // percent of interval that note should be on (0.0 - 1.0)
        bool mode_internal; // if true, switch to next chop when chop sample data end is reached; if not, continue until sample end
        float tune;
        FilterType filter_type;
        float filter_res;
        float filter_cutoff;
        float eg_a_attack;
        float eg_a_decay;
        float eg_a_sustain;
        float eg_a_release;
        bool eg_retrig;
        Waveform lfo_waveform;
        float lfo_freq;
        float lfo_amp;
        float lfo_p_level;
        float lfo_f_level;
        float lfo_a_level;
        float delay_delay;
        float delay_feedback;
        float overdrive_gain;
        float overdrive_drive;
        bool loop;
        std::string sample_file_name;
        uint32_t sample_phase_start[DCHOP_CHOPS];
        uint32_t sample_phase_end[DCHOP_CHOPS];
        uint32_t sample_length;
        uint8_t sample_channels;
    };

    void Init();
    void Set(const Config &);
    void Process(float *, float *);
    void Calc();
    void MidiIn(uint8_t midi_status, uint8_t midi_data0, uint8_t midi_data1 = 0);
    void NoteOn(uint8_t chop, uint8_t midi_note, uint8_t midi_velocity);
    void NoteOn(uint8_t chop, uint8_t midi_velocity);
    void NoteOff(uint8_t chop, uint8_t midi_velocity);

    void SetMode(bool mode);
    void SetLoop(bool loop);
    void SetChopGate(float chop_gate);
    void SetFreq(float);
    void SetTuning(float);
    void SetFilter(FilterType, float, float);
    void SetFilterFreq(float);
    void SetFilterRes(float);
    void SetEG(Target, float, float, float, float);
    void SetEGRetrig(bool retrig);
    void SetLFO(Waveform, float, float, float, float, float);
    void SetDelay(float, float);
    void SetOverdrive(float, float);
    bool Load(std::string, bool reset = true);
    // void SetPhase(uint8_t chop, uint32_t, uint32_t);
    void SetChopNote(uint8_t chop, uint8_t note);
    uint8_t GetChopNote(uint8_t chop);
    void SetChopStart(uint8_t chop, uint32_t pos);
    uint32_t GetChopStart(uint8_t chop);
    uint32_t GetSampleLength();
    float *GetSampleData();
    uint8_t GetSampleChannels();
    // uint32_t *GetSamplePhaseStart();
    // void ChangeParam(DSynth::Param param, float value);

    Config base_config_;

private:
    char settings_name_[DSTUDIO_SETTINGS_NAME_MAX];
    float sample_rate_;
    uint8_t chops_[DCHOP_CHOPS];
    uint8_t chops_notes_[DCHOP_CHOPS];
    float chop_gate_;
    float mode_internal_;
    float tune_;
    FilterType filter_type_;
    float filter_res_;
    float filter_cutoff_;
    float eg_a_attack_;
    float eg_a_decay_;
    float eg_a_sustain_; // level
    float eg_a_release_;
    bool eg_retrig_;
    Waveform lfo_waveform_;
    float lfo_freq_;
    float lfo_amp_;
    float lfo_p_level_;
    float lfo_f_level_;
    float lfo_a_level_;
    float delay_delay_;
    float delay_feedback_;
    float overdrive_gain_;
    float overdrive_drive_;
    bool loop_;
    std::string sample_file_name_;
    uint32_t sample_phase_start_[DCHOP_CHOPS];
    uint32_t sample_phase_end_[DCHOP_CHOPS];
    uint32_t sample_length_;  // set when loading sample; length of sample, < BUFFER_MAX
    uint8_t sample_channels_; // set when loading sample

    uint8_t chop_step_;

    // MIDI
    uint8_t note_midi_;
    float note_freq_;
    float note_velocity_;
    uint32_t sample_phase_gate_;

    // sampler
    float *sample_buffer_ = NULL;

    // sample info
    // sssssssssssssssssssss
    // start lstart lend end
    // gate--<--------->
    // sample - runtime
    float sample_index_;        // index into buffer
    float sample_index_factor_; // how much to advance index for a new sample

    daisysp::WhiteNoise noise_;

    // objects
    daisysp::Adsr eg_a_;
    daisysp::Svf svf_l_;
    daisysp::Svf svf_r_;
    daisysp::Oscillator lfo_;

    daisysp::DelayLine<float, DSYNTH_DELAY_MAX> delay_l_;
    daisysp::DelayLine<float, DSYNTH_DELAY_MAX> delay_r_;
    daisysp::Overdrive overdrive_;
};
