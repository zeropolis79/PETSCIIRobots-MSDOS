#include <stdio.h>
#include <stdint.h>
#include "keyboard.h"
#include "globals.h"

volatile uint8_t BGTIMER1 = 0;
volatile uint8_t KEYTIMER = 0;
volatile uint8_t JOYTIMER = 0;
volatile uint8_t EVENT_TIMER = 0;
uint8_t PROGRAM_ABORT = 0;
uint8_t KEYFAST = 0;
uint8_t KEY_NOREPEAT = 0;
uint8_t LEVEL_SELECTED = 0;		// The selected map when the game loads for the first time
uint8_t DIFFICULTY_LEVEL = 1;	// Difficulty Level - 0 = easy, 1 = normal, 2 = hard
uint8_t MAGNET_ACTIVE = 0;
uint8_t PLASMA_ACTIVE = 0;
uint8_t BIG_EXPLOSION_ACTIVE = 0;
uint8_t MAP_WINDOW_X;
uint8_t MAP_WINDOW_Y;
uint8_t REDRAW_WINDOW;
uint8_t SOUND_ENABLED = 0;
uint8_t CURSOR_ENABLED = 0;
uint8_t EVENT_ENABLED = 0;
uint8_t DAMAGE_FLASH_STEPS_REMAINING = 0;
uint8_t VIDEO_DAMAGE_FLASH_COUNT = 0;
uint8_t EMP_FLASH_STEPS_REMAINING = 0;
uint8_t VIDEO_EMP_FLASH_COUNT = 0;
uint8_t TILE_ANIMATION_FRAME = 0;
uint8_t CINEMA_LAST_FRAME = 0;
uint8_t CINEMA_ANIMATION_FRAME = 0;
uint8_t CINEMA_INDEX = 0;
uint8_t SCREEN_SHAKE = 0;
uint8_t MINIMAP_OPEN = 0;
uint8_t MINIMAP_ROBOTS_ENABLED = 0;
uint8_t MINIMAP_PROXIMITY = 0;
uint8_t MINIMAP_X_OFFSET = 0;
uint8_t TRANSPARENCY = 1;
clockStruct CLOCK;

uint8_t UNIT_TYPE[UNIT_COUNT];
uint8_t UNIT_X[UNIT_COUNT];
uint8_t UNIT_Y[UNIT_COUNT];
uint8_t UNIT_A[UNIT_COUNT];
uint8_t UNIT_B[UNIT_COUNT];
uint8_t UNIT_C[UNIT_COUNT];
uint8_t UNIT_D[UNIT_COUNT];
uint8_t UNIT_HEALTH[UNIT_COUNT];
uint8_t UNIT_TIMER_A[UNIT_COUNT];
uint8_t UNIT_TIMER_B[UNIT_COUNT];
uint8_t UNIT_TILE[UNIT_COUNT];
uint8_t UNIT_TIMER_ANIM[UNIT_COUNT];
uint8_t UNIT_ANIM_BASE[UNIT_COUNT];
uint8_t UNIT_ANIM_FRAME[UNIT_COUNT];
uint8_t UNIT_IS_SPRITE[UNIT_COUNT];
uint8_t MAP[MAP_HEIGHT][MAP_WIDTH];
uint8_t PREVIOUS_DRAW_TILE[7][11];
uint8_t PREVIOUS_DRAW_UNIT[7][11];
uint8_t KEY_INVENTORY[4];
cursorStruct CURSOR;

// Segment data for graphics/sound
uint16_t AUDIO_DATA;
uint16_t MUSIC_TITLE;
uint16_t MUSIC_WIN;
uint16_t MUSIC_LOSE;
uint16_t MUSIC_GAME;
uint16_t SOUNDFX;
uint16_t TITLE;
uint16_t GAMEOVER;
uint16_t HUD;
uint16_t TILES[TILE_COUNT];
uint16_t SPRITES[SPRITE_COUNT];
uint16_t ITEMS[ITEMS_COUNT];
uint16_t HEALTH[HEALTH_COUNT];
uint16_t KEYS[KEY_COUNT];
uint16_t FACES[FACE_COUNT];
uint16_t FONT[FONT_COUNT];
uint16_t MINIMAP[64];
uint8_t *MAP_TILE_COLORS;

