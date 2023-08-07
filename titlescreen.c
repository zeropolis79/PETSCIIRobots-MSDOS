#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "globals.h"
#include "keyboard.h"
#include "video.h"
#include "audio.h"
#include "game.h"
#include "files.h"
#include "interrupt.h"
#include "string.h"

uint8_t CUSTOM_KEYS_USED = 0;
uint8_t CONTROL_SELECTION = 0;

uint8_t MENU[4][11] = {
	"START GAME",
	"SELECT MAP",
	"DIFFICULTY",
	"CONTROLS  "
};

const uint8_t CUSTOM_KEYS_MENU[CUSTOM_KEYS_COUNT][15] = {
	"      MOVE UP:",
	"    MOVE DOWN:",
	"    MOVE LEFT:",
	"   MOVE RIGHT:",
	"      FIRE UP:",
	"    FIRE DOWN:",
	"    FIRE LEFT:",
	"   FIRE RIGHT:",
	"CYCLE WEAPONS:",
	"  CYCLE ITEMS:",
	"   USE OBJECT:",
	"SEARCH OBJECT:",
	"  MOVE OBJECT:"
};

void highlightSelection(int selection, int color){
	// Change the selected menu text to the passed color
	videoPrintString(32, 12 + (selection * 8), 1, MENU[selection], color);
}

void displayMapName(){
	uint8_t color, bitplane;
	
	// Display the map name in the MAP box on the title screen
	videoPrintString(8, 60, 1, MAP_NAMES[LEVEL_SELECTED], VIDEO_FONT_COLOR_MENU);
}

void displayDifficulty(uint8_t updateString){
	// Draw updated face for robot, based on difficulty level
	videoPrintTile(VIDEO_FACE_X, VIDEO_FACE_Y, FACES[DIFFICULTY_LEVEL], VIDEO_FACE_W, VIDEO_FACE_H);
	if (updateString){
		switch (DIFFICULTY_LEVEL){
			case 0:{
				strcpy(MENU[2], "EASY      ");
				break;
			}
			case 2:{
				strcpy(MENU[2], "HARD      ");
				break;
			}
			default:{
				strcpy(MENU[2], "NORMAL    ");
			}
		}
		highlightSelection(2, VIDEO_FONT_COLOR_MENU_SELECTED);
	}
}

void displayControls(){
	switch (CONTROL_SELECTION){
		case 1:{
			strcpy(MENU[3], "CUSTOM KEY");
			break;
		}
		case 2:{
			strcpy(MENU[3], "GAMEPAD   ");
			break;
		}
		default:{
			strcpy(MENU[3], "KEYBOARD  ");
		}
	}
	highlightSelection(3, VIDEO_FONT_COLOR_MENU_SELECTED);
}

void selectCustomKeys(){
	uint8_t i, y, color, str[4];
	
	// Clear screen and set proper palette
	videoClearScreen();
	videoPaletteSet(VIDEO_PALETTE_GAMEOVER, 0);
	videoFadeOut(VIDEO_PALETTE_GAMEOVER, FADE_INSTANT);
	// Draw top and bottom of the border from the game over screen, to use as the custom key screen
	videoPrintTileMultibyte(0, 0, GAMEOVER, 320, 20);
	videoPrintTileMultibyte(0, 140, GAMEOVER + (400 << VIDEO_PLANESHIFT >> VIDEO_MULTIBYTE_BITSHIFT >> VIDEO_BITSHIFT), 320, 60);
	// Erase the bottom of one of the robots from the game over screen
	videoFillRect(16, 140, 288, 50, 0);
	// Draw verticle bars on each side of the screen.
	if (VIDEO_COMPOSITE){
		videoFillRect(2, 20, 4, 120, 5);
		videoFillRect(314, 20, 4, 120, 5);
	} else{
		for (i = 1; i < 10; i++){
			color = videoGetPixel(i, 19);
			videoFillRect(i, 20, 1, 167, color);
			videoFillRect(318 - i, 20, 1, 167, color);
		}
	}
	KEY_NOREPEAT = 1;
	videoPrintString(40, 32, 1, "PRESS THE KEYS YOU WISH TO USE", VIDEO_FONT_COLOR_GAMEOVER);
	videoPrintString(48, 40, 1, "FOR THE FOLLOWING FUNCTIONS ", VIDEO_FONT_COLOR_GAMEOVER);
	y = 56;
	for (i = 0; i < CUSTOM_KEYS_COUNT; i++){
		y += 8;
		videoPrintString(48, y, 1, CUSTOM_KEYS_MENU[i], VIDEO_FONT_COLOR_GAMEOVER);
	}
	y = 56;
	videoFadeIn(VIDEO_PALETTE_GAMEOVER, FADE_SPEED);
	for (i = 0; i < CUSTOM_KEYS_COUNT; i++){
		y += 8;
		CUSTOM_KEYS[i] = keyWait();
		KEY_NOREPEAT = 1;
		str[0] = 0;
		strcatnum(str, CUSTOM_KEYS[i], 3, '0');
		videoPrintString(168, y, 1, str, VIDEO_FONT_COLOR_GAMEOVER);
	}
	CUSTOM_KEYS_USED = 1;
}

