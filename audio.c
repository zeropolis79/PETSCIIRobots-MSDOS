#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include "audio.h"
#include "globals.h"
#include "keyboard.h"
#include "string.h"
#include "memory.h"

uint16_t ADLIB_PORT_ADDR = 0x0388;
uint16_t OPL2LPT_PORT_ADDR = 0x0378;

uint8_t WAIT[CH_MAX];
uint8_t VOLUME[CH_MAX];			// Volume for speaker and tandy
uint8_t *RET[CH_MAX];			// Return pointer from a block reference
uint8_t *PTR[CH_MAX];			// Current pointer
uint8_t *LOOP[CH_MAX];			// Loop pointer
uint16_t PITCH_1[CH_MAX];		// Pitch for speaker, previous pitch for adlib
uint16_t PITCH_2[CH_MAX];		// Pitch for tandy and adlib
uint8_t VOLPREV[CH_MAX];		// Previous volume value to track down the changes
uint8_t *REFPREV[CH_MAX];		// Previous reference pointer
uint8_t KEYOFF[CH_MAX];			// Op12 keyoff flag
uint8_t VOLOPL[CH_MAX];			// Op12 specific volume (more bits and extra data)
uint8_t OPL_REGS[CH_MAX][10];
uint8_t OPL_PREV[CH_MAX][10];

uint8_t SSY_CHANNELS = 0;		// Number of active data streams
uint8_t SSY_DEVICE_SFX = 0;		// Device for sound effects
uint8_t SSY_DEVICE_MUS = 0;		// Device for music
char SSY_SFX_EXT[5];			// File extension for sound effects
char SSY_MUS_EXT[5];			// File extension for music
uint8_t SSY_MUS_ENABLE = 0;		// Music enable flag
uint8_t SSY_USE_DEVICE = 0;		// Flags to handle particular devices (SSY_DEV...) if needed for sound/music
uint8_t SSY_PCSPE_ENABLE = 0;	// Temporary variable for PC Speaker state
uint16_t SSY_PCSPE_PERIOD;		// Temporary variable for PC Speaker state
uint8_t SSY_TANDY_MODE;			// SN76489 noise mode value
uint8_t *SSY_MUSIC;				// Pointer to start of current music being played
uint8_t SSY_DELAY;				// Delay before sending to the register

volatile uint8_t SOUND_PRIORITY;// The current playing sound's priority level. This resets to 0 when the sound is finished playing.
volatile uint8_t SOUND_TIMER;	// The countdown timer for the current sound being played, before it can be interrupted by a lower priority sound

const uint8_t SSY_OPL2_OPERATOR_ORDER[9] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};

const uint8_t SSY_OPL2_INSTRUMENT_REGS[CH_MAX][10] = {
	{0xC0, 0x20, 0x40, 0x60, 0x80, 0xE0, 0x63, 0x83, 0x23, 0xE3},
	{0xC1, 0x21, 0x41, 0x61, 0x81, 0xE1, 0x64, 0x84, 0x24, 0xE4},
	{0xC2, 0x22, 0x42, 0x62, 0x82, 0xE2, 0x65, 0x85, 0x25, 0xE5},
	{0xC3, 0x28, 0x48, 0x68, 0x88, 0xE8, 0x6B, 0x8B, 0x2B, 0xEB},
	{0xC4, 0x29, 0x49, 0x69, 0x89, 0xE9, 0x6C, 0x8C, 0x2C, 0xEC},
	{0xC5, 0x2A, 0x4A, 0x6A, 0x8A, 0xEA, 0x6D, 0x8D, 0x2D, 0xED},
	{0xC6, 0x30, 0x50, 0x70, 0x90, 0xF0, 0x73, 0x93, 0x33, 0xF3},
	{0xC7, 0x31, 0x51, 0x71, 0x91, 0xF1, 0x74, 0x94, 0x34, 0xF4},
	{0xC8, 0x32, 0x52, 0x72, 0x92, 0xF2, 0x75, 0x95, 0x35, 0xF5}
};

const uint8_t SOUND_PLAY_PRIORITY[17] = {
	0,  // BEEP
	2,  // CYCLE_ITEM
	2,  // CYCLE_WEAPON
	1,  // DOOR
	5,  // FIRE_PISTOL
	1,  // BEEP2
	11, // USE_EMP
	3,  // ERROR
	4,  // ITEM_FOUND
	9,  // USE_MAGNET
	10, // USE_MAGNET2
	12, // USE_MEDKIT
	7,  // MOVE_OBJECT
	6,  // FIRE_PLASMA
	8,  // SHOCK
	13, // EXPLOSION
	13, // EXPLOSION2
};

