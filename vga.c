#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "memory.h"
#include "globals.h"
#include "keyboard.h"
#include "interrupt.h"
#include "vga.h"
#include "cga.h"

void VGA_videoInit(int key){
	uint16_t i, j;
	
	VIDEO_MODE = VGA;
	VIDEO_SCREENSEG = 0xA000;
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
	VIDEO_FONT_COLOR_MENU = 8;
	VIDEO_FONT_COLOR_MENU_SELECTED = 16;
	VIDEO_FONT_COLOR_INFO = 9;
	VIDEO_FONT_COLOR_WHITE = 15;
	VIDEO_FONT_COLOR_GAMEOVER = 9;
	VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_VGA;
	VIDEO_USE_PALETTE_ANIMATION = 1;
	VIDEO_DAMAGE_FLASH_COUNT = VGA_DAMAGE_FLASH_COUNT;
	VIDEO_EMP_FLASH_COUNT = VGA_EMP_FLASH_COUNT;
	MAP_TILE_COLORS = MAP_TILE_COLORS_VGA;
	MINIMAP_X_OFFSET = 4;
	strncpy(VIDEO_FILEEXT, ".vga", 5);
	// Set function pointers
	videoSetMode = &VGA_videoSetMode;
	videoUnsetMode = &VGA_videoUnsetMode;
	videoClearScreen = &VGA_videoClearScreen;
	videoDrawFullScreen = &VGA_videoDrawFullScreen;
	videoPrintTile = &VGA_videoPrintTile;
	videoPrintTileMultibyte = &VGA_videoPrintTileMultibyte;
	videoPrintSprite = &VGA_videoPrintSprite;
	videoPrintSpriteAsColor = &VGA_videoPrintSpriteAsColor;
	videoPrintFontAsColor = &VGA_videoPrintFontAsColor;
	videoCinemaAddChar = &VGA_videoCinemaAddChar;
	videoCinemaRotate = &VGA_videoCinemaRotate;
	videoFillRect = &VGA_videoFillRect;
	videoGetPixel = &VGA_videoGetPixel;
	videoTranslate = NULL;
	videoMaskTranslate = &VGA_videoMaskTranslate;
	videoPaletteSet = &VGA_videoPaletteSet;
	videoGetUnitColorForMinimap = &VGA_videoGetUnitColorForMinimap;
	videoDamageFlashStep = &VGA_videoDamageFlashStep;
	videoEMPFlashStep = &VGA_videoEMPFlashStep;
	videoScreenShakeStep = &CGA_videoScreenShakeStep;
	videoFadeIn = &VGA_videoFadeIn;
	videoFadeOut = &VGA_videoFadeOut;
	minimapSet = &VGA_minimapSet;
	minimapPrepareLine = &VGA_minimapPrepareLine;
	minimapCopyLineToScreen = &VGA_minimapCopyLineToScreen;

	VIDEO_PALETTE_GAME = (uint8_t*)PALETTE_GAME_VGA;
	VIDEO_PALETTE_TITLE = (uint8_t*)PALETTE_TITLE_VGA;
	VIDEO_PALETTE_GAMEOVER = (uint8_t*)PALETTE_GAMEOVER_VGA;
}

void VGA_videoSetMode(void){
	union REGS regs;

	// Save old video mode
	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);
	VIDEO_MODE_ON_STARTUP = regs.h.al;

	// Set new video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE;
	int86(0x10, &regs, &regs);
}

void VGA_videoUnsetMode(void){
	union REGS regs;

	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
}

