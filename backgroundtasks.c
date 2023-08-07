#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "globals.h"
#include "video.h"
#include "audio.h"
#include "backgroundtasks.h"
#include "keyboard.h"

inline void checkForWindowRedraw(uint8_t unit){
	// If the unit is on the screen (or right outside the drawing area), flag to redraw the window, if it hasn't already been flagged
	if (REDRAW_WINDOW == 0 && UNIT_X[unit] >= MAP_WINDOW_X - 1 && UNIT_X[unit] < MAP_WINDOW_X + 12 && UNIT_Y[unit] >= MAP_WINDOW_Y - 1 && UNIT_Y[unit] < MAP_WINDOW_Y + 8){
		REDRAW_WINDOW = 1;
	}
}

inline void inflictDamage(uint8_t unit, uint8_t damage){
	// This routine will inflict damage on a unit and handle its death
	if (UNIT_HEALTH[unit] == 0){
		// If unit health is already 0, just return
		return;
	}
	if ((int)(UNIT_HEALTH[unit] - damage) <= 0){
		// Unit dead
		UNIT_HEALTH[unit] = 0;
		// Is it the player that is dead, or a robot?
		if (unit != UNIT_PLAYER){
			// Dead robot. If it isn't already dead, then convert to dead robot
			if (UNIT_TYPE[unit] != TYPE_DEAD_ROBOT){
				// Set unit to dead robot
				UNIT_TYPE[unit] = TYPE_DEAD_ROBOT;
				// Timer for how long to show the dead robot
				UNIT_TIMER_A[unit] = 255;
				// Tile for dead robot
				UNIT_ANIM_BASE[unit] = SPRITE_HOVERBOT_DEAD;
				UNIT_ANIM_FRAME[unit] = 0;
				// Update minimap stats
				if (MINIMAP_OPEN){
					gameMiniMapStats(2);
				}
			}
		}
	} else{
		UNIT_HEALTH[unit] -= damage;
	}
	// Draw player health if it is the player unit
	if (unit == UNIT_PLAYER){
		gameDisplayHealth();
		// Flash the screen due to player damage
		DAMAGE_FLASH_STEPS_REMAINING = VIDEO_DAMAGE_FLASH_COUNT;
		EVENT_ENABLED = 1;
	}
}

inline void rollerbotFireDetect(uint8_t unit){
	unsigned int i;
	
	// If player is alive, then attempt to fire
	if (UNIT_HEALTH[UNIT_PLAYER]){
		// If player lined up with rollerbot, then fire
		if (UNIT_Y[unit] == UNIT_Y[UNIT_PLAYER]){
			// Player lined up vertically, attempt to fire horizontally
			if (UNIT_X[unit] > UNIT_X[UNIT_PLAYER]){
				// Player is to the left of the rollerbot, fire if they are close enough
				if (UNIT_X[unit] - UNIT_X[UNIT_PLAYER] < 6){
					// Try to find an available unit to fire with
					for (i = 28; i < 32; i++){
						if (UNIT_TYPE[i] == TYPE_NONE){
							// Found an available unit. Set it as pistol fire left AI
							UNIT_TYPE[i] = TYPE_WEAPON_FIRE_LEFT;
							// Unit sprite
							UNIT_ANIM_BASE[i] = SPRITE_PISTOL_HORIZ;
							UNIT_ANIM_FRAME[i] = 0;
							UNIT_IS_SPRITE[i] = 1;
							// Fire distance
							UNIT_A[i] = 5;
							// Weapon type = pistol
							UNIT_B[i] = 0;
							// Reset unit timer
							UNIT_TIMER_A[i] = 0;
							// Unit position
							unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
							// PLAY PISTOL SOUND
							ssy_sound_play(FIRE_PISTOL);
							break;
						}
					}
				}
			} else{
				// Player is to the right of the rollerbot, fire if they are close enough
				if (UNIT_X[UNIT_PLAYER] - UNIT_X[unit] < 6){
					// Try to find an available unit to fire with
					for (i = 28; i < 32; i++){
						if (UNIT_TYPE[i] == TYPE_NONE){
							// Found an available unit. Set it as pistol fire right AI
							UNIT_TYPE[i] = TYPE_WEAPON_FIRE_RIGHT;
							// Unit tile
							UNIT_ANIM_BASE[i] = SPRITE_PISTOL_HORIZ;
							UNIT_ANIM_FRAME[i] = 0;
							UNIT_IS_SPRITE[i] = 1;
							// Fire distance
							UNIT_A[i] = 5;
							// Weapon type = pistol
							UNIT_B[i] = 0;
							// Reset unit timer
							UNIT_TIMER_A[i] = 0;
							// Unit position
							unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
							// PLAY PISTOL SOUND
							ssy_sound_play(FIRE_PISTOL);
							break;
						}
					}
				}
			}
		} else if (UNIT_X[unit] == UNIT_X[UNIT_PLAYER]){
			// Player lined up horizontally, attempt to fire vertically
			if (UNIT_Y[unit] > UNIT_Y[UNIT_PLAYER]){
				// Player is above the rollerbot, fire if they are close enough
				if (UNIT_Y[unit] - UNIT_Y[UNIT_PLAYER] < 4){
					// Try to find an available unit to fire with
					for (i = 28; i < 32; i++){
						if (UNIT_TYPE[i] == TYPE_NONE){
							// Found an available unit. Set it as pistol fire up AI
							UNIT_TYPE[i] = TYPE_WEAPON_FIRE_UP;
							// Unit tile
							UNIT_ANIM_BASE[i] = SPRITE_PISTOL_VERT;
							UNIT_ANIM_FRAME[i] = 0;
							UNIT_IS_SPRITE[i] = 1;
							// Fire distance
							UNIT_A[i] = 5;
							// Weapon type = pistol
							UNIT_B[i] = 0;
							// Reset unit timer
							UNIT_TIMER_A[i] = 0;
							// Unit position
							unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
							// PLAY PISTOL SOUND
							ssy_sound_play(FIRE_PISTOL);
							break;
						}
					}
				}
			} else{
				// Player is below the rollerbot, fire if they are close enough
				if (UNIT_Y[UNIT_PLAYER] - UNIT_Y[unit] < 4){
					// Try to find an available unit to fire with
					for (i = 28; i < 32; i++){
						if (UNIT_TYPE[i] == TYPE_NONE){
							// Found an available unit. Set it as pistol fire down AI
							UNIT_TYPE[i] = TYPE_WEAPON_FIRE_DOWN;
							// Unit tile
							UNIT_ANIM_BASE[i] = SPRITE_PISTOL_VERT;
							UNIT_ANIM_FRAME[i] = 0;
							UNIT_IS_SPRITE[i] = 1;
							// Fire distance
							UNIT_A[i] = 5;
							// Weapon type = pistol
							UNIT_B[i] = 0;
							// Reset unit timer
							UNIT_TIMER_A[i] = 0;
							// Unit position
							unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
							// PLAY PISTOL SOUND
							ssy_sound_play(FIRE_PISTOL);
							break;
						}
					}
				}
			}
		}
	}
}

