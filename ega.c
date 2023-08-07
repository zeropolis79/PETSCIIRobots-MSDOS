#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include "video.h"
#include "globals.h"
#include "keyboard.h"
#include "memory.h"
#include "ega.h"
#include "cga.h"

void EGA_videoInit(int key){
	uint16_t i, j;

	VIDEO_MODE = EGA;
	VIDEO_SCREENSEG = 0xA000;
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
	VIDEO_PLANESHIFT = 2;
	VIDEO_BITSHIFT = 3;
	VIDEO_FILE_MULTIBYTE = 1;
	j = 0;
	for (i = 0; i < 200; i++){
		VIDEO_SCREENOFFSET[i] = j;
		j += 40;
	}
	VIDEO_SCREENSIZE = 64000;
	VIDEO_GAMEOVERSIZE = 29952;
	VIDEO_HUDSIZE = 5248;
	VIDEO_FACE_X = 232;
	VIDEO_FACE_Y = 75;
	VIDEO_FONT_COLOR_MENU = 2;
	VIDEO_FONT_COLOR_MENU_SELECTED = 15;
	VIDEO_FONT_COLOR_INFO = 2;
	VIDEO_FONT_COLOR_WHITE = 15;
	VIDEO_FONT_COLOR_GAMEOVER = 9;
	VIDEO_HEALTH_COLORS = (uint8_t*)HEALTH_COLORS_EGA;
	VIDEO_USE_PALETTE_ANIMATION = 0;
	MAP_TILE_COLORS = MAP_TILE_COLORS_VGA;
	MINIMAP_X_OFFSET = 0;
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
	videoSetMode = &EGA_videoSetMode;
	videoUnsetMode = &EGA_videoUnsetMode;
	videoClearScreen = &EGA_videoClearScreen;
	videoDrawFullScreen = &EGA_videoDrawFullScreen;
	videoPrintTile = &EGA_videoPrintTile;
	videoPrintTileMultibyte = &EGA_videoPrintTile;
	videoPrintSprite = &EGA_videoPrintSprite;
	videoPrintSpriteAsColor = &EGA_videoPrintSpriteAsColor;
	videoPrintFontAsColor = &EGA_videoPrintFontAsColor;
	videoCinemaAddChar = &EGA_videoCinemaAddChar;
	videoCinemaRotate = &EGA_videoCinemaRotate;
	videoFillRect = &EGA_videoFillRect;
	videoGetPixel = &EGA_videoGetPixel;
	videoTranslate = &EGA_videoTranslate;
	videoMaskTranslate = NULL;
	videoGetUnitColorForMinimap = &EGA_videoGetUnitColorForMinimap;
	videoScreenShakeStep = &CGA_videoScreenShakeStep;
	minimapSet = &EGA_minimapSet;
	minimapPrepareLine = &EGA_minimapPrepareLine;
	minimapCopyLineToScreen = &EGA_minimapCopyLineToScreen;
}

