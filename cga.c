#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "memory.h"
#include "globals.h"
#include "keyboard.h"
#include "cga.h"
#include "ega.h"

void CGA_videoInit(int key){
	uint16_t i, j;
	
	if (key == SCAN_5){
		VIDEO_MODE = CGACOMP;
		VIDEO_COMPOSITE = 1;
	} else if (key == SCAN_6){
		VIDEO_MODE = TDY_LOW;
		VIDEO_COMPOSITE = 1;
	} else{
		VIDEO_MODE = CGA;
	}
	VIDEO_BITSHIFT = 2;
	VIDEO_FILESHIFT = 2;
	VIDEO_SCREENSEG = 0xB800;
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
	VIDEO_DAMAGE_FLASH_COUNT = CGA_DAMAGE_FLASH_COUNT;
	VIDEO_EMP_FLASH_COUNT = CGA_EMP_FLASH_COUNT;
	MAP_TILE_COLORS = MAP_TILE_COLORS_CGA;
	MINIMAP_X_OFFSET = 1;
	strncpy(VIDEO_FILEEXT, ".cga", 5);
	// Set function pointers
	videoSetMode = &CGA_videoSetMode;
	videoUnsetMode = &CGA_videoUnsetMode;
	videoClearScreen = &CGA_videoClearScreen;
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
	videoDamageFlashStep = &CGA_videoDamageFlashStep;
	videoEMPFlashStep = &CGA_videoEMPFlashStep;
	videoScreenShakeStep = &CGA_videoScreenShakeStep;
	minimapSet = &CGA_minimapSet;
	minimapPrepareLine = &CGA_minimapPrepareLine;
	minimapCopyLineToScreen = &CGA_minimapCopyLineToScreen;
	switch (key){
		case SCAN_3:{ // Regular CGA mode
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
			break;
		}
		case SCAN_4:{ // Inversed CGA mode
			videoTranslate = &CGA_videoTranslate;
			VIDEO_FONT_COLOR_MENU = 2;
			VIDEO_FONT_COLOR_INFO = 2;
			VIDEO_FONT_COLOR_GAMEOVER = 2;
			// Select strobing cursor colors
			for (i = 0; i < 32; i++){
				if (i < 9){
					CURSOR_SELECTED_COLOR[i] = 0;
				} else if (i < 12){
					CURSOR_SELECTED_COLOR[i] = 1;
				} else if (i < 15){
					CURSOR_SELECTED_COLOR[i] = 2;
				} else if (i < 25){
					CURSOR_SELECTED_COLOR[i] = 3;
				} else if (i < 28){
					CURSOR_SELECTED_COLOR[i] = 2;
				} else{
					CURSOR_SELECTED_COLOR[i] = 1;
				}
			}
			break;
		}
		case SCAN_6:  // low-res Tandy mode
		case SCAN_5:{ // CGA Composite mode
			VIDEO_FONT_COLOR_MENU = 2;
			VIDEO_FONT_COLOR_MENU_SELECTED = 15;
			VIDEO_FONT_COLOR_INFO = 10;
			VIDEO_FONT_COLOR_WHITE = 15;
			VIDEO_FONT_COLOR_GAMEOVER = 9;
			VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_CGACOMP;
			videoGetUnitColorForMinimap = &EGA_videoGetUnitColorForMinimap;
			minimapPrepareLine = &CGACOMP_minimapPrepareLine;
			// Use VGA tile colors for mini-map and translate them
			MAP_TILE_COLORS = MAP_TILE_COLORS_VGA;
			for (i = 0; i < TILE_COUNT; i++){
				switch (MAP_TILE_COLORS[i]){
					case  0: break;
					case  1: MAP_TILE_COLORS[i] =  1; break;
					case  2: MAP_TILE_COLORS[i] =  8; break;
					case  3: MAP_TILE_COLORS[i] =  2; break;
					case  4: MAP_TILE_COLORS[i] =  4; break;
					case  5: MAP_TILE_COLORS[i] = 10; break;
					case  6: MAP_TILE_COLORS[i] =  9; break;
					case  7: MAP_TILE_COLORS[i] = 12; break;
					case  8: MAP_TILE_COLORS[i] =  3; break;
					case  9: MAP_TILE_COLORS[i] = 11; break;
					case 10: MAP_TILE_COLORS[i] =  5; break;
					case 11: MAP_TILE_COLORS[i] =  3; break;
					case 12: MAP_TILE_COLORS[i] = 13; break;
					case 13: MAP_TILE_COLORS[i] =  7; break;
					case 14: MAP_TILE_COLORS[i] = 14; break;
					case 15: MAP_TILE_COLORS[i] = 15; break;
				}
			}
			// Select strobing cursor colors
			for (i = 0; i < 32; i++){
				if (i < 9){
					CURSOR_SELECTED_COLOR[i] = 0;
				} else if (i < 12){
					CURSOR_SELECTED_COLOR[i] = 10;
				} else if (i < 15){
					CURSOR_SELECTED_COLOR[i] = 5;
				} else if (i < 25){
					CURSOR_SELECTED_COLOR[i] = 15;
				} else if (i < 28){
					CURSOR_SELECTED_COLOR[i] = 10;
				} else{
					CURSOR_SELECTED_COLOR[i] = 5;
				}
			}
			strncpy(VIDEO_FILEEXT, ".cgc", 5);
			break;
		}
	}
	if (VIDEO_MODE == TDY_LOW){
		VIDEO_FONT_COLOR_MENU = 1;
		VIDEO_FONT_COLOR_INFO = 1;
		videoPrintFontAsColor = &TDY_videoPrintFontAsColor;
	}
}