inline void pistolFireCommon(uint8_t unit){
	uint8_t hitunit;
	
	// Decrease range of weapon
	UNIT_A[unit]--;
	// Check weapon type. 0 = pistol, 1 = plasma
	if (UNIT_B[unit] == 0){
		// Check if weapon hit explosive cannister
		if (MAP[UNIT_Y[unit]][UNIT_X[unit]] == TILE_EXP_CANNISTER){
			// Replace with blown cannister
			mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_BLOWN_CANNISTER);
			// Change this unit to a bomb
			UNIT_TYPE[unit] = TYPE_TIMEBOMB;
			// Set unit tile to explosive cannister
			UNIT_TILE[unit] = TILE_EXP_CANNISTER;
			// Set timer until explosion
			UNIT_TIMER_A[unit] = 5;
			UNIT_A[unit] = 0;
		} else{
			// Check if weapon hit a unit or an object it can't pass through
			if ((hitunit = checkForUnit(UNIT_X[unit], UNIT_Y[unit])) != UNIT_NOT_FOUND || !(tileHasAttribute(UNIT_X[unit], UNIT_Y[unit], ATTRIB_CAN_SEE_THROUGH))){
				// Object hit, convert unit to explosion
				UNIT_TYPE[unit] = TYPE_EXPLOSION_SMALL;
				// Set sprite to first explosion frame
				UNIT_ANIM_BASE[unit] = SPRITE_EXPLOSION;
				UNIT_ANIM_FRAME[unit] = 0;
				UNIT_IS_SPRITE[unit] = 1;
				// Reset timer
				UNIT_TIMER_A[unit] = 1;
				// If a unit was hit, damage the unit
				if (hitunit != UNIT_NOT_FOUND){
					inflictDamage(hitunit, 1);
					// If hit unit is a hoverbot, convert it to attack mode
					if (UNIT_TYPE[hitunit] == TYPE_HOVERBOT_HORIZ || UNIT_TYPE[hitunit] == TYPE_HOVERBOT_VERT){
						UNIT_TYPE[hitunit] = TYPE_HOVERBOT_ATTACK;
					}
				}
			}
		}
	} else{
		// Plasma weapon
		// Check if weapon hit explosive cannister
		if (MAP[UNIT_Y[unit]][UNIT_X[unit]] == TILE_EXP_CANNISTER){
			// Replace with blown cannister
			mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_BLOWN_CANNISTER);
			// Change this unit to a bomb
			UNIT_TYPE[unit] = TYPE_TIMEBOMB;
			// Set unit tile to explosive cannister
			UNIT_TILE[unit] = TILE_EXP_CANNISTER;
			UNIT_IS_SPRITE[unit] = 0;
			// Set timer until explosion
			UNIT_TIMER_A[unit] = 1;
			UNIT_A[unit] = 0;
			PLASMA_ACTIVE = 0;
		} else{
			// Check if weapon hit a unit or an object it can't pass through
			if ((hitunit = checkForUnit(UNIT_X[unit], UNIT_Y[unit])) != UNIT_NOT_FOUND || !(tileHasAttribute(UNIT_X[unit], UNIT_Y[unit], ATTRIB_CAN_SEE_THROUGH))){
				// Object hit. Change this unit to a bomb
				UNIT_TYPE[unit] = TYPE_TIMEBOMB;
				// Set unit tile to explosive cannister
				UNIT_TILE[unit] = MAP[UNIT_Y[unit]][UNIT_X[unit]];
				UNIT_IS_SPRITE[unit] = 0;
				// Set timer until explosion
				UNIT_TIMER_A[unit] = 1;
				UNIT_A[unit] = 0;
				PLASMA_ACTIVE = 0;
			}
		}
	}
}

inline void drawTrashCompactor(uint8_t unit, uint8_t tile1, uint8_t tile2, uint8_t tile3, uint8_t tile4){
	// Plot the tiles for the trash compactor
	mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, tile1);
	mapSet(UNIT_X[unit] + 1, UNIT_Y[unit] - 1, tile2);
	mapSet(UNIT_X[unit], UNIT_Y[unit], tile3);
	mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], tile4);
	// Redraw screen in unit is on it
	checkForWindowRedraw(unit);
}

inline int doorCheckProximity(uint8_t unit){
	// Check if player next to door unit
	if (UNIT_X[UNIT_PLAYER] >= UNIT_X[unit] - 1 && UNIT_X[UNIT_PLAYER] <= UNIT_X[unit] + 1){
		if (UNIT_Y[UNIT_PLAYER] >= UNIT_Y[unit] - 1 && UNIT_Y[UNIT_PLAYER] <= UNIT_Y[unit] + 1){
			// Player is next to door
			return 1;
		}
	}
	// Player is not next to door
	return 0;
}

inline int explosionCreateUnit(uint8_t x, uint8_t y, uint8_t unit, uint8_t forceExplosion){
	uint8_t hitunit;
	
	// Create small explosion unit from an explosion
	if ((hitunit = checkForUnit(x, y)) != UNIT_NOT_FOUND || forceExplosion || tileHasAttribute(x, y, ATTRIB_CAN_SEE_THROUGH)){
		// Enable explosion unit if unit hit or can see through tile
		UNIT_TYPE[unit] = TYPE_EXPLOSION_SMALL;
		// Set sprite to first explosion frame
		UNIT_ANIM_BASE[unit] = SPRITE_EXPLOSION;
		UNIT_ANIM_FRAME[unit] = 0;
		UNIT_IS_SPRITE[unit] = 1;
		// Set coordinates of explosion
		unitPos(unit, x, y);
		// Set timer for explosion
		UNIT_TIMER_A[unit] = 1;
		// Inflict damage if a unit is hit
		if (hitunit != UNIT_NOT_FOUND){
			inflictDamage(hitunit, 11);
		}
		// Set explosive, return success
		return 1;
	}
	// Could not set explosive here, return failure
	return 0;
}

inline void explosion(uint8_t unit){
	uint8_t hitunit;
	
	// This is the large explosion used by the time-bomb and plasma gun.
	if (UNIT_A[unit] == 0){
		// Phase 1, which creates small explosions all around the big explosion
		if (BIG_EXPLOSION_ACTIVE){
			// Another explosion is active, wait for it to finish
			UNIT_TIMER_A[unit] = 10;
		} else{
			// Set flag that an explosion is active, so no other explosions can start until this one ends
			BIG_EXPLOSION_ACTIVE = 1;
			// Set number of frames to shake the screen
			SCREEN_SHAKE = SCREEN_SHAKE_COUNT;
			EVENT_ENABLED = 1;
			// PLAY SOUND_EXPLOSION
			ssy_sound_play(EXPLOSION2);
			// Update cannister to be exploded when explosion starts
			if (UNIT_TILE[unit] == TILE_EXP_CANNISTER){
				UNIT_TILE[unit] = TILE_BLOWN_CANNISTER;
				REDRAW_WINDOW = 1;
			}
			// Explosions Center
			explosionCreateUnit(UNIT_X[unit], UNIT_Y[unit], 64, 1);
			// Explosions North.
			if (explosionCreateUnit(UNIT_X[unit], UNIT_Y[unit] - 1, 65, 0)){
				explosionCreateUnit(UNIT_X[unit], UNIT_Y[unit] - 2, 66, 0);
			}
			// Explosions South
			if (explosionCreateUnit(UNIT_X[unit], UNIT_Y[unit] + 1, 67, 0)){
				explosionCreateUnit(UNIT_X[unit], UNIT_Y[unit] + 2, 68, 0);
			}
			// Explosions West
			if (explosionCreateUnit(UNIT_X[unit] - 1, UNIT_Y[unit], 69, 0)){
				explosionCreateUnit(UNIT_X[unit] - 2, UNIT_Y[unit], 70, 0);
			}
			// Explosions East
			if (explosionCreateUnit(UNIT_X[unit] + 1, UNIT_Y[unit], 71, 0)){
				explosionCreateUnit(UNIT_X[unit] + 2, UNIT_Y[unit], 72, 0);
			}
			// Explosions North West
			if (explosionCreateUnit(UNIT_X[unit] - 1, UNIT_Y[unit] - 1, 73, 0)){
				explosionCreateUnit(UNIT_X[unit] - 2, UNIT_Y[unit] - 2, 74, 0);
			}
			// Explosions South East
			if (explosionCreateUnit(UNIT_X[unit] + 1, UNIT_Y[unit] + 1, 75, 0)){
				explosionCreateUnit(UNIT_X[unit] + 2, UNIT_Y[unit] + 2, 76, 0);
			}
			// Explosions South West
			if (explosionCreateUnit(UNIT_X[unit] - 1, UNIT_Y[unit] + 1, 77, 0)){
				explosionCreateUnit(UNIT_X[unit] - 2, UNIT_Y[unit] + 2, 78, 0);
			}
			// Explosions North East
			if (explosionCreateUnit(UNIT_X[unit] + 1, UNIT_Y[unit] - 1, 79, 0)){
				explosionCreateUnit(UNIT_X[unit] + 2, UNIT_Y[unit] - 2, 80, 0);
			}
			// Move to next phase of explosion
			UNIT_A[unit]++;
			// Set timer until big explosion is finished
			UNIT_TIMER_A[unit] = 25;
		}
	} else{
		// Phase 2, Deactivate this AI
		UNIT_TYPE[unit] = TYPE_NONE;
		// Update the map
		unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
		// Clear big explosion flag
		BIG_EXPLOSION_ACTIVE = 0;
	}
}