void VGA_videoPaletteSet(const uint8_t *palette, uint8_t fade_bit_shift){
	unsigned int i, c;
	int enable_interrupt;

	// Disable interrupt before setting palette, since the interrupt can modify them as well
	enable_interrupt = interruptDisable();
	// Set which color we are starting with
	outp(0x03C8, 0);
	c = 0;
	for (i = 0; i < PALETTE_COUNT; i++){
		// Set the color RGB
		outp(0x03C9, palette[c] >> fade_bit_shift << 2);
		c++;
		outp(0x03C9, palette[c] >> fade_bit_shift << 2);
		c++;
		outp(0x03C9, palette[c] >> fade_bit_shift << 2);
		c++;
	}
	// Add color for cursor
	for (i = 0; i < 4; i++){
		outp(0x03C9, 252 >> fade_bit_shift);
		outp(0x03C9, 252 >> fade_bit_shift);
		outp(0x03C9, 252 >> fade_bit_shift);
	}
	// Add the health bar colors if this is the main palette
	if (palette == PALETTE_GAME_VGA){
		for (i = 0; i < 36; i += 3){
			// Main color for health bar
			outp(0x03C9, (*PALETTE_HEALTH_VGA + i)[0] >> fade_bit_shift << 2);
			outp(0x03C9, (*PALETTE_HEALTH_VGA + i + 1)[0] >> fade_bit_shift << 2);
			outp(0x03C9, (*PALETTE_HEALTH_VGA + i + 2)[0] >> fade_bit_shift << 2);
			// Create highlight color for health bar
			c = (*PALETTE_HEALTH_VGA + i)[0] >> fade_bit_shift << 2;
			c |= 12;
			outp(0x03C9, c);
			c = (*PALETTE_HEALTH_VGA + i + 1)[0] >> fade_bit_shift << 2;
			c |= 12;
			outp(0x03C9, c);
			c = (*PALETTE_HEALTH_VGA + i + 2)[0] >> fade_bit_shift << 2;
			c |= 12;
			outp(0x03C9, c);
			// Create shadow color for health bar
			c = (*PALETTE_HEALTH_VGA + i)[0] >> fade_bit_shift << 2;
			c &= 240;
			outp(0x03C9, c);
			c = (*PALETTE_HEALTH_VGA + i + 1)[0] >> fade_bit_shift << 2;
			c &= 240;
			outp(0x03C9, c);
			c = (*PALETTE_HEALTH_VGA + i + 2)[0] >> fade_bit_shift << 2;
			c &= 240;
			outp(0x03C9, c);
		}
	}
	// Set color 255 for cinema screen
	outp(0x03C8, 255);
	outp(0x03C9, 255 >> fade_bit_shift);
	outp(0x03C9, 255 >> fade_bit_shift);
	outp(0x03C9, 255 >> fade_bit_shift);
	// Reenable the interrupt
	if (enable_interrupt){
		interruptEnable();
	}
}

void VGA_videoClearScreen(){
	// Routine to clear the full screen
	_asm{
		// data we will fill the screen with (0 for black)
		xor ax, ax
		// Set destination segment (ES) to start of screen
		mov bx, 0xA000
		mov es, bx
		// Set destination offset (DI) to start of screen
		mov di, ax
		// Set CX to number of times to repeat
		mov cx, 32000
		// Copy to the screen
		rep stosw
	}
}

// Fill a rectangular area of the screen with specified color
void VGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	
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
		// Get video mode
		mov cl, ss:VIDEO_MODE
		// Move w into DX
		mov dx, w
		// Set destination segment (ES) to start of screen
		mov cx, 0xA000
		mov es, cx
		// Increase DI to X position
		mov cx, ds
		add di, cx
		// Calculate the number of bytes needed to move to the next line, store in BX
		mov bx, 320
		sub bx, dx
		// Set AL to color
		mov ax, color
	vfr_vga_mainloop:
		// Set number of times to repeat (width)
		mov cx, dx
		// Copy color to the screen
		rep stosb
		// Move to next screen row
		add di, bx
		dec si
		jne vfr_vga_mainloop
	}
}

void VGA_videoDrawFullScreen(uint16_t segment){
	// Routine to draw a full screen of graphics data
	_asm{
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
		stosw
		// Loop until CX is 0
		loop vdfs_vga_loop
	}
}

// Copy a tiled graphic from ram to the screen
// 'x' is the x position on screen to draw the tile (upper-left corner) (must be divisible by 8)
// 'y' is the y position on screen to draw the tile (upper-left corner) (must be divisible by 2)
// 'tileseg' should be a starting segment for the tile in ram
// 'w' is the width in pixels of the tile (must be divisible by 8)
// 'h' is the height in pixels of the tile (must be divisible by 2)
void VGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
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
		// Set ES segment to VGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Set AX to h, the number of times to repeat
		mov ax, h
		// Add x to screen position
		add di, x
		// Save number of bytes to copy per row
		mov dx, w
		// Store how many bytes to move to next line
		mov bx, 320
		sub bx, dx
		// Shift number of bytes to copy per row (divide by 2), since we will be moving 2 bytes at a time
		shr dx, 1
		// Loop while drawing to the screen
	vpt_vga_loop:
		// Set number of words to copy
		mov cx, dx
		// Start copy
		rep movsw
		// Increase to next video row
		add di, bx
		// Loop until AX is 0
		dec ax
		jnz vpt_vga_loop
	}
}