void CGA_videoSetMode(void){
	union REGS regs;

	// Save old video mode
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	VIDEO_MODE_ON_STARTUP = regs.h.al;

	// Set new video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE;
	int86(0x10, &regs, &regs);
	if (VIDEO_MODE == CGACOMP){
		// Set color burst mode for CGA Composite
		outp(0x03d8, 0x1a);
	}
	if (VIDEO_MODE == TDY_LOW){
		uint8_t palette[] = {0, 2, 1, 9, 4, 7, 5, 3, 6, 10, 8, 11, 12, 14, 13, 15};
		int i;
		for (i = 0; i < 16; i++){
			regs.w.ax = 0x1000;
			regs.h.bl = i;
			regs.h.bh = palette[i];
			int86(0x10, &regs, &regs);
		}
	}
}

void CGA_videoUnsetMode(void){
	union REGS regs;

	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
}

void CGA_videoClearScreen(void){
	// Routine to clear the full screen
	_asm{
		// data we will fill the screen with (0 for black)
		xor ax, ax
		// Set destination segment (ES) to start of screen
		mov bx, ss:VIDEO_SCREENSEG
		mov es, bx
		// Set destination offset (DI) to start of screen
		mov di, ax
		// Set number of times to repeat
		mov cx, 8096
		// Copy to screen
		rep stosw
	}
}

