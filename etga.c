#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "memory.h"
#include "globals.h"
#include "keyboard.h"
#include "etga.h"
#include "vga.h"
#include "ega.h"
#include "cga.h"

void ETGA_videoInit(int key){
	uint16_t i, j;

	VIDEO_MODE = VGA;
	VIDEO_SCREENSEG = 0xA000;
	// Select strobing cursor colors
	for (i = 0; i < 32; i++){
		if (i < 9){
			CURSOR_SELECTED_COLOR[i] = 0x00;
		} else if (i < 12){
			CURSOR_SELECTED_COLOR[i] = 0x88;
		} else if (i < 15){
			CURSOR_SELECTED_COLOR[i] = 0x77;
		} else if (i < 25){
			CURSOR_SELECTED_COLOR[i] = 0xff;
		} else if (i < 28){
			CURSOR_SELECTED_COLOR[i] = 0x77;
		} else{
			CURSOR_SELECTED_COLOR[i] = 0x88;
		}
	}
	j = 0;
	for (i = 0; i < 200; i++){
		VIDEO_SCREENOFFSET[i] = j;
		j += 320;
	}
	VIDEO_SCREENSIZE = 64000;
	VIDEO_GAMEOVERSIZE = 29952;
	VIDEO_HUDSIZE = 5248;
	VIDEO_FACE_X = 232;
	VIDEO_FACE_Y = 75;
	VIDEO_MULTIBYTE_BITSHIFT = 1;
	VIDEO_FILE_MULTIBYTE = 1;
	VIDEO_SKIP_TRANSLATE_MULTIBYTE = 1;
	VIDEO_FONT_COLOR_MENU = 0xa6;
	VIDEO_FONT_COLOR_MENU_SELECTED = 0xff;
	VIDEO_FONT_COLOR_INFO = 0xa2;
	VIDEO_FONT_COLOR_WHITE = 0xff;
	VIDEO_FONT_COLOR_GAMEOVER = 0x97;
	VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_ETGA;
	VIDEO_USE_PALETTE_ANIMATION = 0;
	MAP_TILE_COLORS = MAP_TILE_COLORS_VGA;
	MINIMAP_X_OFFSET = 4;
	for (i = 0; i < TILE_COUNT; i++){
		MAP_TILE_COLORS[i] = PALETTE_GAME_ETGA[MAP_TILE_COLORS[i]];
	}
	strncpy(VIDEO_FILEEXT, ".vga", 5);
	// Set function pointers
	videoSetMode = &ETGA_videoSetMode;
	videoUnsetMode = &ETGA_videoUnsetMode;
	videoClearScreen = &VGA_videoClearScreen;
	videoDrawFullScreen = &ETGA_videoDrawFullScreen;
	videoPrintTile = &VGA_videoPrintTile;
	videoPrintTileMultibyte = &ETGA_videoPrintTileMultibyte;
	videoPrintSprite = &VGA_videoPrintSprite;
	videoPrintSpriteAsColor = &VGA_videoPrintSpriteAsColor;
	videoPrintFontAsColor = &VGA_videoPrintFontAsColor;
	videoCinemaAddChar = &VGA_videoCinemaAddChar;
	videoCinemaRotate = &VGA_videoCinemaRotate;
	videoFillRect = &VGA_videoFillRect;
	videoGetPixel = &VGA_videoGetPixel;
	videoTranslate = &ETGA_videoTranslate;
	videoMaskTranslate = &VGA_videoMaskTranslate;
	videoPaletteSet = &ETGA_videoPaletteSet;
	videoGetUnitColorForMinimap = &EGA_videoGetUnitColorForMinimap;
	videoScreenShakeStep = &CGA_videoScreenShakeStep;
	minimapSet = &VGA_minimapSet;
	minimapPrepareLine = &VGA_minimapPrepareLine;
	minimapCopyLineToScreen = &VGA_minimapCopyLineToScreen;

	VIDEO_PALETTE_GAME = (uint8_t*)PALETTE_GAME_ETGA;
	VIDEO_PALETTE_TITLE = (uint8_t*)PALETTE_TITLE_ETGA;
	VIDEO_PALETTE_GAMEOVER = (uint8_t*)PALETTE_GAMEOVER_ETGA;
}