// Copy a tiled graphic from ram to the screen. 
// This version expects each byte to hold 2 pixels (for 16-color VGA low/high nibble)
// 'x' is the x position on screen to draw the tile (upper-left corner) (must be divisible by 4)
// 'y' is the y position on screen to draw the tile (upper-left corner) (must be divisible by 2)
// 'tile' should be a pointer to the top-left of the tile in ram
// 'w' is the width in pixels of the tile (must be divisible by 8)
// 'h' is the height in pixels of the tile (must be divisible by 2)
void VGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
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
		mov bx, 320
		sub bx, cx
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
		stosw
		// Loop until CL is 0
		dec cl
		jnz vptm_vga_loop
		// Move to new line and restore count of how many bytes to process
		mov cl, ch
		add di, bx
		// Check if we need to process another row
		dec dl
		jnz vptm_vga_loop
	vptm_done:
	}
}

// Copy a sprite from ram to the screen, using a mask to also draw the tile underneath
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'tile' should be a pointer to the top-left of the tile in ram
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void VGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg){

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
		// Set BX to offset of mask data. Store in DX to reset upon second pass
		mov bx, 576
		mov dx, bx
		// Set ES segment to VGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Add x to screen position
		add di, x
		// Set cx to 0, to make high byte by 0 for copying
		mov cx, si
		// Set ax to 0, used to know which pass we are on, sprite or tile
		mov ax, si
		// Loop while drawing to the screen
	vps_vga_loop:
		// Check if ax is 0 to see if we need to swap the segment register to access the mask data
		cmp ax, 0
		je vps_vga_loop2
		// Swap segment register
		mov ds, dx
	vps_vga_loop2:
		// Fetch mask byte, giving number of bytes to copy and flags for how to process.
		mov cl, byte ptr [ds:bx]
		// If mask byte was 0, then we are done, exit loop
		cmp cl, 0
		jz vps_vga_passdone
		// Increase bx to go to next byte of mask
		inc bx
		// If mask bit 7 (128) is on, then we have reached the end of a row
		cmp cl, 127
		ja vps_vga_loop5
		// If AX != 0, then we are on second pass, flip the 6th bit on CL to draw the opposite and swap segment registers
		cmp ax, 0
		je vps_vga_loop3
		// Flip the bit
		xor cl, 64
		// Swap to the segment register for the tile data
		mov ds, ax
	vps_vga_loop3:
		// If number of bytes to copy has bit 6 (64) set, then show sprite/tile
		cmp cl, 64
		jb vps_vga_loop4
		and cl, 63
		// Start copy
		rep movsb
		jmp vps_vga_loop
	vps_vga_loop4:
		add si, cx
		add di, cx
		jmp vps_vga_loop
	vps_vga_loop5:
		// Process byte AND go to the next screen row. First clear the 7 (128) bit that indicates to go to next row
		and cl, 127
		// If AX != 0, then we are on second pass, flip the 6th bit on CL to draw the opposite and swap segment registers
		cmp ax, 0
		je vps_vga_loop6
		// Flip the bit
		xor cl, 64
		// Swap to the segment register for the tile data
		mov ds, ax
	vps_vga_loop6:
		// If number of bytes to copy has bit 6 (64) set, then show sprite/tile
		cmp cl, 64
		jb vps_vga_loop7
		and cl, 63
		// Start copy
		rep movsb
		// Go to next row
		add di, 296
		jmp vps_vga_loop
	vps_vga_loop7:
		add si, cx
		add di, cx
		// Go to next row
		add di, 296
		jmp vps_vga_loop
	vps_vga_passdone:
		// Check if we are done
		cmp ax, 0
		jne vps_done
		// Do a second pass for tile data
		// First reset BX to start of mask data
		mov bx, 576
		// Store the segment for the mask data in DX
		mov dx, ds
		// Reset screen position back to the top-left of the tile
		sub di, 7680
		// Load pointer to tileseg data and save the segment in DS:SI
		mov ax, ss:tileseg
		mov ds, ax
		xor si, si
		// Do another pass
		jmp vps_vga_loop
	vps_done:
	}
}