// Fill a rectangular area of the screen with specified color
void CGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	
	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row and store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Move x into DS
		mov ax, x
		mov ds, ax
		// Set SI to h, the number of times to repeat
		mov si, h
		// Set DX to color
		mov dx, color
		// Push BP onto the stack
		push bp
		// Move w into BP
		mov ax, w
		mov bp, ax
		// Set destination segment (ES) to start of screen
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// Get video mode
		mov cl, ss:VIDEO_MODE
		// Check if this is CGA Composite
		cmp cl, CGACOMP
		jne vfr_cga_normalcga
		// It is CGA Composite, repeat color using 2 bit-groups
		and dx, 15
		mov cl, dl
		shl cl, 1
		shl cl, 1
		shl cl, 1
		shl cl, 1
		or dl, cl
		jmp vfr_cga_bothmodes
	vfr_cga_normalcga:
		// Make sure the color (DX) falls in the proper range for CGA (make a copy in CX to compare with original color)
		mov cx, dx
		and dx, 3
		jne vfr_cga_nocolorchange
		// If the color after being ANDed is 0 and original color was not 0 then set it to 3 (white)
		cmp cx, 0
		je vfr_cga_nocolorchange
		mov dl, 3
	vfr_cga_nocolorchange:
		// Repeat the color (DL) for all 4 bit-pairs
		mov cl, dl
		shl cl, 1
		shl cl, 1
		or dl, cl
		shl cl, 1
		shl cl, 1
		or dl, cl
		shl cl, 1
		shl cl, 1
		or dl, cl
	vfr_cga_bothmodes:
		// Store FFFF in BX, for starting(BH)/ending(BL) mask bytes
		mov bx, 0xFFFF
		// Get (x + w - 1) formula for getting ending byte/offset and store in AX
		mov ax, ds
		add ax, bp
		dec ax
		// Calculate ending pixel offset ((x + w - 1) & 3) (xor to inverse the bits to shift) and store a mask in BL
		mov cx, ax
		and cx, 3
		xor cx, 3
		je vfr_cga_endingoffsetdone
	vfr_cga_endingoffsetloop:
		shl bl, 1
		shl bl, 1
		dec cl
		jne vfr_cga_endingoffsetloop
	vfr_cga_endingoffsetdone:
		// Get ending byte ((x + w - 1) >> 2) (AL)
		shr ax, 1
		shr ax, 1
		// Calculate starting pixel offset (x & 3) and store a mask in BH
		mov cx, ds
		and cl, 3
		je vfr_cga_startingoffsetdone
	vfr_cga_startingoffsetloop:
		shr bh, 1
		shr bh, 1
		dec cl
		jne vfr_cga_startingoffsetloop
	vfr_cga_startingoffsetdone:
		// Get starting byte (x >> 2) and store in AH
		mov cx, ds
		shr cx, 1
		shr cx, 1
		mov ah, cl
		// Increase DI to X position
		add di, cx
		// Calculate the number of bytes needed to move to the next line, store in BP
		mov dh, al
		sub dh, ah
		mov cx, 79
		sub cl, dh
		mov bp, cx
		// If starting byte == FF and width is 0, then set width to 1
		cmp bh, 0xFF
		jne vfr_cga_startingnotff
		cmp dh, 0
		jne vfr_cga_startingnotff
		inc dh
	vfr_cga_startingnotff:
		// If ending byte == FF, then increase width by 1
		cmp bl, 0xFF
		jne vfr_cga_endingnotff
		inc dh
	vfr_cga_endingnotff:
		// Check if starting byte and ending byte are the same, if so then AND the masks together in the starting mask and fill the ending mask
		cmp al, ah
		jne vfr_cga_bytecalc
		and bh, bl
		mov bl, 0xFF
		cmp dh, 0
		je vfr_cga_bytecalc
		dec dh
	vfr_cga_bytecalc:
		// Calculate number of bytes that is just the filled color (no mask needed) and store in DH
		cmp dh, 0
		je vfr_cga_mainloop
		cmp bh, 0xFF
		je vfr_cga_mainloop
		dec dh
		// Clear cx, so high byte will be clear within loop, since we only load in the low byte
		mov cx, 0
	vfr_cga_mainloop:
		// If starting offset != FF, then we build the first byte using the mask
		cmp bh, 0xFF
		je vfr_cga_skipstartingbyte
		// Read byte from screen and use mask to join new color data together and write the new byte
		mov al, byte ptr [ES:DI]
		mov ah, bh
		xor ah, 0xFF
		and al, ah
		mov ah, bh
		and ah, dl
		or al, ah
		stosb
	vfr_cga_skipstartingbyte:
		// Skip if we byte fill count is 0
		cmp dh, 0
		je vfr_cga_endingbyte
		// Set number of times to repeat
		mov cl, dh
		// Load fill color
		mov al, dl
		// Copy color to the screen
		rep stosb
	vfr_cga_endingbyte:
		// If ending offset != FF, then we build the last byte using the mask
		cmp bl, 0xFF
		je vfr_cga_skipendingbyte
		// Read byte from screen and use mask to join new color data together and write the new byte
		mov al, byte ptr [ES:DI]
		mov ah, bl
		xor ah, 0xFF
		and al, ah
		mov ah, bl
		and ah, dl
		or al, ah
		stosb
	vfr_cga_skipendingbyte:
		// Move to next screen row
		add DI, BP
		xor DI, 0x2000
		cmp DI, 0x2000
		jl vfr_cga_plane1
		// Move back a row, because we are on plane 2 now
		sub DI, 80
	vfr_cga_plane1:
		dec si
		jne vfr_cga_mainloop
		pop BP
	}
}