inline void doorOpenA(uint8_t unit){
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
		// Horizontal Elevator Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_R);
	} else if (UNIT_A[unit] == 1){
		// Vertical Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_TRANS2_T);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_VERT_TRANS2_M);
		mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_TRANS2_B);
	} else{
		// Horizontal Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS_R);
	}
	// Save door state
	UNIT_B[unit] = 1;
	// Reset door animation timer
	UNIT_TIMER_A[unit] = 6;
	// Flag to redraw window if unit is on screen
	checkForWindowRedraw(unit);
}

inline void doorOpenB(uint8_t unit){
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
		// Horizontal Elevator Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_ELEV_HORIZ_OPEN_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_FLOOR);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_OPEN_R);
	} else if (UNIT_A[unit] == 1){
		// Vertical Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_OPEN_T);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_FLOOR);
		mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_OPEN_B);
	} else{
		// Horizontal Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_OPEN_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_FLOOR);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_OPEN_R);
	}
	// Save door state
	UNIT_B[unit] = 2;
	// Reset door animation timer
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
		UNIT_TIMER_A[unit] = 50;
	} else{
		UNIT_TIMER_A[unit] = 30;
	}
	// Flag to redraw window if unit is on screen
	checkForWindowRedraw(unit);
}

inline void doorOpenFull(uint8_t unit){
	// Check if the nobody is near the door or it is an elevator
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR || !doorCheckProximity(unit)){
		// Nobody near door. Lets close it if the tile matches the floor tile and no unit in the way
		if (MAP[UNIT_Y[unit]][UNIT_X[unit]] == TILE_FLOOR && checkForUnit(UNIT_X[unit], UNIT_Y[unit]) == UNIT_NOT_FOUND){
			// It was a floor tile. Start door closing process
			// PLAY DOOR SOUND
			ssy_sound_play(DOOR);
			if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
				// Horizontal Elevator Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_L);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_M);
				mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_R);
			} else if (UNIT_A[unit] == 1){
				// Vertical Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_TRANS2_T);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_VERT_TRANS2_M);
				mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_TRANS2_B);
			} else{
				// Horizontal Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_L);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS2_M);
				mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS_R);
			}
			// Save door state
			UNIT_B[unit] = 3;
			// Reset door animation timer
			UNIT_TIMER_A[unit] = 6;
			// Flag to redraw window if unit is on screen
			checkForWindowRedraw(unit);
			return;
		}
		// Door not closing due to being blocked, reset timer
		UNIT_TIMER_A[unit] = 35;
		return;
	}
	// Door not closing due to player proximity, reset timer
	UNIT_TIMER_A[unit] = 30;
}

inline void doorCloseA(uint8_t unit){
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
		// Horizontal Elevator Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_R);
	} else if (UNIT_A[unit] == 1){
		// Vertical Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_TRANS1_T);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_VERT_TRANS1_M);
		mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_TRNAS1_B);
	} else{
		// Horizontal Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS_R);
	}
	// Save door state
	UNIT_B[unit] = 4;
	// Reset door animation timer
	UNIT_TIMER_A[unit] = 6;
	// Flag to redraw window if unit is on screen
	checkForWindowRedraw(unit);
}

inline void doorCloseB(uint8_t unit){
	char floors[10], highlighted[2], i;
	int key, currentfloor, color;
	
	currentfloor = -1;
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
		// Horizontal Elevator Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_CLOSED_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_CLOSED_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_CLOSED_R);
	} else if (UNIT_A[unit] == 1){
		// Vertical Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_CLOSED_T);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_VERT_CLOSED_M);
		mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_CLOSED_B);
	} else{
		// Horizontal Door. Modify the tiles on the map for the door animation
		mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_CLOSED_L);
		mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_CLOSED_M);
		mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_CLOSED_R);
	}
	// Save door state
	UNIT_B[unit] = 5;
	// Reset door animation timer
	UNIT_TIMER_A[unit] = 6;
	// Flag to redraw window if unit is on screen
	checkForWindowRedraw(unit);
	// If this is an elevator, show panel
	if (UNIT_TYPE[unit] == TYPE_ELEVATOR && UNIT_HEALTH[UNIT_PLAYER] != 0){
		// Check if the player is within the elevator
		if (UNIT_X[UNIT_PLAYER] == UNIT_X[unit] && UNIT_Y[UNIT_PLAYER] == UNIT_Y[unit] - 1){
			// Player within elevator. Reset player animation and face downward
			playerAnimate(SPRITE_PLAYER_DOWN, 1);
			// Print elevator message
			videoInfoPrint("] ELEVATOR PANEL ]  DOWN");
			videoInfoPrint("]  SELECT LEVEL  ]  OPENS");
			// Build and print string containing number of floors
			videoInfoPrint("]                ]  DOOR");
			floors[0] = 0;
			highlighted[1] = 0;
			for (i = 0; i < UNIT_D[unit]; i++){
				floors[i] = '1' + i;
				floors[i + 1] = 0;
			}
			videoPrintString(56, 192, 1, floors, VIDEO_FONT_COLOR_MENU);
			// Turn on the cursor to make the floor number strobe colors, but having Y at 0 makes it not draw the cursor box on the screen
			CURSOR_ENABLED = 1;
			EVENT_ENABLED = 1;
			// Loop while in the elevator, getting user input for selecting floor
			do{
				// Unhighlight all floors if it has changed
				if (currentfloor == -1 || currentfloor != UNIT_C[unit]){
					videoPrintString(56, 192, 1, floors, VIDEO_FONT_COLOR_MENU);
				}
				// Highlight current floor
				highlighted[0] = floors[UNIT_C[unit] - 1];
				// Update the selected floor color when the interrupt changes it
				if (currentfloor != UNIT_C[unit] || CURSOR.color != CURSOR.last_color){
					if (VIDEO_USE_PALETTE_ANIMATION){
						color = VIDEO_FONT_COLOR_MENU_SELECTED;
					} else{
						color = CURSOR.color;
					}
					videoPrintString(48 + (UNIT_C[unit] << 3), 192, 1, highlighted, color);
				}
				currentfloor = UNIT_C[unit];
				// Redraw window
				REDRAW_WINDOW = 1;
				gameMapDraw();
				// Make clock inactive while waiting for the floor to be picked, so it counts down the keytimer
				CLOCK.ACTIVE = 0;
				// Wait for next key press
				key = keyGet();
				if (key == 0){
					key = 1;
				}
				if (key == CUSTOM_KEYS[KEYS_MOVE_RIGHT] || key == SCAN_RIGHT_ARROW || key == SCAN_KP_6){
					// Move right (up a floor)
					if (UNIT_C[unit] < UNIT_D[unit]){
						// Find next elevator entry that is one floor higher
						for (i = 32; i < 48; i++){
							if (UNIT_TYPE[i] == TYPE_ELEVATOR && UNIT_C[i] == UNIT_C[unit] + 1){
								// Set the current unit to the found elevator floor
								unit = i;
								// Set player position to the elevator entrance
								unitPos(UNIT_PLAYER, UNIT_X[unit], UNIT_Y[unit] - 1);
								MAP_WINDOW_X = UNIT_X[UNIT_PLAYER] - 5;
								MAP_WINDOW_Y = UNIT_Y[UNIT_PLAYER] - 3;
								// Play sound when moving to next floor
								ssy_sound_play(CYCLE_ITEM);
								KEY_NOREPEAT = 1;
								break;
							}
						}
					}
				} else if (key == CUSTOM_KEYS[KEYS_MOVE_LEFT] || key == SCAN_LEFT_ARROW || key == SCAN_KP_4){
					// Move left (down a floor)
					if (UNIT_C[unit] > 1){
						// Find next elevator entry that is one floor lower
						for (i = 32; i < 48; i++){
							if (UNIT_TYPE[i] == TYPE_ELEVATOR && UNIT_C[i] == UNIT_C[unit] - 1){
								// Set the current unit to the found elevator floor
								unit = i;
								// Set player position to the elevator entrance
								unitPos(UNIT_PLAYER, UNIT_X[unit], UNIT_Y[unit] - 1);
								MAP_WINDOW_X = UNIT_X[UNIT_PLAYER] - 5;
								MAP_WINDOW_Y = UNIT_Y[UNIT_PLAYER] - 3;
								// Play sound when moving to next floor
								ssy_sound_play(CYCLE_ITEM);
								KEY_NOREPEAT = 1;
								break;
							}
						}
					}
				} else if (key == CUSTOM_KEYS[KEYS_MOVE_DOWN] || key == SCAN_DOWN_ARROW || key == SCAN_KP_2){
					// Move down (exit elevator)
					// Flag that player has exited the elevator
					key = 0;
					// Clear the elevator text in the info box
					videoInfoClear();
					// Reactivate the clock
					CLOCK.ACTIVE = 1;
				}
			} while (key != 0);
			// Disable the cursor
			if (!MINIMAP_OPEN){
				CURSOR_ENABLED = 0;
			}
		}
	}
}

