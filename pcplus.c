#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "memory.h"
#include "globals.h"
#include "keyboard.h"
#include "pcplus.h"
#include "cga.h"
#include "ega.h"

void PCP_videoInit(int key){
	uint16_t i, j;

	VIDEO_MODE = CGA;
	VIDEO_SCREENSEG = 0xB800;
	// Select strobing cursor colors
	for (i = 0; i < 32; i++){
		if (i < 9){
			CURSOR_SELECTED_COLOR[i] = 0;
		} else if (i < 12){
			CURSOR_SELECTED_COLOR[i] = 8;
		} else if (i < 15){
			CURSOR_SELECTED_COLOR[i] = 7;
		} else if (i < 25){
			CURSOR_SELECTED_COLOR[i] = 15;
		} else if (i < 28){
			CURSOR_SELECTED_COLOR[i] = 7;
		} else{
			CURSOR_SELECTED_COLOR[i] = 8;
		}
	}
	j = 0;
	VIDEO_PLANESHIFT = 2;
	VIDEO_BITSHIFT = 3;
	VIDEO_FILE_MULTIBYTE = 1;
	for (i = 0; i < 200; i+=2){
		VIDEO_SCREENOFFSET[i] = j;
		VIDEO_SCREENOFFSET[i + 1] = j + 0x2000;
		j += 80;
	}
	VIDEO_SCREENSIZE = 64000;
	VIDEO_GAMEOVERSIZE = 29952;
	VIDEO_HUDSIZE = 5248;
	VIDEO_FACE_X = 232;
	VIDEO_FACE_Y = 74;
	VIDEO_FONT_COLOR_MENU = 2;
	VIDEO_FONT_COLOR_MENU_SELECTED = 15;
	VIDEO_FONT_COLOR_INFO = 2;
	VIDEO_FONT_COLOR_WHITE = 15;
	VIDEO_FONT_COLOR_GAMEOVER = 9;
	VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_EGA;
	VIDEO_USE_PALETTE_ANIMATION = 0;
	VIDEO_DAMAGE_FLASH_COUNT = CGA_DAMAGE_FLASH_COUNT;
	VIDEO_EMP_FLASH_COUNT = CGA_EMP_FLASH_COUNT;
	MAP_TILE_COLORS = MAP_TILE_COLORS_VGA;
	MINIMAP_X_OFFSET = 1;
	for (i = 0; i < TILE_COUNT; i++){
		switch (MAP_TILE_COLORS[i]){
			case  0: break;
			case  1: MAP_TILE_COLORS[i] =  2; break;
			case  2: MAP_TILE_COLORS[i] =  6; break;
			case  3: MAP_TILE_COLORS[i] =  1; break;
			case  4: MAP_TILE_COLORS[i] =  4; break;
			case  5: MAP_TILE_COLORS[i] =  8; break;
			case  6: MAP_TILE_COLORS[i] = 10; break;
			case  7: MAP_TILE_COLORS[i] =  6; break;
			case  8: MAP_TILE_COLORS[i] =  9; break;
			case  9: MAP_TILE_COLORS[i] = 14; break;
			case 10: MAP_TILE_COLORS[i] =  3; break;
			case 11: MAP_TILE_COLORS[i] =  9; break;
			case 12: MAP_TILE_COLORS[i] = 14; break;
			case 13: MAP_TILE_COLORS[i] =  7; break;
			case 14: MAP_TILE_COLORS[i] = 12; break;
			case 15: MAP_TILE_COLORS[i] = 15; break;
		}
	}
	strncpy(VIDEO_FILEEXT, ".vga", 5);
	// Set function pointers
	videoSetMode = &PCP_videoSetMode;
	videoUnsetMode = &PCP_videoUnsetMode;
	videoClearScreen = &PCP_videoClearScreen;
	videoDrawFullScreen = &PCP_videoDrawFullScreen;
	videoPrintTile = &PCP_videoPrintTile;
	videoPrintTileMultibyte = &PCP_videoPrintTile;
	videoPrintSprite = &PCP_videoPrintSprite;
	videoPrintSpriteAsColor = &PCP_videoPrintSpriteAsColor;
	videoPrintFontAsColor = &PCP_videoPrintFontAsColor;
	videoCinemaAddChar = &PCP_videoCinemaAddChar;
	videoCinemaRotate = &PCP_videoCinemaRotate;
	videoFillRect = &PCP_videoFillRect;
	videoGetPixel = &PCP_videoGetPixel;
	videoTranslate = &PCP_videoTranslate;
	videoMaskTranslate = &CGA_videoMaskTranslate;
	videoGetUnitColorForMinimap = &EGA_videoGetUnitColorForMinimap;
	videoDamageFlashStep = &CGA_videoDamageFlashStep;
	videoEMPFlashStep = &CGA_videoEMPFlashStep;
	videoScreenShakeStep = &CGA_videoScreenShakeStep;
	minimapSet = &PCP_minimapSet;
	minimapPrepareLine = &PCP_minimapPrepareLine;
	minimapCopyLineToScreen = &PCP_minimapCopyLineToScreen;
}

