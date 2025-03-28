#include <fstream>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "../rtDStudio/src/dstudio.h"
#include "dhaxo2.h"
#include "dsynth.h"



void DHaxo::Init()
{
    // read and parse dhaxo_notemap.json
    /*
    {
        "0": 73,
        "1": 85,
        ...
        "13444224": 58,
        "13444240": 58
        }
    }
    */
    std::ifstream file("../rtDStudio/src/dhaxo_notemap.json");
    std::string line;
    while (std::getline(file, line)) {
        if (!((line.rfind("{", 0) == 0) || (line.rfind("}", 0) == 0))) {
            // else split at : and extract string (to int) before it, and uint8_t after            
            size_t index_colon = line.find(":");
            size_t line_len = line.length();
            uint32_t map_key = std::stoi(line.substr(0, index_colon));
            // stoi() doesn't care about trailing comma
            uint8_t map_value = std::stoi(line.substr(index_colon + 1, line_len - index_colon));
            notemap_[map_key] = map_value;
        }
    }
    file.close();

    #ifdef DEBUG
    for(auto it = notemap_.cbegin(); it != notemap_.cend(); ++it)
    {
        std::cout << it->first << " " << (int)it->second << "\n";
    }
    std::cout << "\n";
    #endif

    // I2C - pressure sensor
	char filename[20];
  	const int adapter_nr = 1;
	const int addr = 0x4D;

    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    i2cfile_ = open(filename, O_RDWR);
    if (ioctl(i2cfile_, I2C_SLAVE, addr) < 0)
    {
        exit(1); // TODO
    }

    usleep(1000); // TODO really necessary?

    // TODO calibrate pressure sensor = read when not blowing (+ 10%?)
    pressure_baseline_ = 0;

    // GPIO - keys
	const int pin_r[DHAXO_KEY_ROWS] = {13, 12, 16, 17, 20, 22, 23, 24};
	const int pin_c[DHAXO_KEY_COLS] = {25, 26, 27};
	const char *chipname = "gpiochip0";

    // Open GPIO chip
    chip_ = gpiod_chip_open(chipname);

    // Open GPIO lines
    for (uint8_t i = 0; i < DHAXO_KEY_ROWS; i++)
    {
        line_r_[i] = gpiod_chip_get_line(chip_, pin_r[i]);
        gpiod_line_request_output(line_r_[i], "example1", 1); // HI default
    }
    for (int i = 0; i < DHAXO_KEY_COLS; i++)
    {
        line_c_[i] = gpiod_chip_get_line(chip_, pin_c[i]);
        gpiod_line_request_input(line_c_[i], "example1");
    }

}



void DHaxo::Set(const Config& config)
{
	synth_ = config.synth;
    channel_ = 0;
    controller_connected_ = config.controller;
    controller_targets_ = config.controller_targets;
    for (size_t i = 0; i < controller_targets_; i++)
    {
        controller_target_[i] = config.controller_target[i];
    }
    for (size_t i = 0; i < DHAXO_CONTROLLER_VALUE_MAX; i++)
    {
        controller_value_[i] = 0;
    }

    if (controller_connected_)
    {
        // TODO error check
        #ifdef DEBUG
        std::cout << "Open serial. "  << "\n";
        #endif

        try {
            serial_port_.Open("/dev/ttyACM0");
            serial_port_.SetBaudRate(LibSerial::BaudRate::BAUD_38400);
            serial_port_.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
            serial_port_.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
            serial_port_.SetParity(LibSerial::Parity::PARITY_ODD);
            serial_port_.SetStopBits(LibSerial::StopBits::STOP_BITS_1) ;

            serial_buffer_next_ = 0;
        }
        catch (...)
        {
            std::cout << "Could not open/init serial: " << std::endl;
            controller_connected_ = false;
        }
    }
}



uint8_t DHaxo::map_to_midi(uint32_t input)
{
    std::map<uint32_t, uint8_t>::iterator it;
    uint8_t ret;

    it = notemap_.find(input);
    if (it == notemap_.end())
    {
        ret = MIDI_NOTE_NONE;
    }
    else
    {
        ret = it->second;
    }

    return ret;
}