const char MAP_NAMES[MAP_COUNT][17] = {
	"01-RESEARCH LAB ",
	"02-HEADQUARTERS ",
	"03-THE VILLAGE  ",
	"04-THE ISLANDS  ",
	"05-DOWNTOWN     ",
	"06-PI UNIVERSITY",
	"07-MORE ISLANDS ",
	"08-ROBOT HOTEL  ",
	"09-FOREST MOON  ",
	"10-DEATH TOWER  ",
	"11-RIVER DEATH  ",
	"12-BUNKER       ",
	"13-CASTLE ROBOT ",
	"14-ROCKET CENTER",
	"15-PILANDS      "
};

const char MUSIC_FILENAMES[4][9] = {
	"GETPSYCH\0",
	"METALBOP\0",
	"ROBOATTK\0",
	"RUSHING\0 "
};

const char CINEMA_TEXT[] = {
	"COMING SOON: SPACE BALLS 2 - THE SEARCH FOR MORE MONEY, ATTACK OF THE PAPERCLIPS: CLIPPY'S REVENGE, IT CAME FROM PLANET EARTH, ROCKY 5000, ALL MY CIRCUITS THE MOVIE, CONAN THE LIBRARIAN, AND MORE!           \0"
};

uint8_t CUSTOM_KEYS[CUSTOM_KEYS_COUNT];
const uint8_t CUSTOM_KEYS_DEFAULTS[CUSTOM_KEYS_COUNT] = {
	SCAN_I,		// KEYS_MOVE_UP
	SCAN_K,		// KEYS_MOVE_DOWN
	SCAN_J,		// KEYS_MOVE_LEFT
	SCAN_L,		// KEYS_MOVE_RIGHT
	SCAN_W,		// KEYS_FIRE_UP
	SCAN_S,		// KEYS_FIRE_DOWN
	SCAN_A,		// KEYS_FIRE_LEFT
	SCAN_D,		// KEYS_FIRE_RIGHT
	SCAN_F1,	// KEYS_CYCLE_WEAPONS
	SCAN_F2,	// KEYS_CYCLE_ITEMS
	SCAN_SPACE,	// KEYS_USE_ITEM
	SCAN_Z,		// KEYS_SEARCH_OBJECT
	SCAN_M		// KEYS_MOVE_OBJECT
};

// Cursor colors
uint8_t CURSOR_SELECTED_COLOR[32] = {
	0,
	4,
	8,
	12,
	16,
	20,
	24,
	28,
	32,
	36,
	40,
	44,
	48,
	52,
	56,
	60,
	63,
	60,
	56,
	52,
	48,
	44,
	40,
	36,
	32,
	28,
	24,
	20,
	16,
	12,
	8,
	4,
};

// Title screen RGB palette
const uint8_t PALETTE_TITLE_VGA[PALETTE_COUNT][3] = {
	{0, 0, 0},
	{0, 51, 68},
	{34, 34, 136},
	{170, 17, 51},
	{51, 85, 119},
	{85, 119, 153},
	{119, 85, 170},
	{204, 68, 102},
	{119, 187, 85},
	{102, 119, 204},
	{238, 136, 68},
	{238, 187, 51},
	{136, 170, 204},
	{136, 136, 238},
	{238, 238, 136},
	{238, 238, 238}
};

const uint8_t PALETTE_TITLE_EGA[PALETTE_COUNT] = {
	0,
	1,
	1,
	4,
	8,
	7,
	5,
	12,
	2,
	9,
	6,
	14,
	7,	
	11,	
	10,	
	15
};

const uint8_t PALETTE_TITLE_ETGA[PALETTE_COUNT] = {
	0x00,
	0x30,
	0x91,
	0x44,
	0x98,
	0xb8,
	0xd7,
	0xc5,
	0xa6,
	0x97,
	0xec,
	0xec,
	0xb9,
	0x99,
	0xee,
	0xff
};

uint8_t *VIDEO_PALETTE_TITLE = (uint8_t*)PALETTE_TITLE_EGA;