void EGA_videoSetMode(void){
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

void EGA_videoUnsetMode(void){
	union REGS regs;

	// Restore old video mode
	regs.h.ah = 0x00;
	regs.h.al = VIDEO_MODE_ON_STARTUP;
	int86(0x10, &regs, &regs);
}

void EGA_videoClearScreen(void){
	// Routine to clear the full screen
	_asm{
		// Set destination segment (ES) to start of screen
		mov bx, 0xA000
		mov es, bx
		// Set destination offset (DI) to start of screen
		xor di, di
		// Set EGA to write mode
		mov dx, 0x03C4
		mov al, 2
		out dx, al
		// Prepare port data for selecting plane (port 0x03C5. output 1, 2, 4, 8 to select plane)
		inc dx
		mov bl, 1
	vcs_ega_loop:
		// Select EGA plane
		mov al, bl
		out dx, al
		// Shift left to increase to next plane in next loop
		shl bl, 1
		// Set AX to the color black
		xor ax, ax
		// Set CX to number of times to repeat
		mov cx, 4000
		// Copy to screen plane
		rep stosw
		// Reset destination index di to 0 to start copying to beginning of screen plane
		mov di, ax
		// Loop until done with all 4 planes
		cmp bl, 16
		jne vcs_ega_loop
	}
}

// Fill a rectangular area of the screen with specified color
void EGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	
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
		// Get video mode
		mov cl, ss:VIDEO_MODE
		// Push BP onto the stack
		push bp
		// Move w into BP
		mov ax, w
		mov bp, ax
		// Set destination segment (ES) to start of screen
		mov cx, 0xA000
		mov es, cx
		// Make sure the color (DX) falls in the proper range for EGA (make a copy in CX to compare with original color)
		mov cx, dx
		and dx, 15
		jne vfr_ega_nocolorchange
		// If the color after being ANDed is 0 and original color was not 0 then set it to 15 (white)
		cmp cx, 0
		je vfr_ega_nocolorchange
		mov dl, 15
	vfr_ega_nocolorchange:
		// Push flag that there are no more planes to process
		mov cx, 0xFFFF
		push cx
		// Repeat the color (DL) for all 8 bits and push the color bit for each plane onto the stack
		mov ch, dl
		and ch, 8
		jz vfr_ega_plane4bitoff
		mov ch, 0xFF
	vfr_ega_plane4bitoff:
		mov cl, 8
		push cx
		mov cx, 3
		push cx
		mov ch, dl
		and ch, 4
		jz vfr_ega_plane3bitoff
		mov ch, 0xFF
	vfr_ega_plane3bitoff:
		mov cl, 4
		push cx
		mov cx, 2
		push cx
		mov ch, dl
		and ch, 2
		jz vfr_ega_plane2bitoff
		mov ch, 0xFF
	vfr_ega_plane2bitoff:
		mov cl, 2
		push cx
		mov cx, 1
		push cx
		mov ch, dl
		and ch, 1
		jz vfr_ega_plane1bitoff
		mov ch, 0xFF
	vfr_ega_plane1bitoff:
		// Set read and write enabled for plane 1
		mov al, 4
		mov dx, 0x03CE
		out dx, al
		inc dx
		mov al, 0
		out dx, al
		mov dx, 0x03C4
		mov al, 2
		out dx, al
		inc dx
		dec al
		out dx, al
		// Store color bits for first plane in dl
		mov dl, ch
		// Store FFFF in BX, for starting(BH)/ending(BL) mask bytes
		mov bx, 0xFFFF
		// Get (x + w - 1) formula for getting ending byte/offset and store in AX
		mov ax, ds
		add ax, bp
		dec ax
		// Calculate ending pixel offset ((x + w - 1) & 7) (xor to inverse the bits to shift) and store a mask in BL
		mov cx, ax
		and cx, 7
		xor cx, 7
		je vfr_ega_endingoffsetdone
	vfr_ega_endingoffsetloop:
		shl bl, 1
		dec cl
		jne vfr_ega_endingoffsetloop
	vfr_ega_endingoffsetdone:
		// Get ending byte ((x + w - 1) >> 4) (AL)
		shr ax, 1
		shr ax, 1
		shr ax, 1
		// Calculate starting pixel offset (x & 7) and store a mask in BH
		mov cx, ds
		and cl, 7
		je vfr_ega_startingoffsetdone
	vfr_ega_startingoffsetloop:
		shr bh, 1
		dec cl
		jne vfr_ega_startingoffsetloop
	vfr_ega_startingoffsetdone:
		// Get starting byte (x >> 4) and store in AH
		mov cx, ds
		shr cx, 1
		shr cx, 1
		shr cx, 1
		mov ah, cl
		// Increase DI to X position and push it onto the stack so we can reset for each plane
		add di, cx
		push di
		// Push height we are drawing on the stack
		push si
		// Calculate the number of bytes needed to move to the next line, store in BP
		mov dh, al
		sub dh, ah
		mov cx, 39
		sub cl, dh
		mov bp, cx
		// If starting byte == FF and width is 0, then set width to 1
		cmp bh, 0xFF
		jne vfr_ega_startingnotff
		cmp dh, 0
		jne vfr_ega_startingnotff
		inc dh
	vfr_ega_startingnotff:
		// If ending byte == FF, then increase width by 1
		cmp bl, 0xFF
		jne vfr_ega_endingnotff
		inc dh
	vfr_ega_endingnotff:
		// Check if starting byte and ending byte are the same, if so then AND the masks together in the starting mask and fill the ending mask
		cmp al, ah
		jne vfr_ega_bytecalc
		and bh, bl
		mov bl, 0xFF
		cmp dh, 0
		je vfr_ega_bytecalc
		dec dh
	vfr_ega_bytecalc:
		// Calculate number of bytes that is just the filled color (no mask needed) and store in DH
		cmp dh, 0
		je vfr_ega_mainloop
		cmp bh, 0xFF
		je vfr_ega_mainloop
		dec dh
		// Set CX to 0, so the high byte is always 0
		mov cx, 0
	vfr_ega_mainloop:
		// If starting offset != FF, then we build the first byte using the mask
		cmp bh, 0xFF
		je vfr_ega_skipstartingbyte
		// Read byte from screen and use mask to join new color data together and write the new byte
		mov al, byte ptr [ES:DI]
		mov ah, bh
		xor ah, 0xFF
		and al, ah
		mov ah, bh
		and ah, dl
		or al, ah
		stosb
	vfr_ega_skipstartingbyte:
		// Skip if we byte fill count is 0
		cmp dh, 0
		je vfr_ega_endingbyte
		// Set number of times to repeat
		mov cl, dh
		// Load fill color
		mov al, dl
		// Copy color to the screen
		rep stosb
	vfr_ega_endingbyte:
		// If ending offset != FF, then we build the last byte using the mask
		cmp bl, 0xFF
		je vfr_ega_skipendingbyte
		// Read byte from screen and use mask to join new color data together and write the new byte
		mov al, byte ptr [ES:DI]
		mov ah, bl
		xor ah, 0xFF
		and al, ah
		mov ah, bl
		and ah, dl
		or al, ah
		stosb
	vfr_ega_skipendingbyte:
		// Move to next screen row
		add DI, BP
		dec si
		jne vfr_ega_mainloop
		// Pop height and starting screen location
		pop si
		pop di
		// Save number of bytes to fill with color
		mov cl, dh
		// Pop the next read plane, If it is FFFF, then we are done
		pop ax
		cmp ax, 0xFFFF
		je vfr_done
		mov dx, 0x03CF
		out dx, al
		// Pop the next write plane and color bit from the stack.
		pop ax
		mov dx, 0x03C5
		out dx, al
		mov dl, ah
		// Restore number of bytes to fill with color
		mov dh, cl
		// Push starting screen location and height back on the stack
		push di
		push si
		jmp vfr_ega_mainloop
	vfr_done:
		pop BP
	}
}