bool DHaxo::get_bit_at(uint32_t input, uint8_t n) {
    if (n < 32) {
        return (input & (1 << n)) != 0;
    } else {
        return false;
    }
}

void DHaxo::set_bit_at(uint32_t* output, uint8_t n) {
    if (n < 32) {
        *output |= (1 << n);
    }
}

void DHaxo::clear_bit_at(uint32_t* output, uint8_t n) {
    if (n < 32) {
        *output &= !(1 << n);
    }
}



// get pressure from sensor on I2C
// value is normalized from 0.0-1.0
float DHaxo::Pressure()
{
    uint8_t value[32];
	uint32_t pressure;
 	float pressure_normalized; // 0.0 - 1.0
    #ifdef DEBUG
    static uint32_t pmin = 3000;
    static uint32_t pmax = 0;
    #endif

    ssize_t bytes = read(i2cfile_, value, 2);
    pressure = (value[0] << 8) | (value[1]);

    #ifdef DEBUG
    if (pmin > pressure)
        pmin = pressure;
    if (pmax < pressure)
        pmax = pressure;
        std::cout << "pressure read: " << pressure  << " pmin:" << pmin << " pmax:" << pmax << "\n";
    #endif

    if (pressure < (DHAXO_PRESSURE_START * 0.9)) 
    {
        pressure_normalized = -1.0;
    } else { 
        if (pressure > DHAXO_PRESSURE_START)
        {
            if (pressure > DHAXO_PRESSURE_MAX)
            {
                pressure = DHAXO_PRESSURE_MAX;
            }
            pressure_normalized = 
                (pressure - DHAXO_PRESSURE_START) / (float)(DHAXO_PRESSURE_MAX - DHAXO_PRESSURE_START);
        } else {
            pressure_normalized = 0.0;
        }
    }
    return pressure_normalized;
}



// GPIO
uint32_t DHaxo::Keys()
{
    static uint32_t keymap = 0;
    uint8_t key_index = 0;

    for (uint8_t r = 0; r < DHAXO_KEY_ROWS; r++)
    {
        gpiod_line_set_value(line_r_[r], 0);
        usleep(10); // let row pin settle

        for (uint8_t c = 0; c < DHAXO_KEY_COLS; c++)
        {
            // keystate now
            bool is_pressed = (gpiod_line_get_value(line_c_[c]) == 0);

            // keystate has changed since last check?
            if (get_bit_at(keymap, key_index) != is_pressed) {
                if (is_pressed) {
                    set_bit_at(&keymap, key_index);
                } else {
                    clear_bit_at(&keymap, key_index);
                }
            }
            key_index++;
        }
        gpiod_line_set_value(line_r_[r], 1);
        usleep(10);
    }

    return keymap;
}



void DHaxo::DispatchController(DSynth::Param controller_target, float controller_value)
{

    // 
    // !!!neg values (mod wheel: )
    /*
        float 0.0 - 1.0 or -1.0 - 1.0
        value_normalized can be negative (eg mod wheel)
        but negative values are only allowed/working for:
            transpose, tune, detune
            DSYNTH_PARAM_TUNE,
            DSYNTH_PARAM_DETUNE,
            DSYNTH_PARAM_TRANSPOSE,
    */
   if (!(controller_target == DSynth::DSYNTH_PARAM_TUNE ||
       controller_target == DSynth::DSYNTH_PARAM_DETUNE ||
       controller_target == DSynth::DSYNTH_PARAM_TRANSPOSE))
    {
        controller_value = abs(controller_value);
    }

    #ifdef DEBUG
    #endif

    synth_->ChangeParam(controller_target, controller_value);
}