void ETGA_videoSetMode(void){
	uint8_t etga_crtc_and_ctrl_tab[] = {
		// data for CRTC registers 0-18, index port 0x3d4, data port 0x3d4
		0x71, 0x50, 0x5a, 0xef, 0xff, 6, 0xc8, 0xe2, 2, 0, 0, 0, 0, 0, 0, 0, 0x18, 0, 0x46,
		// data for control registers: index byte followed by data byte
		1, 0x0f, 2, 0, 3, 0x10, 5, 1, 8, 2
	};
	union REGS regs;

	// Save old video mode
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	VIDEO_MODE_ON_STARTUP = regs.h.al;

	// Set new video mode
	_asm {
		push ax
		push dx

		// switch to mode 3
		mov ax, 3
		int 0x10

		// select 640 dot graphics mode with hi-res clock, disable video
		mov dx, 0x3d8
		mov al, 0x13
		out dx, al

		push si
		lea si, etga_crtc_and_ctrl_tab


		xor ax, ax
		cli				// disable interrupts
		cld

		// set CRTC registers
	crtc_loop:
		mov dl, 0xd4	// dx=0x3d4
		out dx, al
		inc ax
		push ax
		ss lodsb
		inc dx
		out dx, al
		pop ax
		cmp al, 19
		jb crtc_loop

		// set control registers
	ctrl_loop:
		mov dl, 0xda	// dx=0x3da
		ss lodsw
		out dx, al
		mov dl, 0xde	// dx=0x3de
		xchg al, ah
		out dx, al
		cmp ah, 8
		jne ctrl_loop

		sti				// enable interrupts

		// clear color select register
		mov dl, 0xd9	// dx=0x3d9
		mov al, 0
		out dx, al

		// disable extended RAM paging
		mov dl, 0xdd	// dx=0x3dd
		mov al, 0
		out dx, al


		pop si

		// select page bit 2 for CRT & CPU
		mov dl, 0xdf	// dx=0x3df
		mov al, 0x24
		out dx, al

		// clear screen
		push es			// save es
		mov ax, 0xa000
		mov es, ax		// es=0xa000
		push di			// save di
		xor di, di		// di=0
		xor ax, ax		// ax=0
		push cx			// save cx
		mov cx, 32000
		rep stosw		// write 64000 black pixels (technically 128000)
		pop cx			// restore cx
		pop di			// restore di
		pop es			// restore es

		// select 640 dot graphics mode with hi-res clock, enable video
		mov dl, 0xd8	// dx = 0x3d8
		mov al, 0x1b
		out dx, al

		pop dx			// restore dx
		pop ax			// restore ax
	}
}

void ETGA_videoUnsetMode(void){
	uint8_t text_crtc_and_ctrl_tab[] = {
		// data for CRTC registers 0-18, index port 0x3d4, data port 0x3d4
		0x71, 0x50, 0x5a, 0xfe, 0x1c, 1, 0x19, 0x1a, 2, 8, 6, 7, 0, 0, 0, 0, 0x18, 0x20, 7,
		// data for control registers: index byte followed by data byte
		1, 0x0f, 2, 0, 3, 0x10, 5, 0, 8, 0
	};
	union REGS regs;

	// switch to a common text mode via hardware registers
	_asm {
		push ax
		push dx

		// select 80 column text mode, disable video
		mov dx, 0x3d8
		mov al, 0x13
		out dx, al

		push si
		lea si, text_crtc_and_ctrl_tab


		xor ax, ax
		cli				// disable interrupts
		cld

		// set CRTC registers
	crtc_loop:
		mov dl, 0xd4	// dx=0x3d4
		out dx, al
		inc ax
		push ax
		ss lodsb
		inc dx
		out dx, al
		pop ax
		cmp al, 19
		jb crtc_loop

		// set control registers
	ctrl_loop:
		mov dl, 0xda	// dx=0x3da
		ss lodsw
		out dx, al
		mov dl, 0xde	// dx=0x3de
		xchg al, ah
		out dx, al
		cmp ah, 8
		jne ctrl_loop

		sti				// enable interrupts

		// clear color select register
		mov dl, 0xd9	// dx=0x3d9
		mov al, 0
		out dx, al

		// disable extended RAM paging
		mov dl, 0xdd	// dx=0x3dd
		mov al, 0
		out dx, al


		pop si

		// select page bits 0-2 for CRT & CPU
		mov dl, 0xdf	// dx=0x3df
		mov al, 0x3f
		out dx, al

		// select 80 column text mode, enable blinking, enable video
		mov dl, 0xd8	// dx = 0x3d8
		mov al, 0x29
		out dx, al

		pop dx			// restore dx
		pop ax			// restore ax
	}

	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
}