void EGA_videoDrawFullScreen(uint16_t segment){
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
		// Set EGA write mode
		mov dx, 0x03C4
		mov al, 2
		out dx, al
		// Prepare port data for selecting plane (port 0x03C5. output 1, 2, 4, 8 to select plane)
		inc dx
		mov al, 1
	vdfs_ega_loop:
		// Select EGA plane
		out dx, al
		// Shift left to increase to next plane in next loop
		shl al, 1
		// Set CX to number of times to repeat
		mov cx, 4000
		// Copy from memory to screen plane
		rep movsw
		// Reset destination index di to 0 to start copying to beginning of screen plane
		xor di, di
		// Loop until done with all 4 planes
		cmp al, 16
		jne vdfs_ega_loop
	}
}

void EGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h){
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
		// Set ES segment to EGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// Set AH to h, the number of times to repeat
		mov ax, h
		mov ah, al
		// add x to screen position. SHR to divide by 8 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		shr dx, 1
		add di, dx
		// Save number of bytes to copy per row. SHR to divide by 8
		mov cx, w
		shr cx, 1
		shr cx, 1
		shr cx, 1
		// Save number of bytes to move to next row in bx
		mov bx, 40
		sub bx, cx
		// Save starting screen position (di) in bp
		push bp
		mov bp, di
		// Set EGA to write mode
		mov dx, 0x03C4
		mov al, 2
		out dx, al
		// Prepare port data for selecting plane (port 0x03C5. output 1, 2, 4, 8 to select plane)
		inc dx
		// Select EGA plane 1
		dec al
		out dx, al
		// Save number of bytes to copy per row in AL
		mov al, cl
		// Save height in dh
		mov dh, ah
		// Clear CX so upper byte is always 0
		xor cx, cx
		// Loop while drawing to the screen
	vpt_ega_loop_plane1:
		// Set number of bytes to copy
		mov cl, al
		// Start copy
		rep movsb
		// Move to next row
		add di, bx
		// Loop until AH is 0
		dec ah
		jnz vpt_ega_loop_plane1
		// Save AL
		mov cl, al
		// Restore height counter
		mov ah, dh
		// Select EGA plane 2
		mov al, 2
		mov dx, 0x03C5
		out dx, al
		// Restore AL
		mov al, cl
		// Save height counter
		mov dh, ah
		// Go back to beginning of tile on screen
		mov di, bp
		// Loop while drawing to the screen
	vpt_ega_loop_plane2:
		// Set number of bytes to copy
		mov cl, al
		// Start copy
		rep movsb
		// Move to next row
		add di, bx
		// Loop until AH is 0
		dec ah
		jnz vpt_ega_loop_plane2
		// Save AL
		mov cl, al
		// Restore height counter
		mov ah, dh
		// Select EGA plane 3
		mov al, 4
		mov dx, 0x03C5
		out dx, al
		// Restore AL
		mov al, cl
		// Save height counter
		mov dh, ah
		// Go back to beginning of tile on screen
		mov di, bp
		// Loop while drawing to the screen
	vpt_ega_loop_plane3:
		// Set number of bytes to copy
		mov cl, al
		// Start copy
		rep movsb
		// Move to next row
		add di, bx
		// Loop until AH is 0
		dec ah
		jnz vpt_ega_loop_plane3
		// Save AL
		mov cl, al
		// Restore height counter
		mov ah, dh
		// Select EGA plane 4
		mov al, 8
		mov dx, 0x03C5
		out dx, al
		// Restore AL
		mov al, cl
		// Go back to beginning of tile on screen
		mov di, bp
		// Loop while drawing to the screen
	vpt_ega_loop_plane4:
		// Set number of bytes to copy
		mov cl, al
		// Start copy
		rep movsb
		// Move to next row
		add di, bx
		// Loop until AH is 0
		dec ah
		jnz vpt_ega_loop_plane4
		// Restore BP and exit function
		pop bp
	}
}