// Game over screen RGB palette
const uint8_t PALETTE_GAMEOVER_VGA[PALETTE_COUNT][3] = {
	{0, 0, 0},
	{0, 51, 68},
	{34, 34, 136},
	{170, 17, 51},
	{51, 85, 119},
	{85, 119, 153},
	{119, 85, 170},
	{0, 102, 0},
	{119, 187, 85},
	{102, 119, 204},
	{238, 136, 68},
	{238, 221, 119},
	{136, 170, 204},
	{221, 85, 119},
	{102, 51, 0},
	{238, 238, 238}
};

const uint8_t PALETTE_GAMEOVER_EGA[PALETTE_COUNT] = {
	0,	
	8,	
	1,	
	4,	
	9,	
	7,	
	6,	
	2,	
	10,	
	9,	
	12,	
	10,	
	7,	
	12,	
	6,	
	15
};

const uint8_t PALETTE_GAMEOVER_ETGA[PALETTE_COUNT] = {
	0x00,
	0x30,
	0x91,
	0x44,
	0x98,
	0xb8,
	0xd7,
	0x22,
	0xa2,
	0x97,
	0xc2,
	0xee,
	0xb5,
	0xdc,
	0x66,
	0xff
};

uint8_t *VIDEO_PALETTE_GAMEOVER = (uint8_t*)PALETTE_GAMEOVER_EGA;

// In-game RGB palette
const uint8_t PALETTE_GAME_VGA[PALETTE_COUNT][3] = {
	{0, 0, 0},			// Black
	{85, 102, 0},		// Dark Green
	{119, 68, 0},		// Brown
	{0, 51, 153},		// Dark Blue/Purple
	{238, 0, 0},		// Red
	{68, 85, 119},		// Dark Gray
	{119, 153, 0},		// Medium Green
	{187, 119, 0},		// Orange
	{51, 102, 187},		// Medium Blue
	{170, 221, 0},		// Light Green
	{119, 136, 153},	// Medium Gray
	{0, 170, 255},		// Light Blue
	{255, 204, 0},		// Yellow
	{170, 187, 204},	// Light Gray
	{238, 170, 153},	// Light Red
	{255, 255, 255},	// White
};

const uint8_t PALETTE_GAME_EGA[PALETTE_COUNT] = {
	0,		// Black
	2,		// Dark Green
	6,		// Brown
	1,		// Dark Blue/Purple
	4,		// Red
	8,		// Dark Gray
	2,		// Medium Green
	6,		// Orange
	9,		// Medium Blue
	14,		// Light Green
	3,		// Medium Gray
	9,		// Light Blue
	14,		// Yellow
	7,		// Light Gray
	12,		// Light Red
	15		// White
};

const uint8_t PALETTE_GAME_ETGA[PALETTE_COUNT] = {
	0x00,	// Black
	0x22,	// Dark Green
	0x66,	// Brown
	0x11,	// Dark Blue/Purple
	0xc4,	// Red
	0x88,	// Dark Gray
	0xa2,	// Medium Green
	0xc2,	// Orange
	0x93,	// Medium Blue
	0xaa,	// Light Green
	0x78,	// Medium Gray
	0xb9,	// Light Blue
	0xee,	// Yellow
	0x77,	// Light Gray
	0xc7,	// Light Red
	0xff	// White
};

uint8_t *VIDEO_PALETTE_GAME = (uint8_t*)PALETTE_GAME_EGA;

// Player damage flash colors
const uint8_t PALETTE_DAMAGE_VGA[VGA_DAMAGE_FLASH_COUNT] = {
	0, 0, 0,		// Black (End of flash, reset to black)
	0, 0, 252,		// Red
	0, 0, 252,		// Red
	0, 0, 252,		// Orange
	204, 252, 252,	// Yellow
	204, 252, 252,	// Yellow
	0, 0, 252,		// Orange
	0, 0, 252,		// Red
	0, 0, 252		// Red (Start of flash) BGR ordering
};

const uint8_t PALETTE_DAMAGE_CGA[CGA_DAMAGE_FLASH_COUNT] = {
	48,		// Black (End of flash, reset to black with original palette)
	4,		// Red
	4,		// Red
	6,		// Orange
	14,		// Yellow
	14,		// Yellow
	6,		// Orange
	4,		// Red
	4		// Red (Start of flash)
};

