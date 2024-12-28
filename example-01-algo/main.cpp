#include "main.h"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <getopt.h>

// standard
#include "../rtDaisySP/src/daisysp.h"
#include "../rtDStudio/src/dstudio.h"

// application - DStudio
#include "../rtDStudio/src/dmixer.h"
#include "../rtDStudio/src/dsynthsub.h"
#include "../rtDStudio/src/dsynthfm.h"
#include "../rtDStudio/src/dbass.h"
#include "../rtDStudio/src/dhihat.h"
#include "../rtDStudio/src/dsnare.h"
#include "../rtDStudio/src/dclap.h"
#include "../rtDStudio/src/dcymbal.h"
#include "../rtDStudio/src/ddrum.h"

#include "rt.h"


//////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////

// standard
bool done_;

// application - DStudio
DMixer dmixer; // mixer
DSynthSub dsynth0; // chord
DSynthSub dsynth1; // chord
DSynthSub dsynth2; // solo portamento
DSynthSub dsynthb; // bass
DSynthFm dsynthfm;
DMixer ddmixer; // drum mixer
DBass dbass;
DSnare dsnare;
DHihat dhihatc;
DHihat dhihato;
DClap dclap;
DCymbal dcrash;
DCymbal dride;
DDrum dtomhi;
DDrum dtomlo;
// "sequencer"
uint8_t testcount = 0;
uint8_t testnote1 = 0;
uint8_t testnote2 = 0;
uint8_t testnote3 = 0;
DInterval clocker;



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

    // synths subtractive
    DSynthSub::Config dsynth_config;

    // synth 0
    dsynth_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsynth_config.voices = 4;
    dsynth_config.waveform0 = DSynthSub::WAVE_SAW;
    dsynth_config.waveform1 = DSynthSub::WAVE_TRI;
    dsynth_config.tune = 0.0f;
    dsynth_config.detune = 12.0f;
    dsynth_config.transpose = 0;
    dsynth_config.osc0_level = 0.3f;
    dsynth_config.osc1_level = 0.5f;
    dsynth_config.noise_level = 0.0f;
    dsynth_config.filter_type = DSynthSub::LOW;
    dsynth_config.filter_cutoff = 800.0f;
    dsynth_config.filter_res = 0.2f;
    dsynth_config.eg_p_level = 0.0f;
    dsynth_config.eg_p_attack = 0.0f;
    dsynth_config.eg_p_decay = 0.0f;
    dsynth_config.eg_p_sustain = 0.0f;
    dsynth_config.eg_p_release = 0.0f;
    dsynth_config.eg_f_level = 1.0f;
    dsynth_config.eg_f_attack = 0.3f;
    dsynth_config.eg_f_decay = 0.01f;
    dsynth_config.eg_f_sustain = 1.0f;
    dsynth_config.eg_f_release = 0.2f;
    dsynth_config.eg_a_attack = 0.3f;
    dsynth_config.eg_a_decay = 0.01f;
    dsynth_config.eg_a_sustain = 1.0f;
    dsynth_config.eg_a_release = 0.4f;
    dsynth_config.lfo_waveform = DSynthSub::WAVE_TRI;
    dsynth_config.lfo_freq = 1.0f;
    dsynth_config.lfo_amp = 0.4f;
    dsynth_config.lfo_p_level = 0.0f;
    dsynth_config.lfo_f_level = 0.0f;
    dsynth_config.lfo_a_level = 0.0f;
    dsynth_config.portamento = 0.0f;
    dsynth_config.delay_delay = 0.3f;
    dsynth_config.delay_feedback = 0.3f;
    dsynth_config.overdrive_gain = 0.0f;
    dsynth_config.overdrive_drive = 0.0f;
    dsynth0.Init();
    dsynth0.Set(dsynth_config);

    // synth 1
    dsynth_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsynth_config.voices = 4;
    dsynth_config.waveform0 = DSynthSub::WAVE_SQUARE;
    dsynth_config.waveform1 = DSynthSub::WAVE_SQUARE;
    dsynth_config.tune = 0.0f;
    dsynth_config.detune = 0.2f;
    dsynth_config.transpose = 0;
    dsynth_config.osc0_level = 0.5f;
    dsynth_config.osc1_level = 0.5f;
    dsynth_config.noise_level = 0.0f;
    dsynth_config.filter_type = DSynthSub::BAND;
    dsynth_config.filter_cutoff = 1000.0f;
    dsynth_config.filter_res = 0.0f;
    dsynth_config.eg_p_level = 0.0f;
    dsynth_config.eg_p_attack = 0.0f;
    dsynth_config.eg_p_decay = 0.0f;
    dsynth_config.eg_p_sustain = 0.0f;
    dsynth_config.eg_p_release = 0.0f;
    dsynth_config.eg_f_level = 1.0f;
    dsynth_config.eg_f_attack = 0.0f;
    dsynth_config.eg_f_decay = 0.0f;
    dsynth_config.eg_f_sustain = 1.0f;
    dsynth_config.eg_f_release = 0.0f;
    dsynth_config.eg_a_attack = 0.01f;
    dsynth_config.eg_a_decay = 0.01f;
    dsynth_config.eg_a_sustain = 1.0f;
    dsynth_config.eg_a_release = 0.8f;
    dsynth_config.lfo_waveform = DSynthSub::WAVE_TRI;
    dsynth_config.lfo_freq = 4.0f;
    dsynth_config.lfo_amp = 0.7f;
    dsynth_config.lfo_p_level = 0.0f;
    dsynth_config.lfo_f_level = 0.0f;
    dsynth_config.lfo_a_level = 0.7f;
    dsynth_config.portamento = 0.0f;
    dsynth_config.delay_delay = 0.3f;
    dsynth_config.delay_feedback = 0.3f;
    dsynth_config.overdrive_gain = 0.2f;
    dsynth_config.overdrive_drive = 0.3f;
    dsynth1.Init();
    dsynth1.Set(dsynth_config);

    // synth 2
    dsynth_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsynth_config.voices = 1;
    dsynth_config.waveform0 = DSynthSub::WAVE_SIN;
    dsynth_config.waveform1 = DSynthSub::WAVE_SAW;
    dsynth_config.tune = 0.0f;
    dsynth_config.detune = 0.2f;
    dsynth_config.transpose = 0;
    dsynth_config.osc0_level = 0.5f;
    dsynth_config.osc1_level = 0.5f;
    dsynth_config.noise_level = 0.3f;
    dsynth_config.filter_type = DSynthSub::BAND;
    dsynth_config.filter_cutoff = 700.0f;
    dsynth_config.filter_res = 0.5f;
    dsynth_config.eg_p_level = 0.0f;
    dsynth_config.eg_p_attack = 0.0f;
    dsynth_config.eg_p_decay = 0.0f;
    dsynth_config.eg_p_sustain = 0.0f;
    dsynth_config.eg_p_release = 0.0f;
    dsynth_config.eg_f_level = 1.0f;
    dsynth_config.eg_f_attack = 0.0f;
    dsynth_config.eg_f_decay = 0.0f;
    dsynth_config.eg_f_sustain = 1.0f;
    dsynth_config.eg_f_release = 0.0f;
    dsynth_config.eg_a_attack = 0.01f;
    dsynth_config.eg_a_decay = 0.01f;
    dsynth_config.eg_a_sustain = 1.0f;
    dsynth_config.eg_a_release = 0.8f;
    dsynth_config.lfo_waveform = DSynthSub::WAVE_TRI;
    dsynth_config.lfo_freq = 0.6f;
    dsynth_config.lfo_amp = 0.8f;
    dsynth_config.lfo_p_level = 0.0f;
    dsynth_config.lfo_f_level = 1.0f;
    dsynth_config.lfo_a_level = 0.0f;
    dsynth_config.portamento = 0.1f;
    dsynth_config.delay_delay = 0.3f;
    dsynth_config.delay_feedback = 0.7f;
    dsynth_config.overdrive_gain = 0.0f;
    dsynth_config.overdrive_drive = 0.0f;
    dsynth2.Init();
    dsynth2.Set(dsynth_config);

    // synth bass
    dsynth_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsynth_config.voices = 1;
    dsynth_config.waveform0 = DSynthSub::WAVE_SIN;
    dsynth_config.waveform1 = DSynthSub::WAVE_SAW;
    dsynth_config.tune = 0.0f;
    dsynth_config.detune = 0.2f;
    dsynth_config.transpose = 0;
    dsynth_config.osc0_level = 0.5f;
    dsynth_config.osc1_level = 0.5f;
    dsynth_config.noise_level = 0.0f;
    dsynth_config.filter_type = DSynthSub::LOW;
    dsynth_config.filter_cutoff = 700.0f;
    dsynth_config.filter_res = 0.5f;
    dsynth_config.eg_p_level = 0.0f;
    dsynth_config.eg_p_attack = 0.0f;
    dsynth_config.eg_p_decay = 0.0f;
    dsynth_config.eg_p_sustain = 0.0f;
    dsynth_config.eg_p_release = 0.0f;
    dsynth_config.eg_f_level = 1.0f;
    dsynth_config.eg_f_attack = 0.01f;
    dsynth_config.eg_f_decay = 0.01f;
    dsynth_config.eg_f_sustain = 1.0f;
    dsynth_config.eg_f_release = 0.5f;
    dsynth_config.eg_a_attack = 0.01f;
    dsynth_config.eg_a_decay = 0.01f;
    dsynth_config.eg_a_sustain = 1.0f;
    dsynth_config.eg_a_release = 0.8f;
    dsynth_config.lfo_waveform = DSynthSub::WAVE_TRI;
    dsynth_config.lfo_freq = 0.0f;
    dsynth_config.lfo_amp = 0.0f;
    dsynth_config.lfo_p_level = 0.0f;
    dsynth_config.lfo_f_level = 0.0f;
    dsynth_config.lfo_a_level = 0.0f;
    dsynth_config.portamento = 0.0f;
    dsynth_config.delay_delay = 0.0f;
    dsynth_config.delay_feedback = 0.0f;
    dsynth_config.overdrive_gain = 0.0f;
    dsynth_config.overdrive_drive = 0.0f;
    dsynthb.Init();
    dsynthb.Set(dsynth_config);

    // synth fm
    DSynthFm::Config dsynthfm_config;
    dsynthfm_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsynthfm_config.voices = 1;
    dsynthfm_config.ratio = 1.0f;
    dsynthfm_config.index = 0.5f;
    dsynthfm_config.tune = 0.0f;
    dsynthfm_config.transpose = 0;
    dsynthfm_config.osc0_level = 0.5f;
    dsynthfm_config.noise_level = 0.0f;
    dsynthfm_config.filter_type = DSynthFm::BAND;
    dsynthfm_config.filter_cutoff = 700.0f;
    dsynthfm_config.filter_res = 0.1f;
    dsynthfm_config.eg_p_level = 0.0f;
    dsynthfm_config.eg_p_attack = 0.0f;
    dsynthfm_config.eg_p_decay = 0.0f;
    dsynthfm_config.eg_p_sustain = 0.0f;
    dsynthfm_config.eg_p_release = 0.0f;
    dsynthfm_config.eg_f_level = 1.0f;
    dsynthfm_config.eg_f_attack = 0.0f;
    dsynthfm_config.eg_f_decay = 0.0f;
    dsynthfm_config.eg_f_sustain = 1.0f;
    dsynthfm_config.eg_f_release = 0.0f;
    dsynthfm_config.eg_a_attack = 0.01f;
    dsynthfm_config.eg_a_decay = 0.01f;
    dsynthfm_config.eg_a_sustain = 1.0f;
    dsynthfm_config.eg_a_release = 0.2f;
    dsynthfm_config.lfo_waveform = DSynthFm::WAVE_TRI;
    dsynthfm_config.lfo_freq = 0.6f;
    dsynthfm_config.lfo_amp = 0.8f;
    dsynthfm_config.lfo_p_level = 0.0f;
    dsynthfm_config.lfo_f_level = 1.0f;
    dsynthfm_config.lfo_a_level = 0.0f;
    dsynthfm_config.portamento = 0.0f;
    dsynthfm_config.delay_delay = 0.5f;
    dsynthfm_config.delay_feedback = 0.6f;
    dsynthfm_config.overdrive_gain = 0.0f;
    dsynthfm_config.overdrive_drive = 0.0f;
    dsynthfm.Init();
    dsynthfm.Set(dsynthfm_config);

    // drum bass
    DBass::Config dbass_config;
    dbass_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dbass_config.type = DTYPE_SYNTHETIC;
    dbass_config.vol = 1.0f;
    dbass_config.freq = 70.0f;
    dbass_config.tone = 0.4f;
    dbass_config.decay = 0.3f;
    // analog
    dbass_config.fm_attack = 0.8f;
    dbass_config.fm_self = 0.8f;
    // synthetic
    dbass_config.dirtiness = 0.5f;
    dbass_config.fm_env_amount = 0.5f;
    dbass_config.fm_env_decay = 0.5f;
    // opd
    dbass_config.min = 0.5;
    dbass.Init();
    dbass.Set(dbass_config);

    // drums snare
    DSnare::Config dsnare_config;
    dsnare_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dsnare_config.type = DTYPE_ANALOG;
    dsnare_config.vol = 1.0f;
    // common
    if (dsnare_config.type == DTYPE_OPD)
        dsnare_config.freq = 200.0f;
    else
        dsnare_config.freq = 60.0f;
    dsnare_config.tone = 0.5f;
    dsnare_config.decay = 0.3f;
    // analog
    dsnare_config.snappy = 0.3f;
    // synthetic
    dsnare_config.fm_amount = 0.5f;
    // opd
    dsnare_config.freq_noise = 1000.0f; // highpass
    dsnare_config.amp = 0.5f;
    dsnare_config.res = 0.3f;
    dsnare_config.drive = 0.3f;
    dsnare_config.min = 0.3f;
    dsnare.Init();
    dsnare.Set(dsnare_config);

    // drum hihat closed
    DHihat::Config dhihat_config;
    dhihat_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dhihat_config.type = DTYPE_OPD;
    dhihat_config.vol = 0.5f;
    // common
    dhihat_config.freq = 5000.0f;
    dhihat_config.tone = 0.8f;
    dhihat_config.decay = 0.05f;
    // analog
    // synthetic
    dhihat_config.noisiness = 0.5f;
    // opd
    dhihat_config.amp = 0.3f;
    dhihat_config.res = 0.3f;
    dhihat_config.drive = 0.3f;
    dhihatc.Init();
    dhihatc.Set(dhihat_config);

    // drum hihat open
    dhihat_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dhihat_config.type = DTYPE_OPD;
    dhihat_config.vol = 1.0f;
    // common
    dhihat_config.freq = 3000.0f;
    dhihat_config.tone = 0.8f;
    dhihat_config.decay = 0.3f;
    // analog
    // synthetic
    dhihat_config.noisiness = 0.5f;
    // opd
    dhihat_config.amp = 0.3f;
    dhihat_config.res = 0.3f;
    dhihat_config.drive = 0.3f;
    dhihato.Init();
    dhihato.Set(dhihat_config);

    // drum clap
    DClap::Config dclap_config;
    dclap_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dclap_config.vol = 1.0;
    dclap_config.freq = 1200.0f;
    dclap_config.res = 0.5f;
    dclap_config.drive = 0.1f;
    dclap_config.amp = 0.8f;
    dclap_config.decay = 0.15f;
    dclap.Init();
    dclap.Set(dclap_config);

    // drum ride
    DCymbal::Config dcymbal_config;
    dcymbal_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dcymbal_config.vol = 1.0f;
    dcymbal_config.freq = 5000.0f;
    dcymbal_config.res = 0.3f;
    dcymbal_config.drive = 0.3f;
    dcymbal_config.amp = 0.4f;
    dcymbal_config.decay = 0.6f;
    dcymbal_config.min = 0.6f;
    dcymbal_config.mix = 0.1f;
    dride.Init();
    dride.Set(dcymbal_config);

    // drum crash
    dcymbal_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dcymbal_config.vol = 1.0f;
    dcymbal_config.freq = 1000.0f;
    dcymbal_config.res = 0.3f;
    dcymbal_config.drive = 0.3f;
    dcymbal_config.amp = 0.4f;
    dcymbal_config.decay = 1.2f;
    dcymbal_config.min = 0.3f;
    dcymbal_config.mix = 0.3f;
    dcrash.Init();
    dcrash.Set(dcymbal_config);

    // tom hi
    DDrum::Config dtomhi_config;
    dtomhi_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dtomhi_config.vol = 1.0f;
    dtomhi_config.freq = 300.0f;
    dtomhi_config.amp = 0.5f;
    dtomhi_config.decay = 0.4f;
    dtomhi_config.min = 0.1f;
    dtomhi.Init();
	dtomhi.Set(dtomhi_config);

    // tom lo
    DDrum::Config dtomlo_config;
    dtomlo_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dtomlo_config.vol = 1.0f;
    dtomlo_config.freq = 150.0f;
    dtomlo_config.amp = 0.5f;
    dtomlo_config.decay = 0.4f;
    dtomlo_config.min = 0.1f;
    dtomlo.Init();
	dtomlo.Set(dtomlo_config);

    // drum submixer = drummachine
    // bass, snare, hh open, hh closed, clap,
    // ride, crash, tomhi, tomlo
    DSound *ddmix_synth[MIXER_CHANNELS_MAX];
    float ddmix_pan[MIXER_CHANNELS_MAX];
    float ddmix_level[MIXER_CHANNELS_MAX];
    float ddmix_chorus_level[MIXER_CHANNELS_MAX];
    float ddmix_reverb_level[MIXER_CHANNELS_MAX];
    bool ddmix_mono[MIXER_CHANNELS_MAX];
    uint8_t ddmix_group[MIXER_CHANNELS_MAX];
    DMixer::Config ddmix_config;

    ddmix_synth[0] = &dbass;
    ddmix_level[0] = 0.9;
    ddmix_pan[0] = 0.5f;
    ddmix_chorus_level[0] = 0.0f;
    ddmix_reverb_level[0] = 0.1f;
    ddmix_mono[0] = true;
    ddmix_group[0] = 0;

    ddmix_synth[1] = &dsnare;
    ddmix_level[1] = 0.7;
    ddmix_pan[1] = 0.6f;
    ddmix_chorus_level[1] = 0.0f;
    ddmix_reverb_level[1] = 0.4f;
    ddmix_mono[1] = true;
    ddmix_group[1] = 1;

    ddmix_synth[2] = &dhihatc;
    ddmix_level[2] = 0.15;
    ddmix_pan[2] = 0.3f;
    ddmix_chorus_level[2] = 0.0f;
    ddmix_reverb_level[2] = 0.5f;
    ddmix_mono[2] = true;
    ddmix_group[2] = 2; // hihats share group as sound is produced with same cymbal

    ddmix_synth[3] = &dhihato;
    ddmix_level[3] = 0.15;
    ddmix_pan[3] = 0.3f;
    ddmix_chorus_level[3] = 0.0f;
    ddmix_reverb_level[3] = 0.5f;
    ddmix_mono[3] = true;
    ddmix_group[3] = 2; // shared with dhihatc

    ddmix_synth[4] = &dclap;
    ddmix_level[4] = 0.35;
    ddmix_pan[4] = 0.7f;
    ddmix_chorus_level[4] = 0.4f;
    ddmix_reverb_level[4] = 0.6f;
    ddmix_mono[4] = true;
    ddmix_group[4] = 4;

    ddmix_synth[5] = &dride;
    ddmix_level[5] = 0.2;
    ddmix_pan[5] = 0.8f;
    ddmix_chorus_level[5] = 0.0f;
    ddmix_reverb_level[5] = 0.6f;
    ddmix_mono[5] = true;
    ddmix_group[5] = 5;

    ddmix_synth[6] = &dcrash;
    ddmix_level[6] = 0.2;
    ddmix_pan[6] = 0.4f;
    ddmix_chorus_level[6] = 0.0f;
    ddmix_reverb_level[6] = 0.6f;
    ddmix_mono[6] = true;
    ddmix_group[6] = 6;

    ddmix_synth[7] = &dtomhi;
    ddmix_level[7] = 0.5;
    ddmix_pan[7] = 0.75f;
    ddmix_chorus_level[7] = 0.0f;
    ddmix_reverb_level[7] = 0.3f;
    ddmix_mono[7] = true;
    ddmix_group[7] = 7;

    ddmix_synth[8] = &dtomlo;
    ddmix_level[8] = 0.6;
    ddmix_pan[8] = 0.9f;
    ddmix_chorus_level[8] = 0.0f;
    ddmix_reverb_level[8] = 0.3f;
    ddmix_mono[8] = true;
    ddmix_group[8] = 8;

    ddmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    ddmix_config.channels = 9;
    ddmix_config.amp = 1.0f;
    ddmix_config.synth = ddmix_synth;
    ddmix_config.pan = ddmix_pan;
    ddmix_config.level = ddmix_level;
    ddmix_config.chorus_level = ddmix_chorus_level;
    ddmix_config.reverb_level = ddmix_reverb_level;
    ddmix_config.mono = ddmix_mono;
    ddmix_config.group = ddmix_group;
    ddmix_config.chorus_return = 0.5;
    ddmix_config.reverb_return = 0.5f;
    ddmix_config.mix_dry = 0.5;
    ddmixer.Init();
    ddmixer.Set(ddmix_config);
    ddmixer.SetType(DSound::MIXER_PERCUSSION);

    // main mixer
    DSound *dmix_synth[MIXER_CHANNELS_MAX];
    float dmix_pan[MIXER_CHANNELS_MAX];
    float dmix_level[MIXER_CHANNELS_MAX];
    float dmix_chorus_level[MIXER_CHANNELS_MAX];
    float dmix_reverb_level[MIXER_CHANNELS_MAX];
    bool dmix_mono[MIXER_CHANNELS_MAX];
    uint8_t dmix_group[MIXER_CHANNELS_MAX];
    DMixer::Config dmix_config;

    dmix_synth[0] = &dsynth0;
    dmix_level[0] = 0.2;
    dmix_pan[0] = 0.1f;
    dmix_chorus_level[0] = 0.1f;
    dmix_reverb_level[0] = 0.6f;
    dmix_mono[0] = true;
    dmix_group[0] = 0;

    dmix_synth[1] = &dsynth1;
    dmix_level[1] = 0.4;
    dmix_pan[1] = 0.9f;
    dmix_chorus_level[1] = 0.1f;
    dmix_reverb_level[1] = 0.6f;
    dmix_mono[1] = true;
    dmix_group[1] = 1;

    dmix_synth[2] = &dsynth2;
    dmix_level[2] = 0.1;
    dmix_pan[2] = 0.5f;
    dmix_chorus_level[2] = 0.6f;
    dmix_reverb_level[2] = 0.4f;
    dmix_mono[2] = true;
    dmix_group[2] = 2;

    dmix_synth[3] = &dsynthfm;
    dmix_level[3] = 0.4;
    dmix_pan[3] = 0.6f;
    dmix_chorus_level[3] = 0.3f;
    dmix_reverb_level[3] = 0.6f;
    dmix_mono[3] = true;
    dmix_group[3] = 3;

    dmix_synth[4] = &dsynthb;
    dmix_level[4] = 0.5;
    dmix_pan[4] = 0.5f;
    dmix_chorus_level[4] = 0.1f;
    dmix_reverb_level[4] = 0.3f;
    dmix_mono[4] = true;
    dmix_group[4] = 4;

    dmix_synth[5] = &ddmixer;
    dmix_level[5] = 5.0;
    dmix_pan[5] = 0.5f;
    dmix_chorus_level[5] = 0.0f;
    dmix_reverb_level[5] = 0.0f;
    dmix_mono[5] = false;
    dmix_group[5] = 5;

    dmix_config.sample_rate = DSTUDIO_SAMPLE_RATE;
    dmix_config.channels = 6;
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
    dmix_config.mix_dry = 0.5;
    dmixer.Init();
    dmixer.Set(dmix_config);

    // demo start
    testcount = 7;
    clocker.Init(500000);

	return retval;
}