// Copy a sprite from ram to the screen, using a mask to also draw the tile underneath
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'tile' should be a pointer to the top-left of the tile in ram
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void EGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to sprite, which has the mask data within it. Set SI to offset of sprite (always 0)
		mov ds, spriteseg
		xor si, si
		// mov bx, si
		// Set ES segment to EGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		shr dx, 1
		add di, dx
		// Disable interrupts
		cli
		// Load pointer to tileseg. Store in SS:SI
		mov ax, ss:tileseg
		// Save SS segment in BP
		push bp
		mov bp, ss
		// Save SS segment for tile pointer
		mov ss, ax
		// Set EGA write port, and set to plane 1
		mov al, 2
		mov dx, 0x03C4
		out dx, al
		inc dx
		dec al
		out dx, al
		// Save DI in BX
		mov bx, di
		// Set number of times to loop
		mov ah, 24
	vps_ega_loop_plane1:
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 288]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 288]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 288]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Move to next row
		add di, 37
		dec ah
		jnz vps_ega_loop_plane1
		// Restore DI to start of tile on screen
		mov di, bx
		// Set to EGA Plane 2
		mov al, 2
		out dx, al
		// Set number of times to loop
		mov ah, 24
	vps_ega_loop_plane2:
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 216]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 216]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 216]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Move to next row
		add di, 37
		dec ah
		jnz vps_ega_loop_plane2
		// Restore DI to start of tile on screen
		mov di, bx
		// Set to EGA Plane 3
		mov al, 4
		out dx, al
		// Set number of times to loop
		mov ah, 24
	vps_ega_loop_plane3:
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 144]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Move to next row
		add di, 37
		dec ah
		jnz vps_ega_loop_plane3
		// Restore DI to start of tile on screen
		mov di, bx
		// Set to EGA Plane 4
		mov al, 8
		out dx, al
		// Set number of times to loop
		mov ah, 24
	vps_ega_loop_plane4:
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 72]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 72]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Fetch mask data into AL
		mov al, byte ptr [DS:SI + 72]
		// Fetch tile data into CH
		mov ch, byte ptr [SS:SI]
		// AND Together
		and ch, al
		// XOR AL
		xor al, 0xFF
		// AND sprite data
		and al, byte ptr [DS:SI]
		// Merge the two together with an OR
		or al, ch
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		// Move to next row
		add di, 37
		dec ah
		jnz vps_ega_loop_plane4
		// Replace SS segment back to what it was
		mov ss, bp
		pop bp
		// Enable interrupts
		sti
	}
}

// Copy a sprite from ram to the screen, using a mask to avoid overwriting nearby pixels
// 'x' is the x position on screen to draw the sprite (upper-left corner)
// 'y' is the y position on screen to draw the sprite (upper-left corner)
// 'sprite' should be a pointer to the top-left of the tile in ram
// 'color' is the color you want the sprite drawn as
// WIDTH and HEIGHT are hard set to 24x24 for this routine
void EGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to sprite DS:SI, which has the mask data within it.
		mov ax, spriteseg
		// Increase to mask data
		add ax, 18
		mov ds, ax
		// Set SI to start of the sprite's mask data (0 offset in this case)
		xor si, si
		// Set ES segment to EGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		shr dx, 1
		add di, dx
		// Set ah to color
		mov ax, color
		mov ah, al
		// Set number of times to loop
		mov cx, 24
		// Move to plane 1
		mov al, 4
		mov dx, 0x03ce
		out dx, al
		inc dx
		xor al, al
		out dx, al
		mov al, 2
		mov dx, 0x03c4
		out dx, al
		dec al
		inc dx
		out dx, al
		// Save DI in BX
		mov bx, di
	vps_ega_loop_plane1:
		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 1
		jz t0p1
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t0p1:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 1
		jz t1p1
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t1p1:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 1
		jz t2p1
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t2p1:
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		add di, 37
		dec cl
		jnz vps_ega_loop_plane1

		// Set number of times to loop
		mov cx, 24
		// Move to plane 2
		mov al, 4
		mov dx, 0x03ce
		out dx, al
		inc dx
		mov al, 1
		out dx, al
		inc al
		mov dx, 0x03c4
		out dx, al
		inc dx
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
	vps_ega_loop_plane2:
		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 2
		jz t0p2
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t0p2:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 2
		jz t1p2
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t1p2:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 2
		jz t2p2
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t2p2:
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		add di, 37
		dec cl
		jnz vps_ega_loop_plane2

		// Set number of times to loop
		mov cx, 24
		// Move to plane 2
		mov al, 4
		mov dx, 0x03ce
		out dx, al
		inc dx
		mov al, 2
		out dx, al
		mov dx, 0x03c4
		out dx, al
		mov al, 4
		inc dx
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
	vps_ega_loop_plane3:
		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 4
		jz t0p3
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t0p3:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 4
		jz t1p3
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t1p3:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 4
		jz t2p3
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t2p3:
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		add di, 37
		dec cl
		jnz vps_ega_loop_plane3

		// Set number of times to loop
		mov cx, 24
		// Move to plane 2
		mov al, 4
		mov dx, 0x03ce
		out dx, al
		inc dx
		dec al
		out dx, al
		dec al
		mov dx, 0x03c4
		out dx, al
		mov al, 8
		inc dx
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
	vps_ega_loop_plane4:
		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 8
		jz t0p4
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t0p4:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 8
		jz t1p4
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t1p4:
		// Output to screen
		stosb
		// Increment to next byte
		inc si

		// Fetch mask data into DL
		mov dl, byte ptr [DS:SI]
		// Fetch screen data into AL
		mov al, byte ptr [ES:DI]
		// AND Together
		and al, dl
		// Check color bit
		mov dh, ah
		and dh, 8
		jz t2p4
		// XOR DL to have inverse mask
		xor dl, 0xFF
		// Merge the two together with an OR
		or al, dl
	t2p4:
		// Output to screen
		stosb
		// Increment to next byte
		inc si
		add di, 37
		dec cl
		jnz vps_ega_loop_plane4
	}
}