const uint8_t SOUND_PLAY_TIMER[17] = {
	5,  // BEEP
	10, // CYCLE_ITEM
	10, // CYCLE_WEAPON
	30, // DOOR
	5,  // FIRE_PISTOL
	5,  // BEEP2
	20, // USE_EMP
	20, // ERROR
	20, // ITEM_FOUND
	50, // USE_MAGNET
	50, // USE_MAGNET2
	15, // USE_MEDKIT
	5,  // MOVE_OBJECT
	10, // FIRE_PLASMA
	20, // SHOCK
	30, // EXPLOSION
	40, // EXPLOSION2
};

//write to adlib register
//IN: AL=register number, CL=register value
void ssy_adlib_write(uint8_t regnum, uint8_t regval){
	uint8_t i;
	
	_asm{
		mov dx, ADLIB_PORT_ADDR
		mov al, regnum
		out dx, al
		// Delay 12 cycles
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		inc dx
		mov al, regval
		out dx, al
		dec dx
		// Delay 84 cycles
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
	}
}

void ssy_opl2lpt_write(uint8_t regnum, uint8_t regval){
	uint8_t i;
	
// PP_NOT_STROBE	=001h
// PP_NOT_AUTOFD	=002h
// PP_INIT			=004h
// PP_NOT_SELECT	=008h
	
	_asm{
		// LPT data
		mov dx, OPL2LPT_PORT_ADDR
		mov al, regnum
		out dx, al
		// LPT ctrl
		add dx, 2
		mov al, 13
		out dx, al
		xor al, 4
		out dx, al
		xor al, 4
		out dx, al
		
		// Delay 1
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		
		// Output sound data
		sub dx, 2
		mov al, regval
		out dx, al

		// LPT ctrl2
		add dx, 2
		mov al, 12
		out dx, al
		xor al, 4
		out dx, al
		xor al, 4
		out dx, al

		// Delay 2
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
		in al, dx
	}
}

void ssy_adlib_shut(){
	uint8_t i, i2;
	
	ssy_adlib_write(0x01, 0x20);
	for (i = 2; i < 0x20; i++){
		ssy_adlib_write(i, 0);
	}
	for (i = 0x20; i < 0xA0; i++){
		ssy_adlib_write(i, 255);
	}
	for (i = 0xA0; i < 0xF6; i++){
		ssy_adlib_write(i, 0);
	}
	for (i = 0; i < 9; i++){
		// Quick attack, longest decay
		ssy_adlib_write(0x60 + SSY_OPL2_OPERATOR_ORDER[i], 0xE0);
		// The same for operator 1 and 2
		ssy_adlib_write(0x63 + SSY_OPL2_OPERATOR_ORDER[i], 0xE0);
		// Max sustain, quick release
		ssy_adlib_write(0x80 + SSY_OPL2_OPERATOR_ORDER[i], 0x0E);
		// These setting remain unchanged
		ssy_adlib_write(0x83 + SSY_OPL2_OPERATOR_ORDER[i], 0x0E);
	}
}

void ssy_opl2lpt_shut(){
	uint8_t i, i2;
	
	ssy_opl2lpt_write(0x01, 0x20);
	for (i = 2; i < 0x20; i++){
		ssy_opl2lpt_write(i, 0);
	}
	for (i = 0x20; i < 0xA0; i++){
		ssy_opl2lpt_write(i, 255);
	}
	for (i = 0xA0; i < 0xF6; i++){
		ssy_opl2lpt_write(i, 0);
	}
	for (i = 0; i < 9; i++){
		// Quick attack, longest decay
		ssy_opl2lpt_write(0x60 + SSY_OPL2_OPERATOR_ORDER[i], 0xE0);
		// The same for operator 1 and 2
		ssy_opl2lpt_write(0x63 + SSY_OPL2_OPERATOR_ORDER[i], 0xE0);
		// Max sustain, quick release
		ssy_opl2lpt_write(0x80 + SSY_OPL2_OPERATOR_ORDER[i], 0x0E);
		// These setting remain unchanged
		ssy_opl2lpt_write(0x83 + SSY_OPL2_OPERATOR_ORDER[i], 0x0E);
	}
}