// Copy a sprite from ram to the screen, using a mask to avoid overwriting pixels on the screen
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'color' is the color to use
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void VGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to sprite, which has the mask data directly after it.
		mov ds, spriteseg
		xor si, si
		// Set BX to offset of mask data.
		mov bx, 576
		// Set ES segment to VGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Add x to screen position
		add di, x
		// Set cx to 0, to make high byte be 0 for copying
		mov cx, si
		// Set ax to color we are drawing sprite as
		mov ax, color
		// Loop while drawing to the screen
	vps_vga_loop:
		// Fetch mask byte, giving number of bytes to copy and flags for how to process.
		mov cl, byte ptr [ds:bx]
		// If mask byte was 0, then we are done, exit loop
		cmp cl, 0
		jz vps_done
		// Increase bx to go to next byte of mask
		inc bx
		// If mask bit 7 (128) is on, then we have reached the end of a row
		cmp cl, 127
		ja vps_vga_loop5
		// If number of bytes to copy has bit 6 (64) set, then show sprite
		cmp cl, 64
		jb vps_vga_loop4
		and cl, 63
		// Set sprite as color
		rep stosb
		jmp vps_vga_loop
	vps_vga_loop4:
		add si, cx
		add di, cx
		jmp vps_vga_loop
	vps_vga_loop5:
		// Process byte AND go to the next screen row. First clear the 7 (128) bit that indicates to go to next row
		and cl, 127
		// If number of bytes to copy has bit 6 (64) set, then show sprite
		cmp cl, 64
		jb vps_vga_loop7
		and cl, 63
		// Start copy
		rep stosb
		// Go to next row
		add di, 296
		jmp vps_vga_loop
	vps_vga_loop7:
		add si, cx
		add di, cx
		// Go to next row
		add di, 296
		jmp vps_vga_loop
	vps_done:
	}
}

// Copy a font character from ram to the screen
// 'x' is the x position on screen to draw the font (upper-left corner)
// 'y' is the y position on screen to draw the font (upper-left corner)
// 'fontseg' should be a segment pointer to the top-left of the tile in ram
// 'color' is the color to use
// WIDTH and HEIGHT are hard set to 8x8 for this routine
void VGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to font
		mov ds, fontseg
		xor si, si
		// Set ES segment to VGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Add x to screen position
		add di, x
		// Set bx to color we are drawing sprite as
		mov bx, color
		// Set cx to the number of times to repeat
		mov cx, 8
		// Loop while drawing to the screen
	vps_vga_loop:
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// Fetch mask byte
		lodsb
		// And byte with color
		and al, bl
		// Output byte to screen
		stosb
		// // Move to next row
		add di, 312
		loop vps_vga_loop
	}
}

// Add a character to the right cinema tile
// letter_segment = a segment pointer to an 8x8 character to add to a tile/memory location
// dest_segment = a segment pointer to the top-left of a destination tile
void VGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment){
	_asm{
		// Set source (DS:SI) to letter
		mov ds, ss:letter_segment
		// Set offset to 0
		xor si, si
		// Set destination (ES:SI) to dest
		mov es, ss:dest_segment
		// Set destination index to 280 (24 * 11 + 16)
		mov di, 280
		// Set CX to 8, the number of times to repeat
		mov cx, 8
		// Loop while drawing to the screen
	vcac_vga_loop:
		// Copy 8 bytes
		movsw
		movsw
		movsw
		movsw
		// Increase to next destination row
		add di, 16
		loop vcac_vga_loop
	}
}

