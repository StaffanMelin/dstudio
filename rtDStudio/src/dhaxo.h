#pragma once

#include <fcntl.h>
#include <sys/ioctl.h>
extern "C"
{
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}

#include <gpiod.h>

#include <map>
#include <cmath>
#include <stdio.h>
#include <iostream>

#include <libserial/SerialPort.h>

#include "dsound.h"
#include "dsynth.h"

#define DHAXO_KEY_ROWS 8
#define DHAXO_KEY_COLS 3
#define DHAXO_PRESSURE_START 2100
#define DHAXO_PRESSURE_MAX 2800 // 3330
#define DHAXO_PRESSURE_THRESHOLD 0.1f

#define DHAXO_CONTROLLER_TARGET_MAX 8
#define DHAXO_DIST_OFF 100
#define DHAXO_CONTROLLER_VALUE_MAX 8

#define DHAXO_SERIAL_BUFFER_SIZE 1024

class DHaxo
{
public:

    enum HaxoControl
    {
        HAXOCONTROL_NONE,
        HAXOCONTROL_PREVSOUND,
        HAXOCONTROL_NEXTSOUND,
        HAXOCONTROL_TURNOFF,
    };

	DHaxo() {}
	~DHaxo() {}

    struct Config
    {
        DSynth *synth;
		bool controller;
		uint8_t controller_targets;
		DSynth::Param controller_target[DHAXO_CONTROLLER_TARGET_MAX];
    };

	void Init();
	void Set(const Config&);
	DHaxo::HaxoControl ProcessControl();
	void Exit();

private:

	// DStudio
    DSynth *synth_;
	uint8_t channel_;
	bool controller_connected_;
	uint8_t controller_targets_;
	DSynth::Param controller_target_[DHAXO_CONTROLLER_TARGET_MAX];
	
	// module
	std::map<uint32_t, uint8_t> notemap_;
	int32_t pressure_baseline_;
	uint8_t note_, note_last_ = MIDI_NOTE_NONE;
	float vol_, vol_last_ = 0.0;
	
	// i2c - pressure sensor
	int i2cfile_;
	struct gpiod_chip *chip_;
	
	// gpio - keys
	struct gpiod_line *line_r_[DHAXO_KEY_ROWS];
	struct gpiod_line *line_c_[DHAXO_KEY_COLS];
	
	// optional controller - serial port
	LibSerial::SerialPort serial_port_;
	char serial_buffer_[DHAXO_SERIAL_BUFFER_SIZE];
	uint16_t serial_buffer_next_;
	float controller_value_[DHAXO_CONTROLLER_VALUE_MAX];

	// private methods
	float Pressure();
	uint32_t Keys();
	void DispatchController(DSynth::Param controller_target, float controller_value);

	// helper methods
	bool get_bit_at(uint32_t input, uint8_t n);
	void set_bit_at(uint32_t* output, uint8_t n);
	void clear_bit_at(uint32_t* output, uint8_t n);
	uint8_t map_to_midi(uint32_t);

	void show(uint32_t keys)
	{
	    for (int b = 0; b < DHAXO_KEY_ROWS * DHAXO_KEY_COLS; b++)
		{
			if (get_bit_at(keys, b))
			{
				std::cout << "1";
			}
		}
		std::cout << " " << keys << "\n";
	}
};
