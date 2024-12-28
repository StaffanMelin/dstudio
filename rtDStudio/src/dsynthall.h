#pragma once

#include "dstudio.h"
#include "dsound.h"
#include "dsynthsub.h"
#include "dsynthvar.h"
#include "dsynthfm.h"
#include "dsampler.h"
#include "dsettings.h"

class DSynthAll : public DSynth
{

	public:

    DSynthAll() {}
    ~DSynthAll() {}

    struct Config
    {
    	float sample_rate;
    };

	void Init();
	void Set(const Config&);
	float Process();
    void Process(float *, float *);
    void MidiIn(uint8_t, uint8_t, uint8_t);
    void NoteOn(uint8_t midi_note, uint8_t midi_velocity = MIDI_VELOCITY_MAX);
	void NoteOff(uint8_t midi_note);

    void Silence();
    void SetWaveform(Waveform, Waveform);
    void SetTuning(float, float);
    void SetTranspose(uint8_t);
    void SetLevel(float, float, float);
    void SetFilter(FilterType, float, float);
    void SetFilterFreq(float);
    void SetFilterRes(float);
    void SetEGLevel(Target, float);
    void SetEG(Target, float, float, float, float);
    void SetLFO(Waveform, float, float, float, float, float);
    void SetPortamento(float);
    void SetDelay(float, float);
    void SetOverdrive(float, float);

    void SetLevel(float);
    void ChangeParam(DSynth::Param param, float value);
    
private:

	float sample_rate_;

    Config base_config_;

    DSettings::DSoundType synth_type;
	DSynthSub dsynthsub;
	DSynthVar dsynthvar;
    DSynthFm dsynthfm;
    DSampler dsampler;
};