void PCP_videoSetMode(void){
	union REGS regs;

	// Save old video mode
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	VIDEO_MODE_ON_STARTUP = regs.h.al;

	// Set new video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE;
	int86(0x10, &regs, &regs);
	// Enable Plantronics ColorPlus enhancements
	outp(0x03dd, 0x10);
}

void PCP_videoUnsetMode(void){
	union REGS regs;

	// Disable Plantronics ColorPlus enhancements
	outp(0x3dd, 0x00);
	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
}

void PCP_videoClearScreen(void){
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
		mov cx, 8192 + 8096
		// Copy to screen
		rep stosw
	}
}

// Fill a rectangular area of the screen with specified color
void PCP_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	CGA_videoFillRect(x, y, w, h, (color >> 1) & 3);
	VIDEO_SCREENSEG += 0x0400;
	CGA_videoFillRect(x, y, w, h, ((color >> 3) | (color << 1)) & 3);
	VIDEO_SCREENSEG -= 0x0400;
}

void PCP_videoDrawFullScreen(uint16_t segment){
	// Routine to draw a full screen of graphics data
	_asm{
		// Set source (DS:SI) to passed segment
		mov ds, segment
		xor si, si
		// Set destination segment (ES) to start of screen
		mov ax, ss:VIDEO_SCREENSEG
		mov es, ax
		xor dx, dx
	y_loop:
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, dx
		shl di, 1
		mov di, [ss:bx + di]
		// Set number of times to repeat
		mov cx, 40
		// Copy from memory to screen
		rep movsw
		// Increment and compare Y coordinate
		inc dx
		cmp dx, 200
		jne y_loop
		// Set destination segment (ES) to start of screen
		mov ax, ss:VIDEO_SCREENSEG
		add ax, 0x0400
		mov es, ax
		xor dx, dx
	y_loop2:
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, dx
		shl di, 1
		mov di, [ss:bx + di]
		// Set number of times to repeat
		mov cx, 40
		// Copy from memory to screen
		rep movsw
		// Increment and compare Y coordinate
		inc dx
		cmp dx, 200
		jne y_loop2
	}
}

// Copy a tiled graphic from ram to the screen
// 'x' is the x position on screen to draw the tile (upper-left corner) (must be divisible by 8)
// 'y' is the y position on screen to draw the tile (upper-left corner) (must be divisible by 2)
// 'tileseg' should be a starting segment for the tile in ram
// 'w' is the width in pixels of the tile (must be divisible by 8)
// 'h' is the height in pixels of the tile (must be divisible by 2)
void PCP_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
	CGA_videoPrintTile(x, y, tileseg, w, h);
	VIDEO_SCREENSEG += 0x0400;
	tileseg += ((w >> 2) * h + 15) >> 4;
	CGA_videoPrintTile(x, y, tileseg, w, h);
	VIDEO_SCREENSEG -= 0x0400;
}

// Copy a sprite from ram to the screen, using a mask to also draw the tile underneath
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'tile' should be a pointer to the top-left of the tile in ram
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void PCP_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg){
	// Handle red/green plane, first. This is basically the CGA code
	// with a mask offset of 288 instead of 144.
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
		mov cl, byte ptr [DS:BX + 288]
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
	// Now handle the blue/intensity plane using the original CGA code
	VIDEO_SCREENSEG += 0x0400;
	spriteseg += 9;
	tileseg += 9;
	CGA_videoPrintSprite(x, y, spriteseg, tileseg);
	VIDEO_SCREENSEG -= 0x0400;
}

// Copy a sprite from ram to the screen, using a mask to avoid overwriting surrounding pixels
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'color' should be the color to draw the sprite as
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void PCP_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color){
	spriteseg += 9;
	CGA_videoPrintSpriteAsColor(x, y, spriteseg, (color >> 1) & 3);
	VIDEO_SCREENSEG += 0x0400;
	CGA_videoPrintSpriteAsColor(x, y, spriteseg, ((color >> 3) | (color << 1)) & 3);
	VIDEO_SCREENSEG -= 0x0400;
}

// Copy a font character from ram to the screen
// 'x' is the x position on screen to draw the font (upper-left corner)
// 'y' is the y position on screen to draw the font (upper-left corner)
// 'fontseg' should be a segment pointer to the top-left of the tile in ram
// 'color' should be the color to draw the font as
// WIDTH and HEIGHT are hard set to 8x8 for this routine
void PCP_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color){
	CGA_videoPrintFontAsColor(x, y, fontseg, (color >> 1) & 3);
	VIDEO_SCREENSEG += 0x0400;
	CGA_videoPrintFontAsColor(x, y, fontseg, ((color >> 3) | (color << 1)) & 3);
	VIDEO_SCREENSEG -= 0x0400;
}

// Add a character to the right cinema tile
// letter_segment = a segment pointer to an 8x8 character to add to a tile/memory location
// dest_segment = a segment pointer to the top-left of a destination tile
void PCP_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment){
	CGA_videoCinemaAddChar(letter_segment, dest_segment);
	CGA_videoCinemaAddChar(letter_segment, dest_segment + 9);
}