void CGA_videoDrawFullScreen(uint16_t segment){
	// Routine to draw a full screen of graphics data
	_asm{
		// Set source (DS:SI) to passed segment
		mov ds, segment
		xor si, si
		// Set destination offset (DI) to start of screen
		mov di, si
		// Set destination segment (ES) to start of screen
		mov ax, ss:VIDEO_SCREENSEG
		mov es, ax
		// Set number of times to repeat
		mov cx, 8096
		// Copy from memory to screen
		rep movsw
	}
}

// Copy a tiled graphic from ram to the screen
// 'x' is the x position on screen to draw the tile (upper-left corner) (must be divisible by 8)
// 'y' is the y position on screen to draw the tile (upper-left corner) (must be divisible by 2)
// 'tileseg' should be a starting segment for the tile in ram
// 'w' is the width in pixels of the tile (must be divisible by 8)
// 'h' is the height in pixels of the tile (must be divisible by 2)
void CGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
	_asm{
		// Set segment (DS:SI) to tileseg
		mov ds, ss:tileseg
		// Set si (offset) to 0
		xor si, si
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row and store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Set ES segment to CGA screen pointer
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		add di, dx
		// Save number of bytes to copy per row. SHR to divide by 4, then shift again because we are copying 2 bytes at a time
		mov dx, w
		shr dx, 1
		shr dx, 1
		shr dx, 1
		// Set AX to h, the number of times to repeat
		mov ax, h
		// Loop while drawing to the screen
	vpt_cga_loop:
		// Save location for next row/plane
		mov bx, di
		// Set number of words to copy
		mov cx, dx
		// Start copy
		rep movsw
		// Move to next plane of the screen
		mov di, bx
		or di, 0x2000
		// Set number of words to copy
		mov cx, dx
		// Start copy
		rep movsw
		// Move back to previous plane of the screen and next row
		mov di, bx
		add di, 80
		// Loop until AX is 0
		dec ax
		dec ax
		jnz vpt_cga_loop
	}
}

// Copy a sprite from ram to the screen, using a mask to also draw the tile underneath
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'tile' should be a pointer to the top-left of the tile in ram
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void CGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to sprite, which has the mask data within it. Set BX to offset of sprite (always 0)
		mov ds, spriteseg
		xor si, si
		mov bx, si
		// Set ES segment to CGA screen pointer
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		add di, dx
		// Save SS segment
		mov dx, ss
		// Disable interrupts
		cli
		// Load pointer to tileseg. Store in SS:SI
		mov ax, ss:tileseg
		mov ss, ax
		// Set number of times to loop
		mov al, 12
	vps_cga_loop:
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		// Move to next plane
		add di, 8187
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		inc di
		// Fetch mask data into CL
		mov cl, byte ptr [DS:BX + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, cl
		// XOR CL
		xor cl, 0xFF
		// AND sprite data
		and cl, byte ptr [DS:BX]
		// Merge the two together with an OR
		or cl, ch
		// Output to screen
		mov ES:DI, cl
		// Increment to next byte
		inc bx
		inc si
		sub di, 8117
		dec al
		jz vps_cga_done
		jmp vps_cga_loop
	vps_cga_done:
		// Replace SS segment back to what it was
		mov ss, dx
		// Enable interrupts
		sti
	}
}

// Copy a sprite from ram to the screen, using a mask to avoid overwriting surrounding pixels
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'color' should be the color to draw the sprite as
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void CGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to sprite [DS:SI], add 144 to point to mask data
		mov ds, spriteseg
		mov si, 144
		// Set ES segment to CGA screen pointer
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		add di, dx
		// Set ah to color, repeated for each pixel
		mov ax, color
		mov ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
		// Number of times to loop (for each plane)
		mov cx, 12
	vps_cga_loop:
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Move to next plane
		add di, 8186
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI]
		// Fetch screen data into BH
		mov bh, byte ptr [ES:DI]
		// AND Together
		and bh, al
		// XOR BL
		xor al, 0xFF
		// AND sprite color data
		and al, ah
		// Merge the two together with an OR
		or al, bh
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		sub di, 8118
		dec cx
		jz vps_done
		jmp vps_cga_loop
	vps_done:
	}
}

