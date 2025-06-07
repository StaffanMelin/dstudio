#include <string>
#include <iostream>

#include "dhits.h"

void DHits::Init()
{
    sample_rate_ = DSTUDIO_SAMPLE_RATE;

    for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
    {
        // oscillators
        // samples
        sample_index_[i] = 0.0f;
        sample_index_factor_[0] = 1.0f;

        // EG - amplitude
        eg_a_[i].Init(sample_rate_);

        // note data
        note_freq_[i] = 0.0f;
        note_velocity_[i] = 0.0f;
    }

    // delay

    delay_l_.Init();
    delay_r_.Init();

    // overdrive

    overdrive_.Init();

    SetType(TUNED);
}

void DHits::Set(const Config &config)
{
    base_config_ = config;

    // sample_rate_ = config.sample_rate;
    for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
    {
        level_[i] = config.level[i];
        pan_[i] = config.pan[i];
        tune_[i] = config.tune[i];
        eg_a_attack_[i] = config.eg_a_attack[i];
        eg_a_decay_[i] = config.eg_a_decay[i];
        eg_a_sustain_[i] = config.eg_a_sustain[i];
        eg_a_release_[i] = config.eg_a_release[i];
        sample_file_name_[i] = config.sample_file_name[i];
        sample_phase_start_[i] = config.sample_phase_start[i];
        sample_phase_end_[i] = config.sample_phase_end[i];
        sample_length_[i] = config.sample_length[i];

        // samples
        sample_index_[i] = 0.0f;
        sample_index_factor_[0] = 1.0f;
        note_freq_[i] = 0;

        // EG
        eg_a_[i].SetTime(daisysp::ADSR_SEG_ATTACK, eg_a_attack_[i]);
        eg_a_[i].SetTime(daisysp::ADSR_SEG_DECAY, eg_a_decay_[i]);
        eg_a_[i].SetTime(daisysp::ADSR_SEG_RELEASE, eg_a_release_[i]);
        eg_a_[i].SetSustainLevel(eg_a_sustain_[i]);
        // std::cout << "dhits/Set():" << eg_a_sustain_[i] << "\n";

        // note data
        note_freq_[i] = 0.0f;
        note_velocity_[i] = 0.0f;
    }

    // delay
    delay_delay_ = config.delay_delay;
    delay_feedback_ = config.delay_feedback;
    delay_l_.SetDelay(sample_rate_ * delay_delay_);
    delay_r_.SetDelay(sample_rate_ * delay_delay_);
    // overdrive
    overdrive_gain_ = config.overdrive_gain;
    overdrive_drive_ = config.overdrive_drive;
    overdrive_.SetDrive(overdrive_drive_);
}

void DHits::Process(float *out_l, float *out_r)
{
    float env_a_out;
    float delay_out_l, delay_out_r;

    bool note_on;

    float a, b;
    float osc_out;
    float mix_l = 0;
    float mix_r = 0;

    // std::cout << "DHits process"  << "\n";

    for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
    {
        // note_on = (note_midi_[i] != 0);

        note_on = (sample_index_[i] == sample_phase_start_[i]);

        // amplitude

        env_a_out = eg_a_[i].Process(note_on); // note off

        /*
        SetFreq(note_freq_[i] *
                powf(2.0f,
                     (tune_[i] / 1200.0)));
        */

        // osc

        if (sample_index_[i] < sample_phase_end_[i])
        {
            // pos in buffer
            uint32_t sample_index_int_ = static_cast<int32_t>(sample_index_[i]);
            // how much did we miss?
            float sample_index_fraction_ = sample_index_[i] - sample_index_int_;
            // get samples and interpolate
            a = sample_buffer_[i][sample_index_int_];
            b = sample_buffer_[i][sample_index_int_ + 1];
            osc_out = (a + (b - a) * sample_index_fraction_) * env_a_out; // * note_velocity_[i];

            sample_index_[i] += sample_index_factor_[i];
            /*
            if (i == 0)
            {
                std::cout << "DHits audio:" << osc_out << "," << a << "\n";

            }
            */
        }
        else
        {
            osc_out = 0.f;
        }

        mix_l += osc_out * level_[i] * (1.0f - pan_[i]);
        mix_r += osc_out * level_[i] * pan_[i];
    }

    // overdrive, no state in overdrive fx so we can use it on both channels
    if (overdrive_drive_ > 0.0f)
    {
        mix_l = overdrive_.Process(mix_l * overdrive_gain_);
        mix_r = overdrive_.Process(mix_r * overdrive_gain_);
    }

    // delay
    if (delay_feedback_ > 0.0f)
    {
        delay_out_l = delay_l_.Read();
        delay_out_r = delay_r_.Read();
        delay_l_.Write((mix_l + delay_out_l) * delay_feedback_);
        delay_r_.Write((mix_r + delay_out_r) * delay_feedback_);
        mix_l += delay_out_l;
        mix_r += delay_out_r;
    }

    *out_l = mix_l;
    *out_r = mix_r;
}

