#include <string>
#include <iostream>

#include "dchop.h"

void DChop::Init()
{
    sample_rate_ = DSTUDIO_SAMPLE_RATE;

    // noise, shared

    // oscillators
    // samples
    sample_index_ = 0.0f;
    sample_index_factor_ = 1.0f;

    // EG
    eg_a_.Init(sample_rate_);

    // filter

    svf_l_.Init(sample_rate_);
    svf_r_.Init(sample_rate_);

    for (uint8_t i = 0; i < DCHOP_CHOPS; i++)
    {
        // note data
        chops_notes_[i] = MIDI_NOTE_NONE;
        note_midi_ = 0;
        note_freq_ = 0.0f;
        note_velocity_ = 0.0f;
    }

    // lfo

    lfo_.Init(sample_rate_);

    // delay

    delay_l_.Init();
    delay_r_.Init();

    // overdrive

    overdrive_.Init();

    // init

    SetType(TUNED);
}

void DChop::Set(const Config &config)
{
    base_config_ = config;
    // chop arrays NOTE copied!

    // sample_rate_ = config.sample_rate;
    tune_ = config.tune;
    filter_type_ = config.filter_type;
    filter_cutoff_ = config.filter_cutoff;
    filter_res_ = config.filter_res;
    eg_a_attack_ = config.eg_a_attack;
    eg_a_decay_ = config.eg_a_decay;
    eg_a_sustain_ = config.eg_a_sustain;
    eg_a_release_ = config.eg_a_release;
    lfo_waveform_ = config.lfo_waveform;
    lfo_freq_ = config.lfo_freq;
    lfo_amp_ = config.lfo_amp;
    lfo_p_level_ = config.lfo_p_level;
    lfo_f_level_ = config.lfo_f_level;
    lfo_a_level_ = config.lfo_a_level;
    delay_delay_ = config.delay_delay;
    delay_feedback_ = config.delay_feedback;
    overdrive_gain_ = config.overdrive_gain;
    overdrive_drive_ = config.overdrive_drive;
    sample_file_name_ = config.sample_file_name;
    for (int i = 0; i < DCHOP_CHOPS; i++)
    {
        sample_phase_start_[i] = config.sample_phase_start[i];
        sample_phase_end_[i] = config.sample_phase_end[i];
        chops_[i] = config.chops[i];
        chops_notes_[i] = config.chops_notes[i];
    }
    chop_gate_ = config.chop_gate;
    mode_internal_ = config.mode_internal;
    loop_ = config.loop;

    sample_length_ = config.sample_length; // length of sample, < BUFFER_MAX
    sample_channels_ = config.sample_channels;
    sample_index_ = 0.0f;
    sample_index_factor_ = 1.0f;

    eg_a_.SetTime(daisysp::ADSR_SEG_ATTACK, eg_a_attack_);
    eg_a_.SetTime(daisysp::ADSR_SEG_DECAY, eg_a_decay_);
    eg_a_.SetTime(daisysp::ADSR_SEG_RELEASE, eg_a_release_);
    eg_a_.SetSustainLevel(eg_a_sustain_);

    // filter

    svf_l_.SetFreq(filter_cutoff_);
    svf_l_.SetRes(filter_res_);
    svf_l_.SetDrive(0.0f); // default
    svf_r_.SetFreq(filter_cutoff_);
    svf_r_.SetRes(filter_res_);
    svf_r_.SetDrive(0.0f); // default

    // lfo

    lfo_.SetWaveform(lfo_waveform_);
    lfo_.SetFreq(lfo_freq_);
    lfo_.SetAmp(lfo_amp_);

    // delay

    delay_l_.SetDelay(sample_rate_ * delay_delay_);
    delay_r_.SetDelay(sample_rate_ * delay_delay_);

    // overdrive

    overdrive_.SetDrive(overdrive_drive_);

    // init
    chop_step_ = 0;
    // note data
    // Calc();
}