// Copy a font character from ram to the screen
// 'x' is the x position on screen to draw the font (upper-left corner)
// 'y' is the y position on screen to draw the font (upper-left corner)
// 'fontseg' should be a segment pointer to the top-left of the tile in ram
// 'color' should be the color to draw the font as
// WIDTH and HEIGHT are hard set to 8x8 for this routine
void CGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to font [DS:SI]
		mov ds, fontseg
		xor si, si
		// Set ES segment to CGA screen pointer
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		add di, dx
		// Number of times to loop (for each plane)
		mov cx, 4
		// Set ah to color, repeated for each pixel
		mov ax, color
		// If color > 3, then assume CGA Composite and change to 2 colors per byte
		cmp ax, 3
		jle vps_cga_4color
		and al, 15
		mov ah, al
		shl ah, 1
		shl ah, 1
		shl ah, 1
		shl ah, 1
		or ah, al
		jmp vps_cga_loop
	vps_cga_4color:
		// Standard CGA 4 colors per byte
		mov ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
		shl ah, 1
		shl ah, 1
		or ah, al
	vps_cga_loop:
		// Fetch mask data into AL
		lodsb
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Fetch mask data into AL
		lodsb
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Move to next plane
		add di, 8190
		// Fetch mask data into AL
		lodsb
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Fetch mask data into AL
		lodsb
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Go back to previous plane
		sub di, 8114
		dec cx
		jz vps_done
		jmp vps_cga_loop
	vps_done:
	}
}

// Copy a font character from ram to the screen
// 'x' is the x position on screen to draw the font (upper-left corner)
// 'y' is the y position on screen to draw the font (upper-left corner)
// 'fontseg' should be a segment pointer to the top-left of the tile in ram
// 'color' should be the color to draw the font as
// WIDTH and HEIGHT are hard set to 8x8 for this routine
void TDY_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to font [DS:SI]
		mov ds, fontseg
		xor si, si
		// Set ES segment to CGA screen pointer
		mov cx, ss:VIDEO_SCREENSEG
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		add di, dx
		// Number of times to loop (for each plane)
		mov cx, 4
		// Set ah to color, repeated for each pixel
		mov ax, color
		// Assume CGA Composite and change to 2 colors per byte
		and al, 15
		mov ah, al
		shl ah, 1
		shl ah, 1
		shl ah, 1
		shl ah, 1
		or ah, al
	vps_cga_loop:
		// Fetch mask data into AL
		lodsb
		mov bl, 0
		test al, 0xf0
		jz no1a
		or bl, 0xf0
		no1a:
		test al, 0x0f
		jz no2a
		or bl, 0x0f
		no2a:
		mov al,bl
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Fetch mask data into AL
		lodsb
		mov bl, 0
		test al, 0x3c
		jz no3a
		or bl, 0xf0
		no3a:
		test al, 0x03
		jz no4a
		or bl, 0x0f
		no4a:
		mov al,bl
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Move to next plane
		add di, 8190
		// Fetch mask data into AL
		lodsb
		mov bl, 0
		test al, 0xf0
		jz no1b
		or bl, 0xf0
		no1b:
		test al, 0x0f
		jz no2b
		or bl, 0x0f
		no2b:
		mov al,bl
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Fetch mask data into AL
		lodsb
		mov bl, 0
		test al, 0x3c
		jz no3b
		or bl, 0xf0
		no3b:
		test al, 0x03
		jz no4b
		or bl, 0x0f
		no4b:
		mov al,bl
		// AND sprite color data
		and al, ah
		// Output to screen
		stosb
		// Go back to previous plane
		sub di, 8114
		dec cx
		jz vps_done
		jmp vps_cga_loop
	vps_done:
	}
}

// Add a character to the right cinema tile
// letter_segment = a segment pointer to an 8x8 character to add to a tile/memory location
// dest_segment = a segment pointer to the top-left of a destination tile
void CGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment){
	_asm{
		// Set source (DS:SI) to letter
		mov ds, ss:letter_segment
		// Set offset to 0
		xor si, si
		// Set destination (ES:SI) to dest
		mov es, ss:dest_segment
		// Add 70 to destination position (24 * 11 + 16) / 4
		mov di, 70
		// Set AX to 8, the number of times to repeat
		mov cx, 8
		// Loop while drawing
	vcac_cga_loop:
		// Copy 1 word
		movsw
		// Increase to next destination row
		add di, 4
		// Loop until CX is 0
		loop vcac_cga_loop
	}
}

