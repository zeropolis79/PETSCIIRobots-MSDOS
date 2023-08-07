#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <i86.h>
#include <string.h>
#include <stdlib.h>
#include "video.h"
#include "globals.h"
#include "keyboard.h"
#include "interrupt.h"
#include "main.h"
#include "string.h"
#include "memory.h"
#include "vga.h"
#include "cga.h"
#include "ega.h"
#include "etga.h"
#include "pcplus.h"
#include "hercules.h"

uint8_t VIDEO_MODE;
uint8_t VIDEO_MODE_ON_STARTUP = NO_MODE;
uint16_t VIDEO_SCREENSEG;
uint16_t VIDEO_SCREENOFFSET[200];
uint8_t VIDEO_TILEW_OFFSET[11];
uint8_t VIDEO_TILEH_OFFSET[7];
uint8_t VIDEO_PLANESHIFT;
uint8_t VIDEO_BITSHIFT;
uint8_t VIDEO_FILESHIFT;
uint8_t VIDEO_MULTIBYTE_BITSHIFT;
uint8_t VIDEO_FILE_MULTIBYTE;
uint8_t VIDEO_SKIP_TRANSLATE_MULTIBYTE;
uint8_t VIDEO_COMPOSITE;
uint8_t VIDEO_FONT_COLOR_MENU;
uint8_t VIDEO_FONT_COLOR_MENU_SELECTED;
uint8_t VIDEO_FONT_COLOR_INFO;
uint8_t VIDEO_FONT_COLOR_WHITE;
uint8_t VIDEO_FONT_COLOR_GAMEOVER;
uint16_t VIDEO_SCREENSIZE;
uint16_t VIDEO_GAMEOVERSIZE;
uint16_t VIDEO_HUDSIZE;
uint16_t VIDEO_TILE_W;
uint16_t VIDEO_TILE_H;
uint16_t VIDEO_ITEM_W;
uint16_t VIDEO_ITEM_H;
uint16_t VIDEO_KEY_W;
uint16_t VIDEO_KEY_H;
uint16_t VIDEO_HEALTH_W;
uint16_t VIDEO_HEALTH_H;
uint16_t VIDEO_FACE_X;
uint16_t VIDEO_FACE_Y;
uint16_t VIDEO_FACE_W;
uint16_t VIDEO_FACE_H;
uint16_t VIDEO_FONT_W;
uint16_t VIDEO_FONT_H;
char VIDEO_FILEEXT[5];
uint8_t INFOLINE = 0;
uint8_t const INFOROW[3] = { 176, 184, 192 };
uint8_t *VIDEO_ACTIVE_PALETTE;
uint8_t VIDEO_USE_PALETTE_ANIMATION;

// for video modes that do not have a palette
void dummy_videoSetPalette(const uint8_t *palette, uint8_t fade_bit_shift){}
void dummy_videoFadeIn(const uint8_t *palette, uint8_t timer){}
void dummy_videoFadeOut(const uint8_t *palette, uint8_t timer){}