DHaxo::HaxoControl DHaxo::ProcessControl()
{
    DHaxo::HaxoControl haxo_control = DHaxo::HAXOCONTROL_NONE;

    uint32_t keys = Keys();
    float pressure = Pressure();

    #ifdef DEBUG
    std::cout << "dhaxo pressure: " << pressure  << "  keys " << keys << "\n";
    #endif

    if (pressure >= 0)
    {
        vol_ = pow(pressure, 0.5);
        if (vol_ != vol_last_)
        {
            synth_->SetLevel(vol_);
            vol_last_ = vol_;
        }

        if (vol_ >= DHAXO_PRESSURE_THRESHOLD)
        {
            uint8_t note_ = map_to_midi(keys);
            if (note_ != MIDI_NOTE_NONE)
            {
                if (note_last_ != MIDI_NOTE_NONE)
                {
                    // finish old note
                    synth_->MidiIn(MIDI_MESSAGE_NOTEOFF + channel_, note_last_, 0);
                }
                if (note_ != note_last_)
                {
                    // start new note
                    synth_->MidiIn(MIDI_MESSAGE_NOTEON + channel_, note_, 100);
                    note_last_ = note_;
                }
            }
        } else if (note_last_ != MIDI_NOTE_NONE) {
            synth_->MidiIn(MIDI_MESSAGE_NOTEOFF + channel_, note_last_, 0);
            note_last_ = MIDI_NOTE_NONE;
        }

    } else {
        #ifdef DEBUG
        show(keys);
        #endif
        switch (keys)
        {
        case 65536: // 2^16, right index finger
            haxo_control = HAXOCONTROL_NEXTSOUND;
            break;
        case 4194304: // 2^22, right ring finger
            haxo_control = HAXOCONTROL_PREVSOUND;
            break;
        case 524288: // 2^19, right middle finger
            haxo_control = HAXOCONTROL_TURNOFF;
            break;
        default:
            break;
        }
    }

    // TODO NOTE if target is pressure/volume/amp then
    // controller will "overwrite" pressure sensor
    // ie TARGET_AMP is stupid?
    if (controller_connected_)
    {
        /*
            Format of input:
            <pitch>,<modwheel>,<distancesensor>\n

            pitch = read value from analog pot
            modwheel = read value from analog pot
            distance sensor = distance

            so the targets and values must match per index
            order of received value -> hexo_value_[n]
            hexo_value_[n] is sent to hexo_target_[n]
        */

        //int available = serial_port_.GetNumberOfBytesAvailable() > 0;
        while (serial_port_.GetNumberOfBytesAvailable() > 0)
        {
            uint8_t controller = 0;
            char c;
            serial_port_.ReadByte(c);
            serial_buffer_[serial_buffer_next_++] = c;
            if (c == '\n')
            {
                serial_buffer_[serial_buffer_next_] = 0;
                char *token;
                char *sbuffer;
                char *token_end;
                sbuffer = serial_buffer_;
                // we now have a line of values, floats separated by commas
                while ((token = strsep(&sbuffer,",")) != NULL)
                {
                    float token_value = strtof(token, &token_end);
                    controller_value_[controller] = token_value;
                    controller++;
                    if (controller >= controller_targets_)
                        break;
                }
                serial_buffer_next_ = 0;
                for (uint8_t i = 0; i < controller_targets_ ; i++)
                {
                    #ifdef DEBUG
                    std::cout << " target: " << (int)controller_target_[i] << " value "  << controller_value_[i];
                    #endif
                    DispatchController(controller_target_[i], controller_value_[i]);
                }
                #ifdef DEBUG
                std::cout << "\n";
                #endif
            }
        }
    }
    return haxo_control;
}



void DHaxo::Exit()
{
    // Release lines and chip
    for (int r = 0; r < DHAXO_KEY_ROWS; r++)
    {
        gpiod_line_release(line_r_[r]);
    }
    for (int c = 0; c < DHAXO_KEY_COLS; c++)
    {
        gpiod_line_release(line_c_[c]);
    }
    gpiod_chip_close(chip_);
    close(i2cfile_);

    if (controller_connected_)
    {
        serial_port_.Close();
        // delete serial_buffer_;
    }
}