// Rotate the cinema tiles
void CGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3){
	_asm{
		// Load in the first tile
		mov ds, ss:tile1
		// Add 66 to destination position (24 * 11) / 4
		mov si, 66
		// Copy source into destination as well
		mov di, si
		mov bx, ds
		mov es, bx
		// Increase source by 4 pixels
		inc si
		// Rotate tile1. Set number of words to copy
		mov cx, 23
		// Start copy
		rep movsw
		movsb
		// Copy left edge of tile2 to tile1
		// lds si, ss:tile2
		mov ds, ss:tile2
		mov si, 66
		sub di, 42
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		// Rotate tile2
		sub si, 42
		mov di, si
		mov bx, ds
		mov es, bx
		dec di
		mov cx, 23
		rep movsw
		movsb
		// Copy left edge of tile3 to tile2
		// lds si, ss:tile3
		mov ds, ss:tile3
		mov si, 66
		sub di, 42
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		add si, 5
		add di, 5
		movsb
		// Rotate tile3
		sub si, 42
		mov di, si
		mov bx, ds
		mov es, bx
		dec di
		mov cx, 23
		rep movsw
		movsb
	}
}

// Return the color of the pixel at X, Y
uint8_t CGA_videoGetPixel(uint16_t x, uint16_t y){
	uint8_t color;
	
	_asm{
		// Set source segment (DS) to start of screen
		mov cx, ss:VIDEO_SCREENSEG
		mov ds, cx
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row and store in SI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov si, y
		shl si, 1
		mov si, [ss:bx + si]
		// Increase si by x / 4
		mov ax, x
		mov cx, ax
		shr ax, 1
		shr ax, 1
		add si, ax
		// Get pixel, shift it to the right until it is in the right-most position
		mov al, byte ptr [DS:SI]
		and cx, 3
		xor cx, 3
		jz pixeldone
	pixelloop:
		shr al, 1
		shr al, 1
		loop pixelloop
	pixeldone:
		// Set 'color'
		and al, 3
		mov byte ptr [color], al
	}
	return color;
}

// Translate the mask data, which is 8 pixels per byte, to cga, which is 4 pixels per byte
uint16_t CGA_videoMaskTranslate(uint8_t *data, uint16_t masksize){
	uint8_t *temp, *ptr, i;
	
	// Get needed segments of temporary ram to copy the mask into
	if ((temp = memorySegToPtr(memoryAssign((masksize + 15) >> 4, 1))) == NULL) return 0;
	// Copy mask data into the temporary ram
	memcpy(temp, data, masksize);
	// Loop through the mask data in temp and build it within data
	ptr = data;
	for (i = 0; i < masksize; i++){
		ptr[0] = 0;
		if (temp[i] & 128){
			ptr[0] |= 192;
		}
		if (temp[i] & 64){
			ptr[0] |= 48;
		}
		if (temp[i] & 32){
			ptr[0] |= 12;
		}
		if (temp[i] & 16){
			ptr[0] |= 3;
		}
		ptr++;
		ptr[0] = 0;
		if (temp[i] & 8){
			ptr[0] |= 192;
		}
		if (temp[i] & 4){
			ptr[0] |= 48;
		}
		if (temp[i] & 2){
			ptr[0] |= 12;
		}
		if (temp[i] & 1){
			ptr[0] |= 3;
		}
		ptr++;
	}
	// Return the new size of the mask data
	return masksize << 1;
}

// This routine inverses CGA color 1 and 2
// *source: the CGA data being converted
// *dest: The location to store the conversion results
// w: the number of pixels in width of the data, must be divisible by 4
// h: the height of the tile
// *palette: the color translation table (not used here)
// multi_byte: 0 = 1 byte per pixel, 1 = 2 bytes per pixel (not used here)
void CGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte){
	uint16_t size;
	unsigned long j;
	
	size = (w * h) >> VIDEO_BITSHIFT;
	if (w == 320 && h == 200){
		// Add the space between the two CGA bitplanes
		size += 192;
	}
	for (j = 0; j < size; j++){
		if ((source[j] & 192) == 128){
			source[j] -= 64;
		} else if ((source[j] & 192) == 64){
			source[j] += 64;
		}
		if ((source[j] & 48) == 32){
			source[j] -= 16;
		} else if ((source[j] & 48) == 16){
			source[j] += 16;
		}
		if ((source[j] & 12) == 8){
			source[j] -= 4;
		} else if ((source[j] & 12) == 4){
			source[j] += 4;
		}
		if ((source[j] & 3) == 2){
			source[j] -= 1;
		} else if ((source[j] & 3) == 1){
			source[j] += 1;
		}
		dest[j] = source[j];
	}
}