// Copy a font character from ram to the screen
// 'x' is the x position on screen to draw the font (upper-left corner)
// 'y' is the y position on screen to draw the font (upper-left corner)
// 'fontseg' should be a pointer to the top-left of the font in ram
// 'color' is the color you want the font drawn as
// WIDTH and HEIGHT are hard set to 8x8 for this routine
void EGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color){

	_asm{
		// Load VIDEO_SCREENOFFSET[y] to get offset of screen row as store in DI
		lea bx, ss:VIDEO_SCREENOFFSET
		mov di, y
		shl di, 1
		mov di, [ss:bx + di]
		// Load pointer to font DS:SI, which has the mask data within it.
		mov ds, fontseg
		// Set SI to start of the font's mask data (0 offset in this case)
		xor si, si
		// Set ES segment to EGA screen pointer
		mov cx, 0xA000
		mov es, cx
		// add x to screen position. SHR to divide by 4 for pixels per byte
		mov dx, x
		shr dx, 1
		shr dx, 1
		shr dx, 1
		add di, dx
		// Set number of times to loop
		mov cx, 8
		// Move to plane 1
		mov ax, 0x0102
		mov dx, 0x03c4
		out dx, ax
		// Set ah to color
		mov ax, color
		mov ah, al
		// Save DI in BX
		mov bx, di
	vps_ega_loop_plane1:
		// Fetch mask data into AL
		lodsb
		// Check color bit
		mov dh, ah
		and dh, 1
		jnz p1
		// Clear AL if color bit is 0
		xor al, al
	p1:
		// Output to screen
		stosb
		// Increment to next byte
		add di, 39
		dec cl
		jnz vps_ega_loop_plane1

		// Set number of times to loop
		mov cx, 8
		// Move to plane 2
		mov al, 2
		mov dx, 0x03c5
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
		// Move to next color bit
	vps_ega_loop_plane2:
		// Fetch mask data into AL
		lodsb
		// Check color bit
		mov dh, ah
		and dh, 2
		jnz p2
		// Clear AL if color bit is 0
		xor al, al
	p2:
		// Output to screen
		stosb
		// Increment to next byte
		add di, 39
		dec cl
		jnz vps_ega_loop_plane2

		// Set number of times to loop
		mov cx, 8
		// Move to plane 3
		mov al, 4
		mov dx, 0x03c5
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
		// Move to next color bit
	vps_ega_loop_plane3:
		// Fetch mask data into AL
		lodsb
		// Check color bit
		mov dh, ah
		and dh, 4
		jnz p3
		// Clear AL if color bit is 0
		xor al, al
	p3:
		// Output to screen
		stosb
		// Increment to next byte
		add di, 39
		dec cl
		jnz vps_ega_loop_plane3

		// Set number of times to loop
		mov cx, 8
		// Move to plane 4
		mov al, 8
		mov dx, 0x03c5
		out dx, al
		// Restore DI from BX
		mov di, bx
		// Move SI to start of mask data (0)
		xor si, si
		// Move to next color bit
	vps_ega_loop_plane4:
		// Fetch mask data into AL
		lodsb
		// Check color bit
		mov dh, ah
		and dh, 8
		jnz p4
		// Clear AL if color bit is 0
		xor al, al
	p4:
		// Output to screen
		stosb
		// Increment to next byte
		add di, 39
		dec cl
		jnz vps_ega_loop_plane4
	}
}

// Add a character to the right cinema tile
// letter_segment = a segment pointer to an 8x8 character to add to a tile/memory location
// dest_segment = a segment pointer to the top-left of a destination tile
void EGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment){
	_asm{
		// Set source (DS:SI) to letter
		mov ax, ss:letter_segment
		mov ds, ax
		// Set offset to 0
		xor si, si
		// Set destination (ES:SI) to dest
		mov ax, ss:dest_segment
		mov es, ax
		// Set destination index to 35 (24 * 11 + 16) / 8
		mov di, 35
		// Set CX to 4, the number of planes to repeat
		mov cx, 4
		// Loop while drawing
	vcac_ega_loop:
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next destination row
		inc di
		inc di
		// Copy 1 byte
		movsb
		// Increase to next plane and clear source index to start writing character again
		add di, 50
		xor si, si
		// Loop until done with all planes
		loop vcac_ega_loop
	}
}

