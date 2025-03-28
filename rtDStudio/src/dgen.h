#pragma once

#include "dstudio.h"
#include "dsound.h"

#include "dseq.h"
#include <vector>

#define DGEN_DRAMA_MIN_LEN 4
#define DGEN_DRAMA_MAX_LEN 16
// MIDI pitch value indicating a rest
#define DREST 0

typedef std::vector<uint8_t> dgen_note_t;


class DGen
{

	public:

    enum ChannelType
    {
        BASS,
        TREBLE,
        PAD,
        MELODY,
        ARPEGGIO,
        EMBELLISH
    };

    enum DramaType
    {
        INTRO,
        VERSE,
        CHORUS,
        BREAK,
        OUTRO
    };

    DGen() {}
    ~DGen() {}

	struct Config
	{
        float bpm;
        uint8_t channels;
        DSound *child;
        ChannelType *channel_type;
        dgen_note_t note_base;
        dgen_note_t note_pad;
        dgen_note_t note_arp;
        dgen_note_t note_melody;
	};

    void Init();
    void Set(const Config&);
    void Start(DramaType);
    void Stop();
    void Process();
    void SetBPM(uint8_t);
    void SetDrama(DramaType);

protected:

	// config
    uint8_t bpm_;
    uint8_t channels_;
    DSound *child_;
    ChannelType channel_type_[MIXER_CHANNELS_MAX];

    uint64_t upt_; // microseconds per tick
    uint64_t now_us_;
    uint64_t prev_us_;
    bool running_;

    uint64_t ticks_[MIXER_CHANNELS_MAX];
    // buffer, sorted so next note to play is last
    std::vector<DMidiSeqStep> queue_[MIXER_CHANNELS_MAX];
    uint32_t queue_note_[MIXER_CHANNELS_MAX]; // position in queue

    uint8_t note_pad_len_;
    uint8_t note_arp_len_;
    uint8_t note_melody_len_;
    dgen_note_t note_base_;
    dgen_note_t note_pad_;
    dgen_note_t note_arp_;
    dgen_note_t note_melody_;

    DramaType drama_;

    void CalcTempo();
    void NoteCreate(uint8_t);
    void NoteQueue(uint8_t c, uint64_t tick, uint8_t pitch, uint8_t velocity, uint64_t length);

};



class DGenDrone  : public DGen
{

	public:

    DGenDrone() {}
    ~DGenDrone() {}

	struct Config
	{
        float bpm;
        uint8_t channels;
        DSound *child;
        ChannelType *channel_type;
        float *drama_fade; // dgendrone
        float *level; // dgendrone
        dgen_note_t note_base;
        dgen_note_t note_pad;
        dgen_note_t note_arp;
        dgen_note_t note_melody;
	};

    void Init();
    void Set(const Config&);
    void Start(DramaType);
    void Process();


private:

	// config
    float drama_fade_[MIXER_CHANNELS_MAX];
    float level_[MIXER_CHANNELS_MAX]; // dgendrone

    uint64_t drama_ticks_;
    uint64_t drama_start_, drama_end_;
    // 60 30 10 probability
    //                              INTRO                   VERSE                   CHORUS                 BREAK                   OUTRO
    DramaType drama_order_[5][3] = {{VERSE, CHORUS, OUTRO}, {CHORUS, BREAK, OUTRO}, {VERSE, BREAK, OUTRO}, {VERSE, CHORUS, OUTRO}, {INTRO, VERSE, CHORUS}};
};