const uint8_t PALETTE_DAMAGE_CGACOMP[CGA_DAMAGE_FLASH_COUNT] = {
	47,		// Black (End of flash, reset to black with original palette)
	44,		// Red
	44,		// Red
	45,		// Orange
	46,		// Yellow
	46,		// Yellow
	45,		// Orange
	44,		// Red
	44		// Red (Start of flash)
};

// EMP flash colors.
const uint8_t PALETTE_EMP_VGA[VGA_EMP_FLASH_COUNT] = {
	0, 0, 0,		// Black (End of flash, reset to black)
	0, 0, 0,		// Dark Blue
	252, 0, 0,		// Blue
	252, 0, 0,		// Light Blue
	252, 0, 0,		// Light Blue
	252, 252, 252,	// White
	252, 252, 252,	// White
	252, 252, 252,	// White
	252, 0, 0,		// Light Blue
	252, 0, 0,		// Light Blue
	252, 0, 0,		// Blue
	0, 0, 0			// Dark Blue (Start of flash) BGR ordering
};

const uint8_t PALETTE_EMP_CGA[CGA_EMP_FLASH_COUNT] = {
	48,		// Black (End of flash, reset to black)
	33,		// Dark Blue
	41,		// Blue
	43,		// Light Blue
	43,		// Light Blue
	47,		// White
	47,		// White
	47,		// White
	43,		// Light Blue
	43,		// Light Blue
	41,		// Blue
	33		// Dark Blue (Start of flash)
};

const uint8_t PALETTE_EMP_CGACOMP[CGA_EMP_FLASH_COUNT] = {
	47,		// Black (End of flash, reset to black)
	 1,		// Dark Blue
	33,		// Blue
	35,		// Light Blue
	35,		// Light Blue
	37,		// White
	37,		// White
	37,		// White
	35,		// Light Blue
	35,		// Light Blue
	33,		// Blue
	 1		// Dark Blue (Start of flash)
};

// Player health bar colors
const uint8_t PALETTE_HEALTH_VGA[12][3] = {
	{ 30,   0, 0},		// Red
	{ 30,   2, 0},		// Red
	{ 30,   2, 0},		// Red
	{ 14,   6, 0},		// Orange
	{ 12,  24, 0},		// Orange
	{ 12,  14, 0},		// Yellow
	{ 10,  30, 0},		// Green
	{ 10,  30, 0},		// Green
	{  6,  30, 0},		// Green
	{  4,  14, 0},		// Green
	{  2,  14, 0},		// Green
	{  0,  14, 0}		// Green
};

const uint8_t HEALTH_COLORS_ETGA[36] = {
	0x44, 0xc4, 0x04,	// Red
	0x44, 0xc4, 0x04,
	0x44, 0xc4, 0x04,
	0x64, 0xc6, 0x06,	// Orange
	0x64, 0xc6, 0x06,
	0xe0, 0xe8, 0x60,	// Yellow
	0xa2, 0xaa, 0xa0,	// Green
	0xa2, 0xaa, 0xa0,
	0x22, 0xa2, 0x20,	//
	0x22, 0xa2, 0x20,
	0x22, 0xa2, 0x20,
	0x22, 0xa2, 0x20
};

const uint8_t HEALTH_COLORS_VGA[36] = {
	HEALTHBARCOLOR,
	HEALTHBARCOLOR + 1,
	HEALTHBARCOLOR + 2,
	HEALTHBARCOLOR + 3,
	HEALTHBARCOLOR + 4,
	HEALTHBARCOLOR + 5,
	HEALTHBARCOLOR + 6,
	HEALTHBARCOLOR + 7,
	HEALTHBARCOLOR + 8,
	HEALTHBARCOLOR + 9,
	HEALTHBARCOLOR + 10,
	HEALTHBARCOLOR + 11,
	HEALTHBARCOLOR + 12,
	HEALTHBARCOLOR + 13,
	HEALTHBARCOLOR + 14,
	HEALTHBARCOLOR + 15,
	HEALTHBARCOLOR + 16,
	HEALTHBARCOLOR + 17,
	HEALTHBARCOLOR + 18,
	HEALTHBARCOLOR + 19,
	HEALTHBARCOLOR + 20,
	HEALTHBARCOLOR + 21,
	HEALTHBARCOLOR + 22,
	HEALTHBARCOLOR + 23,
	HEALTHBARCOLOR + 24,
	HEALTHBARCOLOR + 25,
	HEALTHBARCOLOR + 26,
	HEALTHBARCOLOR + 27,
	HEALTHBARCOLOR + 28,
	HEALTHBARCOLOR + 29,
	HEALTHBARCOLOR + 30,
	HEALTHBARCOLOR + 31,
	HEALTHBARCOLOR + 32,
	HEALTHBARCOLOR + 33,
	HEALTHBARCOLOR + 34,
	HEALTHBARCOLOR + 35
};

