#include <dos.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <signal.h>
#include "game.h"
#include "globals.h"
#include "video.h"
#include "audio.h"
#include "files.h"
#include "keyboard.h"
#include "backgroundtasks.h"
#include "interrupt.h"
#include "string.h"
#include "memory.h"

// Currently selected weapon. If previous weapon != selected weapon, then we redraw weapon display
uint8_t SELECTED_WEAPON = 0;
uint8_t PREVIOUS_WEAPON = 0;
uint8_t PREVIOUS_AMMO = 0;
// Currently selected item. If previous item != selected item, then we redraw item display
uint8_t SELECTED_ITEM = 0;
uint8_t PREVIOUS_ITEM = 0;
uint8_t PREVIOUS_INVENTORY = 0;
// Map variables
uint8_t MAP_PRECALC[MAP_PRECALC_COUNT];
uint8_t const MAP_PRECALC_ROWS[7] = { 0, 11, 22, 33, 44, 55, 66 };

void mapSet(uint8_t x, uint8_t y, uint8_t tile){
	MAP[y][x] = tile;
	if (tile != TILE_RAFT || UNIT_X[UNIT_PLAYER] != x || UNIT_Y[UNIT_PLAYER] != y){
		minimapSet(x, y, MAP_TILE_COLORS[tile]);
	}
}

uint8_t checkForUnit(uint8_t x, uint8_t y){
	uint8_t i;
	
	// Loop through units to see if any are at the x/y position on the map
	for (i = 0; i < 28; i++){
		if (UNIT_TYPE[i] != TYPE_NONE && UNIT_X[i] == x && UNIT_Y[i] == y){
			// Found a unit at this position, return it
			return i;
		}
	}
	// No unit found, return 255
	return UNIT_NOT_FOUND;
}

uint8_t checkForHiddenUnit(uint8_t x, uint8_t y){
	uint8_t i;
	
	// Loop through hidden units to see if any are at the x/y position on the map
	// UNIT_C/UNIT_D contain the width and height to search for the hidden unit
	for (i = 48; i < 64; i++){
		if (UNIT_TYPE[i] != TYPE_NONE && x >= UNIT_X[i] && x <= UNIT_X[i] + UNIT_C[i] && y >= UNIT_Y[i] && y <= UNIT_Y[i] + UNIT_D[i]){
			// Found a unit at this position, return it
			return i;
		}
	}
	// No unit found, return 255
	return UNIT_NOT_FOUND;
}

void unitAnimate(uint8_t unit){
	if (UNIT_TIMER_ANIM[unit] == 0){
		// Reset animation timer
		UNIT_TIMER_ANIM[unit] = 3;
		// Animate
		UNIT_ANIM_FRAME[unit]++;
		if (UNIT_ANIM_FRAME[unit] > 3){
			UNIT_ANIM_FRAME[unit] = 0;
		}
	} else{
		UNIT_TIMER_ANIM[unit]--;
	}
}

int tileHasAttribute(uint8_t x, uint8_t y, uint8_t attrib){
	return (TILE_ATTRIB[MAP[y][x]] & attrib);
}

void unitPos(uint8_t unit, uint8_t x, uint8_t y){
	uint8_t unit2, prevx, prevy, color;
	
	// Update unit's previous x/y position if they have moved
	if (UNIT_X[unit] != x || UNIT_Y[unit] != y){
		prevx = UNIT_X[unit];
		prevy = UNIT_Y[unit];
		UNIT_X[unit] = x;
		UNIT_Y[unit] = y;
		if (prevx < MAP_WIDTH && prevy < MAP_HEIGHT){
			if ((unit2 = checkForUnit(prevx, prevy)) != UNIT_NOT_FOUND){
				// Found a unit in the position this unit was previously at, call to update that unit at this position
				unitPos(unit2, prevx, prevy);
			} else{
				minimapSet(prevx, prevy, MAP_TILE_COLORS[MAP[prevy][prevx]]);
			}
		}
	}
	if (MINIMAP_OPEN){
		// If unit no longer exists, replace the unit variable with a check for another unit at this location
		if (UNIT_TYPE[unit] == TYPE_NONE){
			unit = checkForUnit(x, y);
		}
		if (MINIMAP_ROBOTS_ENABLED || unit == UNIT_PLAYER || unit > 27 || (MINIMAP_PROXIMITY && UNIT_X[unit] >= MAP_WINDOW_X && UNIT_X[unit] < MAP_WINDOW_X + 12 && UNIT_Y[unit] >= MAP_WINDOW_Y && UNIT_Y[unit] < MAP_WINDOW_Y + 8)){
			// If valid unit found, draw them on the map
			if (unit != UNIT_NOT_FOUND){
				color = videoGetUnitColorForMinimap(unit);
				minimapSet(x, y, color);
				return;
			}
		}
		// No unit found here or robots not enabled on map, draw the map tile instead
		minimapSet(x, y, MAP_TILE_COLORS[MAP[y][x]]);
	}
}

int requestWalkXY(uint8_t unit, uint8_t move_attrib, uint8_t x, uint8_t y){
	// Check if unit can walk on requested tile
	if (!tileHasAttribute(x, y, move_attrib)){
		return 0;
	}
	// Check for a unit on the requested tile
	if (checkForUnit(x, y) != UNIT_NOT_FOUND){
		return 0;
	}
	// Update the unit's position and return success flag
	unitPos(unit, x, y);
	return 1;
}

int requestWalkLeft(uint8_t unit, uint8_t move_attrib){
	// Check if unit is already at the edge of the map
	if (UNIT_X[unit] == 5){
		return 0;
	}
	// Check if unit can walk on the requested tile
	return requestWalkXY(unit, move_attrib, UNIT_X[unit] - 1, UNIT_Y[unit]);
}

int requestWalkRight(uint8_t unit, uint8_t move_attrib){
	
	// Check if unit is already at the edge of the map
	if (UNIT_X[unit] == 122){
		return 0;
	}
	// Check if unit can walk on the requested tile
	return requestWalkXY(unit, move_attrib, UNIT_X[unit] + 1, UNIT_Y[unit]);
}

int requestWalkUp(uint8_t unit, uint8_t move_attrib){
	
	// Check if unit is already at the edge of the map
	if (UNIT_Y[unit] == 3){
		return 0;
	}
	// Check if unit can walk on the requested tile
	return requestWalkXY(unit, move_attrib, UNIT_X[unit], UNIT_Y[unit] - 1);
}

int requestWalkDown(uint8_t unit, uint8_t move_attrib){
	
	// Check if unit is already at the edge of the map
	if (UNIT_Y[unit] == 60){
		return 0;
	}
	// Check if unit can walk on the requested tile
	return requestWalkXY(unit, move_attrib, UNIT_X[unit], UNIT_Y[unit] + 1);
}

inline void gameMapCalculateAndRedraw(){
	// Set the map position based on the player's unit (0)
	MAP_WINDOW_X = UNIT_X[UNIT_PLAYER] - 5;
	MAP_WINDOW_Y = UNIT_Y[UNIT_PLAYER] - 3;
	REDRAW_WINDOW = 1;
}