// Rotate the cinema tiles
void EGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3){
	_asm{
		// Load in the first tile
		mov ds, ss:tile1
		// Set destination index to 33 (24 * 11) / 8
		mov si, 33
		// Copy source into destination as well
		mov di, si
		mov bx, ds
		mov es, bx
		// Rotate each byte by 4 pixels (1 nibble)
		mov al, ds:si
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		mov cx, 24
	vcr_ega_rotate1:
		inc si
		mov ah, ds:si
		mov bh, ah
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		or al, ah
		mov es:di, al
		mov es:di + 72, al
		mov es:di + 144, al
		mov es:di + 216, al
		inc di
		mov al, bh
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		loop vcr_ega_rotate1
		// Copy left edge (nibble) of tile2 to tile1
		mov ds, ss:tile2
		mov si, 33
		mov di, 35
		// Times to repeat copy
		mov cx, 8
	vcr_ega_tile2_to_tile1:
		mov al, ds:si
		and al, 0xF0
		shr al, 1
		shr al, 1
		shr al, 1
		shr al, 1
		mov ah, es:di
		and ah, 0xF0
		or al, ah
		mov es:di, al
		mov es:di + 72, al
		mov es:di + 144, al
		mov es:di + 216, al
		add si, 3
		add di, 3
		loop vcr_ega_tile2_to_tile1
		// Rotate tile2
		mov si, 33
		mov di, si
		mov bx, ds
		mov es, bx
		// Rotate each byte by 4 pixels (1 nibble)
		mov al, ds:si
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		mov cx, 24
	vcr_ega_rotate2:
		inc si
		mov ah, ds:si
		mov bh, ah
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		or al, ah
		mov es:di, al
		mov es:di + 72, al
		mov es:di + 144, al
		mov es:di + 216, al
		inc di
		mov al, bh
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		loop vcr_ega_rotate2
		// Copy left edge of tile3 to tile2
		mov ds, ss:tile3
		mov si, 33
		mov di, 35
		// Times to repeat copy
		mov cx, 8
	vcr_ega_tile3_to_tile2:
		mov al, ds:si
		and al, 0xF0
		shr al, 1
		shr al, 1
		shr al, 1
		shr al, 1
		mov ah, es:di
		and ah, 0xF0
		or al, ah
		mov es:di, al
		mov es:di + 72, al
		mov es:di + 144, al
		mov es:di + 216, al
		add si, 3
		add di, 3
		loop vcr_ega_tile3_to_tile2
		// Rotate tile3
		mov si, 33
		mov di, si
		mov bx, ds
		mov es, bx
		// Rotate each byte by 4 pixels (1 nibble)
		mov al, ds:si
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		mov cx, 24
	vcr_ega_rotate3:
		inc si
		mov ah, ds:si
		mov bh, ah
		shr ah, 1
		shr ah, 1
		shr ah, 1
		shr ah, 1
		or al, ah
		mov es:di, al
		mov es:di + 72, al
		mov es:di + 144, al
		mov es:di + 216, al
		inc di
		mov al, bh
		shl al, 1
		shl al, 1
		shl al, 1
		shl al, 1
		loop vcr_ega_rotate3
	}
}

// Return the color of the pixel at X, Y
uint8_t EGA_videoGetPixel(uint16_t x, uint16_t y){
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
		// Increase si by x / 8. Store how many pixels to shift in ES
		mov ax, x
		mov dx, ax
		xor dx, 7
		mov es, dx
		shr ax, 1
		shr ax, 1
		shr ax, 1
		add si, ax
		// Set DI to 0 (color is stored here)
		xor di, di
		// Prepare port data for selecting plane (port 0x03C5. output 1, 2, 4, 8 to select plane)
		mov bx, 4
		mov dx, 0x03CE
	planeloop:
		// Set EGA plane
		mov al, 4
		out dx, al
		inc dx
		dec bl
		mov al, bl
		out dx, al
		dec dx
		// Get pixel, shift it to the right until it is in the right-most position
		mov al, byte ptr [DS:SI]
		mov cx, es
		and cx, 7
		jz pixeldone
	pixelloop:
		shr al, 1
		loop pixelloop
	pixeldone:
		// Set 'color'
		and ax, 1
		shl di, 1
		or di, ax
		cmp bl, 0
		jne planeloop
		mov ax, di
		mov byte ptr [color], al
	}
	return color;
}