void ssy_adlib_update(){
	uint8_t i, channel, channel_max;
	
	// Check which channels and how many channels needs to be updated depending on the assigned devices
	SSY_DELAY = 0;
	channel = 0;
	channel_max = CH_MAX;
	if (SSY_DEVICE_SFX != DEVICE_ADLIB){
		channel = 1;
	}
	if (SSY_DEVICE_MUS != DEVICE_ADLIB){
		channel_max = 1;
	}
	for (; channel < channel_max; channel++){
		if (KEYOFF[channel] != 0){
			KEYOFF[channel] = 0;
			ssy_adlib_write(0xB0 + channel, 0);
		}
		// Update volume if it has changed
		if (VOLOPL[channel] != VOLPREV[channel]){
			VOLPREV[channel] = VOLOPL[channel];
			ssy_adlib_write(0x43 + SSY_OPL2_OPERATOR_ORDER[channel], VOLOPL[channel]);
		}
		// Loop through registers and only update them if they have changed
		for (i = 0; i < 10; i++){
			if (OPL_REGS[channel][i] != OPL_PREV[channel][i]){
				OPL_PREV[channel][i] = OPL_REGS[channel][i];
				ssy_adlib_write(SSY_OPL2_INSTRUMENT_REGS[channel][i], OPL_REGS[channel][i]);
			}
		}
		// Send pitch if it has changed
		if (PITCH_2[channel] != PITCH_1[channel]){
			PITCH_1[channel] = PITCH_2[channel];
			// Set pitch lsb
			ssy_adlib_write(0xA0 + channel, PITCH_2[channel]);
			// Set pitch msb and key on, pitch change always does key on
			ssy_adlib_write(0xB0 + channel, PITCH_2[channel] >> 8);
		}
	}
}

void ssy_opl2lpt_update(){
	uint8_t i, channel, channel_max;
	
	// Check which channels and how many channels needs to be updated depending on the assigned devices
	SSY_DELAY = 0;
	channel = 0;
	channel_max = CH_MAX;
	if (SSY_DEVICE_SFX != DEVICE_OPL2LPT){
		channel = 1;
	}
	if (SSY_DEVICE_MUS != DEVICE_OPL2LPT){
		channel_max = 1;
	}
	for (; channel < channel_max; channel++){
		if (KEYOFF[channel] != 0){
			KEYOFF[channel] = 0;
			ssy_opl2lpt_write(0xB0 + channel, 0);
		}
		// Update volume if it has changed
		if (VOLOPL[channel] != VOLPREV[channel]){
			VOLPREV[channel] = VOLOPL[channel];
			ssy_opl2lpt_write(0x43 + SSY_OPL2_OPERATOR_ORDER[channel], VOLOPL[channel]);
		}
		// Loop through registers and only update them if they have changed
		for (i = 0; i < 10; i++){
			if (OPL_REGS[channel][i] != OPL_PREV[channel][i]){
				OPL_PREV[channel][i] = OPL_REGS[channel][i];
				ssy_opl2lpt_write(SSY_OPL2_INSTRUMENT_REGS[channel][i], OPL_REGS[channel][i]);
			}
		}
		// Send pitch if it has changed
		if (PITCH_2[channel] != PITCH_1[channel]){
			PITCH_1[channel] = PITCH_2[channel];
			// Set pitch lsb
			ssy_opl2lpt_write(0xA0 + channel, PITCH_2[channel]);
			// Set pitch msb and key on, pitch change always does key on
			ssy_opl2lpt_write(0xB0 + channel, PITCH_2[channel] >> 8);
		}
	}
}

void ssy_pcspe_update(){
	uint8_t channel, spkval;
	
	SSY_PCSPE_PERIOD = 0;
	SSY_PCSPE_ENABLE = 0;
	
	// Process music if enabled
	if (SSY_MUS_ENABLE && SSY_DEVICE_MUS == DEVICE_SPEAKER){
		for (channel = 4; channel > 0; channel--){
			if (VOLUME[channel]){
				SSY_PCSPE_ENABLE = 1;
				SSY_PCSPE_PERIOD = PITCH_1[channel];
				break;
			}
		}
	}
	// Process sound effects if enabled
	if (SSY_DEVICE_SFX == DEVICE_SPEAKER){
		if (VOLUME[0]){
			SSY_PCSPE_ENABLE = 1;
			SSY_PCSPE_PERIOD = PITCH_1[0];
		}
	}
	// Send resulting pitch to PC Speaker
	if (SSY_PCSPE_ENABLE){
		spkval = inp(0x061) | 0x03;
		outp(0x61, spkval);
		outp(0x42, SSY_PCSPE_PERIOD & 0x00FF);
		outp(0x42, (SSY_PCSPE_PERIOD & 0xFF00) >> 8);
	} else{
		// Disable speaker
		spkval = inp(0x061) & 0xfc;
		outp(0x61, spkval);
	}
}		