const uint8_t HEALTH_COLORS_EGA[36] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

const uint8_t HEALTH_COLORS_CGA[36] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

const uint8_t HEALTH_COLORS_CGACOMP[36] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

uint8_t *VIDEO_HEALTH_COLORS;

// Screen Shake Values
const uint8_t SCREEN_SHAKE_VAL[SCREEN_SHAKE_COUNT] = {
	0, 0,
	81, 0,
	162, 0,
	243, 0,
	68, 1,
	144, 1,
	225, 1,
	50, 2,
	131, 2,
	211, 2,
	0, 0,
	81, 0,
	162, 0,
	243, 0,
	68, 1,
	144, 1,
	225, 1,
	50, 2,
	131, 2,
	211, 2,
	0, 0,
	81, 0,
	162, 0,
	243, 0,
	68, 1,
	144, 1,
	225, 1,
	50, 2,
	131, 2,
	211, 2			// Start of screen shake
};

unsigned int WEAPON_AMMO[WEAPON_COUNT];
const char WEAPON_NAME[WEAPON_COUNT][11] = {
	"PISTOL\0    ",
	"PLASMA GUN\0",
};	

unsigned int ITEM_INVENTORY[ITEM_COUNT];
const char ITEM_NAME[ITEM_COUNT][11] = {
	"MEDKIT\0    ",
	"EMP DEVICE\0",
	"MAGNET\0    ",
	"TIMEBOMB\0  ",
};

// Initial base sprite/animation for this unit type
const uint8_t ANIM_INIT[19] = {
	255,	// Non-existent unit
	0,		// Player
	49,		// Left/Right Droid
	49,		// Up/Down Droid
	49,		// Hover Attack
	49,		// Water Droid
	255,	// Time Bomb
	255,	// Transporter Pad
	73,		// Dead Hoverbot
	57,		// Evilbot
	255,	// Door AI
	255,	// Small Explosion
	255,	// Pistol Fire Up
	255,	// Pistol Fire Down
	255,	// Pistol Fire Left
	255,	// Pistol Fire Right
	255,	// Trash Compactor
	53,		// Up/Down Rollerbot
	53		// Left/Right Rollerbot
};

const uint8_t DESTRUCT_PATH[TILE_COUNT] = {
	//00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0x09, 0xAA, 0xAA,
	0x09, 0xAA, 0xAA, 0x09, 0x09, 0xAA, 0xAA, 0x09, 0xAA, 0x2A, 0x09, 0xAA, 0xAA, 0x2E, 0x09, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x09, 0xAA, 0xAA, 0xAA, 0x09, // Last one is AA on Pet
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0x05, 0x08, 0xAA, 0x87, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBD, 0xC6, 0xAA, 0xAA, 0xAA, 0xAA, 0xCC, 0xCC, 0xAA, 0xAA,
	0xCE, 0xCF, 0xCE, 0xAA, 0xCE, 0xAA, 0xAA, 0xCE, 0xAA, 0xAA, 0xAA, 0xCF, 0xAA, 0xCC, 0xAA, 0xCF,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA, 0xAA
};