// This routine translates VGA data to EGA data.
// *source: the VGA data being converted
// *dest: The location to store the conversion results
// w: the number of pixels in width of the data, must be divisible by 8
// h: the height of the tile
// *palette: the color translation table
// multi_byte: 0 = 1 byte per pixel, 1 = 2 bytes per pixel
void EGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte){
	uint8_t *data3, *ptr, *blueptr, *greenptr, *redptr, *intensityptr, *destrow;
	uint8_t plane, h2, color, bluebyte, greenbyte, redbyte, intensitybyte, fullscreen;
	uint8_t blue0[16], green0[16], red0[16], intensity0[16];
	uint8_t blue1[16], green1[16], red1[16], intensity1[16];
	uint8_t blue2[16], green2[16], red2[16], intensity2[16];
	uint8_t blue3[16], green3[16], red3[16], intensity3[16];
	uint8_t blue4[16], green4[16], red4[16], intensity4[16];
	uint8_t blue5[16], green5[16], red5[16], intensity5[16];
	uint8_t blue6[16], green6[16], red6[16], intensity6[16];
	uint8_t blue7[16], green7[16], red7[16], intensity7[16];
	uint8_t data2[160];
	uint16_t i, size;
	unsigned long j;
	
	// Copy the color data from the palette into a very simple bit table for fast translation
	for (i = 0; i < 16; i++){
		color = palette[i] & 1;
		blue0[i] = color;
		color <<= 1;
		blue1[i] = color;
		color <<= 1;
		blue2[i] = color;
		color <<= 1;
		blue3[i] = color;
		color <<= 1;
		blue4[i] = color;
		color <<= 1;
		blue5[i] = color;
		color <<= 1;
		blue6[i] = color;
		color <<= 1;
		blue7[i] = color;
		color = (palette[i] >> 1) & 1;
		green0[i] = color;
		color <<= 1;
		green1[i] = color;
		color <<= 1;
		green2[i] = color;
		color <<= 1;
		green3[i] = color;
		color <<= 1;
		green4[i] = color;
		color <<= 1;
		green5[i] = color;
		color <<= 1;
		green6[i] = color;
		color <<= 1;
		green7[i] = color;
		color = (palette[i] >> 2) & 1;
		red0[i] = color;
		color <<= 1;
		red1[i] = color;
		color <<= 1;
		red2[i] = color;
		color <<= 1;
		red3[i] = color;
		color <<= 1;
		red4[i] = color;
		color <<= 1;
		red5[i] = color;
		color <<= 1;
		red6[i] = color;
		color <<= 1;
		red7[i] = color;
		color = (palette[i] >> 3) & 1;
		intensity0[i] = color;
		color <<= 1;
		intensity1[i] = color;
		color <<= 1;
		intensity2[i] = color;
		color <<= 1;
		intensity3[i] = color;
		color <<= 1;
		intensity4[i] = color;
		color <<= 1;
		intensity5[i] = color;
		color <<= 1;
		intensity6[i] = color;
		color <<= 1;
		intensity7[i] = color;
	}
	fullscreen = 1;
	size = (w * h) >> VIDEO_BITSHIFT;
	if (fullscreen){
		ptr = source;
		blueptr = dest;
		greenptr = blueptr + size;
		redptr = greenptr + size;
		intensityptr = redptr + size;
		h2 = 1;
	} else{
		ptr = source;
		destrow = dest;
		h2 = h;
	}
	do {
		if (!fullscreen){
			blueptr = data2;
			greenptr = blueptr + size;
			redptr = greenptr + size;
			intensityptr = redptr + size;
		}
		for (j = 0; j < size; j++){
			color = ptr[0] & 15;
			bluebyte = blue7[color];
			greenbyte = green7[color];
			redbyte = red7[color];
			intensitybyte = intensity7[color];
			if (multi_byte){
				color = ptr[0] >> 4;
			} else{
				ptr++;
				color = ptr[0];
			}
			bluebyte |= blue6[color];
			greenbyte |= green6[color];
			redbyte |= red6[color];
			intensitybyte |= intensity6[color];
			ptr++;
			color = ptr[0] & 15;
			bluebyte |= blue5[color];
			greenbyte |= green5[color];
			redbyte |= red5[color];
			intensitybyte |= intensity5[color];
			if (multi_byte){
				color = ptr[0] >> 4;
			} else{
				ptr++;
				color = ptr[0];
			}
			bluebyte |= blue4[color];
			greenbyte |= green4[color];
			redbyte |= red4[color];
			intensitybyte |= intensity4[color];
			ptr++;
			color = ptr[0] & 15;
			bluebyte |= blue3[color];
			greenbyte |= green3[color];
			redbyte |= red3[color];
			intensitybyte |= intensity3[color];
			if (multi_byte){
				color = ptr[0] >> 4;
			} else{
				ptr++;
				color = ptr[0];
			}
			bluebyte |= blue2[color];
			greenbyte |= green2[color];
			redbyte |= red2[color];
			intensitybyte |= intensity2[color];
			ptr++;
			color = ptr[0] & 15;
			bluebyte |= blue1[color];
			greenbyte |= green1[color];
			redbyte |= red1[color];
			intensitybyte |= intensity1[color];
			if (multi_byte){
				color = ptr[0] >> 4;
			} else{
				ptr++;
				color = ptr[0];
			}
			bluebyte |= blue0[color];
			greenbyte |= green0[color];
			redbyte |= red0[color];
			intensitybyte |= intensity0[color];
			ptr++;
			blueptr[0] = bluebyte;
			greenptr[0] = greenbyte;
			redptr[0] = redbyte;
			intensityptr[0] = intensitybyte;
			blueptr++;
			greenptr++;
			redptr++;
			intensityptr++;
		}
		h2--;
		if (!fullscreen){
			memcpy(destrow, data2, size << 2);
			destrow += (w >> 1);
		}
	} while (h2 > 0);
}

uint8_t EGA_videoGetUnitColorForMinimap(uint8_t unit){
	uint8_t color = CURSOR.color;
	switch (UNIT_TYPE[unit]){
		case TYPE_MAGNET:
		case TYPE_TIMEBOMB:
		case TYPE_EXPLOSION_SMALL:
		case TYPE_EVILBOT:{
			if (color == 15){
				color = 12;
			} else if (color > 0){
				color = 4;
			}
			break;
		}
		case TYPE_HOVERBOT_HORIZ:
		case TYPE_HOVERBOT_VERT:
		case TYPE_MAGNETIZED_ROBOT:
		case TYPE_HOVERBOT_WATER:{
			if (color == 7 || color == 5){
				color = 3;
			} else if (color == 8 || color == 10){
				color = 1;
			} else if (color == 15){
				color = 11;
			}
			break;
		}
		case TYPE_HOVERBOT_ATTACK:
		case TYPE_ROLLERBOT_HORIZ:
		case TYPE_ROLLERBOT_VERT:{
			if (color == 7 || color == 5){
				color = 12;
			} else if (color == 8 || color == 10){
				color = 6;
			} else if (color == 15){
				color = 14;
			}
			break;
		}
	}
	return color;
}