inline void doorCloseFull(uint8_t unit){
	// Check if the player is near the door
	if (doorCheckProximity(unit)){
		// Player is near door. Check if the door is an elevator or unlocked (key 0) or player has the key (key 1 - 3)
		if (UNIT_TYPE[unit] == TYPE_ELEVATOR || KEY_INVENTORY[UNIT_C[unit]] == 1){
			// Start door opening process
			// PLAY DOOR SOUND
			ssy_sound_play(DOOR);
			if (UNIT_TYPE[unit] == TYPE_ELEVATOR){
				// Horizontal Elevator Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_L);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_M);
				mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_ELEV_HORIZ_TRANS2_R);
			} else if (UNIT_A[unit] == 1){
				// Vertical Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit], UNIT_Y[unit] - 1, TILE_DOOR_VERT_TRANS1_T);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_VERT_TRANS1_M);
				mapSet(UNIT_X[unit], UNIT_Y[unit] + 1, TILE_DOOR_VERT_TRNAS1_B);
			} else{
				// Horizontal Door. Modify the tiles on the map for the door animation
				mapSet(UNIT_X[unit] - 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_L);
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS1_M);
				mapSet(UNIT_X[unit] + 1, UNIT_Y[unit], TILE_DOOR_HORIZ_TRANS_R);
			}
			// Save door state
			UNIT_B[unit] = 0;
			// Reset door animation timer
			UNIT_TIMER_A[unit] = 6;
			// Flag to redraw window if unit is on screen
			checkForWindowRedraw(unit);
			return;
		}
	}
	// Door not opening, reset timer
	UNIT_TIMER_A[unit] = 20;
}

inline void magnetizedRobot(uint8_t unit){
	// Get a random number between 0 and 3 and randomly move the robot that direction
	switch (rand() & 3){
		case 0:{ // Up
			requestWalkUp(unit, ATTRIB_CAN_WALK_OVER);
			break;
		}
		case 1:{ // Down
			requestWalkDown(unit, ATTRIB_CAN_WALK_OVER);
			break;
		}
		case 2:{ // Left
			requestWalkLeft(unit, ATTRIB_CAN_WALK_OVER);
			break;
		}
		case 3:{ // Right
			requestWalkRight(unit, ATTRIB_CAN_WALK_OVER);
			break;
		}
	}
	// Update timer until next movement
	UNIT_TIMER_A[unit] = 12;
	// Decrease timer for how long until the effect wears off
	UNIT_TIMER_B[unit]--;
	if (UNIT_TIMER_B[unit] == 0){
		// Effect should wear off now, unless the robot has been affected by 2 magnets at once
		UNIT_TYPE[unit] = UNIT_D[unit];
	}
	// Flag to redraw window if unit is on screen
	checkForWindowRedraw(unit);	
}

inline void magnet(uint8_t unit){
	uint8_t foundunit;
	
	// First let's take care of the timers. This unit runs every cycle so it can detect contact with another
	// unit. But it still needs to count down to termninate, so it uses two timers for a 16-bit value.
	UNIT_TIMER_B[unit]--;
	if (UNIT_TIMER_B[unit] == 0){
		UNIT_A[unit]--;
		if (UNIT_A[unit] == 0){
			// Both timers have reached 0. Time to deactivate the magnet.
			UNIT_TYPE[unit] = TYPE_NONE;
			// Update map
			unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
			// Reset the flag that the magnet is active
			MAGNET_ACTIVE = 0;
			// Flag to redraw the screen if this unit is on it
			checkForWindowRedraw(unit);
			return;
		}
		UNIT_TIMER_B[unit] = 255;
	}
	// See if any units are on top of the magnet
	if ((foundunit = checkForUnit(UNIT_X[unit], UNIT_Y[unit])) != UNIT_NOT_FOUND){
		if (foundunit == UNIT_PLAYER){
			// The player stepped onto the magnet. Pick it up and add it to inventory
			ITEM_INVENTORY[MAGNET - 1]++;
			// Update the inventory display if needed
			gameDisplayItem(0);
		} else{
			// A robot stepped onto the magnet. Make a backup of the unit type
			UNIT_D[foundunit] = UNIT_TYPE[foundunit];
			// Set unit to act crazy
			UNIT_TYPE[foundunit] = TYPE_MAGNETIZED_ROBOT;
			// PLAY SOUND_MAGNET
			ssy_sound_play(USE_MAGNET);
			// Set unit timer
			UNIT_TIMER_B[foundunit] = 60;
		}
		// Deactivate the magnet, since a unit stepped on it
		UNIT_TYPE[unit] = TYPE_NONE;
		// Update map
		unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
		// Reset the flag that the magnet is active
		MAGNET_ACTIVE = 0;
		// Flag to redraw the screen if this unit is on it
		checkForWindowRedraw(unit);
	}
}