// Rotate the cinema tiles
void VGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3){
	_asm{
		// Load in the first tile
		mov ds, ss:tile1
		// Add 264 to destination position (24 * 11)
		mov si, 264
		// Copy source into destination as well
		mov di, si
		mov bx, ds
		mov es, bx
		// Increase source by 4 pixels
		add si, 4
		// Rotate tile1. Set number of words to copy
		mov cx, 94
		// Start copy
		rep movsw
		// Copy left edge of tile2 to tile1
		mov ds, ss:tile2
		mov si, 264
		mov di, 284
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		// Rotate tile2
		mov si, 268
		mov di, si
		mov bx, ds
		mov es, bx
		sub di, 4
		mov cx, 94
		rep movsw
		// Copy left edge of tile3 to tile2
		mov ds, ss:tile3
		mov si, 264
		mov di, 284
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		add si, 20
		add di, 20
		movsw
		movsw
		// Rotate tile3
		mov si, 268
		mov di, si
		mov bx, ds
		mov es, bx
		sub di, 4
		mov cx, 94
		rep movsw
	}
}

// Return the color of the pixel at X, Y
uint8_t VGA_videoGetPixel(uint16_t x, uint16_t y){
	uint8_t color;
	
	_asm{
		// Set source segment (DS) to start of screen
		mov cx, 0xA000
		mov ds, cx
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row and store in SI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov si, y
		shl si, 1
		mov si, [ss:bx + si]
		// Increase si by x
		add si, x
		// Set 'color'
		mov al, byte ptr [DS:SI]
		mov byte ptr [color], al
	}
	return color;
}

// Translate the mask data, which is 8 pixels per byte, to vga, which is 1 pixel per byte
uint16_t VGA_videoMaskTranslate(uint8_t *data, uint16_t masksize){
	uint8_t *temp, *ptr, i, i2, bit, curbit, len;
	
	// Get needed segments of temporary ram to copy the mask into
	if ((temp = memorySegToPtr(memoryAssign((masksize + 15) >> 4, 1))) == NULL) return 0;
	// Copy mask data into the temporary ram
	memcpy(temp, data, masksize);
	ptr = data;
	if (masksize == 72){
		// This is a sprite, use a special format to indicate tile/sprite and length
		masksize = 0;
		for (i = 0; i < 24; i++){
			for (i2 = 0; i2 < 3; i2++){
				if (i2 == 0){
					curbit = (temp[0] >> 7) & 1;
					len = 0;
				}
				for (bit = 0; bit < 8; bit++){
					if (curbit == 0 && ((temp[0] >> (7 - bit)) & 1) == 0){
						len++;
					} else if (curbit == 1 && ((temp[0] >> (7 - bit)) & 1) == 1){
						len++;
					} else{
						// Flag (64) to signify if we should draw the tile or the sprite
						if (curbit == 0){
							len |= 64;
							curbit = 1;
						} else{
							curbit = 0;
						}
						ptr[masksize] = len;
						masksize++;
						len = 1;
					}
				}
				temp++;
			}
			// Flag (64) to signify if we should draw the tile or the sprite
			if (curbit == 0){
				len |= 64;
			}
			// Add 128 to flag end of row
			ptr[masksize] = len | 128;
			masksize++;
			len = 0;
		}
		// End mask data with a 0
		ptr[masksize] = 0;
		masksize++;
		// Return the size of the mask data
		return masksize;
	} else{
		// Loop through the mask data in temp and build it within data
		for (i = 0; i < masksize; i++){
			if (temp[i] & 1){
				ptr[7] = 255;
			} else{
				ptr[7] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[6] = 255;
			} else{
				ptr[6] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[5] = 255;
			} else{
				ptr[5] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[4] = 255;
			} else{
				ptr[4] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[3] = 255;
			} else{
				ptr[3] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[2] = 255;
			} else{
				ptr[2] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[1] = 255;
			} else{
				ptr[1] = 0;
			}
			temp[i] >>= 1;
			if (temp[i] & 1){
				ptr[0] = 255;
			} else{
				ptr[0] = 0;
			}
			ptr += 8;
		}
		// Return the new size of the mask data
		return masksize << 3;
	}
}

uint8_t VGA_videoGetUnitColorForMinimap(uint8_t unit){
	uint8_t color = CURSOR.color;
	switch (UNIT_TYPE[unit]){
		case TYPE_MAGNET:
		case TYPE_TIMEBOMB:
		case TYPE_EXPLOSION_SMALL:
		case TYPE_EVILBOT:{
			color = 19;
			break;
		}
		case TYPE_HOVERBOT_HORIZ:
		case TYPE_HOVERBOT_VERT:
		case TYPE_MAGNETIZED_ROBOT:
		case TYPE_HOVERBOT_WATER:{
			color = 17;
			break;
		}
		case TYPE_HOVERBOT_ATTACK:
		case TYPE_ROLLERBOT_HORIZ:
		case TYPE_ROLLERBOT_VERT:{
			color = 18;
			break;
		}
		default:{
			color = 16;
		}
	}
	return color;
}

