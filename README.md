# dstudio

Code for making sounds with my DSound software synth library (based on Daisy SP)

New:
* support for the [Haxophone hardware](https://github.com/cardonabits/haxo-hw),  [Haxophone](https://www.crowdsupply.com/cardona-bits/haxophone).

License: GPL3

Contact: staffan.melin@oscillator.se

## Setting up on the Raspberry Pi

TODO

## Setting up on the Raspberry Pi with Haxophone

Install raspi lite

`sudo raspi-config`

* expand file system
* enable i2c
* gpio is already enabled

edit /boot/firmware/config.txt
```
# Enable audio (loads snd_bcm2835)
# dtparam=audio=on
dtoverlay=max98357a
```

Add I2C support

`sudo apt install libi2c-dev`

Test:

`i2cdetect -l`

Add GPIO support

libgpiod already installed

`sudo apt install libgpiod-dev`

Test:
`gpiodetect`
`gpioinfo`

Add RtAudio support

`sudo apt install libasound2-dev libevent-dev`

Needs:
* libasound2 libasound2-data *libasound2-dev
* +libevent-pthread-... *libevent-dev
*=not already installed

TODO