void ssy_pcspe_init(){
	outp(0x043, 0x0b6);
}

void ssy_pcspe_shut(){
	uint16_t val;
	
	val = inp(0x061);
	val &= 0x0FC;
	outp(0x061, val);
}

void ssy_adlib_init(){
	ssy_adlib_shut();
}

void ssy_opl2lpt_init(){
	ssy_opl2lpt_shut();
}

void ssy_init(){
	unsigned int i, i2;
	
	// Clear some vars
	SOUND_ENABLED = 1;
	SSY_MUS_ENABLE = 0;
	SSY_CHANNELS = 1;
	// Clear data and force OPL registers update
	for (i = 0; i < CH_MAX; i++){
		WAIT[i] = 0;
		VOLUME[i] = 0;
		RET[i] = 0;
		PTR[i] = 0;
		LOOP[i] = 0;
		PITCH_1[i] = 0;
		PITCH_2[i] = 0;
		VOLPREV[i] = 0;
		REFPREV[i] = 0;
		KEYOFF[i] = 0;
		VOLOPL[i] = 0;
		for (i2 = 0; i2 < 10; i2++){
			OPL_REGS[i][i2] = 0;
			OPL_PREV[i][i2] = 1;
		}
	}
	// Check which sound device was selected
	switch (SSY_DEVICE_SFX){
		case DEVICE_SPEAKER: {
			SSY_USE_DEVICE = SSY_DEV_PCSPE;
			break;
		}
		case DEVICE_ADLIB: {
			SSY_USE_DEVICE = SSY_DEV_ADLIB;
			break;
		}
		case DEVICE_OPL2LPT: {
			SSY_USE_DEVICE = SSY_DEV_OPL2LPT;
			break;
		}
		case DEVICE_SB: {
			SSY_USE_DEVICE = SSY_DEV_SB;
			break;
		}
		default:{
			SSY_USE_DEVICE = 0;
		}
	}
	// Check which music device was selected and OR it with the sound device
	switch (SSY_DEVICE_MUS){
		case DEVICE_SPEAKER: {
			SSY_USE_DEVICE |= SSY_DEV_PCSPE;
			break;
		}
		case DEVICE_TANDY: {
			SSY_USE_DEVICE |= SSY_DEV_TANDY;
			break;
		}
		case DEVICE_ADLIB: {
			SSY_USE_DEVICE |= SSY_DEV_ADLIB;
			break;
		}
		case DEVICE_OPL2LPT: {
			SSY_USE_DEVICE |= SSY_DEV_OPL2LPT;
			break;
		}
	}
	// Init the specific devices that were selected
	if (SSY_USE_DEVICE & SSY_DEV_PCSPE){
		ssy_pcspe_init();
	}
	if (SSY_USE_DEVICE & SSY_DEV_TANDY){
		//ssy_tandy_init();
	}
	if (SSY_USE_DEVICE & SSY_DEV_ADLIB){
		ssy_adlib_init();
	}
	if (SSY_USE_DEVICE & SSY_DEV_OPL2LPT){
		ssy_opl2lpt_init();
	}
	if (SSY_USE_DEVICE & SSY_DEV_SB){
		//ssy_sb_init();
	}
}

void ssy_sound_play(uint8_t sound){
	uint8_t *SFX;
	
	// Check if no sound device selected or no sound effects loaded
	if (!SSY_DEVICE_SFX){
		return;
	}
	
	SFX = memorySegToPtr(SOUNDFX);
	
	// CHECK FOR SAMPLE PLAYED

	// // Check if sound number is not in range
	// if ((sound & 0x1F) > *(SFX + 1)){
		// return;
	// }
	// // Check if sound priority is less than an active effect
	// if ((sound & 0xE0) < PRIORITY[0]){
		// return;
	// }
	// Check if sound finished playing
	if (SOUND_TIMER != 0){
		if (SOUND_PLAY_PRIORITY[sound] < SOUND_PRIORITY){
			return;
		}
	}
	SOUND_TIMER = SOUND_PLAY_TIMER[sound];
	SOUND_PRIORITY = SOUND_PLAY_PRIORITY[sound];
	WAIT[0] = 0;
	VOLUME[0] = 0;
	VOLOPL[0] = 0;
	PITCH_2[0] = 0;
	VOLPREV[0] = 255;
	PITCH_1[0] = 65535;
	KEYOFF[0] = 255;
	PTR[0] = SFX + ((*(SFX + ((sound & 0x1F) << 1) + 3) << 8) | *(SFX + ((sound & 0x1F) << 1) + 2));
	// PRIORITY[0] = (sound & 0xE0);
}