void VGA_minimapSet(uint8_t x, uint8_t y, uint8_t color){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	p[x << 1] = color;
	p[(x << 1) + 1] = color;
	if (MINIMAP_OPEN){
		videoFillRect((x << 1) + 4, (y << 1) + 20, 2, 2, color);
	}
}

void VGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y){
	uint8_t x;
	for (x = 0; x < 128; x++){
		*vidptr2 = MAP_TILE_COLORS[MAP[y][x]];
		vidptr2++;
		*vidptr2 = *(vidptr2 - 1);
		vidptr2++;
	}
}

void VGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	memcpy(vidptr2, p, 256);
}

void VGA_videoDamageFlashStep(void){
	// Update the color for palette entry #0 (background color)
	outp(0x03C8, 0);
	DAMAGE_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_DAMAGE_VGA[DAMAGE_FLASH_STEPS_REMAINING]);
	DAMAGE_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_DAMAGE_VGA[DAMAGE_FLASH_STEPS_REMAINING]);
	DAMAGE_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_DAMAGE_VGA[DAMAGE_FLASH_STEPS_REMAINING]);
}

void VGA_videoEMPFlashStep(void){
	// Update the color for palette entry #0 (black background)
	outp(0x03C8, 0);
	EMP_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_EMP_VGA[EMP_FLASH_STEPS_REMAINING]);
	EMP_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_EMP_VGA[EMP_FLASH_STEPS_REMAINING]);
	EMP_FLASH_STEPS_REMAINING--;
	outp(0x03C9, PALETTE_EMP_VGA[EMP_FLASH_STEPS_REMAINING]);
}

void VGA_videoAnimateCursorAndRobotColors(void){
	// Update the color for palette entry #16 (Player, White flashing)
	outp(0x03C8, 16);
	outp(0x03C9, CURSOR.color);
	outp(0x03C9, CURSOR.color);
	outp(0x03C9, CURSOR.color);
	if (MINIMAP_OPEN){
		// Update the color for palette entry #17 (Non-aggressive bots, Cyan flashing)
		outp(0x03C9, CURSOR.color & 0x08);
		outp(0x03C9, CURSOR.color);
		outp(0x03C9, CURSOR.color);
		// Update the color for palette entry #18 (Aggressive bots, Orange flashing)
		outp(0x03C9, CURSOR.color);
		outp(0x03C9, CURSOR.color & 0x60);
		outp(0x03C9, 0);
		// Update the color for palette entry #19 (Evil-Bot, Red flashing)
		outp(0x03C9, CURSOR.color);
		outp(0x03C9, CURSOR.color & 0x08);
		outp(0x03C9, CURSOR.color & 0x08);
	}
}

void VGA_videoFadeIn(const uint8_t *palette, uint8_t timer){
	uint8_t i;
	
	// Determine the number of steps for fade-in. If timer is 0, do it instantly
	if (timer > 0){
		i = 9;
	} else{
		i = 1;
	}
	// Loop, setting an event timer between each fade step
	do {
		i--;
		if (i == 3){
			i = 0;
		}
		videoPaletteSet(palette, i);
		EVENT_TIMER = timer;
		EVENT_ENABLED = 1;
		while (EVENT_TIMER){}
	} while (i != 0);
}

void VGA_videoFadeOut(const uint8_t *palette, uint8_t timer){
	uint8_t i;
	
	// Determine the number of steps for fade-out. If timer is 0, do it instantly
	if (timer > 0){
		i = 4;
	} else{
		i = 8;
	}
	// Make sure the cursor isn't updating its color
	CURSOR_ENABLED = 0;
	// Loop, setting an event timer between each fade step
	do {
		videoPaletteSet(palette, i);
		EVENT_TIMER = timer;
		EVENT_ENABLED = 1;
		while (EVENT_TIMER){}
		i++;
	} while (i < 9);
}