void DChop::Process(float *out_l, float *out_r)
{
    float lfo_out;
    float env_a_out;
    float filter_out_l, filter_out_r;
    float delay_out_l, delay_out_r;

    bool note_on;

    float a, b;
    float osc_out_l;
    float osc_out_r;

    uint8_t chop = chops_[chop_step_];

    // osc

    if (mode_internal_)
    {
        if (sample_index_ < sample_phase_end_[chop])
        {
            // pos in buffer
            uint32_t sample_index_int_ = static_cast<int32_t>(sample_index_);
            // how much did we miss?
            float sample_index_fraction_ = sample_index_ - sample_index_int_;
            uint32_t index = sample_index_int_ * sample_channels_;
            // get samples and interpolate
            switch (sample_channels_)
            {
            case 1:
                a = sample_buffer_[index];
                b = sample_buffer_[index + 1];
                osc_out_l = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                osc_out_r = osc_out_l;
                break;
            case 2:
                a = sample_buffer_[index];
                b = sample_buffer_[index + 2];
                osc_out_l = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                a = sample_buffer_[index + sample_channels_];
                b = sample_buffer_[index + sample_channels_ + 2];
                osc_out_r = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                break;
            default:
                osc_out_l = 0.0f;
                osc_out_r = 0.0f;
                break;
            }

            sample_index_ += sample_index_factor_;

            if (sample_index_ > sample_phase_gate_)
            {
                if (note_midi_ != MIDI_NOTE_NONE)
                {
                    note_midi_ = MIDI_NOTE_NONE;
                    // std::cout << "  GATE!" << std::endl;
                }
            }

            if (loop_ && (sample_index_ >= sample_phase_end_[chop]))
            {
                sample_index_ = sample_phase_start_[chop];
            }
        }
        else
        {
            osc_out_l = 0.f;
            osc_out_r = 0.f;
            chop_step_++;
            if (chop_step_ >= DCHOP_CHOPS)
            {
                chop_step_ = 0;
            }
            Calc();
        }
    }
    else
    {
        if (sample_index_ < sample_length_)
        {
            // pos in buffer
            uint32_t sample_index_int_ = static_cast<int32_t>(sample_index_);
            // how much did we miss?
            float sample_index_fraction_ = sample_index_ - sample_index_int_;
            uint32_t index = sample_index_int_ * sample_channels_;
            // get samples and interpolate
            switch (sample_channels_)
            {
            case 1:
                a = sample_buffer_[index];
                b = sample_buffer_[index + 1];
                osc_out_l = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                osc_out_r = osc_out_l;
                break;
            case 2:
                a = sample_buffer_[index];
                b = sample_buffer_[index + 2];
                osc_out_l = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                a = sample_buffer_[index + 1];
                b = sample_buffer_[index + 3];
                osc_out_r = (a + (b - a) * sample_index_fraction_); // * env_a_out;
                break;
            default:
                osc_out_l = 0.0f;
                osc_out_r = 0.0f;
                break;
            }

            sample_index_ += sample_index_factor_;

            if (loop_ && (sample_index_ >= sample_length_))
            {
                sample_index_ = sample_phase_start_[chop];
            }
        }
        else
        {
            osc_out_l = 0.f;
            osc_out_r = 0.f;
        }
    }

    // lfo + apply
    lfo_out = lfo_.Process();

    note_on = (note_midi_ != MIDI_NOTE_NONE);

    // amplitude
    // amp can be affected by:
    // lfo, eg (always), velocity

    env_a_out = eg_a_.Process(note_on) * (1 + lfo_out * lfo_a_level_);

    // osc - pitch

    float f;
    f = note_freq_ *
        powf(2.0f,
             (tune_ / 1200.0 + lfo_out * lfo_p_level_));

    SetFreq(f);

    osc_out_l = osc_out_l * env_a_out * note_velocity_;
    osc_out_r = osc_out_r * env_a_out * note_velocity_;

    // filter
    // cutoff can be affected by:
    // eg, lfo

    filter_out_l = 0.0f;
    filter_out_r = 0.0f;

    f = filter_cutoff_ * (1 + lfo_out * lfo_f_level_);

    svf_l_.SetFreq(f);
    svf_r_.SetFreq(f);

    svf_l_.Process(osc_out_l);
    svf_r_.Process(osc_out_r);
    switch (filter_type_)
    {
    case BAND:
        filter_out_l += svf_l_.Band();
        filter_out_r += svf_r_.Band();
        break;
    case HIGH:
        filter_out_l += svf_l_.High();
        filter_out_r += svf_r_.High();
        break;
    case LOW:
        filter_out_l += svf_l_.Low();
        filter_out_r += svf_r_.Low();
        break;
    case NOTCH:
        filter_out_l += svf_l_.Notch();
        filter_out_r += svf_r_.Notch();
        break;
    case PEAK:
        filter_out_l += svf_l_.Peak();
        filter_out_r += svf_r_.Peak();
        break;
    default:
        filter_out_l += osc_out_l;
        filter_out_r += osc_out_r;
    }

    // overdrive
    // no state in overdrive fx so we can use it on both channels
    if (overdrive_drive_ > 0.0f)
    {
        filter_out_l = overdrive_.Process(filter_out_l * overdrive_gain_);
        filter_out_r = overdrive_.Process(filter_out_r * overdrive_gain_);
    }

    // delay
    if (delay_feedback_ > 0.0f)
    {
        delay_out_l = delay_l_.Read();
        delay_out_r = delay_r_.Read();
        delay_l_.Write((filter_out_l + delay_out_l) * delay_feedback_);
        delay_r_.Write((filter_out_r + delay_out_r) * delay_feedback_);
        filter_out_l += delay_out_l;
        filter_out_r += delay_out_r;
    }

    *out_l = filter_out_l;
    *out_r = filter_out_r;
}