void ETGA_videoPaletteSet(const uint8_t *palette, uint8_t fade_bit_shift){
	VIDEO_ACTIVE_PALETTE = (uint8_t*)palette;
}

void ETGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte){
	uint16_t size = (w * h) >> multi_byte;
	uint16_t i;
	if (multi_byte){ // just copy
		for (i = 0; i < size; i++){
			*dest++ = *source++;
		}
	} else{
		for (i = 0; i < size; i++){
			*dest++ = palette[*source++];
		}
	}
}

void ETGA_videoDrawFullScreen(uint16_t segment){
	// Routine to draw a full screen of graphics data
	_asm{
		lea bx, ss:VIDEO_ACTIVE_PALETTE
		mov bx, [ss:bx]
		// Set source (DS:SI) to passed segment
		mov ds, segment
		xor si, si
		// Set destination offset (DI) to start of screen
		mov di, si
		// Set destination segment (ES) to start of screen
		mov ax, 0xA000
		mov es, ax
		// Set CX to number of times to repeat
		mov cx, 16000
		// Loop while drawing to the screen
	vdfs_vga_loop:
		// Fetch two bytes, which contains 4 pixels
		lodsw
		// Save the second byte
		mov dh, ah
		// Set AL to be first nibble and AH to be second nibble
		mov ah, al
		and al, 15
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		// Write AL and AH as two bytes to the screen
		xlatb ss:bx
		xchg al, ah
		xlatb ss:bx
		xchg al, ah
		stosw
		// Retrieve saved second byte
		mov ah, dh
		mov al, dh
		// Set AL to be first nibble and AH to be second nibble
		and al, 15
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		// Write AL and AH as two bytes to the screen
		xlatb ss:bx
		xchg al, ah
		xlatb ss:bx
		xchg al, ah
		stosw
		// Loop until CX is 0
		loop vdfs_vga_loop
	}
}

// Copy a tiled graphic from ram to the screen.
// This version expects each byte to hold 2 pixels (for 16-color VGA low/high nibble)
// 'x' is the x position on screen to draw the tile (upper-left corner) (must be divisible by 4)
// 'y' is the y position on screen to draw the tile (upper-left corner) (must be divisible by 2)
// 'tile' should be a pointer to the top-left of the tile in ram
// 'w' is the width in pixels of the tile (must be divisible by 8)
// 'h' is the height in pixels of the tile (must be divisible by 2)
void ETGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
	_asm{
		// Set segment (DS:SI) to tileseg
		mov ds, ss:tileseg
		// Set offset to 0
		xor si, si
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row and store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load LUT pointer for color conversion
		lea bx, ss:VIDEO_ACTIVE_PALETTE
		mov bx, [ss:bx]
		// Set AX to h, the number of times to repeat vertically
		mov dx, h
		// Set ES segment to VGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Add x to screen position
		add di, x
		// Save number of bytes to copy per row
		mov cx, w
		// Store how many bytes to move to next line
		push bp
		mov bp, 320
		sub bp, cx
		// Shift number of bytes to copy per row (divide by 2), since we will be moving 2 bytes at a time
		shr cx, 1
		// Then shift again, because there are two pixels per byte
		shr cx, 1
		// Store how many bytes to move per row
		mov ch, cl
		// Loop while drawing to the screen
	vptm_vga_loop:
		// Fetch two bytes, which contains 4 pixels
		lodsw
		// Save the second byte
		mov dh, ah
		// Set AL to be first nibble and AH to be second nibble
		mov ah, al
		and al, 15
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		// Write AL and AH as two bytes to the screen
		xlatb ss:bx
		xchg al, ah
		xlatb ss:bx
		xchg al, ah
		stosw
		// Retrieve saved second byte
		mov ah, dh
		mov al, dh
		// Set AL to be first nibble and AH to be second nibble
		and al, 15
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		// Write AL and AH as two bytes to the screen
		xlatb ss:bx
		xchg al, ah
		xlatb ss:bx
		xchg al, ah
		stosw
		// Loop until CL is 0
		dec cl
		jnz vptm_vga_loop
		// Move to new line and restore count of how many bytes to process
		mov cl, ch
		add di, bp
		// Check if we need to process another row
		dec dl
		jnz vptm_vga_loop
	vptm_done:
		pop bp
	}
}
