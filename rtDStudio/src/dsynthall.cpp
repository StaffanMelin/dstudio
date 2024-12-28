#include "dsynthall.h"



void DSynthAll::Init()
{
	synth_type = DSettings::DSYNTHSUB;
    dsynthsub.Init();
    dsynthvar.Init();
    dsynthfm.Init();
    dsampler.Init();

    SetType(TUNED);
}   



void DSynthAll::Set(const Config& config)
{
    base_config_ = config;

    // don't allow manual settings
}   




float DSynthAll::Process()
{
    switch (synth_type)
    {
    case DSettings::DSAMPLER:
        return dsampler.Process();
        break;
    case DSettings::DSYNTHFM:
        return dsynthfm.Process();
        break;
    case DSettings::DSYNTHSUB:
        return dsynthsub.Process();
        break;
    case DSettings::DSYNTHVAR:
        return dsynthvar.Process();
        break;
    }
}



void DSynthAll::Process(float *out_l, float *out_r)
{
    *out_l = Process();
    *out_r = *out_l;
}



void DSynthAll::MidiIn(uint8_t midi_status, uint8_t midi_data0, uint8_t midi_data1 = 0)
{
}



void DSynthAll::NoteOn(uint8_t midi_note, uint8_t midi_velocity)
{
}



void DSynthAll::NoteOff(uint8_t midi_note)
{
	for (uint8_t i = 0; i < voices_; i++)
	{
        if (note_midi_[i] == midi_note + transpose_)
		{
			note_midi_[i] = 0;
		}
	}
}	

void DSynthAll::Silence()
{
    for (uint8_t i = 0; i < voices_; i++)
    {
        note_midi_[i] = 0;
        note_velocity_[i] = 0;
    }
}

void DSynthAll::SetWaveform(Waveform waveform0, Waveform waveform1)
{
    waveform0_ = waveform0;
    waveform1_ = waveform1;

    for (uint8_t i = 0; i < voices_; i++)
    {
        osc0_[i].SetWaveform(waveform0_);
        osc1_[i].SetWaveform(waveform1_);
    }
}

void DSynthAll::SetTuning(float tune, float detune)
{
    tune_ = tune;
    detune_ = detune;
}



void DSynthAll::SetTranspose(uint8_t transpose)
{
    transpose_ = transpose;
}



void DSynthAll::SetLevel(float osc0_level, float osc1_level, float noise_level)
{
    osc0_level_ = osc0_level;
    osc1_level_ = osc1_level;
    noise_level_ = noise_level;
}



void DSynthAll::SetFilter(FilterType filter_type, float filter_cutoff, float filter_res)
{
    filter_type_ = filter_type;
    filter_cutoff_ = filter_cutoff;
    filter_res_ = filter_res;

    for (uint8_t i = 0; i < voices_; i++)
    {
        svf_[i].SetFreq(filter_cutoff_);
        svf_[i].SetRes(filter_res_);
    }
}

void DSynthAll::SetFilterFreq(float filter_cutoff)
{
    filter_cutoff_ = filter_cutoff;

    for (uint8_t i = 0; i < voices_; i++)
    {
        svf_[i].SetFreq(filter_cutoff_);
    }
}

void DSynthAll::SetFilterRes(float filter_res)
{
    filter_res_ = filter_res;

    for (uint8_t i = 0; i < voices_; i++)
    {
        svf_[i].SetRes(filter_res_);
    }
}



void DSynthAll::SetEGLevel(Target target, float level)
{
    switch (target)
    {
        case PITCH:
            eg_p_level_ = level;
            break;
        case FILTER:
            eg_f_level_ = level;
            break;
        default:
            break;
    }
}