inline void transporter(uint8_t unit){
	uint8_t i;
	//UNIT_A: 0=always active	1=only active when all robots are dead
	//UNIT_B: 0=completes level 1=send to coordinates
	//UNIT_C: X-coordinate
	//UNIT_D: Y-coordinate
	
	// Is the player standing on the transporter?
	if (UNIT_X[unit] == UNIT_X[UNIT_PLAYER] && UNIT_Y[unit] == UNIT_Y[UNIT_PLAYER]){
		if (UNIT_A[unit] != 0){
			// Transporter pad not active yet
			videoInfoPrint("TRANSPORTER WILL NOT ACTIVATE");
			videoInfoPrint("UNTIL ALL ROBOTS DESTROYED.");
			// PLAY SOUND_ERROR
			ssy_sound_play(ERROR);
			// Reset time until next check
			UNIT_TIMER_A[unit] = 100;
		} else{
			// Start transport process. Convert to dematerialize AI
			UNIT_TYPE[unit] = TYPE_DEMATERIALIZE;
			// Set timer to start
			UNIT_TIMER_A[unit] = 5;
			// PLAY SOUND_TELEPORT
			// Set player to first frame of dematerializing
			UNIT_ANIM_BASE[UNIT_PLAYER] = SPRITE_DEMATERIALIZE;
			UNIT_ANIM_FRAME[UNIT_PLAYER] = 0;
			// Flag to redraw window
			REDRAW_WINDOW = 1;
		}
	} else{
		// Player not present. Check if transporter pad is only active when all robots are dead
		if (UNIT_A[unit] == 1){
			// Check if all robots are dead
			for (i = 1; i < 28; i++){
				if (UNIT_TYPE[i] != TYPE_NONE){
					// Found a robot alive, reset timer before we test again
					UNIT_TIMER_A[unit] = 30;
					return;
				}
			}
			// All robots dead. Make trasporter pad active
			UNIT_A[unit] = 0;
		} else{
			// Trasporter is active. Animate the tile
			if (UNIT_TIMER_B[unit] == 1){
				UNIT_TIMER_B[unit] = 0;
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_TRANSPORTER_ANIM_2);
			} else{
				UNIT_TIMER_B[unit] = 1;
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_TRANSPORTER_ANIM_1);
			}
			// Check if unit is on screen to see if we should redraw window
			checkForWindowRedraw(unit);
			// Set time before next animation
			UNIT_TIMER_A[unit] = 30;
		}
	}
}