void ssy_music_stop(){
	uint8_t channel, i2;
	
	SSY_MUS_ENABLE = 0;
	SSY_CHANNELS = 1;
	// Set all music channels volume to 0 to mute the music without stopping sound effects
	for (channel = 1; channel < CH_MAX; channel++){
		VOLUME[channel] = 0;
		VOLOPL[channel] = 0xFF;
		VOLPREV[channel] = 0x55;
		// Added to actually turn off the volume
		if (SSY_DEVICE_MUS == DEVICE_ADLIB){
			ssy_adlib_write(0x43 + SSY_OPL2_OPERATOR_ORDER[channel], VOLOPL[channel]);
			for (i2 = 0; i2 < 10; i2++){
				// OPL_REGS[channel][i2] = 0;
				// OPL_PREV[channel][i2] = 255;
				// KEYOFF[i2] = 1;
				ssy_adlib_write(SSY_OPL2_INSTRUMENT_REGS[channel][i2], 0);
			}
		} else if (SSY_DEVICE_MUS == DEVICE_OPL2LPT){
			ssy_opl2lpt_write(0x43 + SSY_OPL2_OPERATOR_ORDER[channel], VOLOPL[channel]);
			for (i2 = 0; i2 < 10; i2++){
				// OPL_REGS[channel][i2] = 0;
				// OPL_PREV[channel][i2] = 255;
				// KEYOFF[i2] = 1;
				ssy_opl2lpt_write(SSY_OPL2_INSTRUMENT_REGS[channel][i2], 0);
			}
		}
	}
	// // Added to fix issues:
	// for (channel = 0; channel < CH_MAX; channel++){
	// }
}

void ssy_music_play(uint16_t musicseg){
	uint8_t channel, i, *MUSIC;
	
	ssy_music_stop();
	MUSIC = memorySegToPtr(musicseg);
	// Check if a music device is assigned and the music is loaded
	if (SSY_DEVICE_MUS && MUSIC){
		// Clear all music channel vars
		for (channel = 1; channel < CH_MAX; channel++){
			WAIT[channel] = 0;
			VOLUME[channel] = 0;
			RET[channel] = 0;
			PTR[channel] = 0;
			LOOP[channel] = 0;
			PITCH_1[channel] = 0;
			PITCH_2[channel] = 0;
			VOLPREV[channel] = 0;
			// PRIORITY[channel] = 0;
			REFPREV[channel] = 0;
			KEYOFF[channel] = 0;
			VOLOPL[channel] = 0;
			for (i = 0; i < 10; i++){
				OPL_REGS[channel][i] = 0;
				OPL_PREV[channel][i] = 0;
			}
		}
		SSY_MUSIC = MUSIC;
		// Read number of channels from music data
		SSY_CHANNELS = *(MUSIC + 1) + 1;
		// Set up channels
		for (channel = 1; channel < CH_MAX; channel++){
			if (channel < SSY_CHANNELS){
				PTR[channel] = MUSIC + ((*(MUSIC + (channel << 1) + 1) << 8) | *(MUSIC + (channel << 1)));
				LOOP[channel] = PTR[channel];
				// Force volume update
				VOLPREV[channel] = 0xff;
			} else{
				PTR[channel] = NULL;
			}
		}
		SSY_MUS_ENABLE = 1;
	}
}