void DChop::Calc()
{
    uint8_t step = chops_[chop_step_];
    note_midi_ = chops_notes_[step];
    note_freq_ = daisysp::mtof(note_midi_ + tune_);

    sample_index_ = sample_phase_start_[step];
    sample_index_factor_ = (note_freq_ / DCHOP_BASE_FREQ);
    sample_phase_gate_ = sample_phase_start_[step] + (sample_phase_end_[step] - sample_phase_start_[step]) * chop_gate_;

    // eg_a_.Retrigger(false);
    eg_a_.Retrigger(true);
    //std::cout << "DChop - Calc() - chop_step " << (int)chop_step_ << " step " << (int)step << " note_midi " << (int)note_midi_ << std::endl;
}

// midi_data0 is the number of the chop to be played
void DChop::MidiIn(uint8_t midi_status, uint8_t midi_data0, uint8_t midi_data1)
{
    // std::cout << "DChop/MidiIn IN " << (int)(midi_data0 & MIDI_DATA_MASK) << std::endl;

    // OMNI - ignore channel
    uint8_t midi_message = midi_status & MIDI_MESSAGE_MASK;
    switch (midi_message)
    {
    case MIDI_MESSAGE_NOTEON:
        if ((midi_data1 & MIDI_DATA_MASK) > 0)
        {
            // TODO
            // NoteOn(midi_data0 & MIDI_DATA_MASK, midi_data1 & MIDI_DATA_MASK);
            //std::cout << "DChop/MidiIn " << (int)(midi_data0 & MIDI_DATA_MASK) << std::endl;
            NoteOn(midi_data0 & MIDI_DATA_MASK, midi_data1 & MIDI_DATA_MASK);
        }
        else
        {
            NoteOff(midi_data0 & MIDI_DATA_MASK, midi_data1 & MIDI_DATA_MASK);
        }
        break;
    case MIDI_MESSAGE_NOTEOFF:
        NoteOff(midi_data0 & MIDI_DATA_MASK, midi_data1 & MIDI_DATA_MASK);
        break;
    default:
        break;
    }
}

void DChop::NoteOn(uint8_t chop, uint8_t midi_velocity)
{
    chop_step_ = chop;
    note_velocity_ = midi_velocity / 100.0f;

    Calc();
}

// TODO
void DChop::NoteOn(uint8_t chop, uint8_t midi_note, uint8_t midi_velocity)
{
    chop_step_ = chop;
    note_velocity_ = midi_velocity / 100.0f;

    Calc();
}

void DChop::NoteOff(uint8_t chop, uint8_t midi_velocity)
{
    note_velocity_ = midi_velocity / 100.0f;

    if (!mode_internal_)
    {
        note_midi_ = MIDI_NOTE_NONE;
    }
}

void DChop::SetMode(bool mode)
{
    mode_internal_ = mode;
}

void DChop::SetLoop(bool loop)
{
    loop_ = loop;
}

void DChop::SetChopGate(float chop_gate)
{
    chop_gate_ = chop_gate;
}

void DChop::SetFreq(float freq)
{
    sample_index_factor_ = ((freq / DCHOP_BASE_FREQ) * sample_channels_);
}