const uint8_t TILE_ATTRIB[TILE_COUNT] = {
	//00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F
	0x10, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, // 2 & 3 are AA on Pet
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x18, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x13, 0x13, // 7 is AA on Pet
	0x1C, 0x10, 0x13, 0x1C, 0x1C, 0x10, 0x13, 0x1C, 0x50, 0x5C, 0x1C, 0x50, 0x50, 0x5C, 0x1C, 0x50,
	0x00, 0x00, 0x00, 0x50, 0x00, 0x40, 0x40, 0x50, 0x00, 0x40, 0x1C, 0x50, 0x50, 0x50, 0x00, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
	0x13, 0x13, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x0C, 0x10, 0x10, 0x13, 0xAA, // 0 & 1 are AA on Pet
	0x40, 0x00, 0x10, 0xAA, 0x53, 0x13, 0x10, 0x40, 0x10, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x08, 0x08, 0xAA, 0x1C, 0x40, 0x53, 0xAA, 0x14, 0x53, 0x53, 0x40, 0x53, 0xAA, 0xAA, 0xAA, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x33, 0x13, 0x33, 0x33, 0x00, 0x00, 0x50, 0x50, 0x00, 0x00, 0x50, 0x13,
	0x13, 0x13, 0x13, 0x13, 0xAA, 0xAA, 0x40, 0x53, 0x53, 0x13, 0x40, 0x53, 0x00, 0x00, 0x00, 0xAA, // 0 & 1 & 2 are AA on Pet
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x10, 0x10, 0x10, 0x00, 0x10, 0x33, 0x10, 0x00, // 7 is AA on Pet
	0x10, 0x10, 0x10, 0x00, 0x40, 0x40, 0x1C, 0x5C, 0x40, 0x40, 0x00, 0x40, 0x12, 0x1B, 0x13, 0x13,
	0x1B, 0x1B, 0x1B, 0x00, 0x00, 0x00, 0x13, 0x18, 0x10, 0x10, 0x10, 0x08, 0x10, 0x10, 0x10, 0x08,
	0x10, 0x10, 0x10, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x10, 0x10, 0xAA, 0xAA,
	0xAA, 0xAA, 0x13, 0xAA
};

// Color for tiles on the mini-map
uint8_t MAP_TILE_COLORS_VGA[TILE_COUNT] = {
	0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x03, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x0F,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x05, 0x06, 0x0F, 0x0F, 0x0F, 0x0F, 0x08, 0x0D, 0x0A,
	0x04, 0x0C, 0x0A, 0x04, 0x04, 0x0C, 0x03, 0x04, 0x0A, 0x0C, 0x0C, 0x07, 0x0A, 0x0C, 0x0C, 0x07,
	0x0F, 0x0F, 0x0F, 0x07, 0x0F, 0x01, 0x01, 0x0A, 0x0F, 0x01, 0x04, 0x0A, 0x06, 0x06, 0x0F, 0x0F, 
	0x0F, 0x0F, 0x0B, 0x0C, 0x0F, 0x0F, 0x0F, 0x0F, 0x0B, 0x0B, 0x0B, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x0F, 0x0B, 0x0F, 0x0A, 0x0F, 0x0B, 0x0F, 0x0B, 0x0F, 0x0A, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0A, 
	0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0C, 0x0A, 0x0A, 0x0A, 0x04,
	0x0C, 0x0F, 0x0D, 0x0A, 0x03, 0x03, 0x0D, 0x0C, 0x0F, 0x0F, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x0F, 0x0F, 0x00, 0x09, 0x0F, 0x0F, 0x00, 0x01, 0x0F, 0x0F, 0x0A, 0x03, 0x0B, 0x0B, 0x0B, 0x0A,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0E, 0x03, 0x0E, 0x0E, 0x0F, 0x0F, 0x0A, 0x0A, 0x04, 0x04, 0x0A, 0x03,
	0x05, 0x05, 0x05, 0x03, 0x00, 0x00, 0x0F, 0x08, 0x03, 0x0A, 0x0F, 0x08, 0x0F, 0x0F, 0x0F, 0x00,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x05, 0x02, 0x02, 0x02, 0x0F, 0x02, 0x00, 0x02, 0x0F, 
	0x02, 0x02, 0x02, 0x0F, 0x0A, 0x0A, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 0x0A, 0x03, 0x02, 0x07, 0x02,
	0x01, 0x01, 0x01, 0x06, 0x0A, 0x0A, 0x05, 0x09, 0x0F, 0x0F, 0x0F, 0x06, 0x0F, 0x0F, 0x0F, 0x09,
	0x0F, 0x0F, 0x0F, 0x02, 0x05, 0x05, 0x02, 0x02, 0x05, 0x05, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x00
};