void gameDrawHUD(){
	videoPrintTileMultibyte(0, 168, HUD, 264, 8);
	videoPrintTileMultibyte(264, 0, HUD + (228 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 200);
	videoPrintTileMultibyte(272, 0, HUD + (132 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 48, 8);
	videoPrintTileMultibyte(272, 40, HUD + (156 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 48, 8);
	videoPrintTileMultibyte(272, 88, HUD + (180 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 48, 8);
	videoPrintTileMultibyte(272, 120, HUD + (204 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 48, 8);
}

void gameDrawGameOver(){
	uint8_t x, y;
	uint8_t str[30];

	// Copy the game over screen into video memory
	// Draw top and bottom of game over screen
	videoPrintTileMultibyte(0, 0, GAMEOVER, 320, 20);
	videoPrintTileMultibyte(0, 140, GAMEOVER + (400 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 320, 60);
	// Draw SCENARIO:
	videoPrintTileMultibyte(88, 56, GAMEOVER + (1600 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 88, 8);
	videoPrintTileMultibyte(168, 56, GAMEOVER + (1644 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 8);
	// Draw ELAPSED TIME:
	videoPrintTileMultibyte(56, 72, GAMEOVER + (1648 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 120, 8);
	videoPrintTileMultibyte(168, 72, GAMEOVER + (1644 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 8);
	// Draw ROBOTS REMAINING:
	videoPrintTileMultibyte(16, 88, GAMEOVER + (1752 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 64, 8);
	videoPrintTileMultibyte(80, 88, GAMEOVER + (1708 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 88, 8);
	videoPrintTileMultibyte(168, 88, GAMEOVER + (1644 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 8);
	// Draw SECRETS REMAINING:
	videoPrintTileMultibyte(8, 104, GAMEOVER + (1784 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 72, 8);
	videoPrintTileMultibyte(80, 104, GAMEOVER + (1708 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 88, 8);
	videoPrintTileMultibyte(168, 104, GAMEOVER + (1644 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 8);
	// Draw DIFFICULTY:
	videoPrintTileMultibyte(74, 120, GAMEOVER + (1820 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 104, 8);
	videoPrintTileMultibyte(168, 120, GAMEOVER + (1644 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 8, 8);
	// Draw verticle bars on each side of the screen.
	if (VIDEO_COMPOSITE){// == CGACOMP){
		videoFillRect(2, 20, 4, 120, 5);
		videoFillRect(314, 20, 4, 120, 5);
	} else{
		for (x = 1; x < 10; x++){
			str[0] = videoGetPixel(x, 150);
			videoFillRect(x, 20, 1, 120, str[0]);
			videoFillRect(318 - x, 20, 1, 120, str[0]);
		}
	}
	// Draw YOU WIN!/YOU LOSE!
	videoFillRect(126, 24, 72, 8, 0);
	if (UNIT_TYPE[UNIT_PLAYER] == TYPE_PLAYER_WINS){
		videoPrintString(126, 32, 1, "YOU WIN! ", VIDEO_FONT_COLOR_GAMEOVER);
		ssy_music_play(MUSIC_WIN);
	} else{
		videoPrintString(126, 32, 1, "YOU LOSE!", VIDEO_FONT_COLOR_GAMEOVER);
		ssy_music_play(MUSIC_LOSE);
	}
	// Draw the scenario text
	videoPrintString(176, 56, 1, MAP_NAMES[LEVEL_SELECTED], VIDEO_FONT_COLOR_GAMEOVER);
	// Draw elapsed time
	strcpynum(str, CLOCK.HOURS, 2, '0');
	strncat(str, ":", 1);
	strcatnum(str, CLOCK.MINUTES, 2, '0');
	strncat(str, ":", 1);
	strcatnum(str, CLOCK.SECONDS, 2, '0');
	videoPrintString(176, 72, 1, str, VIDEO_FONT_COLOR_GAMEOVER);
	// Draw robots remaining
	y = 0;
	for (x = 1; x < 28; x++){
		if (UNIT_TYPE[x] != TYPE_NONE && UNIT_TYPE[x] != TYPE_DEAD_ROBOT){
			y++;
		}
	}
	strcpynum(str, y, 3, '0');
	videoPrintString(176, 88, 1, str, VIDEO_FONT_COLOR_GAMEOVER);
	// Draw secrets remaining
	y = 0;
	for (x = 48; x < 63; x++){
		if (UNIT_TYPE[x] != TYPE_NONE){
			y++;
		}
	}
	strcpynum(str, y, 3, '0');
	videoPrintString(176, 104, 1, str, VIDEO_FONT_COLOR_GAMEOVER);
	// Draw difficulty
	switch (DIFFICULTY_LEVEL){
		case 0:{
			videoPrintString(176, 120, 1, "EASY", VIDEO_FONT_COLOR_GAMEOVER);
			break;
		}
		case 1:{
			videoPrintString(176, 120, 1, "NORMAL", VIDEO_FONT_COLOR_GAMEOVER);
			break;
		}
		case 2:{
			videoPrintString(176, 120, 1, "HARD", VIDEO_FONT_COLOR_GAMEOVER);
			break;
		}
	}		
}

void playerAnimate(uint8_t animation_offset, uint8_t reset){
	// Animate Player
	if (reset || UNIT_ANIM_BASE[UNIT_PLAYER] != (SELECTED_WEAPON << 4) + animation_offset){
		// Reset player animation. Make sure it matches selected weapon.
		UNIT_ANIM_BASE[UNIT_PLAYER] = (SELECTED_WEAPON << 4) + animation_offset;
		// Reset animation to starting frame, unless called with reset = 2
		if (reset != 2){
			UNIT_ANIM_FRAME[UNIT_PLAYER] = 0;
		}
	} else{
		// Increase animation frame
		UNIT_ANIM_FRAME[UNIT_PLAYER]++;
		if (UNIT_ANIM_FRAME[UNIT_PLAYER] > 3){
			UNIT_ANIM_FRAME[UNIT_PLAYER] = 0;
		}
	}
	gameMapCalculateAndRedraw();
}

void gameDisplayWeapon(int forceDraw){
	unsigned int i;
	char ammostr[4];
	
	// Check to see if our weapon has run out of ammo. If so, set SELECTED_WEAPON to the next weapon with ammo, or 0 if none found
	if (SELECTED_WEAPON){
		for (i = SELECTED_WEAPON - 1; i < WEAPON_COUNT; i++){
			if (WEAPON_AMMO[i] != 0){
				SELECTED_WEAPON = i + 1;
				break;
			}
		}
		if (SELECTED_WEAPON > WEAPON_COUNT || WEAPON_AMMO[SELECTED_WEAPON - 1] == 0){
			SELECTED_WEAPON = 0;
		}
	}
	// If no weapon is selected, check if there is ammo for another weapon and switch to that
	if (!SELECTED_WEAPON){
		for (i = 0; i < WEAPON_COUNT; i++){
			if (WEAPON_AMMO[i] != 0){
				SELECTED_WEAPON = i + 1;
				break;
			}
		}
	}
	// If the current weapon does not match the previous weapon, then we need to draw the weapon
	if (forceDraw || SELECTED_WEAPON != PREVIOUS_WEAPON){
		if (!SELECTED_WEAPON){
			videoFillRect(272, 12, 48, 21, 0);
		} else{
			videoPrintTile(272, 12, ITEMS[SELECTED_WEAPON - 1], VIDEO_ITEM_W, VIDEO_ITEM_H);// - 2);
		}
		// Change player base animation
		while (UNIT_ANIM_BASE[UNIT_PLAYER] > 15){
			UNIT_ANIM_BASE[UNIT_PLAYER] -= 16;
		}
		UNIT_ANIM_BASE[UNIT_PLAYER] += (SELECTED_WEAPON << 4);
		// Flag to redraw the window, since the player base animation has changed
		REDRAW_WINDOW = 1;
		// Force the ammo number to be drawn
		forceDraw = 1;
	}
	// If the current ammo does not match the previous ammo or weapon has changed to none, then redraw the ammo count
	if (forceDraw || WEAPON_AMMO[SELECTED_WEAPON - 1] != PREVIOUS_AMMO || (SELECTED_WEAPON == 0 && SELECTED_WEAPON != PREVIOUS_WEAPON)){
		if (!SELECTED_WEAPON){
			videoFillRect(296, 32, 24, 8, 0);
			PREVIOUS_AMMO = 0;
		} else{
			strcpynum(ammostr, WEAPON_AMMO[SELECTED_WEAPON - 1], 3, '0');
			videoPrintString(296, 32, 0, ammostr, VIDEO_FONT_COLOR_WHITE);
			PREVIOUS_AMMO = WEAPON_AMMO[SELECTED_WEAPON - 1];
		}
	}
	// Save the newly selected weapon, so we can tell when it changes
	PREVIOUS_WEAPON = SELECTED_WEAPON;
}

void gameDisplayItem(int forceDraw){
	unsigned int i;
	char inventorystr[4];
	
	// Check to see if our item has run out of inventory. If so, set SELECTED_ITEM to next item with inventory or 0 if not found
	if (SELECTED_ITEM){
		for (i = SELECTED_ITEM - 1; i < ITEM_COUNT; i++){
			if (ITEM_INVENTORY[i] != 0){
				SELECTED_ITEM = i + 1;
				break;
			}
		}
		if (SELECTED_ITEM > ITEM_COUNT || ITEM_INVENTORY[SELECTED_ITEM - 1] == 0){
			SELECTED_ITEM = 0;
		}
	}
	// If no item is selected, check if there is inventory for another item and switch to that
	if (!SELECTED_ITEM){
		for (i = 0; i < ITEM_COUNT; i++){
			if (ITEM_INVENTORY[i] != 0){
				SELECTED_ITEM = i + 1;
				break;
			}
		}
	}
	// If the current inventory does not match the previous inventory or the item has changed to none, then redraw the inventory count
	if (forceDraw || ITEM_INVENTORY[SELECTED_ITEM - 1] != PREVIOUS_INVENTORY || (SELECTED_ITEM == 0 && SELECTED_ITEM != PREVIOUS_ITEM)){
		if (!SELECTED_ITEM){
			videoFillRect(296, 80, 24, 8, 0);
			PREVIOUS_ITEM = 0;
		} else{
			strcpynum(inventorystr, ITEM_INVENTORY[SELECTED_ITEM - 1], 3, '0');
			videoPrintString(296, 80, 0, inventorystr, VIDEO_FONT_COLOR_WHITE);
			PREVIOUS_INVENTORY = ITEM_INVENTORY[SELECTED_ITEM - 1];
		}
	}
	// If the current item does not match the previous item, then we need to draw the item
	if (forceDraw || SELECTED_ITEM != PREVIOUS_ITEM){
		if (!SELECTED_ITEM){
			videoFillRect(272, 54, 48, 21, 0);
		} else{
			videoPrintTile(272, 54, ITEMS[SELECTED_ITEM + 1], VIDEO_ITEM_W, VIDEO_ITEM_H);
		}
		PREVIOUS_ITEM = SELECTED_ITEM;
	}
}

void gameDisplayKeys(int clearFirst){
	uint8_t i, keyoffset, str[7];
	
	// Clear the keys if flag set
	if (clearFirst){
		videoFillRect(272, 96, 48, 24, 0);
	}
	keyoffset = 0;VIDEO_KEY_W;
	for (i = 0; i < 3; i++){
		if (KEY_INVENTORY[i + 1]){
			videoPrintTile(272 + keyoffset, 106, KEYS[i], VIDEO_KEY_W, VIDEO_KEY_H);
		}
		keyoffset += VIDEO_KEY_W;
	}
}

void gameDisplayHealth(uint8_t init){
	int face_frame, x, y, color, colorshade1, colorshade2;
	uint8_t i;
	
	switch (UNIT_HEALTH[UNIT_PLAYER]){
		case 10:
		case  9:
		case  8:{ // LightDamaged
			face_frame = 1;
			break;
		}
		case  7:
		case  6:
		case  5:{ // Damaged
			face_frame = 2;
			break;
		}
		case  4:
		case  3:{ // Hurt bad
			face_frame = 3;
			break;
		}
		case  2:
		case  1:{ // Nearly dead
			face_frame = 4;
			break;
		}
		case  0:{ // Dead
			face_frame = 5;
			break;
		}
		default:{ // Healthy
			face_frame = 0;
		}
	}
	// Draw face
	videoPrintTile(272, 130, HEALTH[face_frame], VIDEO_HEALTH_W, VIDEO_HEALTH_H);
	// Draw health bar
	x = 272;
	y = 184;
	// First erase the health bar
	videoFillRect(x, y, 48, 8, 0);
	// Health bar color
	colorshade1 = 1;
	colorshade2 = 2;
	// Now draw each block of the health bar
	for (i = 0; i < UNIT_HEALTH[UNIT_PLAYER]; i++){
		color = VIDEO_HEALTH_COLORS[i * 3];
		switch (VIDEO_MODE){
			case TDY_LOW:
			case CGACOMP:{
				videoFillRect(x, y, 2, 8, color);
				break;
			}
			case CGA:
			case EGA:{
				videoFillRect(x, y, 3, 8, color);
				break;
			}
			default:{
				videoFillRect(x, y, 3, 8, color);
				videoFillRect(x, y, 3, 1, color + colorshade1);
				videoFillRect(x + 2, y, 1, 8, color + colorshade1);
				videoFillRect(x, y + 7, 3, 1, color + colorshade2);
				videoFillRect(x, y, 1, 8, color + colorshade2);
			}
		}
		x += 4;
	}
	if (init){
		// Draw arrows under health bar if 'init' is set to 1, which is the initial draw
		x = 272;
		y = 193;
		for (i = 0; i < 6; i++){
			color = VIDEO_HEALTH_COLORS[i * 6];
			switch (VIDEO_MODE){
				case TDY_LOW:
				case CGACOMP:{
					videoFillRect(x + 3 + (i << 3), y, 2, 2, color);
					videoFillRect(x + 1 + (i << 3), y + 2, 6, 2, color);
					videoFillRect(x + (i << 3), y + 4, 8, 2, color);
					break;
				}
				default:{
					videoFillRect(x + 3 + (i << 3), y, 1, 1, color);
					videoFillRect(x + 2 + (i << 3), y + 1, 3, 1, color);
					videoFillRect(x + 1 + (i << 3), y + 2, 5, 1, color);
					videoFillRect(x + (i << 3), y + 3, 7, 1, color);
				}
			}
		}
	}
}

inline void fireWeapon(uint8_t weapon, uint8_t type, uint8_t sprite, uint8_t distance){
	unsigned int i;
	
	// If firing the plasma gun (weapon 2), don't fire if a previous plasma or explosion is still active
	if (weapon == PLASMA && (BIG_EXPLOSION_ACTIVE == 1 || PLASMA_ACTIVE == 1)){
		return;
	}
	// Return if we have no ammo for weapon
	if (WEAPON_AMMO[weapon - 1] == 0){
		return;
	}
	// Find an empty unit to create weapon fire AI routine
	for (i = 28; i < 32; i++){
		if (UNIT_TYPE[i] == TYPE_NONE){
			// Found an empty unit. Convert to weapon fire
			UNIT_TYPE[i] = type;
			// Set sprite for weapon unit
			UNIT_ANIM_BASE[i] = sprite;
			UNIT_ANIM_FRAME[i] = 0;
			UNIT_IS_SPRITE[i] = 1;
			// Set distance weapon can travel
			UNIT_A[i] = distance;
			// Set weapon used
			UNIT_B[i] = weapon - 1;
			// Reset timer for unit
			UNIT_TIMER_A[i] = 0;
			// Set unit to player's x/y position
			unitPos(i, UNIT_X[UNIT_PLAYER], UNIT_Y[UNIT_PLAYER]);
			switch (weapon){
				case PISTOL:{
					// PLAY PISTOL SOUND
					ssy_sound_play(FIRE_PISTOL);
					break;
				}
				case PLASMA:{
					// PLAY PLASMA SOUND
					ssy_sound_play(FIRE_PLASMA);
					// Mark plasma as active
					PLASMA_ACTIVE = 1;
					break;
				}
			}
			WEAPON_AMMO[weapon - 1]--;
			gameDisplayWeapon(0);
			break;
		}
	}
}	

void gameSetDifficulty(){
	unsigned int i;
	
	// This routine is run after the map is loaded, but before the game starts.
	// If the difficulty is set to normal, nothing actually happens. But if it
	// is set to easy or hard, then some changes occur accordingly.
	switch (DIFFICULTY_LEVEL){
		case 0:{ // Easy. Find all hidden items (except keys) and double the quantity.
			for (i = 48; i < 64; i++){
				if (UNIT_TYPE[i] == TYPE_NONE || UNIT_TYPE[i] == TYPE_HIDDEN_KEY){
					continue;
				}
				UNIT_A[i] <<= 1;
			}
			break;
		}
		case 2:{ // Hard. Find all hoverbots and change AI to attack mode
			for (i = 1; i < 28; i++){
				if (UNIT_TYPE[i] == TYPE_HOVERBOT_HORIZ || UNIT_TYPE[i] == TYPE_HOVERBOT_VERT){
					UNIT_TYPE[i] = TYPE_HOVERBOT_ATTACK;
				}
			}
			break;
		}
	}
}

void gameMiniMapStats(uint8_t update){
	char str[25];
	uint8_t x, y;
	
	// update=1 means to force all to be drawn. update=2 means to update robots/secrets remaining
	if (!MINIMAP_OPEN){
		return;
	}
	if (update == 1){
		// Clear screen around map
		videoFillRect(0, 0, 264, 20, 0);
		videoFillRect(0, 20, 4, 128, 0);
		videoFillRect(256, 20, 8, 128, 0);
		videoFillRect(0, 148, 264, 20, 0);
		// Show stats
		strcpy(str, MAP_NAMES[LEVEL_SELECTED]);
		videoPrintString(4, 6, 1, str, VIDEO_FONT_COLOR_WHITE);
	}
	if (update == 1 || CLOCK.UPDATE){
		strcpynum(str, CLOCK.HOURS, 2, '0');
		strncat(str, ":", 1);
		strcatnum(str, CLOCK.MINUTES, 2, '0');
		strncat(str, ":", 1);
		strcatnum(str, CLOCK.SECONDS, 2, '0');
		videoPrintString(196, 6, 1, str, VIDEO_FONT_COLOR_WHITE);
		CLOCK.UPDATE = 0;
	}
	if (update){
		y = 0;
		for (x = 1; x < 28; x++){
			if (UNIT_TYPE[x] != TYPE_NONE && UNIT_TYPE[x] != TYPE_DEAD_ROBOT){
				y++;
			}
		}
		strcpy(str, "ROBOTS:");
		strcatnum(str, y, 3, '0');
		videoPrintString(4, 154, 1, str, VIDEO_FONT_COLOR_WHITE);
		// Write secrets remaining
		y = 0;
		for (x = 48; x < 63; x++){
			if (UNIT_TYPE[x] != TYPE_NONE){
				y++;
			}
		}
		strcpy(str, "SECRETS:");
		strcatnum(str, y, 3, '0');
		videoPrintString(172, 154, 1, str, VIDEO_FONT_COLOR_WHITE);
	}
}

void gameMiniMapDraw(uint16_t vidseg[]){
	uint8_t i, x, y, x2, y2, plane, *vidptr2, *p;

	if (vidseg == MINIMAP){
		// If drawing on MINIMAP ram, start at 0,0
		x2 = 0;
		y2 = 0;
	} else{
		// If drawing directly on screen, start at an offset position
		x2 = MINIMAP_X_OFFSET;
		y2 = 20;
		// Show stats around the mini-map. 1=Force updating all stats
		if (MINIMAP_OPEN){
			gameMiniMapStats(1);
		}
	}
	// Loop through each row of the map
	for (y = 0; y < 64; y++){
		// Loop twice for each row to draw two lines for each row
		for (i = 0; i < 2; i++){
			if (vidseg == MINIMAP){
				vidptr2 = memorySegToPtr(vidseg[y2]) + x2;
			} else{
				vidptr2 = memorySegOfsToPtr(VIDEO_SCREENSEG, vidseg[y2] + x2);
			}
			// If we are drawing to the screen and we have the map in ram, do a memcpy of it
			if (vidseg != MINIMAP){
				minimapCopyLineToScreen(vidptr2, y);
				y2++;
				continue;
			}
			// Draw each map color horizontally, 2 pixels thick
			minimapPrepareLine(vidptr2, y);
			y2++;
			// If we are drawing the map in the MINIMAP ram, only do one line per map row
			if (vidseg == MINIMAP){
				break;
			}
		}
	}
	// Force drawing the player after the map opens
	if (MINIMAP_OPEN){
		unitPos(UNIT_PLAYER, UNIT_X[UNIT_PLAYER], UNIT_Y[UNIT_PLAYER]);
	}
}
			
void gameMapDraw(){
	uint8_t i, x, y, x_offset, y_offset, mapx, mapy, pos, tile, unit, unit_tile, use_sprite, cinema_updated;

	// First, this routine checks all units from 0 to 31 and figured out if they should be displayed
	// on screen, and then grabs that unit's id and stores it in the MAP_PRECALC array
	// so that when the window is drawn, it does not have to search for units during the
	// draw, speeding up the display routine
	REDRAW_WINDOW = 0;
	if (MINIMAP_OPEN){
		return;
	}
	// Set all map window entries to have no unit before checking where the units are
	memset(MAP_PRECALC, UNIT_NOT_FOUND, MAP_PRECALC_COUNT);
	for (i = 0; i < UNIT_COUNT; i++){
		// Check if unit falls within the display window (except player unit 0, since it is always drawn)
		if (i != UNIT_PLAYER){
			if (i & 32){
				// Hidden units, don't draw, skip ahead
				i = 63;
				continue;
			}
			if (UNIT_TYPE[i] == TYPE_NONE){
				// No unit at this index position
				continue;
			}
			if (UNIT_X[i] < MAP_WINDOW_X || UNIT_X[i] > MAP_WINDOW_X + 10){
				// Unit not on screen
				continue;
			}
			if (UNIT_Y[i] < MAP_WINDOW_Y || UNIT_Y[i] > MAP_WINDOW_Y + 6){
				// Unit not on screen
				continue;
			}
		} else{
			// If player has won, don't draw them
			if (UNIT_TYPE[UNIT_PLAYER] == TYPE_PLAYER_WINS){
				continue;
			}
		}
		// Unit found in map window, now add that unit's tile to the precalc map.
		pos = MAP_PRECALC_ROWS[UNIT_Y[i] - MAP_WINDOW_Y] + (UNIT_X[i] - MAP_WINDOW_X);
		if (MAP_PRECALC[pos] != UNIT_NOT_FOUND && (UNIT_TYPE[i] == TYPE_TIMEBOMB || UNIT_TYPE[i] == TYPE_MAGNET)){
			// If the unit is a bomb or magnet, then skip drawing it if another unit exists here
			continue;
		}
		// This unit should display on the map. Set this position to the unit's id
		MAP_PRECALC[pos] = i;
	}
	// Draw map
	pos = 0;
	y_offset = 0;
	cinema_updated = 0;
	for (y = 0; y < 7; y++){
		x_offset = 0;
		for (x = 0; x < 11; x++){
			mapx = x + MAP_WINDOW_X;
			mapy = y + MAP_WINDOW_Y;
			// Plot map tile if it is different from the previous drawn map tile/unit
			tile = MAP[mapy][mapx];
			use_sprite = 0;
			// If this tile is animated, then update it to the proper animation frame
			if (TILE_ANIMATED[tile]){
				unit_tile = TILE_ANIMATION[TILE_ANIMATED[tile] + 4];
				tile = TILE_ANIMATION[TILE_ANIMATED[tile] + TILE_ANIMATION_FRAME];
				if (unit_tile != 0){
					use_sprite = 1;
					// Check if this is a cinema tile
					if (tile >= 20 && tile <= 22){
						if (!cinema_updated){
							if (CINEMA_LAST_FRAME != TILE_ANIMATION_FRAME){
								cinema_updated = 1;
								CINEMA_LAST_FRAME = TILE_ANIMATION_FRAME;
								CINEMA_ANIMATION_FRAME++;
								// Rotate cinema text to the left
								videoCinemaRotate(TILES[TILE_CINEMA_L], TILES[TILE_CINEMA_M], TILES[TILE_CINEMA_R]);
								if (CINEMA_ANIMATION_FRAME == 2){
									CINEMA_ANIMATION_FRAME = 0;
									// Add new letter to the cinema text
									videoCinemaAddChar(FONT[CINEMA_TEXT[CINEMA_INDEX] & 191], TILES[TILE_CINEMA_R]);
									CINEMA_INDEX++;
									if (CINEMA_TEXT[CINEMA_INDEX] == 0){
										CINEMA_INDEX = 0;
									}
								}
							}
						}
						if (cinema_updated){
							// Force draw of cinema tiles
							PREVIOUS_DRAW_TILE[y][x] = 255;
						}
					}
				} else{
					unit_tile = TILE_NOT_PLOTTED;
				}
			}
			// Get the unit tile/sprite we will be drawing, unless we've already got a sprite set
			unit = MAP_PRECALC[pos];
			if (!use_sprite){
				if (unit == UNIT_NOT_FOUND){
					unit_tile = TILE_NOT_PLOTTED;
				} else{
					if (UNIT_IS_SPRITE[unit]){
						unit_tile = UNIT_ANIM_BASE[unit] + UNIT_ANIM_FRAME[unit];
						use_sprite = 1;
					} else{
						unit_tile = UNIT_TILE[unit];
					}
				}
			}
			// Only draw if the tile or unit tile/sprite has changed
			if (tile != PREVIOUS_DRAW_TILE[y][x] || unit_tile != PREVIOUS_DRAW_UNIT[y][x]){
				if (unit != UNIT_NOT_FOUND || use_sprite){
					// Plot tile that contains sprite or a unit that overrides the tile
					if (use_sprite){
						if (TRANSPARENCY){
							videoPrintSprite(x_offset, y_offset, SPRITES[unit_tile], TILES[tile]);
						} else{
							videoPrintTile(x_offset, y_offset, SPRITES[unit_tile], VIDEO_TILE_W, VIDEO_TILE_H);
						}
					} else{
						videoPrintTile(x_offset, y_offset, TILES[unit_tile], VIDEO_TILE_W, VIDEO_TILE_H);
					}
				} else{
					// Plot normal tile
					videoPrintTile(x_offset, y_offset, TILES[tile], VIDEO_TILE_W, VIDEO_TILE_H);
				}
				// Save what tile and unit tile/sprite is at a position on the screen, to prevent drawing next time
				PREVIOUS_DRAW_TILE[y][x] = tile;
				PREVIOUS_DRAW_UNIT[y][x] = unit_tile;
			}
			pos++;
			x_offset += VIDEO_TILE_W;
		}
		y_offset += VIDEO_TILE_H;
	}
	// Draw cursor if it is enabled
	if (CURSOR.X != 0 && CURSOR.Y != 0){
		if (!VIDEO_USE_PALETTE_ANIMATION){
			i = CURSOR.color;
		} else{
			i = 16;
		}
		x_offset = VIDEO_TILEW_OFFSET[CURSOR.X];
		y_offset = VIDEO_TILEH_OFFSET[CURSOR.Y];
		if (CURSOR.animating){
			PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
			switch (TILE_ANIMATION_FRAME){
				case 0:{
					videoPrintSpriteAsColor(x_offset, y_offset - 8, SPRITES[CURSOR.sprite], i);
					PREVIOUS_DRAW_TILE[CURSOR.Y - 1][CURSOR.X] = TILE_NOT_PLOTTED;
					break;
				}
				case 1:{
					videoPrintSpriteAsColor(x_offset + 8, y_offset, SPRITES[CURSOR.sprite], i);
					PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X + 1] = TILE_NOT_PLOTTED;
					break;
				}
				case 2:{
					videoPrintSpriteAsColor(x_offset, y_offset + 8, SPRITES[CURSOR.sprite], i);
					PREVIOUS_DRAW_TILE[CURSOR.Y + 1][CURSOR.X] = TILE_NOT_PLOTTED;
					break;
				}
				case 3:{
					videoPrintSpriteAsColor(x_offset - 8, y_offset, SPRITES[CURSOR.sprite], i);
					PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X - 1] = TILE_NOT_PLOTTED;
					break;
				}
			}
		} else{
			videoPrintSpriteAsColor(x_offset, y_offset, SPRITES[CURSOR.sprite], i);
		}
	}
}

void gameUserSelectObject(uint8_t x, uint8_t y, int animate){
	int key;
	uint8_t unit;
	
	// This command is called to display a cursor for the user to be able to select or place an item.
	// PLAY SOUND_BEEP
	ssy_sound_play(BEEP);
	// Turn on the cursor
	CURSOR.X = x;
	CURSOR.Y = y;
	CURSOR_ENABLED = 1;
	EVENT_ENABLED = 1;
	CURSOR.color_index = 16;
	// Prevent the key from repeating
	KEY_NOREPEAT = 1;
	// Force redraw of screen to draw cursor
	REDRAW_WINDOW = 1;
	// Loop while we wait for user input, as long as the player is still alive
	do{
		// Call backgroundTasks to update units while we wait for user input
		backgroundTasks();
		// Redraw window if needed
		if (REDRAW_WINDOW){
			gameMapDraw();
		}
		// Check for user input to move the cursor
		if (key = keyGet()){
			if (key == CUSTOM_KEYS[KEYS_MOVE_LEFT] || key == SCAN_LEFT_ARROW || key == SCAN_KP_4){
				// Move Cursor Left
				// Force redraw of tile/sprite under cursor
				PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
				PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
				// Update cursor on minimap
				if ((unit = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
				} else{
					minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
				}
				CURSOR.X--;
				if (animate){
					// Make player face left
					playerAnimate(SPRITE_PLAYER_LEFT, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_RIGHT] || key == SCAN_RIGHT_ARROW || key == SCAN_KP_6){
				// Move Cursor Right
				// Force redraw of tile/sprite under cursor
				PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
				PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
				// Update cursor on minimap
				if ((unit = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
				} else{
					minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
				}
				CURSOR.X++;
				if (animate){
					// Make player face right
					playerAnimate(SPRITE_PLAYER_RIGHT, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_UP] || key == SCAN_UP_ARROW || key == SCAN_KP_8){
				// Move Cursor Up
				// Force redraw of tile/sprite under cursor
				PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
				PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
				// Update cursor on minimap
				if ((unit = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
				} else{
					minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
				}
				CURSOR.Y--;
				if (animate){
					// Make player face up
					playerAnimate(SPRITE_PLAYER_UP, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_DOWN] || key == SCAN_DOWN_ARROW || key == SCAN_KP_2){
				// Move Cursor Down
				// Force redraw of tile/sprite under cursor
				PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
				PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
				// Update cursor on minimap
				if ((unit = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
				} else{
					minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
				}
				CURSOR.Y++;
				if (animate){
					// Make player face down
					playerAnimate(SPRITE_PLAYER_DOWN, 1);
				}
			} else if (key == (SCAN_TILDE | MODIFIER_SHIFT)){
				// Gravis Gamepad tapping the search/move button a second time, change to move choice
				CURSOR.sprite = SPRITE_CURSOR_MOVE;
				PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
				PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
				REDRAW_WINDOW = 1;
				gameMapDraw();
			} else if (key == (SCAN_M | MODIFIER_SHIFT)){
				// Gravis Gamepad Toggle Music
				// Shift-M to toggle music
				// If music device has been selected, then toggle the music
				if (SSY_DEVICE_MUS){
					if (SSY_MUS_ENABLE){
						ssy_music_stop();
						videoInfoPrint("MUSIC DISABLED");
					} else{
						// ssy_music_play(memorySegToPtr(MUSIC_GAME));
						ssy_music_play(MUSIC_GAME);
						videoInfoPrint("MUSIC ENABLED");
					}
				}
				KEY_NOREPEAT = 1;
				break;
			} else{
				// Force key to 0 if no valid selection, thus remaining in loop
				key = 0;
			}
		}
	} while (key == 0 && UNIT_HEALTH[UNIT_PLAYER] != 0);
}

void gameSearchObject(uint8_t x, uint8_t y){
	char str[18];
	uint8_t i, unit2;
	
	// Check if tile is searchable
	if (tileHasAttribute(x, y, ATTRIB_SEARCHABLE)){
		// Check if this is a crate
		switch(MAP[y][x]){
			case TILE_CRATE_BIG:	// Big crate
			case TILE_CRATE_SMALL:	// Small crate
			case TILE_CRATE_PI:{	// Pi crate
				// Switch to an open crate
				mapSet(x, y, DESTRUCT_PATH[MAP[y][x]]);
				break;
			}
		}
		CURSOR.animating = 1;
		strcpy(str, "SEARCHING");
		videoInfoPrint(str);
		// Delay while searching, adding a periods to the info text
		for (i = 0; i < 8; i++){
			// Set countdown timer
			EVENT_TIMER = 18;
			EVENT_ENABLED = 1;
			// Run background tasks and update the screen where needed while searching
			while (EVENT_TIMER != 0){
				backgroundTasks();
				if (REDRAW_WINDOW){
					gameMapDraw();
				}
			}
			// Add a period onto the "SEARCHING" string
			INFOLINE--;
			strcat(str, ".");
			videoInfoPrint(str);
		}
		CURSOR.animating = 0;
		// Check if there is a hidden unit at this location
		if ((i = checkForHiddenUnit(x, y)) != UNIT_NOT_FOUND){
			// PLAY SOUND_ITEM_FOUND
			ssy_sound_play(ITEM_FOUND);
			// Give hidden item to the player
			switch(UNIT_TYPE[i]){
				case TYPE_HIDDEN_KEY:{ // Key
					// Add key to inventory
					KEY_INVENTORY[UNIT_A[i] + 1] = 1;
					gameDisplayKeys(0);
					videoInfoPrint("YOU FOUND A KEY CARD!");
					break;
				}
				case TYPE_HIDDEN_TIMEBOMB:{ // TIME BOMB
					// Add bombs to inventory
					ITEM_INVENTORY[3] += UNIT_A[i];
					if (ITEM_INVENTORY[3] > 255){
						ITEM_INVENTORY[3] = 255;
					}
					gameDisplayItem(0);
					videoInfoPrint("YOU FOUND A TIMEBOMB!");
					break;
				}
				case TYPE_HIDDEN_EMP:{ // EMP
					// Add EMP charges to inventory
					ITEM_INVENTORY[1] += UNIT_A[i];
					if (ITEM_INVENTORY[1] > 255){
						ITEM_INVENTORY[1] = 255;
					}
					gameDisplayItem(0);
					videoInfoPrint("YOU FOUND AN EMP DEVICE!");
					break;
				}
				case TYPE_HIDDEN_PISTOL:{ // PISTOL
					// Add pistol/ammo to inventory
					WEAPON_AMMO[0] += UNIT_A[i];
					if (WEAPON_AMMO[0] > 255){
						WEAPON_AMMO[0] = 255;
					}
					gameDisplayWeapon(0);
					videoInfoPrint("YOU FOUND A PISTOL!");
					break;
				}
				case TYPE_HIDDEN_PLASMA:{ // PLASMA GUN
					// Add plasma gun/ammo to inventory
					WEAPON_AMMO[1] += UNIT_A[i];
					if (WEAPON_AMMO[1] > 255){
						WEAPON_AMMO[1] = 255;
					}
					gameDisplayWeapon(0);
					videoInfoPrint("YOU FOUND A PLASMA GUN!");
					break;
				}
				case TYPE_HIDDEN_MEDKIT:{ // MEDKIT
					// Add medkits to inventory
					ITEM_INVENTORY[0] += UNIT_A[i];
					if (ITEM_INVENTORY[0] > 255){
						ITEM_INVENTORY[0] = 255;
					}
					gameDisplayItem(0);
					videoInfoPrint("YOU FOUND A MEDKIT!");
					break;
				}
				case TYPE_HIDDEN_MAGNET:{ // MAGNET
					// Add magnets to inventory
					ITEM_INVENTORY[2] += UNIT_A[i];
					if (ITEM_INVENTORY[2] > 255){
						ITEM_INVENTORY[2] = 255;
					}
					gameDisplayItem(0);
					videoInfoPrint("YOU FOUND A MAGNET!");
					break;
				}
			}
			// Delete hidden item
			UNIT_TYPE[i] = TYPE_NONE;
			// Force redraw of tile/sprite under cursor
			PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
			PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
			// Update cursor on minimap
			if ((unit2 = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
				unitPos(unit2, UNIT_X[unit2], UNIT_Y[unit2]);
			} else{
				minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
			}
			// Turn off cursor
			if (!MINIMAP_OPEN){
				CURSOR_ENABLED = 0;
			}
			CURSOR.X = 0;
			// Update MiniMap stats
			if (MINIMAP_OPEN){
				gameMiniMapStats(2);
			}
			// Flag to update screen
			REDRAW_WINDOW = 1;
			return;
		}
	}
	videoInfoPrint("NOTHING FOUND HERE.");
	// Force redraw of tile/sprite under cursor
	PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
	PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
	// Update cursor on minimap
	if ((unit2 = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
		unitPos(unit2, UNIT_X[unit2], UNIT_Y[unit2]);
	} else{
		minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
	}
	// Turn off cursor
	if (!MINIMAP_OPEN){
		CURSOR_ENABLED = 0;
	}
	CURSOR.X = 0;
	// Flag to update screen
	REDRAW_WINDOW = 1;
}

void gameMoveObject(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2){
	uint8_t orig_tile, unit2;
	
	// Update cursor on minimap
	if ((unit2 = checkForUnit(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y)) != UNIT_NOT_FOUND){
		unitPos(unit2, UNIT_X[unit2], UNIT_Y[unit2]);
	} else{
		minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, MAP_TILE_COLORS[MAP[MAP_WINDOW_Y + CURSOR.Y][MAP_WINDOW_X + CURSOR.X]]);
	}
	// Check if the destination (x2, y2) is blocked by a tile or unit
	if (!tileHasAttribute(x2, y2, ATTRIB_CAN_MOVE_ONTO) || checkForUnit(x2, y2) != UNIT_NOT_FOUND){
		//PLAY SOUND_ERROR
		ssy_sound_play(ERROR);
		videoInfoPrint("BLOCKED!");
		// Force redraw of tile under cursor
		PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
		PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
		// Turn off cursor
		if (!MINIMAP_OPEN){
			CURSOR_ENABLED = 0;
		}
		CURSOR.X = 0;
		// Flag to update screen
		REDRAW_WINDOW = 1;
		return;
	}
	//PLAY SOUND_MOVEOBJ
	ssy_sound_play(MOVE_OBJECT);
	// Grab the tile being moved onto
	orig_tile = MAP[y2][x2];
	// Move the object on the map
	mapSet(x2, y2, MAP[y][x]);
	// Replace the original position of the object with orig_tile, if not trash compactor
	if (orig_tile == TILE_TRASH_COMPACTOR_FLOOR){
		orig_tile = TILE_FLOOR;
	}
	mapSet(x, y, orig_tile);
	// Check if there is a hidden unit to move
	unit2 = checkForHiddenUnit(x, y);
	// If a hidden unit was found, then move it
	if (unit2 != UNIT_NOT_FOUND){
		UNIT_X[unit2] = x2;
		UNIT_Y[unit2] = y2;
	}
	// Force redraw of tile under cursor
	PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
	PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
	// Turn off cursor
	if (!MINIMAP_OPEN){
		CURSOR_ENABLED = 0;
	}
	CURSOR.X = 0;
	// Flag to update screen
	REDRAW_WINDOW = 1;
}

void gameUseItem(){
	unsigned int x, y, i;
	
	// Check if the player has an item selected and has inventory for it
	if (SELECTED_ITEM && ITEM_INVENTORY[SELECTED_ITEM - 1] != 0){
		// Use the item
		switch (SELECTED_ITEM){
			case MEDKIT:{
				// If player has less than full HP, then attempt to heal them
				if (UNIT_HEALTH[UNIT_PLAYER] < 12){
					if (ITEM_INVENTORY[SELECTED_ITEM - 1] >= 12 - UNIT_HEALTH[UNIT_PLAYER]){
						// We have enough medkits to fully heal player
						ITEM_INVENTORY[SELECTED_ITEM - 1] -= 12 - UNIT_HEALTH[UNIT_PLAYER];
						UNIT_HEALTH[UNIT_PLAYER] = 12;
					} else{
						// Heal player partially, not enough medkits
						UNIT_HEALTH[UNIT_PLAYER] += ITEM_INVENTORY[SELECTED_ITEM - 1];
						ITEM_INVENTORY[SELECTED_ITEM - 1] = 0;
					}
					// PLAY SOUND_MEDKIT
					ssy_sound_play(USE_MEDKIT);
					// Update player's health display
					gameDisplayHealth(0);
					videoInfoPrint("AHHH, MUCH BETTER!");
				}
				break;
			}
			case EMP:{
				// Create background EMP flash
				EMP_FLASH_STEPS_REMAINING = VIDEO_EMP_FLASH_COUNT;
				EVENT_ENABLED = 1;
				// PLAY SOUND_EMP
				ssy_sound_play(USE_EMP);
				// Decrease EMP inventory
				ITEM_INVENTORY[SELECTED_ITEM - 1]--;
				// Loop through units to check if they are on the screen
				for (i = 1; i < 28; i++){
					// Does this unit exist?
					if (UNIT_TYPE[i] != TYPE_NONE){
						// Is this unit on the screen?
						if (UNIT_X[i] >= MAP_WINDOW_X && UNIT_X[i] < MAP_WINDOW_X + 11 && UNIT_Y[i] >= MAP_WINDOW_Y && UNIT_Y[i] < MAP_WINDOW_Y + 7){
							// Reset the unit timer to disable the robot
							UNIT_TIMER_A[i] = 255;
							// Is this robot above water?
							if (MAP[UNIT_Y[i]][UNIT_X[i]] == TILE_WATER){
								// Electricute the robot
								UNIT_TYPE[i] = TYPE_HOVERBOT_WATER;
								UNIT_TIMER_A[i] = 5;
								UNIT_A[i] = 60;
								UNIT_TILE[i] = TILE_ROBOT_WATER_ANIM_START;
								UNIT_IS_SPRITE[i] = 0;
							}
						}
					}
				}
				videoInfoPrint("EMP ACTIVATED!");
				videoInfoPrint("NEARBY ROBOTS ARE REBOOTING.");
				break;
			}
			case MAGNET:{
				// Only allow one magnet to be active at a time
				if (MAGNET_ACTIVE){
					break;
				}
				// Show cursor at player position, wait for input, and face player that direction
				CURSOR.sprite = SPRITE_CURSOR_USE;
				gameUserSelectObject(5, 3, 1);
				// If player died while selecting break out of function
				if (UNIT_HEALTH[UNIT_PLAYER] == 0){
					break;
				}
				// Place magnet at selected position if not blocked
				x = MAP_WINDOW_X + CURSOR.X;
				y = MAP_WINDOW_Y + CURSOR.Y;
				// Check if the select placement will be blocked
				if (!tileHasAttribute(x, y, ATTRIB_CAN_WALK_OVER)){
					// PLAY SOUND_BLOCKED
					ssy_sound_play(ERROR);
					videoInfoPrint("BLOCKED!");
					break;
				}
				// Attempt to find an empty weapon unit to place the item
				for (i = 28; i < 32; i++){
					// Check if this unit is not in use
					if (UNIT_TYPE[i] == TYPE_NONE){
						// PLAY MOVEOBJ SOUND
						ssy_sound_play(MOVE_OBJECT);
						// Create magnet unit
						UNIT_TYPE[i] = TYPE_MAGNET;
						UNIT_ANIM_BASE[i] = SPRITE_MAGNET;
						UNIT_ANIM_FRAME[i] = 0;
						UNIT_IS_SPRITE[i] = 1;
						unitPos(i, x, y);
						// How long until activation
						UNIT_TIMER_A[i] = 1;
						// How long until deactivation (uses 2 timers for 16 bit value)
						UNIT_TIMER_B[i] = 255;
						UNIT_A[i] = 5;
						// Set global flag that a magnet is active (only one allowed at a time)
						MAGNET_ACTIVE = 1;
						// Remove a magnet from inventory
						ITEM_INVENTORY[SELECTED_ITEM - 1]--;
						break;
					}
				}
				break;
			}
			case TIMEBOMB:{
				// Show cursor at player position, wait for input, and face player that direction
				CURSOR.sprite = SPRITE_CURSOR_USE;
				gameUserSelectObject(5, 3, 1);
				// If player died while selecting break out of function
				if (UNIT_HEALTH[UNIT_PLAYER] == 0){
					break;
				}
				// Place bomb at selected position if not blocked
				x = MAP_WINDOW_X + CURSOR.X;
				y = MAP_WINDOW_Y + CURSOR.Y;
				// Check if the select placement will be blocked
				if (!tileHasAttribute(x, y, ATTRIB_CAN_WALK_OVER) || checkForUnit(x, y) != UNIT_NOT_FOUND){
					// PLAY SOUND_BLOCKED
					ssy_sound_play(ERROR);
					videoInfoPrint("BLOCKED!");
					break;
				}
				// Attempt to find an empty weapon unit to place the item
				for (i = 28; i < 32; i++){
					// Check if this unit is not in use
					if (UNIT_TYPE[i] == TYPE_NONE){
						// PLAY MOVEOBJ SOUND
						ssy_sound_play(MOVE_OBJECT);
						// Create bomb unit
						UNIT_TYPE[i] = TYPE_TIMEBOMB;
						UNIT_ANIM_BASE[i] = SPRITE_BOMB;
						UNIT_ANIM_FRAME[i] = 0;
						UNIT_IS_SPRITE[i] = 1;
						unitPos(i, x, y);
						// How long until explosion
						UNIT_TIMER_A[i] = 100;
						UNIT_A[i] = 0;
						// Remove a bomb from inventory
						ITEM_INVENTORY[SELECTED_ITEM - 1]--;
						break;
					}
				}
				break;
			}
		}
		// If the cursor is on, then disable it
		if (CURSOR_ENABLED){
			// Force redraw of tile under cursor
			PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
			PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
			// Turn off cursor
			if (!MINIMAP_OPEN){
				CURSOR_ENABLED = 0;
			}
			CURSOR.X = 0;
			// Flag to update screen
			REDRAW_WINDOW = 1;
		}
		// Update inventory display after attempting to use item
		gameDisplayItem(0);
	}
}

int gameMainLoop(){
	int key;
	uint8_t x, y, playing;
	// char str[30];

	
	playing = 1;
	while (playing){
		// Run backgroudTasks to update any robot movements
		backgroundTasks();
		// Redraw screen if needed
		if (REDRAW_WINDOW){
			gameMapDraw();
		}
		// If the player is about to transport, then don't continue further for keyboard checks
		if (UNIT_ANIM_BASE[UNIT_PLAYER] == SPRITE_DEMATERIALIZE){
			continue;
		}
		// If the player has died or won the game, then delay a bit and break out of loop to show game over
		if (UNIT_HEALTH[UNIT_PLAYER] == 0 || UNIT_TYPE[UNIT_PLAYER] == TYPE_PLAYER_WINS){
			// If player is dead, update the animation to show the dead player
			if (UNIT_ANIM_BASE[UNIT_PLAYER] != SPRITE_PLAYER_DEAD){
				UNIT_ANIM_BASE[UNIT_PLAYER] = SPRITE_PLAYER_DEAD;
				UNIT_ANIM_FRAME[UNIT_PLAYER] = 0;
				KEYTIMER = 100;
				REDRAW_WINDOW = 1;
			}
			// Wait for KEYTIMER to hit 0, then break out of this loop to display game over
			if (KEYTIMER != 0){
				continue;
			}
			// Stop the clock
			CLOCK.ACTIVE = 0;
			break;
		}
		
		// Check for key press
		if ((key = keyGet()) != 0){
			if (key == CUSTOM_KEYS[KEYS_MOVE_LEFT] || key == SCAN_LEFT_ARROW || key == SCAN_LEFT_ARROW_TANDY || key == SCAN_KP_4){
				// Move Left
				if (requestWalkLeft(UNIT_PLAYER, ATTRIB_CAN_WALK_OVER)){
					playerAnimate(SPRITE_PLAYER_LEFT, 0);
					// Allow key repeat for faster movement
					if (KEYFAST){
						// Subsequent repeats are faster
						KEYTIMER = 6;
					} else{
						// First repeat
						KEYFAST = 1;
						KEYTIMER = 15;
					}
				} else{
					// Reset player animation
					playerAnimate(SPRITE_PLAYER_LEFT, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_RIGHT] || key == SCAN_RIGHT_ARROW || key == SCAN_RIGHT_ARROW_TANDY || key == SCAN_KP_6){
				// Move Right
				if (requestWalkRight(UNIT_PLAYER, ATTRIB_CAN_WALK_OVER)){
					playerAnimate(SPRITE_PLAYER_RIGHT, 0);
					// Allow key repeat for faster movement
					if (KEYFAST){
						// Subsequent repeats are faster
						KEYTIMER = 6;
					} else{
						// First repeat
						KEYFAST = 1;
						KEYTIMER = 15;
					}
				} else{
					// Reset player animation
					playerAnimate(SPRITE_PLAYER_RIGHT, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_UP] || key == SCAN_UP_ARROW || key == SCAN_UP_ARROW_TANDY || key == SCAN_KP_8){
				// Move Up
				if (requestWalkUp(UNIT_PLAYER, ATTRIB_CAN_WALK_OVER)){
					playerAnimate(SPRITE_PLAYER_UP, 0);
					// Allow key repeat for faster movement
					if (KEYFAST){
						// Subsequent repeats are faster
						KEYTIMER = 6;
					} else{
						// First repeat
						KEYFAST = 1;
						KEYTIMER = 15;
					}
				} else{
					// Reset player animation
					playerAnimate(SPRITE_PLAYER_UP, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_MOVE_DOWN] || key == SCAN_DOWN_ARROW || key == SCAN_DOWN_ARROW_TANDY || key == SCAN_KP_2){
				if (requestWalkDown(UNIT_PLAYER, ATTRIB_CAN_WALK_OVER)){
					playerAnimate(SPRITE_PLAYER_DOWN, 0);
					// Allow key repeat for faster movement
					if (KEYFAST){
						// Subsequent repeats are faster
						KEYTIMER = 6;
					} else{
						// First repeat
						KEYFAST = 1;
						KEYTIMER = 15;
					}
				} else{
					// Reset player animation
					playerAnimate(SPRITE_PLAYER_DOWN, 1);
				}
			} else if (key == CUSTOM_KEYS[KEYS_FIRE_UP]){
				// Fire Up
				switch (SELECTED_WEAPON){
					case 1:{
						// Pistol Fire (pistol, type, tile, distance)
						fireWeapon(PISTOL, TYPE_WEAPON_FIRE_UP, SPRITE_PISTOL_VERT, 3);
						playerAnimate(SPRITE_PLAYER_UP, 2);
						break;
					}
					case 2:{
						// Plasma Fire (plasma, type, tile, distance)
						fireWeapon(PLASMA, TYPE_WEAPON_FIRE_UP, SPRITE_PLASMA_VERT, 3);
						playerAnimate(SPRITE_PLAYER_UP, 2);
						break;
					}
				}
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_FIRE_DOWN]){
				// Fire Down
				switch (SELECTED_WEAPON){
					case 1:{
						// Pistol Fire (pistol, type, tile, distance)
						fireWeapon(PISTOL, TYPE_WEAPON_FIRE_DOWN, SPRITE_PISTOL_VERT, 3);
						playerAnimate(SPRITE_PLAYER_DOWN, 2);
						break;
					}
					case 2:{
						// Plasma Fire (plasma, type, tile, distance)
						fireWeapon(PLASMA, TYPE_WEAPON_FIRE_DOWN, SPRITE_PLASMA_VERT, 3);
						playerAnimate(SPRITE_PLAYER_DOWN, 2);
						break;
					}
				}
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_FIRE_LEFT]){
				// Fire Left
				switch (SELECTED_WEAPON){
					case 1:{
						// Pistol Fire (pistol, type, tile, distance)
						fireWeapon(PISTOL, TYPE_WEAPON_FIRE_LEFT, SPRITE_PISTOL_HORIZ, 5);
						playerAnimate(SPRITE_PLAYER_LEFT, 2);
						break;
					}
					case 2:{
						// Plasma Fire (plasma, type, tile, distance)
						fireWeapon(PLASMA, TYPE_WEAPON_FIRE_LEFT, SPRITE_PLASMA_HORIZ, 5);
						playerAnimate(SPRITE_PLAYER_LEFT, 2);
						break;
					}
				}
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_FIRE_RIGHT]){
				// Fire Right
				switch (SELECTED_WEAPON){
					case 1:{
						// Pistol Fire (pistol, type, tile, distance)
						fireWeapon(PISTOL, TYPE_WEAPON_FIRE_RIGHT, SPRITE_PISTOL_HORIZ, 5);
						playerAnimate(SPRITE_PLAYER_RIGHT, 2);
						break;
					}
					case 2:{
						// Plasma Fire (plasma, type, tile, distance)
						fireWeapon(PLASMA, TYPE_WEAPON_FIRE_RIGHT, SPRITE_PLASMA_HORIZ, 5);
						playerAnimate(SPRITE_PLAYER_RIGHT, 2);
						break;
					}
				}
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_CYCLE_WEAPONS]){
				// F1 - Cycle Weapon
				// Remember what the previous weapon was
				x = SELECTED_WEAPON;
				// Increase weapon
				SELECTED_WEAPON++;
				// Display the weapon
				gameDisplayWeapon(0);
				// Play cycle weapon sound
				ssy_sound_play(CYCLE_WEAPON);
				// Set key timer to wait for next key check
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_CYCLE_ITEMS]){
				// F2 - Cycle Item
				// Remember what the previous item was
				x = SELECTED_ITEM;
				// Increase item
				SELECTED_ITEM++;
				// Display the weapon
				gameDisplayItem(0);
				// Play cycle item sound
				ssy_sound_play(CYCLE_ITEM);
				// Set key timer to wait for next key check
				KEYTIMER = 20;
			} else if (key == CUSTOM_KEYS[KEYS_SEARCH_OBJECT] || key == CUSTOM_KEYS[KEYS_MOVE_OBJECT] || key == (SCAN_TILDE | MODIFIER_SHIFT)){
				// Search an object for a hidden item or move an object
				// Show cursor at player position, wait for input, and face player that direction
				if (key == CUSTOM_KEYS[KEYS_MOVE_OBJECT]){
					CURSOR.sprite = SPRITE_CURSOR_MOVE;
				} else{
					CURSOR.sprite = SPRITE_CURSOR_SEARCH;
				}
				if (CURSOR.sprite == SPRITE_CURSOR_SEARCH){
					gameUserSelectObject(5, 3, 1);
					// If we are still searching (and it hasn't changed to moving with a second tap of a gravis button)
					if (CURSOR.sprite == SPRITE_CURSOR_SEARCH){
						// If the player is still alive, then do the search
						if (UNIT_HEALTH[UNIT_PLAYER] > 0){
							// Attempt to search object
							gameSearchObject(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y);
						}
					}
				}
				if (CURSOR.sprite == SPRITE_CURSOR_MOVE){
					gameUserSelectObject(5, 3, 1);
					// If the player is still alive, then do the search
					if (UNIT_HEALTH[UNIT_PLAYER] > 0){
						// Store position of object we are trying to move
						x = MAP_WINDOW_X + CURSOR.X;
						y = MAP_WINDOW_Y + CURSOR.Y;
						// Check if tile is not movable
						if (!tileHasAttribute(x, y, ATTRIB_MOVABLE)){
							// PLAY SOUND_ERROR
							ssy_sound_play(ERROR);
							videoInfoPrint("CAN'T MOVE THAT!");
							// Force redraw of tile where cursor was at
							PREVIOUS_DRAW_TILE[CURSOR.Y][CURSOR.X] = TILE_NOT_PLOTTED;
							PREVIOUS_DRAW_UNIT[CURSOR.Y][CURSOR.X] = UNIT_NOT_FOUND;
							// Turn off cursor
							if (!MINIMAP_OPEN){
								CURSOR_ENABLED = 0;
							}
							CURSOR.X = 0;
							// Set flag to redraw screen
							REDRAW_WINDOW = 1;
						} else{
							// Show cursor at object position to get direction to move it, wait for input
							gameUserSelectObject(CURSOR.X, CURSOR.Y, 0);
							// If player is still alive, try to move the object
							if (UNIT_HEALTH[UNIT_PLAYER] > 0){
								// Attempt to move object
								gameMoveObject(x, y, MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y);
							}
						}
					}
				}
				KEY_NOREPEAT = 1;
			} else if (key == CUSTOM_KEYS[KEYS_USE_ITEM] || key == (SCAN_NUMPAD_ENTER | MODIFIER_SHIFT)){
				// Use selected item
				gameUseItem();
				KEY_NOREPEAT = 1;
			} else if (key == SCAN_ESC || key == SCAN_Q){
				// Quit game
				CLOCK.ACTIVE = 0;
				videoInfoPrint("QUIT GAME? (Y/N)");
				ssy_sound_play(BEEP);
				do {
					key = keyWait();
				} while (key != SCAN_Y && key != SCAN_N && key != SCAN_ESC);
				if (key == SCAN_Y){
					playing = 0;
				} else{
					videoInfoClear();
					ssy_sound_play(BEEP2);
					KEY_NOREPEAT = 1;
					CLOCK.ACTIVE = 1;
				}
			} else if (key == (SCAN_ESC | MODIFIER_SHIFT)){
				// Quit game
				CLOCK.ACTIVE = 0;
				KEY_NOREPEAT = 1;
				videoInfoPrint("QUIT GAME?");
				videoInfoPrint("Y - PRESS BUTTON COMBO AGAIN.");
				videoInfoPrint("N - PRESS D-PAD TO CONTINUE.");
				ssy_sound_play(BEEP);
				while ((key = keyGet()) != 0);
				do {
					key = keyGet();
					if (key == SCAN_Y || key == (SCAN_ESC | MODIFIER_SHIFT)){
						playing = 0;
						break;
					}
				} while (key != SCAN_N && key != SCAN_ESC && key != SCAN_LEFT_ARROW && key != SCAN_RIGHT_ARROW && key != SCAN_UP_ARROW && key != SCAN_DOWN_ARROW);
				if (playing){
					videoInfoClear();
					ssy_sound_play(BEEP2);
					KEY_NOREPEAT = 1;
					CLOCK.ACTIVE = 1;
				}
			} else if (key == (SCAN_P | MODIFIER_SHIFT)){
				// Pause game
				CLOCK.ACTIVE = 0;
				videoInfoClear();
				videoInfoPrint("GAME PAUSED.");
				ssy_sound_play(BEEP);
				// Prevent the key from repeating
				KEY_NOREPEAT = 1;
				do {
					key = keyGet();
				} while (key != (SCAN_P | MODIFIER_SHIFT) && key != SCAN_SPACE && key != SCAN_ESC);
				videoInfoClear();
				ssy_sound_play(BEEP2);
				KEY_NOREPEAT = 1;
				CLOCK.ACTIVE = 1;
			} else if (key == (SCAN_M | MODIFIER_SHIFT)){
				// Shift-M to toggle music
				// If music device has been selected, then toggle the music
				if (SSY_DEVICE_MUS){
					if (SSY_MUS_ENABLE){
						ssy_music_stop();
						videoInfoPrint("MUSIC DISABLED");
					} else{
						// ssy_music_play(memorySegToPtr(MUSIC_GAME));
						ssy_music_play(MUSIC_GAME);
						videoInfoPrint("MUSIC ENABLED");
					}
				}
				KEY_NOREPEAT = 1;
			} else if (key == (SCAN_T | MODIFIER_SHIFT)){
				// Shift-T to toggle transparency
				if (TRANSPARENCY == 0){
					TRANSPARENCY = 1;
					videoInfoPrint("SPRITE TRANSPARENCY ENABLED");
				} else{
					TRANSPARENCY = 0;
					videoInfoPrint("SPRITE TRANSPARENCY DISABLED");
				}
				KEY_NOREPEAT = 1;
			} else if (key == SCAN_TAB || (!MINIMAP_OPEN && key == (SCAN_TAB | MODIFIER_CTRL))){
				// Tab (Mini Map)
				if (MINIMAP_OPEN){
					MINIMAP_OPEN = 0;
					CURSOR_ENABLED = 0;
					// Clear memory of previously plotted tiles within the window
					memset(PREVIOUS_DRAW_TILE, TILE_NOT_PLOTTED, 77);
					memset(PREVIOUS_DRAW_UNIT, UNIT_NOT_FOUND, 77);
					// Flag to redraw window
					ssy_sound_play(BEEP2);
					REDRAW_WINDOW = 1;
				} else{
					// Draw mini map
					MINIMAP_OPEN = 1;
					CURSOR_ENABLED = 1;
					EVENT_ENABLED = 1;
					ssy_sound_play(BEEP);
					gameMiniMapDraw(VIDEO_SCREENOFFSET);
				}
				KEY_NOREPEAT = 1;
			} else if (key == (SCAN_TAB | MODIFIER_SHIFT) || key == (SCAN_TAB | MODIFIER_CTRL)){
				// Shift-Tab (Mini Map Enable Robots
				#ifndef SHAREWARE
					if (MINIMAP_OPEN){
						if (MINIMAP_ROBOTS_ENABLED){
							MINIMAP_ROBOTS_ENABLED = 0;
							if (key == (SCAN_TAB | MODIFIER_CTRL)){
								// Close Mini-Map
								MINIMAP_OPEN = 0;
								CURSOR_ENABLED = 0;
								// Clear memory of previously plotted tiles within the window
								memset(PREVIOUS_DRAW_TILE, TILE_NOT_PLOTTED, 77);
								memset(PREVIOUS_DRAW_UNIT, UNIT_NOT_FOUND, 77);
								// Flag to redraw window
								REDRAW_WINDOW = 1;
							}
							ssy_sound_play(BEEP2);
						} else{
							MINIMAP_ROBOTS_ENABLED = 1;
							ssy_sound_play(BEEP);
						}
						// Force drawing of units on map, for units not moving
						for (x = 0; x < 28; x++){
							unitPos(x, UNIT_X[x], UNIT_Y[x]);
						}
					}
				#endif
				KEY_NOREPEAT = 1;
			} else if (key == (SCAN_8 | MODIFIER_SHIFT | MODIFIER_CTRL)){
				// Cheat
				#ifndef SHAREWARE
					videoInfoPrint("CHEATER!");
					// Give player all weapons, items, and keys
					for (x = 0; x < WEAPON_COUNT; x++){
						WEAPON_AMMO[x] = 100;
					}
					for (x = 0; x < ITEM_COUNT; x++){
						ITEM_INVENTORY[x] = 100;
					}
					for (x = 1; x < 4; x++){
						KEY_INVENTORY[x] = 1;
					}
					gameDisplayWeapon(0);
					gameDisplayItem(0);
					gameDisplayKeys(0);
				#endif
				KEY_NOREPEAT = 1;
			} else if (key == (SCAN_1 | MODIFIER_SHIFT | MODIFIER_CTRL)){
				// Cheat, kill all robots
				#ifndef SHAREWARE
					videoInfoPrint("CHEATER! ALL ROBOTS DESTROYED!");
					// Loop through robot units and destroy them all
					for (x = 1; x < 28; x++){
						UNIT_TYPE[x] = TYPE_NONE;
						unitPos(x, UNIT_X[x], UNIT_Y[x]);
					}
					if (MINIMAP_OPEN){
						gameMiniMapStats(2);
					}
				#endif
				KEY_NOREPEAT = 1;
				REDRAW_WINDOW = 1;
			} else if (key == (SCAN_F | MODIFIER_SHIFT)){
				#ifndef SHAREWARE
					// Mini-map Proximity
					if (MINIMAP_OPEN){
						MINIMAP_ROBOTS_ENABLED = 0;
						if (MINIMAP_PROXIMITY == 0){
							MINIMAP_PROXIMITY = 1;
							videoInfoPrint("ROBOT PROXIMITY ENABLED!");
						} else{
							MINIMAP_PROXIMITY = 0;
							videoInfoPrint("ROBOT PROXIMITY DISABLED!");
						}
					}
				#endif
				KEY_NOREPEAT = 1;
				REDRAW_WINDOW = 1;
			} else{
				// strcpy(str, "UNKNOWN KEYCODE: ");
				// strcatnum(str, key, 0, 0);
				// videoInfoPrint(str);
				KEY_NOREPEAT = 1;
			}
		}
	}
	// Display GAME OVER
	videoFillRect(96, 72, 88, 24, 0);
	videoPrintString(104, 80, 1, "GAME OVER", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Draw box around GAME OVER text
	videoFillRect(100, 75, 80, 1, VIDEO_FONT_COLOR_MENU_SELECTED);
	videoFillRect(100, 91, 80, 1, VIDEO_FONT_COLOR_MENU_SELECTED);
	videoFillRect(100, 75, 1, 17, VIDEO_FONT_COLOR_MENU_SELECTED);
	videoFillRect(180, 75, 1, 17, VIDEO_FONT_COLOR_MENU_SELECTED);
	// Turn on cursor, to make the game over text strobe
	CURSOR_ENABLED = 1;
	EVENT_ENABLED = 1;
	// Wait for keypress. First see if there is a key held
	keyGet();
	KEY_NOREPEAT = 1;
	KEYTIMER = 50;
	do {
		key = keyGet();
	} while (key == 0);
	// Turn off cursor
	// if (!MINIMAP_OPEN){
	CURSOR_ENABLED = 0;
	// }
	// Display the game over screen. First set the VGA palette
	
	videoFadeOut(VIDEO_PALETTE_GAME, FADE_SPEED);
	videoClearScreen();
	videoPaletteSet(VIDEO_PALETTE_GAMEOVER, 0);
	videoFadeOut(VIDEO_PALETTE_GAMEOVER, FADE_INSTANT);
	gameDrawGameOver();
	videoFadeIn(VIDEO_PALETTE_GAMEOVER, FADE_SPEED);
	// Wait for keypress and then exit back to title screen
	KEY_NOREPEAT = 1;
	do {
		key = keyGet();
	} while (key == 0);
	videoFadeOut(VIDEO_PALETTE_GAMEOVER, FADE_SPEED);
	return 1;
}

int gameInit(){
	unsigned int x, y;

	// Reset screen shake, damage/emp flash, etc.
	EVENT_ENABLED = 0;
	SCREEN_SHAKE = 0;
	DAMAGE_FLASH_STEPS_REMAINING = 0;
	EMP_FLASH_STEPS_REMAINING = 0;

	// Reset flag that a magnet, plasma shot, or big explosion is active
	MAGNET_ACTIVE = 0;
	PLASMA_ACTIVE = 0;
	BIG_EXPLOSION_ACTIVE = 0;

	// Reset inventory
	SELECTED_WEAPON = 0;
	for (x = 0; x < WEAPON_COUNT; x++){
		WEAPON_AMMO[x] = 0;
	}
	SELECTED_ITEM = 0;
	for (x = 0; x < ITEM_COUNT; x++){
		ITEM_INVENTORY[x] = 0;
	}
	// KEY_INVENTORY[0] is always 1. It is used as a comparison to open unlocked doors
	KEY_INVENTORY[0] = 1;
	// The other keys start as 0, until you find the key and then change to 1
	for (x = 1; x < 4; x++){
		KEY_INVENTORY[x] = 0;
	}
	
	// Set the in-game palette and draw the initial game HUD
	videoClearScreen();
	videoPaletteSet(VIDEO_PALETTE_GAME, 0);
	videoFadeOut(VIDEO_PALETTE_GAME, FADE_INSTANT);
	gameDrawHUD();
	gameDisplayWeapon(1);
	gameDisplayItem(1);
	gameDisplayKeys(1);
	
	// Init the cursor
	CURSOR_ENABLED = 0;
	CURSOR.X = 0;
	CURSOR.color_index = 0;
	
	// Load the level map
	videoPrintString(0, 80, 1, "LOADING MAP:", VIDEO_FONT_COLOR_WHITE);
	videoPrintString(96, 80, 1, MAP_NAMES[LEVEL_SELECTED], VIDEO_FONT_COLOR_WHITE);
	videoFadeIn(VIDEO_PALETTE_GAME, FADE_SPEED);
	if (!fileReadMap(LEVEL_SELECTED)) return 0;

	// Load the music for this map
	if (SSY_DEVICE_MUS){
		ssy_music_stop();
		if (fileRead(memorySegToPtr(MUSIC_GAME), MUSIC_FILENAMES[LEVEL_SELECTED & 3], SSY_MUS_EXT, 0)){
			ssy_music_play(MUSIC_GAME);
		}
	}

	// Draw minimap in RAM
	MINIMAP_OPEN = 0;
	MINIMAP_ROBOTS_ENABLED = 0;
	gameMiniMapDraw(MINIMAP);

	// Set initial animations for robot units
	for (x = 1; x < 28; x++){
		if (UNIT_TYPE[x] != TYPE_NONE){
			UNIT_ANIM_BASE[x] = ANIM_INIT[UNIT_TYPE[x]];
			UNIT_ANIM_FRAME[x] = 0;
			UNIT_TIMER_ANIM[x] = 0;
			if (UNIT_ANIM_BASE[x] != 255){
				UNIT_IS_SPRITE[x] = 1;
			} else{
				UNIT_IS_SPRITE[x] = 0;
			}
			unitPos(x, UNIT_X[x], UNIT_Y[x]);
		}
	}
	
	// Clear weapon units
	for (x = 28; x < 32; x++){
		UNIT_TYPE[x] = TYPE_NONE;
	}
	
	// Clear explosion units
	for (x = 64; x < UNIT_COUNT; x++){
		UNIT_TYPE[x] = TYPE_NONE;
	}
	
	gameSetDifficulty();

	// Make sure the player unit is setup correctly
	UNIT_TYPE[UNIT_PLAYER] = TYPE_PLAYER;
	UNIT_IS_SPRITE[UNIT_PLAYER] = 1;
	unitPos(UNIT_PLAYER, UNIT_X[UNIT_PLAYER], UNIT_Y[UNIT_PLAYER]);
	playerAnimate(SPRITE_PLAYER_DOWN, 1);

	// Clear memory of previously plotted tiles within the window
	for (y = 0; y < 7; y++){
		for (x = 0; x < 11; x++){
			PREVIOUS_DRAW_TILE[y][x] = TILE_NOT_PLOTTED;
			PREVIOUS_DRAW_UNIT[y][x] = UNIT_NOT_FOUND;
		}
	}

	// Draw map
	gameMapDraw();

	// Draw player face and health bar
	gameDisplayHealth(1);

	// Print welcome message in the information box
	videoInfoClear();
	videoInfoPrint("WELCOME TO PETSCII ROBOTS!");
	videoInfoPrint("BY DAVID MURRAY 2021");
	videoInfoPrint("MS-DOS PORT BY JIM WRIGHT");

	// Set initial time before we check key input
	KEYTIMER = 30;
	KEYFAST = 0;

	// Reset clock
	CLOCK.ACTIVE = 0;
	CLOCK.CYCLES = 0;
	CLOCK.SECONDS = 0;
	CLOCK.MINUTES = 0;
	CLOCK.HOURS = 0;
	CLOCK.ACTIVE = 1;
	
	return gameMainLoop();
}
