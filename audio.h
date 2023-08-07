#ifndef __AUDIO_H_INCLUDED__
#define __AUDIO_H_INCLUDED__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dos.h>
#include "globals.h"
#include "keyboard.h"

// Sound FX
#define BEEP			0
#define CYCLE_ITEM		1
#define CYCLE_WEAPON	2
#define DOOR			3
#define FIRE_PISTOL		4
#define BEEP2			5
#define USE_EMP			6
#define ERROR			7
#define ITEM_FOUND		8
#define USE_MAGNET		9
#define USE_MAGNET2		10
#define USE_MEDKIT		11
#define MOVE_OBJECT		12
#define FIRE_PLASMA		13
#define SHOCK			14
#define EXPLOSION		15
#define EXPLOSION2		16

#define DEVICE_NONE		0
#define DEVICE_SPEAKER	1
#define DEVICE_TANDY	2
#define DEVICE_ADLIB	3
#define DEVICE_OPL2LPT	4
#define DEVICE_SB		5

#define SSY_DEV_PCSPE	1
#define SSY_DEV_TANDY	2
#define SSY_DEV_ADLIB	4
#define SSY_DEV_OPL2LPT	8
#define SSY_DEV_SB		16

// #define ADLIB_PORT_ADDR	0x0388

#define TANDY_PORT		0x00C0

#define PP_NOT_STROBE	0x0001
#define PP_NOT_AUTOFD	0x0002
#define PP_INIT			0x0004
#define PP_NOT_SELECT	0x0008

#define CH_MAX			9

extern uint8_t SSY_DEVICE_SFX;		// Device for sound effects
extern uint8_t SSY_DEVICE_MUS;		// Device for music
extern char SSY_SFX_EXT[5];			// File extension for sound effects
extern char SSY_MUS_EXT[5];			// File extension for music
extern uint8_t SSY_MUS_ENABLE;		// Music enable flag

int soundSelect(const char *arg);
int musicSelect(const char *arg);
void ssy_sound_play(uint8_t sound);
void ssy_music_play(uint16_t musicseg);
void ssy_music_stop();
void ssy_timer_isr();


#endif //__AUDIO_H_INCLUDED__