//////////////////////////////////////////////////
// Application logic
//////////////////////////////////////////////////

void ProcessControl()
{
	if (clocker.Process()) {
		testcount++;
		if (testcount >= 8) {
			testcount = 0;
		}
		switch (testcount)
		{
		case 0:
			testnote1 = rand() % 20 + 40;

			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, testnote1, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, testnote1 + 3, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, testnote1 + 7, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 0, testnote1 + 10, MIDI_VELOCITY_MAX);

			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 1, testnote2, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 1, testnote2 + 7, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 1, testnote2 + 3, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 1, testnote2 + 10, MIDI_VELOCITY_MAX);

			testnote3 = rand() % 20 + 60;
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 2, testnote3, MIDI_VELOCITY_MAX);

			// dbass, ddmixer 0
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 0, MIDI_VELOCITY_MAX);
			break;

		case 1:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 4, testnote1 - 12, MIDI_VELOCITY_MAX);

			// dhihatc, ddmixer 2
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 2, MIDI_VELOCITY_MAX);
			break;

		case 2:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 3, testnote3 + 7, MIDI_VELOCITY_MAX);

			// dhihato, ddmixer 3
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 3, MIDI_VELOCITY_MAX);
			// dride, ddmixer 5
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 5, MIDI_VELOCITY_MAX);
			break;

		case 3:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 4, testnote1 - 12, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 3, testnote3 + 7, MIDI_VELOCITY_MAX);

			// dhihatc, ddmixer 2
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 2, MIDI_VELOCITY_MAX);
			break;

		case 4:
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 0, testnote1, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 0, testnote1 + 3, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 0, testnote1 + 7, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 0, testnote1 + 10, MIDI_VELOCITY_MAX);

			testnote2 = rand() % 20 + 40;
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, testnote2, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, testnote2 + 3, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, testnote2 + 7, MIDI_VELOCITY_MAX);
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 1, testnote2 + 10, MIDI_VELOCITY_MAX);

			dsnare.NoteOn(MIDI_VELOCITY_MAX);
			break;

		case 5:
			// dhihatc, ddmixer 2
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 2, MIDI_VELOCITY_MAX);

			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 3, testnote3, MIDI_VELOCITY_MAX);

			// dcrash, ddmixer 6
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 6, MIDI_VELOCITY_MAX);
			break;

		case 6:
			// dhihato, ddmixer 3
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 3, MIDI_VELOCITY_MAX);

			// dclap, ddmixer 3
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 4, MIDI_VELOCITY_MAX);

			// dtomhi, ddmixer 7
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 7, MIDI_VELOCITY_MAX);

			dmixer.MidiIn(MIDI_MESSAGE_NOTEOFF + 3, testnote3, MIDI_VELOCITY_MAX);
			break;

		case 7:
			// dhihatc, ddmixer 2
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 2, MIDI_VELOCITY_MAX);

			// dtomlo, ddmixer 3
			dmixer.MidiIn(MIDI_MESSAGE_NOTEON + 5, MIDI_PERCUSSION_START + 8, MIDI_VELOCITY_MAX);
			break;

		} // switch
	} // if


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
	ExitRtAudio();

	return 0;
}