//------------------------------------------------------------------------------
//sound system update
//call it from timer ISR, at 72.8 HZ rate (8253 divider 16384, 1193180/16384=72.8)
//IN: none
void ssy_timer_isr(){
	uint8_t channel, data;
	
	if (SOUND_ENABLED){
		// Process channels
		for (channel = 0; channel < SSY_CHANNELS; channel++){
			// Wait counter. Decrease if non-zero, update channel if zero
			if (WAIT[channel] != 0){
				WAIT[channel]--;
				if (WAIT[channel] != 0){
					continue;
				}
			}
			// Update channel if channel data pointer is not zero
			if (PTR[channel] != 0){
				while (1) {
					// Read next byte from channel data
					data = *(PTR[channel]);
					// printf("%u\n", data);
					// Advance channel data pointer
					PTR[channel]++;
					// Check if the data is a wait value
					if (data < 0xC0){
						// Set channel wait time
						WAIT[channel] = data;
						break;
					} else if (data < 0xD0){
						// Set channel volume
						VOLUME[channel] = (data & 0x0F);
					} else if (data == 0xD0){
						// Set Pitch 1 LSB
						VOLUME[channel] = data;
						PITCH_1[channel] &= 0xFF00;
						PITCH_1[channel] |= *(PTR[channel]);
						PTR[channel]++;
						break;
					} else if (data == 0xD1){
						// Set Pitch 1 MSB
						VOLUME[channel] = data;
						PITCH_1[channel] &= 0x00FF;
						PITCH_1[channel] |= (*(PTR[channel]) << 8);
						PTR[channel]++;
						break;
					} else if (data == 0xD2){
						// Set Pitch 1 Word
						VOLUME[channel] = data;
						PITCH_1[channel] = (*(PTR[channel] + 1) << 8) | *(PTR[channel]);
						PTR[channel] += 2;
						break;
					} else if (data == 0xD3){
						// Read reference short pointer
						data = *(PTR[channel]);
						PTR[channel]++;
						RET[channel] = PTR[channel];
						PTR[channel] -= (2 + data);
						REFPREV[channel] = PTR[channel];
					} else if (data == 0xD4){
						// Set Pitch 2 LSB
						PITCH_2[channel] &= 0xFF00;
						PITCH_2[channel] |= *(PTR[channel]);
						// Force pitch update and opl2 keyon
						PITCH_1[channel] = ~PITCH_2[channel];
						PTR[channel]++;
						break;
					} else if (data == 0xD5){
						// Set Pitch 2 MSB
						PITCH_2[channel] &= 0x00FF;
						PITCH_2[channel] |= (*(PTR[channel]) << 8);
						// Force pitch update and opl2 keyon
						PITCH_1[channel] = ~PITCH_2[channel];
						PTR[channel]++;
						break;
					} else if (data == 0xD6){
						// Set Pitch 2 Word
						PITCH_2[channel] = (*(PTR[channel] + 1) << 8) | *(PTR[channel]);
						// Force pitch update and opl2 keyon
						PITCH_1[channel] = ~PITCH_2[channel];
						PTR[channel] += 2;
						break;
					} else if (data == 0xD7){
						// Read reference repeat
						RET[channel] = PTR[channel];
						PTR[channel] = REFPREV[channel];
					} else if (data == 0xD8){
						SSY_TANDY_MODE = *(PTR[channel]);
						PTR[channel]++;
					} else if (data < 0xE3){
						// Write byte to virtual CPL register array
						OPL_REGS[channel][data - 0xD9] = *(PTR[channel]);
						PTR[channel]++;
					} else if (data == 0xE3){
						// Get volume byte
						VOLOPL[channel] = *(PTR[channel]);
						PTR[channel]++;
					} else if (data == 0xE4){
						// Set keyoff to a non-zero value
						KEYOFF[channel] = data;
					} else if (data == 0xFC){
						// Return from reference pointer
						if (channel == 0){
							// Stop the channel
							PTR[channel] = 0;
							// Reset volume
							VOLUME[channel] = 0;
							// Reset channel priority for sound effects
							// PRIORITY[channel] = 0;
							break;
						}
						if (RET[channel] != 0){
							PTR[channel] = RET[channel];
							RET[channel] = 0;
						}
					} else if (data == 0xFD){
						// Call long reference
						RET[channel] = PTR[channel] + 2;
						PTR[channel] = SSY_MUSIC + ((*(PTR[channel] + 1) << 8) | *(PTR[channel]));
						REFPREV[channel] = PTR[channel];
					} else if (data == 0xFE){
						// Start loop
						LOOP[channel] = PTR[channel];
					} else if (data == 0xFF){
						// Take loop
						PTR[channel] = LOOP[channel];
					}
				}
			}
		}
		if (SSY_CHANNELS){
			// Update/Send Data to devices that are in use
			if (SSY_USE_DEVICE & SSY_DEV_ADLIB){
				ssy_adlib_update();
			}
			if (SSY_USE_DEVICE & SSY_DEV_PCSPE){
				ssy_pcspe_update();
			}
			if (SSY_USE_DEVICE & SSY_DEV_TANDY){
				//ssy_tandy_update();
			}
			if (SSY_USE_DEVICE & SSY_DEV_OPL2LPT){
				ssy_opl2lpt_update();
			}
		}
		// Update timer for sound being played
		if (SOUND_TIMER != 0){
			SOUND_TIMER--;
			if (SOUND_TIMER == 0){
				SOUND_PRIORITY = 0;
			}
		}
	}
}