// Function pointers for calling routine for correct video mode
void (*videoSetMode)(void);
void (*videoUnsetMode)(void);
void (*videoClearScreen)(void);
void (*videoDrawFullScreen)(uint16_t segment);
void (*videoPrintTile)(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void (*videoPrintTileMultibyte)(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void (*videoPrintSprite)(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
void (*videoPrintSpriteAsColor)(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
void (*videoPrintFontAsColor)(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void (*videoCinemaAddChar)(uint16_t letter_segment, uint16_t dest_segment);
void (*videoCinemaRotate)(uint16_t tile1, uint16_t tile2, uint16_t tile3);
void (*videoFillRect)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void (*videoTranslate)(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
uint16_t (*videoMaskTranslate)(uint8_t *source, uint16_t masksize);
uint8_t (*videoGetPixel)(uint16_t x, uint16_t y);
void (*videoPaletteSet)(const uint8_t *palette, uint8_t fade_bit_shift) = dummy_videoSetPalette;
uint8_t (*videoGetUnitColorForMinimap)(uint8_t unit);
void (*videoDamageFlashStep)(void);
void (*videoEMPFlashStep)(void);
void (*videoScreenShakeStep)(void);
void (*videoFadeIn)(const uint8_t *palette, uint8_t timer) = dummy_videoFadeIn;
void (*videoFadeOut)(const uint8_t *palette, uint8_t timer) = dummy_videoFadeOut;
void (*minimapSet)(uint8_t x, uint8_t y, uint8_t color);
void (*minimapPrepareLine)(uint8_t *vidptr2, uint8_t y);
void (*minimapCopyLineToScreen)(uint8_t *vidptr2, uint8_t y);

int videoSelect(const char *arg){
	int key;
	unsigned int i, j;
	uint8_t str[10];

	// Prompt to choose which video mode to use
	puts("Choose Video Mode:");
	puts("-------------------");
	if (arg[0] == 0 || arg[0] == '1'){
		puts("1-VGA 320x200 / 256-Color");
		key = SCAN_1;
	}
	if (arg[0] == 0 || arg[0] == '2'){
		puts("2-EGA 320x200 / 16-Color");
		key = SCAN_2;
	}
	if (arg[0] == 0 || arg[0] == '3'){
		puts("3-CGA 320x200 / 4-Color");
		key = SCAN_3;
	}
	if (arg[0] == 0 || arg[0] == '4'){
		puts("4-CGA 320x200 Inverted Colors");
		key = SCAN_4;
	}
	if (arg[0] == 0 || arg[0] == '5'){
		puts("5-CGA Composite 160x200 / 16-Color");
		key = SCAN_5;
	}
	if (arg[0] == 0 || arg[0] == '6'){
		puts("6-TDY Tandy 160x200 / 16-Color");
		key = SCAN_6;
	}
	if (arg[0] == 0 || arg[0] == '7'){
		puts("7-PCP Plantronics 320x200 / 16-Color");
		key = SCAN_7;
	}
	if (arg[0] == 0 || arg[0] == '8'){
		puts("8-HGC Hercules 640x300 / 2-Color");
		key = SCAN_8;
	}
	if (arg[0] == 0 || arg[0] == '9'){
		puts("9-TDY Tandy 640x200 / 256-Color");
		key = SCAN_9;
	}
	if (arg[0] == 0){
		key = 0;
	}
	// Default Settings
	VIDEO_COMPOSITE = 0;
	VIDEO_PLANESHIFT = 0;
	VIDEO_BITSHIFT = 0;
	VIDEO_FILESHIFT = 0;
	VIDEO_MULTIBYTE_BITSHIFT = 0;
	VIDEO_FILE_MULTIBYTE = 0;
	VIDEO_SKIP_TRANSLATE_MULTIBYTE = 0;
	VIDEO_TILE_W = 24;
	VIDEO_TILE_H = 24;
	VIDEO_ITEM_W = 48;
	VIDEO_ITEM_H = 22;
	VIDEO_KEY_W = 16;
	VIDEO_KEY_H = 14;
	VIDEO_HEALTH_W = 48;
	VIDEO_HEALTH_H = 52;
	VIDEO_FACE_W = 24;
	VIDEO_FACE_H = 24;
	VIDEO_FONT_W = 8;
	VIDEO_FONT_H = 8;
	// Wait for keypress until we get a valid choice
	do{
		if (key == 0){
			key = keyWait();
		}
		switch (key){
			case SCAN_1:{ // VGA 320x200 256-Color
				VGA_videoInit(key);
				break;
			}
			case SCAN_2:{ // EGA 320x200 16-Color
				EGA_videoInit(key);
				break;
			}
			case SCAN_3:  // CGA 320x200 4-Color
			case SCAN_4:  // CGA 320x200 4-Color Inversed
			case SCAN_5:  // CGA 160x200 16-Color Composite
			case SCAN_6:{ // TDY 160x200 16-Color
				CGA_videoInit(key);
				break;
			}
			case SCAN_7:{ // Plantronics ColorPlus 320x200 16-Color
				PCP_videoInit(key);
				break;
			}
			case SCAN_8:{ // Hercules 640x300 2-Color
				HGC_videoInit(key);
				break;
			}
			case SCAN_9:{ // TDY 640x200 16-Color
				ETGA_videoInit(key);
				break;
			}
			// Continue while loop if we did not have a valid choice
			default: key = 0;
		}
	} while (key == 0);
	// Build an array of tile offset size, to avoid multiplication
	for (i = 0; i < 11; i++){
		VIDEO_TILEW_OFFSET[i] = i * 24;
	}
	for (i = 0; i < 7; i++){
		VIDEO_TILEH_OFFSET[i] = i * 24;
	}
	// Output keypress
	str[0] = 47 + key;
	str[1] = 0;
	puts(str);
	return 1;
}

void videoPrintString(uint16_t x, uint16_t y, uint8_t text, const char *str, uint16_t color){
	unsigned int i, pos;
	uint8_t c;
	
	for (i = 0; i < strlen(str); i++){
		c = str[i];
		// If this is a text string, modify the character
		if (text && c >= 'A' && c <= 'Z'){
			c &= 191;
		}
		videoPrintFontAsColor(x, y, FONT[c], color);
		x += VIDEO_FONT_W;
	}
}

void videoInfoClear(void){
	// Clear the information text area
	INFOLINE = 0;
	videoFillRect(0, INFOROW[0], 264, 24, 0);
}

void videoInfoPrint(const char *str){
	uint8_t i, i2, loopuntil, planes, writeplane;
	uint16_t width;
	
	if (INFOLINE == 3){
		// Scroll information text up one line
		loopuntil = VIDEO_FONT_H << 1;
		planes = 1;
		writeplane = 1;
		switch (VIDEO_MODE){
			case EGA:{
				width = 33;
				planes = 4;
				break;
			}
			case CGACOMP:
			case TDY_LOW:
			case CGA:{
				width = 66;
				break;
			}
			default:{
				width = 264;
			}
		}
		for (i = 0; i < planes; i++){
			if (VIDEO_MODE == EGA){
				outp(0x03CE, 0x04);
				outp(0x03CF, i);
				outp(0x03C5, writeplane);
				writeplane <<= 1;
			}
			for (i2 = 0; i2 < loopuntil; i2++){
				memcpy(memorySegOfsToPtr(VIDEO_SCREENSEG, VIDEO_SCREENOFFSET[INFOROW[0] + i2]), memorySegOfsToPtr(VIDEO_SCREENSEG, VIDEO_SCREENOFFSET[INFOROW[1] + i2]), width);
			}
		}
		videoFillRect(0, INFOROW[2], 264, 8, 0);
		INFOLINE = 2;
	}
	// Print the information text
	videoPrintString(0, INFOROW[INFOLINE], 1, str, VIDEO_FONT_COLOR_INFO);
	INFOLINE++;
}