void DChop::SetTuning(float tune)
{
    tune_ = tune;
}

void DChop::SetFilter(FilterType filter_type, float filter_cutoff, float filter_res)
{
    filter_type_ = filter_type;
    filter_cutoff_ = filter_cutoff;
    filter_res_ = filter_res;

    svf_l_.SetFreq(filter_cutoff_);
    svf_l_.SetRes(filter_res_);
    svf_r_.SetFreq(filter_cutoff_);
    svf_r_.SetRes(filter_res_);
}

void DChop::SetFilterFreq(float filter_cutoff)
{
    filter_cutoff_ = filter_cutoff;

    svf_l_.SetFreq(filter_cutoff_);
    svf_r_.SetFreq(filter_cutoff_);
}

void DChop::SetFilterRes(float filter_res)
{
    filter_res_ = filter_res;

    svf_l_.SetRes(filter_res_);
    svf_r_.SetRes(filter_res_);
}

void DChop::SetEG(Target target, float eg_attack, float eg_decay, float eg_sustain, float eg_release)
{
    switch (target)
    {
    case AMP:
        eg_a_attack_ = eg_attack;
        eg_a_decay_ = eg_decay;
        eg_a_sustain_ = eg_sustain;
        eg_a_release_ = eg_release;
        eg_a_.SetTime(daisysp::ADSR_SEG_ATTACK, eg_a_attack_);
        eg_a_.SetTime(daisysp::ADSR_SEG_DECAY, eg_a_decay_);
        eg_a_.SetTime(daisysp::ADSR_SEG_RELEASE, eg_a_release_);
        eg_a_.SetSustainLevel(eg_a_sustain_);
        break;
    default:
        break;
    }
}

void DChop::SetLFO(Waveform lfo_waveform, float lfo_freq, float lfo_amp, float lfo_p_level, float lfo_f_level, float lfo_a_level)
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

void DChop::SetDelay(float delay_delay, float delay_feedback)
{
    delay_delay_ = delay_delay;
    delay_feedback_ = delay_feedback;
    delay_l_.SetDelay(sample_rate_ * delay_delay_);
    delay_r_.SetDelay(sample_rate_ * delay_delay_);
}

void DChop::SetOverdrive(float overdrive_gain, float overdrive_drive)
{
    overdrive_gain_ = overdrive_gain;
    overdrive_drive_ = overdrive_drive;
    overdrive_.SetDrive(overdrive_drive_);
}

bool DChop::Load(const std::string sample_file_name, bool reset)
{
    SNDFILE *sample_file;
    SF_INFO sample_file_info;
    bool retval = false;

    sample_file_info.format = 0;
    const char *c_file_name = sample_file_name.c_str();
    sample_file = sf_open(c_file_name, SFM_READ, &sample_file_info);
    std::cout << "DChop Load " << sample_file_name << (sample_file != NULL) << std::endl;

    if (sample_file != NULL)
    {
        // handle mono/stereo
        uint8_t frame_size;

        switch (sample_file_info.channels)
        {
        case 1: // mono
            frame_size = 1;
            break;
        case 2: // stereo
            frame_size = 2;
            break;
        default:
            // we don't handle anything else
            frame_size = 0;
            break;
        }

        // we are assumeing that a frame is interleaved channels

        sf_count_t frame_count = sample_file_info.frames;

        if (frame_count < DCHOP_SAMPLE_BUFFER_MAX)
        {
            // read sample data
            frame_count = sf_readf_float(sample_file, sample_buffer_, sample_file_info.frames);
            std::cout << "DChop Load,  frame_count:" << frame_count << " frame size" << (int)frame_size << "\n";

            if (reset)
            {
                sample_length_ = frame_count;
                // sample_phase_start_ = 0;
                //  if (sample_phase_end_ == 0)
                // sample_phase_end_ = frame_count - 1;
                uint32_t unit = sample_length_ / 8;
                for (uint8_t i = 0; i < DCHOP_CHOPS; i++)
                {
                    // should be step * unit brlow?
                    sample_phase_start_[i] = (i * unit);
                    sample_phase_end_[i] = (i * unit + unit - 1);
                }
                sample_file_name_ = sample_file_name;
            }
            // always set from sample data
            sample_channels_ = frame_size;
        }
        else
        {
            // failed to load so always reset values
            sample_length_ = 0;
            for (uint32_t i = 0; i < DCHOP_CHOPS; i++)
            {
                sample_phase_start_[i] = 0;
                sample_phase_end_[i] = 0;
            }
            sample_channels_ = frame_size;
        }

        sf_close(sample_file);

        sample_index_ = sample_phase_start_[0];
        sample_index_factor_ = 1.0f;

        retval = true;
    }
    return (retval);
}