uint8_t titleScreen(uint8_t firstLoad){
	int key, selection;
	unsigned int i;
	
	if (!firstLoad){
		// Display the title screen. First set the VGA palette
		videoClearScreen();
		videoPaletteSet(VIDEO_PALETTE_TITLE, 0);
		videoFadeOut(VIDEO_PALETTE_TITLE, FADE_INSTANT);
		// Copy the title screen into video memory
		videoDrawFullScreen(TITLE);
	}
	// Display the map name in the MAP box
	displayMapName();
	// Display current difficulty level
	displayDifficulty(0);
	// Highlight the current cursor selection
	selection = 0;
	highlightSelection(0, VIDEO_FONT_COLOR_MENU_SELECTED);
	highlightSelection(1, VIDEO_FONT_COLOR_MENU);
	highlightSelection(2, VIDEO_FONT_COLOR_MENU);
	#ifdef SHAREWARE
		highlightSelection(3, 0);
	#else
		highlightSelection(3, VIDEO_FONT_COLOR_MENU);
	#endif
	if (!firstLoad){
		videoFadeIn(VIDEO_PALETTE_TITLE, FADE_SPEED);
	}
	// Turn on the cursor, so that the interrupt rotates the color
	CURSOR_ENABLED = 1;
	EVENT_ENABLED = 1;
	// Play music
	ssy_music_play(MUSIC_TITLE);
	// Main title screen loop
	while(1){
		if (!VIDEO_USE_PALETTE_ANIMATION){
			// Update the text if not in VGA mode where we can just adjust the palette
			if (CURSOR.last_color != CURSOR.color){
				highlightSelection(selection, CURSOR.color);
			}
		}
		// Check which key pressed
		key = keyGet();
		KEY_NOREPEAT = 1;
		switch (key){
			case SCAN_KP_8:
			case SCAN_UP_ARROW:{ // Up arrow
				if (selection > 0){
					// Change current selection back to green
					highlightSelection(selection, VIDEO_FONT_COLOR_MENU);
					// Decrease selection and change it to strobing cursor color
					selection--;
					highlightSelection(selection, VIDEO_FONT_COLOR_MENU_SELECTED);
					// PLAY SOUND MENU_BEEP
					ssy_sound_play(BEEP);
				}
				break;
			}
			case SCAN_KP_2:
			case SCAN_DOWN_ARROW:{ // Down arrow
				#ifdef SHAREWARE
					if (selection >= 2){
						break;
					}
				#endif
				if (selection < 3){
					// Change current selection back to green
					highlightSelection(selection, VIDEO_FONT_COLOR_MENU);
					// Increase selection and change it to strobing cursor color
					selection++;
					highlightSelection(selection, VIDEO_FONT_COLOR_MENU_SELECTED);
					// PLAY SOUND MENU_BEEP
					ssy_sound_play(BEEP);
				}
				break;
			}
			case SCAN_SPACE:  // Spacebar
			case SCAN_NUMPAD_ENTER:
			case SCAN_NUMPAD_ENTER | MODIFIER_SHIFT: // Use key for gravis controller
			case SCAN_KP_6:
			case SCAN_RIGHT_ARROW:
			case SCAN_ENTER:{ // Enter key
				// Take action, based on which menu item was selected
				switch (selection){
					case 0:{ // Start game
						if (key != SCAN_KP_6 && key != SCAN_RIGHT_ARROW){
							ssy_sound_play(BEEP);
							videoFadeOut(VIDEO_PALETTE_TITLE, FADE_SPEED);
							if (CONTROL_SELECTION == 0){
								// Reset the flag that remembers if they previously used custom keys
								CUSTOM_KEYS_USED = 0;
							}
							if (CUSTOM_KEYS_USED == 0){
								// Loop through the default keys and add those as our keys
								for (i = 0; i < CUSTOM_KEYS_COUNT; i++){
									CUSTOM_KEYS[i] = CUSTOM_KEYS_DEFAULTS[i];
								}
							}
							if (CONTROL_SELECTION == 1 && !CUSTOM_KEYS_USED){
								selectCustomKeys();
								ssy_sound_play(BEEP);
							}
							ssy_music_stop();
							return 1;
						}
						break;
					}
					case 1:{ // Select map
						ssy_sound_play(BEEP);
						LEVEL_SELECTED++;
						#ifdef SHAREWARE
							if (LEVEL_SELECTED > 3){
								LEVEL_SELECTED = 0;
							} else{
								LEVEL_SELECTED = 3;
							}
						#else
							if (LEVEL_SELECTED == MAP_COUNT){
								LEVEL_SELECTED = 0;
							}
						#endif
						displayMapName();
						break;
					}
					case 2:{ // Difficulty Level
						ssy_sound_play(BEEP);
						DIFFICULTY_LEVEL++;
						if (DIFFICULTY_LEVEL == 3){
							DIFFICULTY_LEVEL = 0;
						}
						displayDifficulty(1);
						break;
					}
					case 3:{ // Controls
						ssy_sound_play(BEEP);
						CONTROL_SELECTION++;
						if (CONTROL_SELECTION == 3){
							CONTROL_SELECTION = 0;
						}
						displayControls();
						break;
					}
				}
				break;
			}
			case SCAN_KP_4:
			case SCAN_LEFT_ARROW:{
				// Move choice back one, it this selection has that option
				switch (selection){
					case 1:{ // Select map
						ssy_sound_play(BEEP);
						LEVEL_SELECTED--;
						#ifdef SHAREWARE
							if (LEVEL_SELECTED == 255){
								LEVEL_SELECTED = 3;
							} else{
								LEVEL_SELECTED = 0;
							}
						#else
							if (LEVEL_SELECTED == 255){
								LEVEL_SELECTED = MAP_COUNT - 1;
							}
						#endif
						displayMapName();
						break;
					}
					case 2:{ // Difficulty Level
						ssy_sound_play(BEEP);
						DIFFICULTY_LEVEL--;
						if (DIFFICULTY_LEVEL == 255){
							DIFFICULTY_LEVEL = 2;
						}
						displayDifficulty(1);
						break;
					}
					case 3:{ // Controls
						ssy_sound_play(BEEP);
						CONTROL_SELECTION--;
						if (CONTROL_SELECTION == 255){
							CONTROL_SELECTION = 2;
						}
						displayControls();
						break;
					}
				}
				break;
			}
			case SCAN_A | MODIFIER_SHIFT:{
				ssy_sound_play(BEEP);
				break;
			}
			case SCAN_B | MODIFIER_SHIFT:{
				ssy_sound_play(CYCLE_ITEM);
				break;
			}
			case SCAN_C | MODIFIER_SHIFT:{
				ssy_sound_play(CYCLE_WEAPON);
				break;
			}
			case SCAN_D | MODIFIER_SHIFT:{
				ssy_sound_play(DOOR);
				break;
			}
			case SCAN_E | MODIFIER_SHIFT:{
				ssy_sound_play(FIRE_PISTOL);
				break;
			}
			case SCAN_F | MODIFIER_SHIFT:{
				ssy_sound_play(BEEP2);
				break;
			}
			case SCAN_G | MODIFIER_SHIFT:{
				ssy_sound_play(USE_EMP);
				break;
			}
			case SCAN_H | MODIFIER_SHIFT:{
				ssy_sound_play(ERROR);
				break;
			}
			case SCAN_I | MODIFIER_SHIFT:{
				ssy_sound_play(ITEM_FOUND);
				break;
			}
			case SCAN_J | MODIFIER_SHIFT:{
				ssy_sound_play(USE_MAGNET);
				break;
			}
			case SCAN_K | MODIFIER_SHIFT:{
				ssy_sound_play(USE_MAGNET2);
				break;
			}
			case SCAN_L | MODIFIER_SHIFT:{
				ssy_sound_play(USE_MEDKIT);
				break;
			}
			case SCAN_M | MODIFIER_SHIFT:{
				ssy_sound_play(MOVE_OBJECT);
				break;
			}
			case SCAN_N | MODIFIER_SHIFT:{
				ssy_sound_play(FIRE_PLASMA);
				break;
			}
			case SCAN_O | MODIFIER_SHIFT:{
				ssy_sound_play(SHOCK);
				break;
			}
			case SCAN_P | MODIFIER_SHIFT:{
				ssy_sound_play(EXPLOSION);
				break;
			}
			case SCAN_Q | MODIFIER_SHIFT:{
				ssy_sound_play(EXPLOSION2);
				break;
			}
			case SCAN_ESC:{
				return 0;
			}
		}
	}
}