void DSynthAll::SetEG(Target target, float eg_attack, float eg_decay, float eg_sustain, float eg_release)
{
    switch (target)
    {
        case PITCH:
            eg_p_attack_ = eg_attack;
            eg_p_decay_ = eg_decay;
            eg_p_sustain_ = eg_sustain;
            eg_p_release_ = eg_release;
            for (uint8_t i = 0; i < voices_; i++)
            {
                eg_p_[i].SetTime(daisysp::ADSR_SEG_ATTACK, eg_p_attack_);
                eg_p_[i].SetTime(daisysp::ADSR_SEG_DECAY, eg_p_decay_);
                eg_p_[i].SetTime(daisysp::ADSR_SEG_RELEASE, eg_p_release_);
                eg_p_[i].SetSustainLevel(eg_p_sustain_);
            }
            break;
        case FILTER:
            eg_f_attack_ = eg_attack;
            eg_f_decay_ = eg_decay;
            eg_f_sustain_ = eg_sustain;
            eg_f_release_ = eg_release;
            for (uint8_t i = 0; i < voices_; i++)
            {
                eg_f_[i].SetTime(daisysp::ADSR_SEG_ATTACK, eg_f_attack_);
                eg_f_[i].SetTime(daisysp::ADSR_SEG_DECAY, eg_f_decay_);
                eg_f_[i].SetTime(daisysp::ADSR_SEG_RELEASE, eg_f_release_);
                eg_f_[i].SetSustainLevel(eg_f_sustain_);
            }
            break;
        case AMP:
            eg_a_attack_ = eg_attack;
            eg_a_decay_ = eg_decay;
            eg_a_sustain_ = eg_sustain;
            eg_a_release_ = eg_release;
            for (uint8_t i = 0; i < voices_; i++)
            {
                eg_a_[i].SetTime(daisysp::ADSR_SEG_ATTACK, eg_a_attack_);
//                eg_a_[i].SetAttackTime(eg_a_attack_, 1.0f);
                eg_a_[i].SetTime(daisysp::ADSR_SEG_DECAY, eg_a_decay_);
                eg_a_[i].SetTime(daisysp::ADSR_SEG_RELEASE, eg_a_release_);
                eg_a_[i].SetSustainLevel(eg_a_sustain_);
            }
            break;
        default:
            break;
    }
}



void DSynthAll::SetLFO(Waveform lfo_waveform, float lfo_freq, float lfo_amp, float lfo_p_level, float lfo_f_level, float lfo_a_level)
{
    lfo_waveform_ = lfo_waveform;
    lfo_freq_ = lfo_freq;
    lfo_amp_ = lfo_amp;
    lfo_p_level_ = lfo_p_level;
    lfo_f_level_ = lfo_f_level;
    lfo_a_level_ = lfo_a_level;
    lfo_.SetWaveform(lfo_waveform_);
    lfo_.SetFreq(lfo_freq_);
    lfo_.SetAmp(lfo_amp_);
}



void DSynthAll::SetPortamento(float portamento)
{
    portamento_ = portamento;
    for (uint8_t i = 0; i < voices_; i++)
    {
        port_[i].SetHtime(portamento_);
    }
}



void DSynthAll::SetDelay(float delay_delay, float delay_feedback)
{
    delay_delay_ = delay_delay;
    delay_feedback_ = delay_feedback;
    delay_.SetDelay(sample_rate_ * delay_delay_);
}



void DSynthAll::SetOverdrive(float overdrive_gain, float overdrive_drive)
{
    overdrive_gain_ = overdrive_gain;
    overdrive_drive_ = overdrive_drive;
    overdrive_.SetDrive(overdrive_drive_);
}

/*
    value goes from 0 to +1.0
*/
void DSynthAll::ChangeParam(DSynth::Param param, float value)
{
    switch (param)
    {
        case DSynth::DSYNTH_PARAM_AMP:
            SetLevel(base_config_.osc0_level * value,
                    base_config_.osc1_level * value,
                    base_config_.noise_level * value);
            break;
        case DSynth::DSYNTH_PARAM_DELAY_FEEDBACK:
            delay_feedback_ = base_config_.delay_feedback * value;
            break;
        case DSynth::DSYNTH_PARAM_DELAY_FREQ:
            SetDelay(base_config_.delay_delay * value, delay_feedback_);
            break;
        case DSynth::DSYNTH_PARAM_DETUNE:
            SetTuning(tune_, base_config_.detune * value);
            break;
        case DSynth::DSYNTH_PARAM_FILTER_CUTOFF:
            //if (value > .1f)
            SetFilterFreq(base_config_.filter_cutoff * value);
            break;
        case DSynth::DSYNTH_PARAM_FILTER_RES:
            SetFilterRes(base_config_.filter_res * value);
            break;
        case DSynth::DSYNTH_PARAM_LFO_AMP:
            lfo_amp_ = base_config_.lfo_amp * value;
            lfo_.SetAmp(lfo_amp_);
            break;
        case DSynth::DSYNTH_PARAM_LFO_FREQ:
            lfo_freq_ = base_config_.lfo_freq * value;
            lfo_.SetFreq(lfo_freq_);
            break;
        case DSynth::DSYNTH_PARAM_OVERDRIVE:
            SetOverdrive(overdrive_gain_, base_config_.overdrive_drive + value);
            break;
        case DSynth::DSYNTH_PARAM_TRANSPOSE:
            SetTranspose(base_config_.transpose * value);
            break;
        case DSynth::DSYNTH_PARAM_TUNE:
            SetTuning(base_config_.tune * value, detune_);
            break;
        case DSynth::DSYNTH_PARAM_FREQ:
            note_freq_[0] = value;
            break;
    }   

}



void DSynthAll::SetLevel(float level)
{
    osc0_level_ = base_config_.osc0_level * level;
    osc1_level_ = base_config_.osc0_level * level;
    noise_level_ = base_config_.noise_level * level;
}