int soundSelect(const char *arg){
	int key;
	uint8_t str[10];
	
	// Prompt to choose which sound card to use
	puts("Choose Sound Device:");
	puts("-------------------");
	if (arg[0] == 0 || arg[0] == '0'){
		puts("0-None");
		key = SCAN_0;
	}
	if (arg[0] == 0 || arg[0] == '1'){
		puts("1-Adlib");
		key = SCAN_1;
	}
	if (arg[0] == 0 || arg[0] == '2'){
		puts("2-OPL2LPT");
		key = SCAN_2;
	}
	if (arg[0] == 0 || arg[0] == '3'){
		puts("3-PC Speaker");
		key = SCAN_3;
	}
	if (arg[0] == 0){
		key = 0;
	}
	SSY_DEVICE_SFX = 0;
	// Wait for keypress until we get a valid choice
	do{
		if (key == 0){
			key = keyWait();
		}
		switch (key){
			case SCAN_0:{
				SSY_DEVICE_SFX = 0;
				break;
			}
			case SCAN_1:{
				SSY_DEVICE_SFX = DEVICE_ADLIB;
				strcpy(SSY_SFX_EXT, ".ADL");
				break;
			}
			case SCAN_2:{
				SSY_DEVICE_SFX = DEVICE_OPL2LPT;
				strcpy(SSY_SFX_EXT, ".ADL");
				break;
			}
			case SCAN_3:{
				SSY_DEVICE_SFX = DEVICE_SPEAKER;
				strcpy(SSY_SFX_EXT, ".SPK");
				break;
			}
			default: key = 0;
		}
	} while (key == 0);
	if (key == SCAN_0){
		str[0] = '0';
	} else{
		str[0] = 47 + key;
	}
	str[1] = 0;
	puts(str);
	
	// SET BLASTER IF SELECTED
	
	return key;
}

int musicSelect(const char *arg){
	int key;
	uint8_t str[10];
	
	// Prompt to choose which sound card to use
	puts("Choose Music Device:");
	puts("-------------------");
	if (arg[0] == 0 || arg[0] == '0'){
		puts("0-None");
		key = SCAN_0;
	}
	if (arg[0] == 0 || arg[0] == '1'){
		puts("1-Adlib");
		key = SCAN_1;
	}
	if (arg[0] == 0 || arg[0] == '2'){
		puts("2-OPL2LPT");
		key = SCAN_2;
	}
	if (arg[0] == 0 || arg[0] == '3'){
		puts("3-PC Speaker");
		key = SCAN_3;
	}
	if (arg[0] == 0){
		key = 0;
	}
	// Wait for keypress until we get a valid choice
	do{
		if (key == 0){
			key = keyWait();
		}
		switch (key){
			case SCAN_0:{
				SSY_DEVICE_MUS = 0;
				break;
			}
			case SCAN_1:{
				SSY_DEVICE_MUS = DEVICE_ADLIB;
				strcpy(SSY_MUS_EXT, ".ADL");
				break;
			}
			case SCAN_2:{
				SSY_DEVICE_MUS = DEVICE_OPL2LPT;
				strcpy(SSY_MUS_EXT, ".ADL");
				break;
			}
			case SCAN_3:{
				SSY_DEVICE_MUS = DEVICE_SPEAKER;
				strcpy(SSY_MUS_EXT, ".SPK");
				break;
			}
			// Continue while loop if we did not have a valid choice
			default: key = 0;
		}
	} while (key == 0);
	// Output keypress
	if (key == SCAN_0){
		str[0] = '0';
	} else{
		str[0] = 47 + key;
	}
	str[1] = 0;
	puts(str);
	if (SSY_DEVICE_MUS | SSY_DEVICE_SFX){
		ssy_init();
	}
	return key;
}


