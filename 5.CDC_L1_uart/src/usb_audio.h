#ifndef _USB_AUDIO_H_
#define _USB_AUDIO_H_

#define USB_CLASS_AUDIO                 1
#define USB_SUBCLASS_AUDIOCONTROL       1
#define USB_SUBCLASS_AUDIOSTREAMING     2
#define USB_SUBCLASS_MIDISTREAMING      3

#define USBAUDIO_IF_HEADER              1
#define USBAUDIO_IF_TERM_IN             2
#define USBAUDIO_IF_TERM_OUT            3
#define USBAUDIO_IF_MIXER               4
#define USBAUDIO_IF_SELECTOR            5
#define USBAUDIO_IF_FEATURE             6
#define USBAUDIO_IF_PROCESSING          7
#define USBAUDIO_IF_EXTENSION           8

#define USBAUDIO_AS_GENERAL             1
#define USBAUDIO_AS_FORMAT              2

#define USBAUDIO_TERMINAL_USB           0x0101
#define USBAUDIO_TERMINAL_MIC           0x0201
#define USBAUDIO_TERMINAL_MIC_PERSONAL  0x0203
#define USBAUDIO_TERMINAL_SPEAKER       0x0301
#define USBAUDIO_TERMINAL_HEADPHONES    0x0302
#define USBAUDIO_TERMINAL_ANALOG        0x0601
#define USBAUDIO_TERMINAL_DIGITAL       0x0602
#define USBAUDIO_TERMINAL_LINE          0x0603

#define USBAUDIO_LOC_LEFTFRONT          (1<<0)
#define USBAUDIO_LOC_RIGHTFRONT         (1<<1)
#define USBAUDIO_LOC_CENTERFRONT        (1<<2)
#define USBAUDIO_LOC_LOWFREQ_ENCHAN     (1<<3)
#define USBAUDIO_LOC_LEFTSURROUND       (1<<4)
#define USBAUDIO_LOC_RIGHTSURROUND      (1<<5)
#define USBAUDIO_LOC_LEFTCENTER         (1<<6)
#define USBAUDIO_LOC_RIGHTCENTER        (1<<7)
#define USBAUDIO_LOC_SURROUND           (1<<8)
#define USBAUDIO_LOC_SIDELEFT           (1<<9)
#define USBAUDIO_LOC_SIDERIGHT          (1<<10)
#define USBAUDIO_LOC_TOP                (1<<11)

#define USBAUDIO_FEATURE_NONE           0
#define USBAUDIO_FEATURE_MUTE           (1<<0)
#define USBAUDIO_FEATURE_VOLUME         (1<<1)
#define USBAUDIO_FEATURE_BASS           (1<<2)
#define USBAUDIO_FEATURE_MID            (1<<3)
#define USBAUDIO_FEATURE_TREBLE         (1<<4)
#define USBAUDIO_FEATURE_EQUALIZER      (1<<5)
#define USBAUDIO_FEATURE_AUTO_GAIN      (1<<6)
#define USBAUDIO_FEATURE_DELAY          (1<<7)
#define USBAUDIO_FEATURE_LOUDNESS       (1<<8)

#define USBAUDIO_FORMAT_PCM             0x0001 //signed integer with X bit resolution
#define USBAUDIO_FORMAT_PCM8            0x0001 //uint8_t

#define USB_AC24(x) ((x)&0xFF), (((x)>>8)&0xFF), (((x)>>16)&0xFF)

#endif
