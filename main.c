#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "globals.h"
#include "keyboard.h"
#include "video.h"
#include "audio.h"
#include "titlescreen.h"
#include "game.h"
#include "files.h"
#include "interrupt.h"
#include "string.h"
#include "memory.h"

// The game has ended or a critical error has caused us to abort.
int cleanup(){
	// Switch back to old video mode, if necessary
	if (VIDEO_MODE_ON_STARTUP != NO_MODE){
		videoFadeOut(VIDEO_PALETTE_TITLE, FADE_SPEED);
		videoUnsetMode();
	}
	interruptDisable();
	keyboardUninstallInterrupt();
	ssy_music_stop();
	return 1;
}

// Load sprites, tiles, etc for our current video mode. This should happen after loading the title screen.
// Also print periods to indicate loading progress.
int loadAssets(){
	uint8_t i;
	uint16_t intptr, tempmem;
	
	// Assign memory and load initial music and sound
	if (SSY_DEVICE_MUS){
		if (SSY_DEVICE_MUS == DEVICE_SPEAKER){
			if ((MUSIC_TITLE = memoryAssign(280, 0)) == 0) return 0;
			if ((MUSIC_GAME = memoryAssign(687, 0)) == 0) return 0;
			if ((MUSIC_WIN = memoryAssign(20, 0)) == 0) return 0;
			if ((MUSIC_LOSE = memoryAssign(8, 0)) == 0) return 0;
		} else{
			if ((MUSIC_TITLE = memoryAssign(1079, 0)) == 0) return 0;
			if ((MUSIC_GAME = memoryAssign(2973, 0)) == 0) return 0;
			if ((MUSIC_WIN = memoryAssign(160, 0)) == 0) return 0;
			if ((MUSIC_LOSE = memoryAssign(77, 0)) == 0) return 0;
		}
		if (!fileRead(memorySegToPtr(MUSIC_TITLE), "METAHEAD", SSY_MUS_EXT, 0)) return 0;
		if (!fileRead(memorySegToPtr(MUSIC_WIN), "WIN", SSY_MUS_EXT, 0)) return 0;
		if (!fileRead(memorySegToPtr(MUSIC_LOSE), "LOSE", SSY_MUS_EXT, 0)) return 0;
	}
	if (SSY_DEVICE_SFX){
		if (SSY_DEVICE_MUS == DEVICE_SPEAKER){
			if ((SOUNDFX = memoryAssign(78, 0)) == 0) return 0;
		} else{
			if ((SOUNDFX = memoryAssign(108, 0)) == 0) return 0;
		}
		if (!fileRead(memorySegToPtr(SOUNDFX), "SOUNDFX", SSY_SFX_EXT, 0)) return 0;
	}
	videoPrintString(72, 60, 0, ".", VIDEO_FONT_COLOR_MENU_SELECTED);

	// Load HUD/game screen, which was pre-cut into sections to save ram
	HUD = memoryAssign(VIDEO_HUDSIZE >> (4 + VIDEO_FILE_MULTIBYTE), 0);
	if (videoTranslate != NULL && !(VIDEO_FILE_MULTIBYTE && VIDEO_SKIP_TRANSLATE_MULTIBYTE)){
		tempmem = memoryAssign(VIDEO_HUDSIZE >> (4 + VIDEO_FILE_MULTIBYTE), 1);
	} else{
		tempmem = HUD;
	}
	if (!fileRead(memorySegToPtr(tempmem), "hud", VIDEO_FILEEXT, VIDEO_HUDSIZE >> VIDEO_FILE_MULTIBYTE)) return 0;
	if (videoTranslate != NULL && !(VIDEO_FILE_MULTIBYTE && VIDEO_SKIP_TRANSLATE_MULTIBYTE)){
		intptr = tempmem;
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD), 264, 8, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
		tempmem += (132 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD + tempmem - intptr), 48, 8, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
		tempmem += (24 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD + tempmem - intptr), 48, 8, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
		tempmem += (24 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD + tempmem - intptr), 48, 8, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
		tempmem += (24 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD + tempmem - intptr), 48, 8, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
		tempmem += (24 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(HUD + tempmem - intptr), 8, 200, VIDEO_PALETTE_GAME, VIDEO_FILE_MULTIBYTE);
	}
	videoPrintString(80, 60, 0, ".", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Load the game over screen, which was pre-cut into sections to save ram
	GAMEOVER = memoryAssign(VIDEO_GAMEOVERSIZE >> (4 + VIDEO_FILE_MULTIBYTE), 0);
	if (videoTranslate != NULL && !VIDEO_SKIP_TRANSLATE_MULTIBYTE){
		tempmem = memoryAssign(VIDEO_GAMEOVERSIZE >> (4 + VIDEO_FILE_MULTIBYTE), 1);
	} else{
		tempmem = GAMEOVER;
	}
	if (!fileRead(memorySegToPtr(tempmem), "gameover", VIDEO_FILEEXT, VIDEO_GAMEOVERSIZE >> VIDEO_FILE_MULTIBYTE)) return 0;
	if (videoTranslate != NULL && !VIDEO_SKIP_TRANSLATE_MULTIBYTE){
		intptr = tempmem;
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER), 320, 20, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (400 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 320, 60, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (1200 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 88, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (44 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 8, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (4 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 120, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (60 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 88, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (44 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 64, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (32 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 72, 8, VIDEO_PALETTE_GAMEOVER, 1);
		tempmem += (36 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
		videoTranslate(memorySegToPtr(tempmem), memorySegToPtr(GAMEOVER + tempmem - intptr), 104, 8, VIDEO_PALETTE_GAMEOVER, 1);
	}
	videoPrintString(88, 60, 0, "..", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Load sprites. 24 width x 24 height x 83 tiles
	if (!fileReadTiles("sprites", VIDEO_FILEEXT, VIDEO_TILE_W, VIDEO_TILE_H, SPRITES, SPRITE_COUNT)) return 0;
	// Load tiles. 24 width x 24 height x 253 tiles
	if (!fileReadTiles("tiles", VIDEO_FILEEXT, VIDEO_TILE_W, VIDEO_TILE_H, TILES, TILE_COUNT)) return 0;
	videoPrintString(104, 60, 0, "..", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Load weapons/items. 48 width x 32 height x 6 tiles
	if (!fileReadTiles("items", VIDEO_FILEEXT, VIDEO_ITEM_W, VIDEO_ITEM_H, ITEMS, ITEMS_COUNT)) return 0;
	videoPrintString(120, 60, 0, ".", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Load health/doom guy. 48 width x 56 height x 6 tiles
	if (!fileReadTiles("health", VIDEO_FILEEXT, VIDEO_HEALTH_W, VIDEO_HEALTH_H, HEALTH, HEALTH_COUNT)) return 0;
	videoPrintString(128, 60, 0, ".", VIDEO_FONT_COLOR_MENU_SELECTED);
	// Load keys. 16 width x 14 height x 3 tiles
	if (!fileReadTiles("keys", VIDEO_FILEEXT, VIDEO_KEY_W, VIDEO_KEY_H, KEYS, KEY_COUNT)) return 0;
	return 1;
}

// Main program entry point
int main(int argc, char **argv){
	uint16_t i, memtemp;
	
	char str[10],str2[10];
	
	uint8_t *test;
	
	// Ensure the direction flag is cleared for the repeat assembly instruction
	_asm{
		cld
	}
	
	puts("");
	puts("Attack of the Petscii Robots - v1.1");
	puts("By David Murray - The 8-Bit Guy");
	puts("Ported by Jim Wright");
	puts("");
	
	// Seed the random number generator
	srand(time(0));
	// Set the clock as inactive
	CLOCK.ACTIVE = 0;
	// Initialize array that holds allocated memory
	memoryInit();
	// Install keyboard interrupt
	keyboardInstallInterrupt();
	// Prompt for Video and Audio settings
	str[0] = 0;
	for (i = 1; i < argc; i++){
		if (strncmp(argv[i], "video=", 6) == 0){
			str[0] = argv[i][6];
			break;
		}
		if (strncmp(argv[i], "v=", 2) == 0){
			str[0] = argv[i][2];
			break;
		}
	}
	videoSelect(str);
	str[0] = 0;
	for (i = 1; i < argc; i++){
		if (strncmp(argv[i], "sound=", 6) == 0){
			str[0] = argv[i][6];
			break;
		}
		if (strncmp(argv[i], "s=", 2) == 0){
			str[0] = argv[i][2];
			break;
		}
	}
	soundSelect(str);
	str[0] = 0;
	for (i = 1; i < argc; i++){
		if (strncmp(argv[i], "music=", 6) == 0){
			str[0] = argv[i][6];
			break;
		}
		if (strncmp(argv[i], "m=", 2) == 0){
			str[0] = argv[i][2];
			break;
		}
	}
	musicSelect(str);
	puts("Waking up the robots...\n");
	
	// Load the title screen
	TITLE = memoryAssign(VIDEO_SCREENSIZE >> (4 + VIDEO_MULTIBYTE_BITSHIFT), 0);
	if (videoTranslate != NULL && VIDEO_MODE != CGACOMP && VIDEO_MODE != TDY_LOW){
		memtemp = memoryAssign(VIDEO_SCREENSIZE >> (4 + VIDEO_FILE_MULTIBYTE), 0);
	} else{
		memtemp = TITLE;
	}
	if (videoTranslate != NULL && VIDEO_MODE != CGACOMP && VIDEO_MODE != TDY_LOW){
		if (!fileRead(memorySegToPtr(memtemp), "title", VIDEO_FILEEXT, VIDEO_SCREENSIZE >> VIDEO_FILE_MULTIBYTE)) return cleanup();
		videoTranslate(memorySegToPtr(memtemp), memorySegToPtr(TITLE), 320, 200, PALETTE_TITLE_EGA, VIDEO_FILE_MULTIBYTE);
	} else{
		if (!fileRead(memorySegToPtr(TITLE), "title", VIDEO_FILEEXT, VIDEO_SCREENSIZE >> VIDEO_FILE_MULTIBYTE)) return cleanup();
	}
	// Assign memory for Minimap (also used as temporary ram for loading some other data)
	if ((MINIMAP[0] = memoryAssign(1024 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT, 0)) == 0) return 0;
	for (i = 1; i < 64; i++){
		MINIMAP[i] = MINIMAP[i - 1] + (16 << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT);
	}
	// Load petscii font. 8 width x 8 height x 256 tiles
	if (!fileReadTiles("petfont", ".gfx", VIDEO_FONT_W, VIDEO_FONT_H, FONT, FONT_COUNT)) return cleanup();
	// Load robot faces on title screen. 16 width x 24 height x 3 tiles
	if (!fileReadTiles("faces", VIDEO_FILEEXT, VIDEO_FACE_W, VIDEO_FACE_H, FACES, FACE_COUNT)) return cleanup();
	// Switch into the selected video mode
	videoSetMode();
	// Enable the interrupt
	interruptEnable();
	// Set the VGA palette
	videoPaletteSet(VIDEO_PALETTE_TITLE, 0);
	videoFadeOut(VIDEO_PALETTE_TITLE, FADE_INSTANT);
	
	// Show Shareware splash screen
	#ifdef SHAREWARE
		videoPrintString( 20,  16, 1, "ATTACK OF THE PETSCII ROBOTS MS-DOS", VIDEO_FONT_COLOR_MENU);
		videoPrintString(  4,  24, 1, "SHAREWARE VERSION IS FREE TO DISTRIBUTE", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 20,  40, 1, "YOU CAN DOWNLOAD A FREE COPY OF THE", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 12,  48, 1, "USER'S MANUAL OR BUY THE FULL VERSION", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 76,  56, 1, "AT WWW.THE8BITGUY.COM", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 44,  72, 1, "*** FULL VERSION INCLUDES ***", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60,  84, 1, "- ALL 15 LEVELS", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60,  98, 1, "- 4 DIFFERENT IN-GAME TUNES", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60, 112, 1, "- GRAVIS CONTROLLER SUPPORT", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60, 126, 1, "- HOTKEY FOR ROBOT LOCATIONS", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60, 140, 1, "- CUSTOMIZABLE KEYBOARD CONTROLS", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60, 154, 1, "- GET RID OF THIS NAGGING SCREEN", VIDEO_FONT_COLOR_MENU);
		videoPrintString( 60, 176, 1, "PRESS ANY KEY TO CONTINUE", VIDEO_FONT_COLOR_MENU);
		videoFadeIn(VIDEO_PALETTE_TITLE, FADE_SPEED);
		keyWait();
		videoFadeOut(VIDEO_PALETTE_TITLE, FADE_INSTANT);
	#endif
	
	// Copy the title screen into video memory
	videoDrawFullScreen(TITLE);
	// Redraw menu
	videoPrintString(32, 12, 1, "START GAME", VIDEO_FONT_COLOR_MENU);
	videoPrintString(32, 20, 1, "SELECT MAP", VIDEO_FONT_COLOR_MENU);
	videoPrintString(32, 28, 1, "DIFFICULTY", VIDEO_FONT_COLOR_MENU);
	#ifdef SHAREWARE
		videoPrintString(32, 36, 1, "          ", 0);
	#else
		videoPrintString(32, 36, 1, "CONTROLS  ", VIDEO_FONT_COLOR_MENU);
	#endif
	// Print "LOADING." in the map box
	videoPrintString(8, 60, 1, "LOADING.", VIDEO_FONT_COLOR_MENU_SELECTED);
	videoFadeIn(VIDEO_PALETTE_TITLE, FADE_SPEED);
	// Load all other needed assets, except the level itself
	if (!loadAssets()){
		// Error loading assets. Exit program.
		return cleanup();
	}

	// Enable the interrupt
	// interruptEnable();

	// Entry point for title screen/game loop. 'i' is 1 on first load of the title screen (to not fade in, since we did for loading initially)
	i = 1;
	while (titleScreen(i) && gameInit()) i = 0;
	// Cleanup and exit
	if (!PROGRAM_ABORT){
		cleanup();
		#ifdef SHAREWARE
			puts("Thanks for trying the shareware version of");
			puts("");
			puts("ATTACK OF THE PETSCII ROBOTS");
			puts("");
			puts("Full version available for sale at www.The8BitGuy.com");
			puts("");
		#endif
		puts("Have a nice DOS.");
	}
	return 0;
}