uint8_t MAP_TILE_COLORS_CGA[TILE_COUNT] = {
	0,0,3,3,3,3,3,3,3,0,3,3,3,3,0,3,
	3,3,3,3,3,3,3,2,1,3,3,3,3,1,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	3,3,3,1,3,1,1,1,3,1,1,1,1,1,1,3,
	3,3,3,3,3,3,3,3,2,2,2,3,3,3,3,3,
	3,2,3,1,3,2,3,1,3,2,3,3,3,3,3,1,
	2,2,0,0,0,0,0,0,3,3,3,1,1,1,1,2,
	3,3,1,1,0,0,1,3,3,3,2,2,2,2,2,2,
	3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,0,1,1,3,1,0,1,3,1,1,1,3,1,
	3,3,3,3,3,1,1,1,3,3,3,3,3,0,3,3,
	3,3,3,3,1,1,1,1,1,1,1,1,1,2,1,1,
	2,2,2,3,3,3,1,1,3,3,3,1,3,3,3,3,
	3,3,3,1,1,1,1,1,1,1,1,1,3,3,0,0,
	0,0,2,0
};

const uint8_t TILE_ANIMATED[TILE_COUNT] = {
	  0,  0, 76, 81,  0,  0,  0, 66,  0,  0, 56,  0,  0, 61,  0,  0, // 0   WALL_WATER, WALL2_WATER, WALL_FLOOR, WALL_GROUND, WALL2_GROUND
	 71,  0,  0,  0, 96,101,106,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 16  WALL2_FLOOR
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 32
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 48
	  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 64  FLAG
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 80
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 96
	  0,  0,  0,  0,  0,  0,  0,  0, 41,  0,  0,  0,  0,  0,  0,  0, // 112 DIVING_BOARD
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 31, // 128 MAINFRAME
	  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 144 COMPACTOR_FLOOR
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 160
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 86, // 176 WALL_ROOF
	  0,  0,  0, 91, 11, 16,  0,  0, 21, 26,  0,  0, 36,  0,  0,  0, // 192 WALL2_ROOF, HVAC_TL, HVAC_TR, HVAC_BL, HVAC_BR, WATER
	  0,  0,  0,  0,  0,  0,  0,  0,  0, 51,  0,  0,  0, 46,  0,  0, // 208 RAILING_GROUND, RAILING_WATER
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 224
	  0,  0,  0,  0	// 240
};

const uint8_t TILE_ANIMATION[111] = {
	// Animation Tile 1, 2, 3, 4, Unit Tile,
	  0,
	 66,   2,   3,   7, 0,							// Flag
	148,  10,  16, 241, 0,							// Trash Compactor Floor
	196, 243, 196, 243, 0,							// HVAC Top Left
	197, 120, 197, 120, 0,							// HVAC Top Right
	200, 191, 200, 191, 0,							// HVAC Bottom Left
	201, 195, 201, 195, 0,							// HVAC Bottom Right
	143, 217, 221, 240, 0,							// Mainframe
	204, 115, 130, 134, 0,							// Water
	204, 115, 130, 134, SPRITE_DIVING_BOARD,		// Diving Board/Water
	204, 115, 130, 134, SPRITE_RAILING,				// Railing/Water
	208, 208, 208, 208, SPRITE_RAILING, 			// Railing/Ground
	208, 208, 208, 208, SPRITE_WALL_TRANSITION_1, 	// Wall/Ground
	208, 208, 208, 208, SPRITE_WALL_TRANSITION_2, 	// Wall2/Ground
	  9,   9,   9,   9, SPRITE_WALL_TRANSITION_1,	// Wall/Floor
	  9,   9,   9,   9, SPRITE_WALL_TRANSITION_2,	// Wall2/Floor
	204, 115, 130, 134, SPRITE_WALL_TRANSITION_1,	// Wall/Water
	204, 115, 130, 134, SPRITE_WALL_TRANSITION_2,	// Wall2/Water
	189, 189, 189, 189, SPRITE_WALL_TRANSITION_1,	// Wall/Roof
	189, 189, 189, 189, SPRITE_WALL_TRANSITION_2,	// Wall2/Roof
	 20,  20,  20,  20, SPRITE_CINEMA_L,			// Cinema Left
	 21,  21,  21,  21, SPRITE_CINEMA_M,			// Cinema Middle
	 22,  22,  22,  22, SPRITE_CINEMA_R				// Cinema Right
};