void backgroundTasks(){
	uint8_t i, unit, map_update, temp;
	
	// See if it is time to run background tasks
	if (BGTIMER1 == 0){
		return;
	}
	BGTIMER1 = 0;
	// Save flag if we should update the colors of the units on the minimap.
	if (CURSOR_ENABLED){
		map_update = 0;
		if (!VIDEO_USE_PALETTE_ANIMATION){
			// Modify cursor color
			if (CURSOR.color != CURSOR.last_color){
				map_update = 1;
				REDRAW_WINDOW = 1;
			}
		}
		// Update cursor on map if it is enabled
		if (MINIMAP_OPEN && CURSOR.X != 0){
			if (CLOCK.CYCLES & 1){
				temp = VIDEO_FONT_COLOR_WHITE;
			} else{
				temp = 0;
			}
			minimapSet(MAP_WINDOW_X + CURSOR.X, MAP_WINDOW_Y + CURSOR.Y, temp);
		}
		// Update clock on minimap if flagged to do so
		if (CLOCK.UPDATE){
			gameMiniMapStats(0);
		}
	}
	// Loop through the units and handle AI routines
	for (unit = 0; unit < UNIT_COUNT; unit++){
		// Skip unit if it does not exist
		if (UNIT_TYPE[unit] == TYPE_NONE){
			continue;
		}
		// If the mini-map is open and not VGA mode and cursor color has changed, draw the unit on the map
		if (!(unit & 32) && map_update){
			if (MINIMAP_ROBOTS_ENABLED || unit == UNIT_PLAYER || unit > 27 || (MINIMAP_PROXIMITY && UNIT_X[unit] >= MAP_WINDOW_X && UNIT_X[unit] < MAP_WINDOW_X + 12 && UNIT_Y[unit] >= MAP_WINDOW_Y && UNIT_Y[unit] < MAP_WINDOW_Y + 8)){
				unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
			}
		}
		if (unit == UNIT_PLAYER){
			continue;
		}
		// Unit exists, check it's timer. Code won't run until it reaches 0
		if (UNIT_TIMER_A[unit] != 0){
			UNIT_TIMER_A[unit]--;
			continue;
		}
		// Unit exists and timer has triggered. The unit type determines which AI routine is run.
		switch (UNIT_TYPE[unit]){
			case TYPE_HOVERBOT_HORIZ:{  // LEFT/RIGHT DROID
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 12;
				// Check to see if we should animate the droid yet
				unitAnimate(unit);
				// Get direction. 0 = left, 1 = right
				if (UNIT_A[unit] == 1){
					// Moving right
					if (!requestWalkRight(unit, ATTRIB_CAN_HOVER_OVER)){
						// Blocked, change direction to left
						UNIT_A[unit] = 0;
					}
				} else{
					// Moving left
					if (!requestWalkLeft(unit, ATTRIB_CAN_HOVER_OVER)){
						// Blocked, change direction to right
						UNIT_A[unit] = 1;
					}
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_HOVERBOT_VERT:{  // UP/DOWN DROID
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 12;
				// Check to see if we should animate the droid yet
				unitAnimate(unit);
				// Get direction. 0 = up, 1 = down
				if (UNIT_A[unit] == 1){
					// Moving down
					if (!requestWalkDown(unit, ATTRIB_CAN_HOVER_OVER)){
						// Blocked, change direction to up
						UNIT_A[unit] = 0;
					}
				} else{
					// Moving up
					if (!requestWalkUp(unit, ATTRIB_CAN_HOVER_OVER)){
						// Blocked, change direction to down
						UNIT_A[unit] = 1;
					}
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_HOVERBOT_ATTACK:{  // HOVER ATTACK
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 9;
				// Reset animation timer for this unit
				UNIT_TIMER_B[unit] = 0;
				// Animate droid
				unitAnimate(unit);
				// Move toward player if possible
				if (UNIT_X[unit] < UNIT_X[UNIT_PLAYER]){
					// Move Right
					requestWalkRight(unit, ATTRIB_CAN_HOVER_OVER);
				} else if (UNIT_X[unit] > UNIT_X[UNIT_PLAYER]){
					// Move Left
					requestWalkLeft(unit, ATTRIB_CAN_HOVER_OVER);
				}
				if (UNIT_Y[unit] < UNIT_Y[UNIT_PLAYER]){
					// Move Down
					requestWalkDown(unit, ATTRIB_CAN_HOVER_OVER);
				} else if (UNIT_Y[unit] > UNIT_Y[UNIT_PLAYER]){
					// Move Up
					requestWalkUp(unit, ATTRIB_CAN_HOVER_OVER);
				}
				// Check if hoverbot is next to player
				if (UNIT_X[unit] == UNIT_X[UNIT_PLAYER] || UNIT_Y[unit] == UNIT_Y[UNIT_PLAYER]){
					if ((UNIT_X[unit] >= UNIT_X[UNIT_PLAYER] - 1 && UNIT_X[unit] <= UNIT_X[UNIT_PLAYER] + 1) && (UNIT_Y[unit] >= UNIT_Y[UNIT_PLAYER] - 1 && UNIT_Y[unit] <= UNIT_Y[UNIT_PLAYER] + 1)){
						// Hoverbot is next to the player, damage player
						inflictDamage(UNIT_PLAYER, 1);
						// PLAY SOUND_SHOCK
						ssy_sound_play(SHOCK);
						// Attempt to create small explosion unit on top of player
						for (i = 28; i < 32; i++){
							if (UNIT_TYPE[i] == TYPE_NONE){
								// Found an available unit. Set it to a small explosion
								UNIT_TYPE[i] = TYPE_EXPLOSION_SMALL;
								// Set sprite on explosion
								UNIT_ANIM_BASE[i] = SPRITE_EXPLOSION;
								UNIT_ANIM_FRAME[i] = 0;
								UNIT_IS_SPRITE[i] = 1;
								// Set location of explosion
								unitPos(i, UNIT_X[UNIT_PLAYER], UNIT_Y[UNIT_PLAYER]);
								// Set timer on explosion
								UNIT_TIMER_A[i] = 1;
								break;
							}
						}
						// Set rate of attack on player
						UNIT_TIMER_A[unit] = 37;
					}
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_HOVERBOT_WATER:{  // DROID DEATH IN WATER
				// Animate tile of robot in water
				UNIT_TILE[unit]++;
				if (UNIT_TILE[unit] > TILE_ROBOT_WATER_ANIM_END){
					UNIT_TILE[unit] = TILE_ROBOT_WATER_ANIM_START;
				}
				// Kill robot after timer countdown reaches zero
				UNIT_A[unit]--;
				if (UNIT_A[unit] == 0){
					UNIT_TYPE[unit] = TYPE_DEAD_ROBOT;
					UNIT_TIMER_A[unit] = 255;
					UNIT_ANIM_BASE[unit] = SPRITE_HOVERBOT_DEAD;
					UNIT_ANIM_FRAME[unit] = 0;
					UNIT_IS_SPRITE[unit] = 1;
					// Update minimap stats
					if (MINIMAP_OPEN){
						gameMiniMapStats(2);
					}
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_TIMEBOMB:{  // TIME BOMB
				explosion(unit);
				break;
			}
			case TYPE_TRANSPORTER_PAD:{  // TRANSPORTER PAD
				transporter(unit);
				break;
			}
			case TYPE_DEAD_ROBOT:{  // DEAD ROBOT
				// Remove dead robot
				UNIT_TYPE[unit] = TYPE_NONE;
				// Update map
				unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_EVILBOT:{  // EVILBOT
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 6;
				// First animate unit
				UNIT_TIMER_ANIM[unit] = 0;
				unitAnimate(unit);
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				// Decrement move timer for Evilbot until it reaches 0
				if (UNIT_TIMER_B[unit] != 0){
					UNIT_TIMER_B[unit]--;
				} else{
					// Reset move timer for Evilbot
					UNIT_TIMER_B[unit] = 1;
					// Save a flag 'temp', which will indicate if Evilbot successfully moved
					temp = 0;
					// Compare evil bot to player to see which way he should walk
					if (UNIT_X[unit] < UNIT_X[UNIT_PLAYER]){
						// Attempt to walk right
						if (requestWalkRight(unit, ATTRIB_CAN_WALK_OVER)){
							// Successfully walked. Set base animation to that direction
							UNIT_ANIM_BASE[unit] = SPRITE_EVILBOT_RIGHT;
							// Flag to animate Evilbot
							temp = 1;
						}
					} else if (UNIT_X[unit] > UNIT_X[UNIT_PLAYER]){
						// Attempt to walk left
						if (requestWalkLeft(unit, ATTRIB_CAN_WALK_OVER)){
							// Successfully walked. Set base animation to that direction
							UNIT_ANIM_BASE[unit] = SPRITE_EVILBOT_LEFT;
							// Flag to animate Evilbot
							temp = 1;
						}
					}
					if (UNIT_Y[unit] < UNIT_Y[UNIT_PLAYER]){
						// Attempt to walk down
						if (requestWalkDown(unit, ATTRIB_CAN_WALK_OVER)){
							// Successfully walked. Set base animation to that direction
							UNIT_ANIM_BASE[unit] = SPRITE_EVILBOT_DOWN;
							// Flag to animate Evilbot
							temp = 1;
						}
					} else if (UNIT_Y[unit] > UNIT_Y[UNIT_PLAYER]){
						// Attempt to walk up
						if (requestWalkUp(unit, ATTRIB_CAN_WALK_OVER)){
							// Successfully walked. Set base animation to that direction
							UNIT_ANIM_BASE[unit] = SPRITE_EVILBOT_UP;
							// Flag to animate Evilbot
							temp = 1;
						}
					}
					// If Evilbot walked ('temp' flag), then animate him
					if (temp){
						UNIT_TIMER_ANIM[unit] = 0;
						unitAnimate(unit);
						// Redraw screen in unit is on it
						checkForWindowRedraw(unit);
					} else{
						// Check if Evilbot is next to player and didn't move this turn
						if (UNIT_X[unit] == UNIT_X[UNIT_PLAYER] || UNIT_Y[unit] == UNIT_Y[UNIT_PLAYER]){
							if ((UNIT_X[unit] >= UNIT_X[UNIT_PLAYER] - 1 && UNIT_X[unit] <= UNIT_X[UNIT_PLAYER] + 1) && (UNIT_Y[unit] >= UNIT_Y[UNIT_PLAYER] - 1 && UNIT_Y[unit] <= UNIT_Y[UNIT_PLAYER] + 1)){
								// Evilbot is next to the player, damage player
								inflictDamage(UNIT_PLAYER, 5);
								// PLAY SOUND_SHOCK
								ssy_sound_play(SHOCK);
								// Attempt to create small explosion unit on top of player
								for (i = 28; i < 32; i++){
									if (UNIT_TYPE[i] == TYPE_NONE){
										// Found an available unit. Set it to a small explosion
										UNIT_TYPE[i] = TYPE_EXPLOSION_SMALL;
										// Set sprite on explosion
										UNIT_ANIM_BASE[i] = SPRITE_EXPLOSION;
										UNIT_ANIM_FRAME[i] = 0;
										UNIT_IS_SPRITE[i] = 1;
										// Set location of explosion
										unitPos(i, UNIT_X[UNIT_PLAYER], UNIT_Y[UNIT_PLAYER]);
										// Set timer on explosion
										UNIT_TIMER_A[i] = 1;
										break;
									}
								}
								// Set rate of attack on player
								UNIT_TIMER_A[unit] = 18;
							}
						}
					}
				}
				break;
			}
			case TYPE_ELEVATOR: // ELEVATOR (calls the same functions as door)
			case TYPE_DOOR:{    // AI DOOR
				// Call appropriate door function, based on the door unit's state
				switch (UNIT_B[unit]){
					case 0:{
						doorOpenA(unit);
						break;
					}
					case 1:{
						doorOpenB(unit);
						break;
					}
					case 2:{
						doorOpenFull(unit);
						break;
					}
					case 3:{
						doorCloseA(unit);
						break;
					}
					case 4:{
						doorCloseB(unit);
						break;
					}
					case 5:{
						doorCloseFull(unit);
						break;
					}
				}
				break;
			}
			case TYPE_EXPLOSION_SMALL:{ // SMALL EXPLOSION
				UNIT_TIMER_A[unit] = 1;
				// Do explosion animation
				UNIT_ANIM_FRAME[unit]++;
				if (UNIT_ANIM_FRAME[unit] > 4){
					// Animation done, delete unit
					UNIT_TYPE[unit] = TYPE_NONE;
					// Update map
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
					// If this explosion is from a big explosion, then handle further destruction
					if (unit > 63){
						if (MAP[UNIT_Y[unit]][UNIT_X[unit]] == TILE_EXP_CANNISTER){
							// This is an explosive cannister. Set it to an exploded tile on the map
							mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_BLOWN_CANNISTER);
							// Try to create a bomb unit at its location
							for (i = 28; i < 32; i++){
								if (UNIT_TYPE[i] == TYPE_NONE){
									// Found an unused unit, create bomb
									UNIT_TYPE[i] = TYPE_TIMEBOMB;
									// Set the tile of the unit to be the cannister
									UNIT_TILE[i] = TILE_EXP_CANNISTER;
									UNIT_IS_SPRITE[i] = 0;
									// Set the unit's location
									unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
									// Set the time until the explosion happens
									// UNIT_TIMER_A[i] = 10;
									UNIT_TIMER_A[i] = 25;
									UNIT_A[i] = 0;
									break;
								}
							}
						} else if (tileHasAttribute(UNIT_X[unit], UNIT_Y[unit], ATTRIB_CAN_BE_DESTROYED)){
							// Update tile with destroyed tile
							mapSet(UNIT_X[unit], UNIT_Y[unit], DESTRUCT_PATH[MAP[UNIT_Y[unit]][UNIT_X[unit]]]);
							REDRAW_WINDOW = 1;
						}
					}
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_WEAPON_FIRE_UP:{ // FIRE UP
				// If weapon has reached it's limit, then deactivate it
				if (UNIT_A[unit] == 0){
					// Deactivate weapon
					UNIT_TYPE[unit] = TYPE_NONE;
					// Update map
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Weapon was plasma gun, flag it as no longer in use
						UNIT_B[unit] = 0;
						PLASMA_ACTIVE = 0;
					}
				} else{
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit] - 1);
					if (UNIT_B[unit] == 1){
						// Add small delay for plasma gun
						UNIT_TIMER_A[unit] = 1;
					}
					pistolFireCommon(unit);
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_WEAPON_FIRE_DOWN:{ // FIRE DOWN
				// If weapon has reached it's limit, then deactivate it
				if (UNIT_A[unit] == 0){
					// Deactivate weapon
					UNIT_TYPE[unit] = TYPE_NONE;
					// Update map
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Weapon was plasma gun, flag it as no longer in use
						UNIT_B[unit] = 0;
						PLASMA_ACTIVE = 0;
					}
				} else{
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit] + 1);
					if (UNIT_B[unit] == 1){
						// Add small delay for plasma gun
						UNIT_TIMER_A[unit] = 1;
					}
					pistolFireCommon(unit);
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_WEAPON_FIRE_LEFT:{ // FIRE LEFT
				// If weapon has reached it's limit, then deactivate it
				if (UNIT_A[unit] == 0){
					// Deactivate weapon
					UNIT_TYPE[unit] = TYPE_NONE;
					// Update map
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Weapon was plasma gun, flag it as no longer in use
						UNIT_B[unit] = 0;
						PLASMA_ACTIVE = 0;
					}
				} else{
					unitPos(unit, UNIT_X[unit] - 1, UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Add small delay for plasma gun
						UNIT_TIMER_A[unit] = 1;
					}
					pistolFireCommon(unit);
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_WEAPON_FIRE_RIGHT:{ // FIRE RIGHT
				// If weapon has reached it's limit, then deactivate it
				if (UNIT_A[unit] == 0){
					// Deactivate weapon
					UNIT_TYPE[unit] = TYPE_NONE;
					// Update map
					unitPos(unit, UNIT_X[unit], UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Weapon was plasma gun, flag it as no longer in use
						UNIT_B[unit] = 0;
						PLASMA_ACTIVE = 0;
					}
				} else{
					unitPos(unit, UNIT_X[unit] + 1, UNIT_Y[unit]);
					if (UNIT_B[unit] == 1){
						// Add small delay for plasma gun
						UNIT_TIMER_A[unit] = 1;
					}
					pistolFireCommon(unit);
				}
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_TRASH_COMPACTOR:{ // TRASH COMPACTOR
				// Check state of trash compactor
				switch (UNIT_A[unit]){
					case 0:{ // Open state
						// Reset unit timer
						UNIT_TIMER_A[unit] = 20;
						// Check if the tile within the trash compactor isn't the default one, or an alive unit is present
						if (MAP[UNIT_Y[unit]][UNIT_X[unit]] != TILE_TRASH_COMPACTOR_FLOOR || checkForUnit(UNIT_X[unit], UNIT_Y[unit]) != UNIT_NOT_FOUND){
							drawTrashCompactor(unit, TILE_TRASH_COMP_TRANS_TL, TILE_TRASH_COMP_TRANS_TR, TILE_TRASH_COMP_TRANS_BL, TILE_TRASH_COMP_TRANS_BR);
							// Increase trash compactor state
							UNIT_A[unit]++;
							// Reset unit timer to 10 if this was a tile that made it go off
							if (MAP[UNIT_Y[unit]][UNIT_X[unit]] != TILE_TRASH_COMPACTOR_FLOOR || MAP[UNIT_Y[unit]][UNIT_X[unit]] != TILE_TRASH_COMPACTOR_FLOOR){
								UNIT_TIMER_A[unit] = 10;
							}
							// PLAY SOUND_DOOR
							ssy_sound_play(DOOR);
						}	
						break;
					}
					case 1:{ // Mid-closing state
						drawTrashCompactor(unit, TILE_TRASH_COMP_CLOSED_TL, TILE_TRASH_COMP_CLOSED_TR, TILE_TRASH_COMP_CLOSED_BL, TILE_TRASH_COMP_CLOSED_BR);
						// Increase trash compactor state
						UNIT_A[unit]++;
						// Reset unit timer
						UNIT_TIMER_A[unit] = 50;
						// Check for any live units in the compactor
						if ((temp = checkForUnit(UNIT_X[unit], UNIT_Y[unit])) != UNIT_NOT_FOUND || (temp = checkForUnit(UNIT_X[unit] + 1, UNIT_Y[unit])) != UNIT_NOT_FOUND){
							videoInfoPrint("YOU'RE TERMINATED!");
							// PLAY SOUND_EXPLOSION
							ssy_sound_play(EXPLOSION);
							// Destroy unit
							UNIT_TYPE[temp] = TYPE_NONE;
							UNIT_HEALTH[temp] = 0;
							// Update map
							unitPos(temp, UNIT_X[temp], UNIT_Y[temp]);
							// Create small explosion unit if we can find an available unit to do so
							for (i = 28; i < 32; i++){
								if (UNIT_TYPE[i] == TYPE_NONE){
									// Found available unit. Set type to small explosion
									UNIT_TYPE[i] = TYPE_EXPLOSION_SMALL;
									// Set up the sprite animation
									UNIT_ANIM_BASE[i] = SPRITE_EXPLOSION;
									UNIT_ANIM_FRAME[i] = 0;
									UNIT_IS_SPRITE[i] = 1;
									// Set location of the unit
									unitPos(i, UNIT_X[unit], UNIT_Y[unit]);
									// Reset timer for explosion
									UNIT_TIMER_A[i] = 1;
									// Check if it was the player who died
									if (temp == UNIT_PLAYER){
										// It was the player that died
										gameDisplayHealth();
										// Flash the screen due to player damage
										DAMAGE_FLASH_STEPS_REMAINING = VIDEO_DAMAGE_FLASH_COUNT;
										EVENT_ENABLED = 1;
									}
									break;
								}
							}
							// Redraw screen in unit is on it
							checkForWindowRedraw(temp);
						}
						break;
					}
					case 2:{ // Closed state
						drawTrashCompactor(unit, TILE_TRASH_COMP_TRANS_TL, TILE_TRASH_COMP_TRANS_TR, TILE_TRASH_COMP_TRANS_BL, TILE_TRASH_COMP_TRANS_BR);
						// Increase trash compactor state
						UNIT_A[unit]++;
						// Reset unit timer
						UNIT_TIMER_A[unit] = 10;
						break;
					}
					case 3:{ // Mid-opening state
						drawTrashCompactor(unit, TILE_TRASH_COMP_OPEN_TL, TILE_TRASH_COMP_OPEN_TR, TILE_TRASH_COMPACTOR_FLOOR, TILE_TRASH_COMPACTOR_FLOOR);
						// Reset trash compactor state
						UNIT_A[unit] = 0;
						// Reset unit timer
						UNIT_TIMER_A[unit] = 20;
						// PLAY DOOR SOUND
						ssy_sound_play(DOOR);
						break;
					}
				}
				break;
			}
			case TYPE_ROLLERBOT_VERT:{ // UP/DOWN ROLLERBOT
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 8;
				// Check to see if we should animate the droid yet
				unitAnimate(unit);
				// Get direction. 0 = up, 1 = down
				if (UNIT_A[unit] == 1){
					// Moving down
					if (!requestWalkDown(unit, ATTRIB_CAN_WALK_OVER)){
						// Blocked, change direction to up
						UNIT_A[unit] = 0;
					}
				} else{
					// Moving up
					if (!requestWalkUp(unit, ATTRIB_CAN_WALK_OVER)){
						// Blocked, change direction to down
						UNIT_A[unit] = 1;
					}
				}
				rollerbotFireDetect(unit);
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_ROLLERBOT_HORIZ:{ // LEFT/RIGHT ROLLERBOT
				// Reset timer for this unit
				UNIT_TIMER_A[unit] = 8;
				// Check to see if we should animate the droid yet
				unitAnimate(unit);
				// Get direction. 0 = left, 1 = right
				if (UNIT_A[unit] == 1){
					// Moving right
					if (!requestWalkRight(unit, ATTRIB_CAN_WALK_OVER)){
						// Blocked, change direction to left
						UNIT_A[unit] = 0;
					}
				} else{
					// Moving left
					if (!requestWalkLeft(unit, ATTRIB_CAN_WALK_OVER)){
						// Blocked, change direction to right
						UNIT_A[unit] = 1;
					}
				}
				rollerbotFireDetect(unit);
				// Redraw screen in unit is on it
				checkForWindowRedraw(unit);
				break;
			}
			case TYPE_MAGNET:{ // MAGNET
				magnet(unit);
				break;
			}
			case TYPE_MAGNETIZED_ROBOT:{ // MAGNETIZED ROBOT
				magnetizedRobot(unit);
				break;
			}
			case TYPE_WATER_RAFT_HORIZ:{ // WATER RAFT LEFT/RIGHT
				// Change raft to water at it's current position
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_WATER);
				// Check which direction the raft is moving
				if (UNIT_A[unit] == 1){
					// Raft moving right. Check if player is on raft.
					if (UNIT_X[UNIT_PLAYER] == UNIT_X[unit] && UNIT_Y[UNIT_PLAYER] == UNIT_Y[unit]){
						// Player on raft, move player right
						unitPos(UNIT_PLAYER, UNIT_X[UNIT_PLAYER] + 1, UNIT_Y[UNIT_PLAYER]);
						// Move window right as well
						MAP_WINDOW_X++;
						// Flag to redraw window
						REDRAW_WINDOW = 1;
					} 
					// Move raft right
					unitPos(unit, UNIT_X[unit] + 1, UNIT_Y[unit]);
					// Check if unit on screen to see if redraw should happen
					checkForWindowRedraw(unit);
					// Check if the raft should change direction
					if (UNIT_C[unit] == UNIT_X[unit] || MAP[UNIT_Y[unit]][UNIT_X[unit] + 1] != TILE_WATER){
						UNIT_A[unit] = 0;
						// Make the raft stay longer at the edge of the water
						UNIT_TIMER_A[unit] = 120;
					} else{
						// Reset normal timer for raft moving
						UNIT_TIMER_A[unit] = 7;
					}
				} else{
					// Raft moving left. Check if player is on raft.
					if (UNIT_X[UNIT_PLAYER] == UNIT_X[unit] && UNIT_Y[UNIT_PLAYER] == UNIT_Y[unit]){
						// Player on raft, move player right
						unitPos(UNIT_PLAYER, UNIT_X[UNIT_PLAYER] - 1, UNIT_Y[UNIT_PLAYER]);
						// Move window left as well
						MAP_WINDOW_X--;
						// Flag to redraw window
						REDRAW_WINDOW = 1;
					} 
					// Move raft right
					unitPos(unit, UNIT_X[unit] - 1, UNIT_Y[unit]);
					// Check if unit on screen to see if redraw should happen
					checkForWindowRedraw(unit);
					// Check if the raft should change direction
					if (UNIT_B[unit] == UNIT_X[unit] || MAP[UNIT_Y[unit]][UNIT_X[unit] - 1] != TILE_WATER){
						UNIT_A[unit] = 1;
						// Make the raft stay longer at the edge of the water
						UNIT_TIMER_A[unit] = 120;
					} else{
						// Reset normal timer for raft moving
						UNIT_TIMER_A[unit] = 7;
					}
				}
				// Put the raft tile back on the map at new location
				mapSet(UNIT_X[unit], UNIT_Y[unit], TILE_RAFT);
				break;
			}
			case TYPE_DEMATERIALIZE:{ // DEMATERIALIZE
				if (UNIT_TYPE[UNIT_PLAYER] != TYPE_PLAYER_WINS){
					// Loop to animate dematerialization
					for (i = 0; i < 7; i++){
						// Increase player dematerialization frame
						UNIT_ANIM_FRAME[UNIT_PLAYER] = i;
						// Draw screen to update player
						gameMapDraw();
						// Wait 9 times for a new frame before continuing to next animation
						for (temp = 0; temp < 9; temp++){
							while (BGTIMER1 == 0){
								continue;
							}
							BGTIMER1 = 0;
						}
					}
					// Transport complete. Reset Player Animation
					UNIT_ANIM_BASE[UNIT_PLAYER] = SPRITE_PLAYER_DOWN;
					UNIT_ANIM_FRAME[UNIT_PLAYER] = 0;
					// Flag to redraw screen
					REDRAW_WINDOW = 1;
					// Set transporter back to normal
					UNIT_TYPE[unit] = TYPE_TRANSPORTER_PAD;
					UNIT_TIMER_A[unit] = 100;
					// Check if this should transport the player to another location
					if (UNIT_B[unit] == 1){
						// Set player's new location
						unitPos(UNIT_PLAYER, UNIT_C[unit], UNIT_D[unit]);
						MAP_WINDOW_X = UNIT_X[UNIT_PLAYER] - 5;
						MAP_WINDOW_Y = UNIT_Y[UNIT_PLAYER] - 3;
						// Flag to redraw screen
						REDRAW_WINDOW = 1;
						// Don't repeat a held key
						KEY_NOREPEAT = 1;
					} else{
						// Game over condition. Player has won
						UNIT_TYPE[UNIT_PLAYER] = TYPE_PLAYER_WINS;
					}
				}
				break;
			}
		}
		
	}
}