void DHits::MidiIn(uint8_t midi_status, uint8_t midi_data0, uint8_t midi_data1 = 0)
{
    // OMNI - ignore channel
    uint8_t midi_message = midi_status & MIDI_MESSAGE_MASK;
    switch (midi_message)
    {
    case MIDI_MESSAGE_NOTEON:
        if ((midi_data1 & MIDI_DATA_MASK) > 0)
        {
            NoteOn(midi_data0 & MIDI_DATA_MASK, midi_data1 & MIDI_DATA_MASK);
        }
        else
        {
            NoteOff(midi_data0 & MIDI_DATA_MASK);
        }
        break;
    case MIDI_MESSAGE_NOTEOFF:
        NoteOff(midi_data0 & MIDI_DATA_MASK);
        break;
    case MIDI_MESSAGE_CC:
        break;
    default:
        break;
    }
}

void DHits::NoteOn(uint8_t midi_note, uint8_t midi_velocity)
{
    uint8_t hit = midi_note;
    if (hit < DHITS_HITS_MAX)
    {
        //std::cout << "DHits note on:" << (int)hit << "\n";

        note_freq_[hit] = DHITS_BASE_FREQ * tune_[hit];
        note_velocity_[hit] = (float)midi_velocity / MIDI_VELOCITY_MAX;

        sample_index_[hit] = sample_phase_start_[hit];
        sample_index_factor_[hit] = (note_freq_[hit] / DHITS_BASE_FREQ);

        eg_a_[hit].Retrigger(false); // TODO false?
        eg_a_[hit].Process(true);    // note on
    }
}

void DHits::NoteOff(uint8_t midi_note)
{
}

void DHits::Silence()
{
    for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
    {
        sample_index_[i] = sample_phase_end_[i];
    }
}

void DHits::SetTune(uint8_t hit, float tune)
{
    tune_[hit] = tune;
}

void DHits::SetFreq(uint8_t hit, float freq)
{
    sample_index_factor_[hit] = (freq / DHITS_BASE_FREQ);
}

void DHits::SetEG(uint8_t hit, float eg_attack, float eg_decay, float eg_sustain, float eg_release)
{
    eg_a_attack_[hit] = eg_attack;
    eg_a_decay_[hit] = eg_decay;
    eg_a_sustain_[hit] = eg_sustain;
    eg_a_release_[hit] = eg_release;
    eg_a_[hit].SetTime(daisysp::ADSR_SEG_ATTACK, eg_a_attack_[hit]);
    eg_a_[hit].SetTime(daisysp::ADSR_SEG_DECAY, eg_a_decay_[hit]);
    eg_a_[hit].SetTime(daisysp::ADSR_SEG_RELEASE, eg_a_release_[hit]);
    eg_a_[hit].SetSustainLevel(eg_a_sustain_[hit]);
}

void DHits::SetDelay(float delay_delay, float delay_feedback)
{
    delay_delay_ = delay_delay;
    delay_feedback_ = delay_feedback;
    delay_l_.SetDelay(sample_rate_ * delay_delay_);
    delay_r_.SetDelay(sample_rate_ * delay_delay_);
}

void DHits::SetOverdrive(float overdrive_gain, float overdrive_drive)
{
    overdrive_gain_ = overdrive_gain;
    overdrive_drive_ = overdrive_drive;
    overdrive_.SetDrive(overdrive_drive_);
}

bool DHits::Load(uint8_t hit, const std::string sample_file_name, bool reset)
{
    SNDFILE *sample_file;
    SF_INFO sample_file_info;
    bool retval = false;

    sample_file_info.format = 0;
    const char *c_file_name = sample_file_name.c_str();
    sample_file = sf_open(c_file_name, SFM_READ, &sample_file_info);

    // handle mono only
    uint8_t frame_size = 1;

    // we are assumeing that a frame is interleaved channels

    sf_count_t frame_count = sample_file_info.frames;

    std::cout << "DHits frame_count:" << (int)frame_count << "\n";

    if (frame_count < DHITS_SAMPLE_BUFFER_MAX)
    {
        // read sample data
        frame_count = sf_readf_float(sample_file, sample_buffer_[hit], sample_file_info.frames);
        std::cout << "DHits frame_count read:" << (int)frame_count << "\n";

        if (reset)
        {
            sample_length_[hit] = frame_count;
            sample_phase_start_[hit] = 0;
            sample_phase_end_[hit] = frame_count - 1;
            sample_file_name_[hit] = sample_file_name;
        }
        std::cout << "DHits start, end:" << sample_phase_start_[hit] << ", " << sample_phase_end_[hit] << "\n";
    }
    else
    {
        // failed to load so always reset values
        sample_length_[hit] = 0;
        sample_phase_start_[hit] = 0;
        sample_phase_end_[hit] = 0;
    }

    sf_close(sample_file);

    for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
    {
        sample_index_[i] = sample_phase_start_[i];
        sample_index_factor_[i] = 1.0f;
    }

    return (retval);
}