// Rotate the cinema tiles
void PCP_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3){
	CGA_videoCinemaRotate(tile1, tile2, tile3);
	CGA_videoCinemaRotate(tile1 + 9, tile2 + 9, tile3 + 9);
}

// Return the color of the pixel at X, Y
uint8_t PCP_videoGetPixel(uint16_t x, uint16_t y){
	uint8_t color;
	VIDEO_SCREENSEG += 0x0400;
	color = CGA_videoGetPixel(x, y);
	VIDEO_SCREENSEG -= 0x0400;
	color = ((color & 1) << 3) | (color >> 1) | (CGA_videoGetPixel(x, y) << 1);
	return color;
}

// This routine translates VGA data to Plantronics ColorPlus data.
// *source: the VGA data being converted
// *dest: The location to store the conversion results
// w: the number of pixels in width of the data, must be divisible by 4
// h: the height of the tile
// *palette: the color translation table
// multi_byte: 0 = 1 byte per pixel, 1 = 2 bytes per pixel
void PCP_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte){
	uint16_t size = (w >> 2) * h;
	// if the RG plane starts at a segment boundary, the BI plane should, too
	uint16_t bi_offset = (size + 15) & 0xfff0;
	uint8_t c1, c2, c3, c4, rg, bi;
	uint8_t pal[16];
	uint16_t i;
	// convert palette from IRGB to RGBI
	for (i = 0; i < 16; i++){
		c1 = palette[i];
		c1 = ((c1 & 7) << 1) | (c1 >> 3);
		pal[i] = c1;
	}
	if (!multi_byte){
		// iterate over input data, one pixel per byte
		for (i = 0; i < size; i++){
			// read four source pixels
			c1 = pal[*source++];
			c2 = pal[*source++];
			c3 = pal[*source++];
			c4 = pal[*source++];
			// create byte values for RG and BI plane
			rg = ((c1 & 12) << 4) | ((c2 & 12) << 2) | (c3 & 12) | ((c4 & 12) >> 2);
			bi = ((c1 & 3) << 6) | ((c2 & 3) << 4) | ((c3 & 3) << 2) | (c4 & 3);
			// write to destination buffer
			*dest = rg;
			*(dest + bi_offset) = bi;
			dest++;
		}
	} else{
		// iterate over input data, two pixels per byte
		for (i = 0; i < size; i++){
			// read four source pixels
			c1 = pal[*source & 15];
			c2 = pal[*source++ >> 4];
			c3 = pal[*source & 15];
			c4 = pal[*source++ >> 4];
			// create byte values for RG and BI plane
			rg = ((c1 & 12) << 4) | ((c2 & 12) << 2) | (c3 & 12) | ((c4 & 12) >> 2);
			bi = ((c1 & 3) << 6) | ((c2 & 3) << 4) | ((c3 & 3) << 2) | (c4 & 3);
			// write to destination buffer
			*dest = rg;
			*(dest + bi_offset) = bi;
			dest++;
		}
	}
}

void PCP_minimapSet(uint8_t x, uint8_t y, uint8_t color){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	uint8_t rgrg, bibi;

	color &= 0x0F;
	rgrg = (color >> 1) & 0x03;
	rgrg |= rgrg << 2;
	bibi = ((color << 1) | (color >> 3)) & 0x03;
	bibi |= bibi << 2;
	if (x & 1){
		p[x >> 1] = ((p[x >> 1] & 0xF0) | rgrg);
		p[(x >> 1) + 64] = ((p[(x >> 1) + 64] & 0xF0) | bibi);
	} else{
		p[x >> 1] = ((p[x >> 1] & 0x0F) | (rgrg << 4));
		p[(x >> 1) + 64] = ((p[(x >> 1) + 64] & 0x0F) | (bibi << 4));
	}
	if (MINIMAP_OPEN){
		videoFillRect((x << 1) + 4, (y << 1) + 20, 2, 2, color);
	}
}

void PCP_minimapPrepareLine(uint8_t *vidptr2, uint8_t y){
	uint8_t x, rgrg1, bibi1, rgrg2, bibi2;
	for (x = 0; x < 128; x += 2){
		rgrg1 = (MAP_TILE_COLORS[MAP[y][x]] >> 1) & 0x03;
		rgrg1 |= rgrg1 << 2;
		bibi1 = ((MAP_TILE_COLORS[MAP[y][x]] >> 3) | (MAP_TILE_COLORS[MAP[y][x]] << 1)) & 0x03;
		bibi1 |= bibi1 << 2;
		rgrg2 = (MAP_TILE_COLORS[MAP[y][x + 1]] >> 1) & 0x03;
		rgrg2 |= rgrg2 << 2;
		bibi2 = ((MAP_TILE_COLORS[MAP[y][x + 1]] >> 3) | (MAP_TILE_COLORS[MAP[y][x + 1]] << 1)) & 0x03;
		bibi2 |= bibi2 << 2;
		*vidptr2 = (rgrg1 << 4) | rgrg2;
		*(vidptr2 + 64) = (bibi1 << 4) | bibi2;
		vidptr2++;
	}
}

void PCP_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	memcpy(vidptr2, p, 64);
	memcpy(vidptr2 + 0x4000, p + 64, 64);
}