void EGA_minimapSet(uint8_t x, uint8_t y, uint8_t color){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	color &= 0x0F;
	if (x & 3 == 3){
		p[x >> 2] = ((p[x >> 2] & 0xFC) | (color & 0x01) | ((color & 0x01) << 1));
		p[(x >> 2) + 32] = ((p[(x >> 2) + 32] & 0xFC) | ((color >> 1) & 0x01) | (((color >> 1) & 0x01) << 1));
		p[(x >> 2) + 64] = ((p[(x >> 2) + 64] & 0xFC) | ((color >> 2) & 0x01) | (((color >> 2) & 0x01) << 1));
		p[(x >> 2) + 96] = ((p[(x >> 2) + 96] & 0xFC) | ((color >> 3) & 0x01) | (((color >> 3) & 0x01) << 1));
	} else if (x & 2){
		p[x >> 2] = ((p[x >> 2] & 0xF3) | ((color & 0x01) << 2) | ((color & 0x01) << 3));
		p[(x >> 2) + 32] = ((p[(x >> 2) + 32] & 0xF3) | (((color >> 1) & 0x01) << 2) | (((color >> 1) & 0x01) << 3));
		p[(x >> 2) + 64] = ((p[(x >> 2) + 64] & 0xF3) | (((color >> 2) & 0x01) << 2) | (((color >> 2) & 0x01) << 3));
		p[(x >> 2) + 96] = ((p[(x >> 2) + 96] & 0xF3) | (((color >> 3) & 0x01) << 2) | (((color >> 3) & 0x01) << 3));
	} else if (x & 1){
		p[x >> 2] = ((p[x >> 2] & 0xCF) | ((color & 0x01) << 4) | ((color & 0x01) << 5));
		p[(x >> 2) + 32] = ((p[(x >> 2) + 32] & 0xCF) | (((color >> 1) & 0x01) << 4) | (((color >> 1) & 0x01) << 5));
		p[(x >> 2) + 64] = ((p[(x >> 2) + 64] & 0xCF) | (((color >> 2) & 0x01) << 4) | (((color >> 2) & 0x01) << 5));
		p[(x >> 2) + 96] = ((p[(x >> 2) + 96] & 0xCF) | (((color >> 3) & 0x01) << 4) | (((color >> 3) & 0x01) << 5));
	} else{
		p[x >> 2] = ((p[x >> 2] & 0x3F) | ((color & 0x01) << 6) | ((color & 0x01) << 7));
		p[(x >> 2) + 32] = ((p[(x >> 2) + 32] & 0x3F) | (((color >> 1) & 0x01) << 6) | (((color >> 1) & 0x01) << 7));
		p[(x >> 2) + 64] = ((p[(x >> 2) + 64] & 0x3F) | (((color >> 2) & 0x01) << 6) | (((color >> 2) & 0x01) << 7));
		p[(x >> 2) + 96] = ((p[(x >> 2) + 96] & 0x3F) | (((color >> 3) & 0x01) << 6) | (((color >> 3) & 0x01) << 7));
	}
	if (MINIMAP_OPEN){
		videoFillRect((x << 1), (y << 1) + 20, 2, 2, color);
	}
}

void EGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y){
	uint8_t x, plane;
	for (x = 0; x < 128; x += 2){
		outp(0x03C4, 2);
		for (plane = 0; plane < 4; plane++){
			outp(0x03C5, 1 << plane);
			for (x = 0; x < 128; x += 4){
				*vidptr2 = ((MAP_TILE_COLORS[MAP[y][x]] & (0x01 << plane)) >> plane << 7) | ((MAP_TILE_COLORS[MAP[y][x]] & (0x01 << plane)) >> plane << 6) | ((MAP_TILE_COLORS[MAP[y][x + 1]] & (0x01 << plane)) >> plane << 5) | ((MAP_TILE_COLORS[MAP[y][x + 1]] & (0x01 << plane)) >> plane << 4) | ((MAP_TILE_COLORS[MAP[y][x + 2]] & (0x01 << plane)) >> plane << 3) | ((MAP_TILE_COLORS[MAP[y][x + 2]] & (0x01 << plane)) >> plane << 2) | ((MAP_TILE_COLORS[MAP[y][x + 3]] & (0x01 << plane)) >> plane << 1) | ((MAP_TILE_COLORS[MAP[y][x + 3]] & (0x01 << plane)) >> plane );
				vidptr2++;
			}
		}
	}
}

void EGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y){
	uint8_t *p = memorySegToPtr(MINIMAP[y]);
	outp(0x03C4, 2);
	outp(0x03C5, 1);
	memcpy(vidptr2, p, 32);
	outp(0x03C5, 2);
	memcpy(vidptr2, p + 32, 32);
	outp(0x03C5, 4);
	memcpy(vidptr2, p + 64, 32);
	outp(0x03C5, 8);
	memcpy(vidptr2, p + 96, 32);
}