void DHits::GetPhase(uint8_t hit,
                     uint32_t *sample_phase_start,
                     uint32_t *sample_phase_end)
{
    *sample_phase_start = sample_phase_start_[hit];
    *sample_phase_end = sample_phase_end_[hit];
}

void DHits::SetPhase(uint8_t hit,
                     uint32_t sample_phase_start,
                     uint32_t sample_phase_end)
{
    if (sample_phase_start < sample_length_[hit] - 1)
    {
        sample_phase_start_[hit] = sample_phase_start;
    }
    if (sample_phase_end < sample_length_[hit] - 1)
    {
        sample_phase_end_[hit] = sample_phase_end;
    }

    // reset
    sample_index_[hit] = sample_phase_start_[hit];
    sample_index_factor_[hit] = 1.0f;
}

uint32_t DHits::GetLength(uint8_t hit)
{
    return (sample_length_[hit]);
}

void DHits::SetLevel(uint8_t hit, float level)
{
    level_[hit] = level;
}

void DHits::SetPan(uint8_t hit, float pan)
{
    pan_[hit] = pan;
}

void DHits::LoadHits(std::string file_name)
{
    TiXmlDocument *pDoc;
    TiXmlElement *pRoot;
    Config config;

    pDoc = new TiXmlDocument();
    if (!pDoc->LoadFile(file_name.c_str()))
    {
        std::cout << "ERROR: Couldn't load " << file_name << "\n";
        exit(0);
    }
    pRoot = pDoc->RootElement();

    if (NULL != pRoot)
    {
                for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
        {
            config.sample_file_name[i] = "";

        }
        config.sample_rate = DSTUDIO_SAMPLE_RATE;

        // get info
        TiXmlElement *pElt;
        const char *str_c;
        pElt = pRoot->FirstChildElement("name");
        str_c = pElt->GetText(); // cchar
        std::string name = str_c;
        pElt = pRoot->FirstChildElement("type");
        str_c = pElt->GetText(); // cchar
        std::string type = str_c;
        std::cout << "Load DHits:"
                  << name << " "
                  << type << " "
                  << std::endl;
        pElt = pRoot->FirstChildElement("delay_delay");
        str_c = pElt->GetText(); // cchar
        config.delay_delay = atof(str_c);
        pElt = pRoot->FirstChildElement("delay_feedback");
        str_c = pElt->GetText(); // cchar
        config.delay_feedback = atof(str_c);
        pElt= pRoot->FirstChildElement("overdrive_gain");
        str_c = pElt->GetText(); // cchar
        config.overdrive_gain = atof(str_c);
        pElt = pRoot->FirstChildElement("overdrive_drive");
        str_c = pElt->GetText(); // cchar
        config.overdrive_drive = atof(str_c);

        // get notes
        pElt = pRoot->FirstChildElement("voice");
        while (pElt)
        {
            str_c = pElt->GetText(); // cchar
            std::string file = str_c;
            int channel;
            float level, pan, tune;
            float eg_a_attack, eg_a_decay, eg_a_sustain, eg_a_release;
            pElt->QueryIntAttribute("channel", &channel);
            pElt->QueryFloatAttribute("level", &level);
            pElt->QueryFloatAttribute("pan", &pan);
            pElt->QueryFloatAttribute("tune", &tune);
            pElt->QueryFloatAttribute("eg_a_attack", &eg_a_attack);
            pElt->QueryFloatAttribute("eg_a_decay", &eg_a_decay);
            pElt->QueryFloatAttribute("eg_a_sustain", &eg_a_sustain);
            pElt->QueryFloatAttribute("eg_a_release", &eg_a_release);

            config.level[channel]=level;
            config.pan[channel] = pan;
            config.tune[channel] = tune;
            config.eg_a_attack[channel] = eg_a_attack;
            config.eg_a_decay[channel] = eg_a_decay;
            config.eg_a_sustain[channel] = eg_a_sustain;
            config.eg_a_release[channel] = eg_a_release;
            config.sample_file_name[channel] = file;

            std::cout << "Note (level, pan, tune, file):"
                      << (float)level
                      << " "
                      << (float)pan
                      << " "
                      << (float)tune
                      << " "
                      << file
                      << std::endl;

            pElt = pElt->NextSiblingElement("voice");
        }
        Set(config);
        for (uint8_t i = 0; i < DHITS_HITS_MAX; i++)
        {
            if (config.sample_file_name[i] != "")
            {
            Load(i, config.sample_file_name[i], true);
            }

        }
    }

    delete pDoc;
}