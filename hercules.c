#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "memory.h"
#include "globals.h"
#include "keyboard.h"
#include "hercules.h"
#include "cga.h"
#include "pcplus.h"

void HGC_videoInit(int key){
	uint16_t i, j;

	VIDEO_MODE = CGA;
	VIDEO_BITSHIFT = 2;
	VIDEO_FILESHIFT = 2;
	VIDEO_SCREENSEG = 0xB000;
	// Select strobing cursor colors
	for (i = 0; i < 32; i++){
		if (i < 9){
			CURSOR_SELECTED_COLOR[i] = 0;
		} else if (i < 12){
			CURSOR_SELECTED_COLOR[i] = 2;
		} else if (i < 15){
			CURSOR_SELECTED_COLOR[i] = 1;
		} else if (i < 25){
			CURSOR_SELECTED_COLOR[i] = 3;
		} else if (i < 28){
			CURSOR_SELECTED_COLOR[i] = 1;
		} else{
			CURSOR_SELECTED_COLOR[i] = 2;
		}
	}
	j = 0;
	for (i = 0; i < 200; i+=2){
		VIDEO_SCREENOFFSET[i] = j;
		VIDEO_SCREENOFFSET[i + 1] = j + 0x2000;
		j += 80;
	}
	VIDEO_SCREENSIZE = 16192;
	VIDEO_GAMEOVERSIZE = 7488;
	VIDEO_HUDSIZE = 1312;
	VIDEO_FACE_X = 228;
	VIDEO_FACE_Y = 74;
	VIDEO_FACE_H = 26;
	VIDEO_FONT_COLOR_MENU = 1;
	VIDEO_FONT_COLOR_MENU_SELECTED = 3;
	VIDEO_FONT_COLOR_INFO = 1;
	VIDEO_FONT_COLOR_WHITE = 3;
	VIDEO_FONT_COLOR_GAMEOVER = 1;
	VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_CGA;
	VIDEO_USE_PALETTE_ANIMATION = 0;
	MAP_TILE_COLORS = MAP_TILE_COLORS_CGA;
	MINIMAP_X_OFFSET = 1;
	strncpy(VIDEO_FILEEXT, ".cga", 5);
	// Set function pointers
	videoSetMode = &HGC_videoSetMode;
	videoUnsetMode = &HGC_videoUnsetMode;
	videoClearScreen = &PCP_videoClearScreen;
	videoDrawFullScreen = &CGA_videoDrawFullScreen;
	videoPrintTile = &CGA_videoPrintTile;
	videoPrintTileMultibyte = &CGA_videoPrintTile;
	videoPrintSprite = &CGA_videoPrintSprite;
	videoPrintSpriteAsColor = &CGA_videoPrintSpriteAsColor;
	videoPrintFontAsColor = &CGA_videoPrintFontAsColor;
	videoCinemaAddChar = &CGA_videoCinemaAddChar;
	videoCinemaRotate = &CGA_videoCinemaRotate;
	videoFillRect = &CGA_videoFillRect;
	videoGetPixel = &CGA_videoGetPixel;
	videoTranslate = NULL;
	videoMaskTranslate = &CGA_videoMaskTranslate;
	videoGetUnitColorForMinimap = &CGA_videoGetUnitColorForMinimap;
	videoScreenShakeStep = &HGC_videoScreenShakeStep;
	minimapSet = &CGA_minimapSet;
	minimapPrepareLine = &CGA_minimapPrepareLine;
	minimapCopyLineToScreen = &CGA_minimapCopyLineToScreen;
}

void HGC_videoSetMode(void){
	uint8_t hercules_crtc_tab[] = {
		// CRTC table for 640x300, 3 fields
		0x35, 0x28, 0x2c, 0x07, 0x79, 0x03, 0x64, 0x6c, 0x02, 0x02, 0x00, 0x00
	};
	union REGS regs;

	// Save old video mode
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	VIDEO_MODE_ON_STARTUP = regs.h.al;

	// switch to 640x300x2 Hercules mode
	_asm {
		push ax
		push dx

		// switch to mode 7
		mov ax, 7
		int 0x10

		// disable video (graphics mode with page 0)
		mov dx, 0x3b8
		mov al, 0x02
		out dx, al

		// set CRTC registers

		push si

		lea si, hercules_crtc_tab
		xor ax, ax
		cli				// disable interrupts
		cld

sh_crtc_loop:
		mov dl, 0xb4	// dx=0x3b4
		out dx, al
		inc ax
		push ax
		lodsb
		inc dx
		out dx, al
		pop ax
		cmp al, 12
		jb sh_crtc_loop

		sti				// enable interrupts
		pop si

		// allow graphics mode, upper page disabled
		mov dl, 0xbf	// dx=0x3bf
		mov al, 1		// 3 if upper page enabled
		out dx, al

		// clear screen
		push es			// save es
		mov ax, 0xb000
		mov es, ax		// es=0xb000
		push di			// save di
		xor di, di		// di=0
		xor ax, ax		// ax=0
		push cx			// save cx
		mov cx, 16384
		rep stosw		// clear the entire 32KiB page
		pop cx			// restore cx
		pop di			// restore di
		pop es			// restore es

		// enable video (graphics mode with page 0)
		mov dl, 0xb8	// dx = 0x3b8
		mov al, 0x0a
		out dx, al

		pop dx			// restore dx
		pop ax			// restore ax
	}
}

void HGC_videoUnsetMode(void){
	union REGS regs;

	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
	// Re-enable port 3b8 protection
	outp(0x3bf, 0);
}

void HGC_videoScreenShakeStep(void){
	// Decrease screen shake, once we hit 0, we will reset screen back to normal
	// Select register 12 to modify screen pointer
	outp(0x3b4, 12);
	SCREEN_SHAKE--;
	outp(0x3b5, SCREEN_SHAKE_VAL[SCREEN_SHAKE]);
	outp(0x3b4, 13);
	SCREEN_SHAKE--;
	outp(0x3b5, SCREEN_SHAKE_VAL[SCREEN_SHAKE]);
}