uint8_t CGA_videoGetUnitColorForMinimap(uint8_t unit){
	uint8_t color = CURSOR.color;
	switch (UNIT_TYPE[unit]){
		case TYPE_EVILBOT:{
			if (color == 1 || color == 3){
				color = 2;
			} else{
				color = 0;
			}
			break;
		}
		case TYPE_HOVERBOT_HORIZ:
		case TYPE_HOVERBOT_VERT:
		case TYPE_MAGNETIZED_ROBOT:
		case TYPE_HOVERBOT_WATER:{
			if (color == 1 || color == 3){
				color = 1;
			} else{
				color = 0;
			}
			break;
		}
		case TYPE_HOVERBOT_ATTACK:
		case TYPE_ROLLERBOT_HORIZ:
		case TYPE_ROLLERBOT_VERT:{
			if (color == 3){
				color = 1;
			} else if (color == 1){
				color = 2;
			}
			break;
		}
		case TYPE_MAGNET:
		case TYPE_TIMEBOMB:
		case TYPE_DEAD_ROBOT:{
			if (color != 3){
				color = 0;
			}
			break;
		}
	}
	return color;
}

void CGA_minimapSet(uint8_t x, uint8_t y, uint8_t color){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	if (VIDEO_COMPOSITE){
		color &= 0x0F;
	} else{
		color &= 0x03;
		color |= (color << 2);
	}
	if (x & 1){
		p[x >> 1] = ((p[x >> 1] & 0xF0) | color);
	} else{
		p[x >> 1] = ((p[x >> 1] & 0x0F) | (color << 4));
	}
	if (MINIMAP_OPEN){
		videoFillRect((x << 1) + 4, (y << 1) + 20, 2, 2, color);
	}
}

void CGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y){
	uint8_t x;
	for (x = 0; x < 128; x += 2){
		*vidptr2 = (MAP_TILE_COLORS[MAP[y][x + 1]] & 0x03) | ((MAP_TILE_COLORS[MAP[y][x + 1]] & 0x03) << 2) | ((MAP_TILE_COLORS[MAP[y][x]] & 0x03) << 4) | ((MAP_TILE_COLORS[MAP[y][x]] & 0x03) << 6);
		vidptr2++;
	}
}

void CGACOMP_minimapPrepareLine(uint8_t *vidptr2, uint8_t y){
	uint8_t x;
	for (x = 0; x < 128; x += 2){
		*vidptr2 = (MAP_TILE_COLORS[MAP[y][x + 1]] & 0x0F) | ((MAP_TILE_COLORS[MAP[y][x]] & 0x0F) << 4);
		vidptr2++;
	}
}

void CGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	memcpy(vidptr2, p, 64);
}

void CGA_videoDamageFlashStep(void){
	DAMAGE_FLASH_STEPS_REMAINING--;
	if (VIDEO_MODE == CGACOMP){
		outp(0x03D9, PALETTE_DAMAGE_CGACOMP[DAMAGE_FLASH_STEPS_REMAINING]);
	} else{
		outp(0x03D9, PALETTE_DAMAGE_CGA[DAMAGE_FLASH_STEPS_REMAINING]);
	}
}

void CGA_videoEMPFlashStep(void){
	EMP_FLASH_STEPS_REMAINING--;
	if (VIDEO_MODE == CGACOMP){
		outp(0x03D9, PALETTE_EMP_CGACOMP[EMP_FLASH_STEPS_REMAINING]);
	} else{
		outp(0x03D9, PALETTE_EMP_CGA[EMP_FLASH_STEPS_REMAINING]);
	}
}

void CGA_videoScreenShakeStep(void){
	// Decrease screen shake, once we hit 0, we will reset screen back to normal
	// Select register 12 to modify screen pointer
	outp(0x3D4, 12);
	SCREEN_SHAKE--;
	outp(0x3D5, SCREEN_SHAKE_VAL[SCREEN_SHAKE]);
	outp(0x3D4, 13);
	SCREEN_SHAKE--;
	outp(0x3D5, SCREEN_SHAKE_VAL[SCREEN_SHAKE]);
}