/*
void DChop::SetPhases(uint32_t sample_phase_start,
                     uint32_t sample_phase_end)
{
    for (int i = 0; i < DCHOP_CHOPS; i++)
    {
        sample_phase_start_[i] = config.sample_phase_start[i];
        sample_phase_end_[i] = config.sample_phase_end[i];
        chops_[i] = config.chops[i];
        chops_notes_[i] = config.chops_notes[i];

    }

    if (sample_phase_start < sample_length_ - 1)
    {
        sample_phase_start_ = sample_phase_start;
    }
    if (sample_phase_end < sample_length_ - 1)
    {
        sample_phase_end_ = sample_phase_end;
    }

    // reset
    sample_index_ = sample_phase_start_;
    sample_index_factor_ = 1.0f;
}
*/

void DChop::SetChopStart(uint8_t chop, uint32_t pos)
{
    if (chop < DCHOP_CHOPS)
    {
        sample_phase_start_[chop] = pos;
    }
}

uint32_t DChop::GetChopStart(uint8_t chop)
{
    if (chop < DCHOP_CHOPS)
    {
        return sample_phase_start_[chop];
    }
    else
    {
        return 0;
    }
}

void DChop::SetChopNote(uint8_t chop, uint8_t note)
{
    if (chop < DCHOP_CHOPS)
    {
        chops_notes_[chop] = note;
    }
}

uint8_t DChop::GetChopNote(uint8_t chop)
{
    if (chop < DCHOP_CHOPS)
    {
        return chops_notes_[chop];
    }
    else
    {
        return 0;
    }
}

uint32_t DChop::GetSampleLength()
{
    return (sample_length_);
}

float *DChop::GetSampleData()
{
    return (sample_buffer_);
}

uint8_t DChop::GetSampleChannels()
{
    return (sample_channels_);
}

/*
uint32_t *DChop::GetSamplePhaseStart()
{
    return (sample_phase_start_);
}
*/
/*
    value goes from 0 to +1.0
*/
/*
void DChop::ChangeParam(DSynth::Param param, float value)
{
    switch (param)
    {
    case DSynth::DSYNTH_PARAM_AMP:
        SetLevel(value);
        break;
    case DSynth::DSYNTH_PARAM_DELAY_FEEDBACK:
        delay_feedback_ = base_config_.delay_feedback * value;
        break;
    case DSynth::DSYNTH_PARAM_DELAY_FREQ:
        SetDelay(base_config_.delay_delay * value, delay_feedback_);
        break;
    case DSynth::DSYNTH_PARAM_DETUNE:
        // SetTuning(tune_, base_config_.detune * value);
        break;
    case DSynth::DSYNTH_PARAM_FILTER_CUTOFF:
        SetFilterFreq(base_config_.filter_cutoff * value);
        break;
    case DSynth::DSYNTH_PARAM_FILTER_RES:
        SetFilterRes(base_config_.filter_res * value);
        break;
    case DSynth::DSYNTH_PARAM_LFO_AMP:
        lfo_amp_ = value; // base_config_.lfo_amp * value;
        lfo_.SetAmp(lfo_amp_);
        break;
    case DSynth::DSYNTH_PARAM_LFO_FREQ:
        lfo_freq_ = base_config_.lfo_freq * value * 10;
        lfo_.SetFreq(lfo_freq_);
        break;
    case DSynth::DSYNTH_PARAM_OVERDRIVE:
        SetOverdrive(overdrive_gain_, base_config_.overdrive_drive + value);
        break;
    case DSynth::DSYNTH_PARAM_TRANSPOSE:
        SetTranspose(base_config_.transpose * value);
        break;
    case DSynth::DSYNTH_PARAM_TUNE:
        // SetTuning(base_config_.tune * value);
        SetTuning(value * 100);
        break;
    case DSynth::DSYNTH_PARAM_FREQ:
        note_freq_[0] = value;
        break;
    }
}
*/