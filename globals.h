#define TILE_COUNT					244
#define SPRITE_COUNT				104
#define ITEMS_COUNT					6
#define HEALTH_COUNT				6
#define KEY_COUNT					3
#define FACE_COUNT					3
#define FONT_COUNT					256
#define PALETTE_COUNT				16
#define VGA_DAMAGE_FLASH_COUNT		27
#define CGA_DAMAGE_FLASH_COUNT		9
#define VGA_EMP_FLASH_COUNT			36
#define CGA_EMP_FLASH_COUNT			12
#define SCREEN_SHAKE_COUNT			60
#define UNIT_COUNT					81
#define MAP_UNIT_COUNT				64
#define WEAPON_COUNT				2
#define ITEM_COUNT 					4
#define MAP_COUNT					15
#define MAP_WIDTH 					128
#define MAP_HEIGHT 					64
#define MAP_PRECALC_COUNT			77	// Map window 11 tiles wide * 7 tiles high
#define HEALTHBARCOLOR				20	// Start of the color palette for the health bar
#define CUSTOM_KEYS_COUNT			13

// Weapons
#define PISTOL						1
#define PLASMA						2

// Items
#define MEDKIT						1
#define EMP							2
#define MAGNET						3
#define TIMEBOMB					4

// Tiles
#define TILE_WALL_TRANS_1_WATER		2
#define TILE_WALL_TRANS_2_WATER		3
#define TILE_WALL_TRANS_1_FLOOR		7
#define TILE_FLOOR					9
#define TILE_WALL_TRANS_1_GROUND	10
#define TILE_WALL_TRANS_2_GROUND	13
#define TILE_DOOR_VERT_OPEN_B		15
#define TILE_WALL_TRANS_2_FLOOR		16
#define TILE_DOOR_HORIZ_OPEN_L		17
#define TILE_CINEMA_L				20
#define TILE_CINEMA_M				21
#define TILE_CINEMA_R				22
#define TILE_DOOR_VERT_OPEN_T		27
#define TILE_TRANSPORTER_ANIM_1		30
#define TILE_TRANSPORTER_ANIM_2		31
#define TILE_CRATE_BIG				41
#define TILE_CRATE_SMALL			45
#define TILE_FLAG					66
#define TILE_DOOR_VERT_CLOSED_T		68
#define TILE_DOOR_VERT_TRANS1_T		69
#define TILE_DOOR_VERT_TRANS2_T		70
#define TILE_DOOR_VERT_CLOSED_M		72
#define TILE_DOOR_VERT_TRANS1_M		73
#define TILE_DOOR_VERT_TRANS2_M		74
#define TILE_DOOR_VERT_CLOSED_B		76
#define TILE_DOOR_VERT_TRNAS1_B		77
#define TILE_DOOR_VERT_TRANS2_B		78
#define TILE_DOOR_HORIZ_CLOSED_L	80
#define TILE_DOOR_HORIZ_CLOSED_M	81
#define TILE_DOOR_HORIZ_CLOSED_R	82
#define TILE_DOOR_HORIZ_TRANS1_L	84
#define TILE_DOOR_HORIZ_TRANS1_M	85
#define TILE_DOOR_HORIZ_TRANS_R		86
#define TILE_DOOR_HORIZ_TRANS2_L	88
#define TILE_DOOR_HORIZ_TRANS2_M	89
#define TILE_DOOR_HORIZ_OPEN_R		91
#define TILE_DIVING_BOARD			120
#define TILE_EXP_CANNISTER			131
#define TILE_BLOWN_CANNISTER		135
#define TILE_ROBOT_WATER_ANIM_START	140
#define TILE_ROBOT_WATER_ANIM_END	142
#define TILE_MAINFRAME				143
#define TILE_TRASH_COMP_OPEN_TL		144
#define TILE_TRASH_COMP_OPEN_TR		145
#define TILE_TRASH_COMP_TRANS_TL	146
#define TILE_TRASH_COMP_TRANS_TR	147
#define TILE_TRASH_COMPACTOR_FLOOR	148
#define TILE_TRASH_COMP_TRANS_BL	150
#define TILE_TRASH_COMP_TRANS_BR	151
#define TILE_TRASH_COMP_CLOSED_TL	152
#define TILE_TRASH_COMP_CLOSED_TR	153
#define TILE_TRASH_COMP_CLOSED_BL	156
#define TILE_TRASH_COMP_CLOSED_BR	157
#define TILE_ELEV_HORIZ_OPEN_R		172
#define TILE_ELEV_HORIZ_TRANS2_R	173
#define TILE_ELEV_HORIZ_CLOSED_R	174
#define TILE_ELEV_HORIZ_TRANS2_L	181
#define TILE_ELEV_HORIZ_OPEN_L		182
#define TILE_ROOF					189
#define TILE_WALL_TRANS_1_ROOF		191
#define TILE_WALL_TRANS_2_ROOF		195
#define TILE_HVAC_TL				196
#define TILE_HVAC_TR				197
#define TILE_CRATE_PI				199
#define TILE_HVAC_BL				200
#define TILE_HVAC_BR				201
#define TILE_WATER					204
#define TILE_GROUND					208
#define TILE_RAILING_GROUND			217
#define TILE_RAILING_WATER			221
#define TILE_RAFT					242
#define TILE_NOT_PLOTTED			255

// Sprites
#define SPRITE_PLAYER_DOWN			0
#define SPRITE_PLAYER_RIGHT			4
#define SPRITE_PLAYER_UP			8
#define SPRITE_PLAYER_LEFT			12
#define SPRITE_PLAYER_DEAD			48
#define SPRITE_HOVERBOT				49
#define SPRITE_ROLLERBOT			53
#define SPRITE_EVILBOT_DOWN			57
#define SPRITE_EVILBOT_RIGHT		61
#define SPRITE_EVILBOT_UP			65
#define SPRITE_EVILBOT_LEFT			69
#define SPRITE_HOVERBOT_DEAD		73
#define SPRITE_ROLLERBOT_DEAD		74
#define SPRITE_EVILBOT_DEAD			75
#define SPRITE_DEMATERIALIZE		76
#define SPRITE_PISTOL_VERT			83
#define SPRITE_PISTOL_HORIZ			84
#define SPRITE_PLASMA_VERT			85
#define SPRITE_PLASMA_HORIZ			86
#define SPRITE_BOMB					87
#define SPRITE_MAGNET				88
#define SPRITE_EXPLOSION			89
#define SPRITE_RAILING				94
#define SPRITE_DIVING_BOARD			95
#define SPRITE_WALL_TRANSITION_1	96
#define SPRITE_WALL_TRANSITION_2	97
#define SPRITE_CINEMA_L				98
#define SPRITE_CINEMA_M				99
#define SPRITE_CINEMA_R				100
#define SPRITE_CURSOR_USE			101
#define SPRITE_CURSOR_SEARCH		102
#define SPRITE_CURSOR_MOVE			103

// Units
#define UNIT_PLAYER					0
#define UNIT_NOT_FOUND				255

// Unit types
#define TYPE_NONE					0
#define TYPE_PLAYER					1
#define TYPE_HOVERBOT_HORIZ			2
#define TYPE_HOVERBOT_VERT			3
#define TYPE_HOVERBOT_ATTACK		4
#define TYPE_HOVERBOT_WATER			5
#define TYPE_TIMEBOMB				6
#define TYPE_TRANSPORTER_PAD		7
#define TYPE_DEAD_ROBOT				8
#define TYPE_EVILBOT				9
#define TYPE_DOOR					10
#define TYPE_EXPLOSION_SMALL		11
#define TYPE_WEAPON_FIRE_UP			12
#define TYPE_WEAPON_FIRE_DOWN		13
#define TYPE_WEAPON_FIRE_LEFT		14
#define TYPE_WEAPON_FIRE_RIGHT		15
#define TYPE_TRASH_COMPACTOR		16
#define TYPE_ROLLERBOT_VERT			17
#define TYPE_ROLLERBOT_HORIZ		18
#define TYPE_ELEVATOR				19
#define TYPE_MAGNET					20
#define TYPE_MAGNETIZED_ROBOT		21
#define TYPE_WATER_RAFT_HORIZ		22
#define TYPE_DEMATERIALIZE			23
#define TYPE_HIDDEN_KEY				128
#define TYPE_HIDDEN_TIMEBOMB		129
#define TYPE_HIDDEN_EMP				130
#define TYPE_HIDDEN_PISTOL			131
#define TYPE_HIDDEN_PLASMA			132
#define TYPE_HIDDEN_MEDKIT			133
#define TYPE_HIDDEN_MAGNET			134
#define TYPE_PLAYER_LOSES			253
#define TYPE_PLAYER_WINS			254

// Tile attributes
#define ATTRIB_CAN_WALK_OVER		1
#define ATTRIB_CAN_HOVER_OVER		2
#define ATTRIB_MOVABLE				4
#define ATTRIB_CAN_BE_DESTROYED		8
#define ATTRIB_CAN_SEE_THROUGH		16
#define ATTRIB_CAN_MOVE_ONTO		32
#define ATTRIB_SEARCHABLE			64

// Custom keys
#define KEYS_MOVE_UP				0
#define KEYS_MOVE_DOWN				1
#define KEYS_MOVE_LEFT				2
#define KEYS_MOVE_RIGHT				3
#define KEYS_FIRE_UP				4
#define KEYS_FIRE_DOWN				5
#define KEYS_FIRE_LEFT				6
#define KEYS_FIRE_RIGHT				7
#define KEYS_CYCLE_WEAPONS			8
#define KEYS_CYCLE_ITEMS			9
#define KEYS_USE_ITEM				10
#define KEYS_SEARCH_OBJECT			11
#define KEYS_MOVE_OBJECT			12

#ifndef __GLOBALS_H_INCLUDED__
#define __GLOBALS_H_INCLUDED__

#include <stdint.h>

// Struct the hold clock information
typedef struct clockStruct{
	uint8_t ACTIVE;
	uint8_t CYCLES;
	uint8_t SECONDS;
	uint8_t MINUTES;
	uint16_t HOURS;
	uint8_t UPDATE;
} clockStruct;

// Struct the hold cursor information (for searching, moving, placing items). If X=0 then cursor is off.
typedef struct cursorStruct{
	uint8_t X;
	uint8_t Y;
	uint8_t timer;
	int color;
	int last_color;
	uint8_t color_index;
	uint8_t animating;
	uint8_t sprite;
} cursorStruct;

// Make these global variables available to other scripts by flagging them 'extern'
volatile extern uint8_t BGTIMER1;
volatile extern uint8_t KEYTIMER;
volatile extern uint8_t JOYTIMER;
volatile extern uint8_t EVENT_TIMER;
extern uint8_t PROGRAM_ABORT;
extern uint8_t KEYFAST;
extern uint8_t KEY_NOREPEAT;
extern uint8_t LEVEL_SELECTED;
extern uint8_t DIFFICULTY_LEVEL;
extern uint8_t MAGNET_ACTIVE;
extern uint8_t PLASMA_ACTIVE;
extern uint8_t BIG_EXPLOSION_ACTIVE;
extern uint8_t MAP_WINDOW_X;
extern uint8_t MAP_WINDOW_Y;
extern uint8_t REDRAW_WINDOW;
extern uint8_t SOUND_ENABLED;
extern uint8_t CURSOR_ENABLED;
extern uint8_t EVENT_ENABLED;
extern uint8_t DAMAGE_FLASH_STEPS_REMAINING;
extern uint8_t VIDEO_DAMAGE_FLASH_COUNT;
extern uint8_t EMP_FLASH_STEPS_REMAINING;
extern uint8_t VIDEO_EMP_FLASH_COUNT;
extern uint8_t TILE_ANIMATION_FRAME;
extern uint8_t CINEMA_LAST_FRAME;
extern uint8_t CINEMA_ANIMATION_FRAME;
extern uint8_t CINEMA_INDEX;
extern uint8_t SCREEN_SHAKE;
extern uint8_t MINIMAP_OPEN;
extern uint8_t MINIMAP_ROBOTS_ENABLED;
extern uint8_t MINIMAP_PROXIMITY;
extern uint8_t MINIMAP_X_OFFSET;
extern uint8_t TRANSPARENCY;
extern clockStruct CLOCK;

extern uint8_t UNIT_TYPE[UNIT_COUNT];
extern uint8_t UNIT_X[UNIT_COUNT];
extern uint8_t UNIT_Y[UNIT_COUNT];
extern uint8_t UNIT_A[UNIT_COUNT];
extern uint8_t UNIT_B[UNIT_COUNT];
extern uint8_t UNIT_C[UNIT_COUNT];
extern uint8_t UNIT_D[UNIT_COUNT];
extern uint8_t UNIT_HEALTH[UNIT_COUNT];
extern uint8_t UNIT_TIMER_A[UNIT_COUNT];
extern uint8_t UNIT_TIMER_B[UNIT_COUNT];
extern uint8_t UNIT_TILE[UNIT_COUNT];
extern uint8_t UNIT_TIMER_ANIM[UNIT_COUNT];
extern uint8_t UNIT_ANIM_BASE[UNIT_COUNT];
extern uint8_t UNIT_ANIM_FRAME[UNIT_COUNT];
extern uint8_t UNIT_IS_SPRITE[UNIT_COUNT];
extern uint8_t MAP[MAP_HEIGHT][MAP_WIDTH];
extern uint8_t PREVIOUS_DRAW_TILE[7][11];
extern uint8_t PREVIOUS_DRAW_UNIT[7][11];
extern const uint8_t ANIM_INIT[19];
extern const uint8_t DESTRUCT_PATH[TILE_COUNT];
extern const uint8_t TILE_ATTRIB[TILE_COUNT];
extern uint8_t MAP_TILE_COLORS_VGA[TILE_COUNT];
extern uint8_t MAP_TILE_COLORS_CGA[TILE_COUNT];
extern const uint8_t TILE_ANIMATED[TILE_COUNT];
extern const uint8_t TILE_ANIMATION[111];
extern uint8_t KEY_INVENTORY[4];
extern uint8_t CUSTOM_KEYS[CUSTOM_KEYS_COUNT];
const extern uint8_t CUSTOM_KEYS_DEFAULTS[CUSTOM_KEYS_COUNT];
extern cursorStruct CURSOR;

// Segment data for graphics/sound
extern uint16_t AUDIO_DATA;
extern uint16_t MUSIC_TITLE;
extern uint16_t MUSIC_WIN;
extern uint16_t MUSIC_LOSE;
extern uint16_t MUSIC_GAME;
extern uint16_t SOUNDFX;
extern uint16_t TITLE;
extern uint16_t GAMEOVER;
extern uint16_t HUD;
extern uint16_t TILES[TILE_COUNT];
extern uint16_t SPRITES[SPRITE_COUNT];
extern uint16_t ITEMS[ITEMS_COUNT];
extern uint16_t HEALTH[HEALTH_COUNT];
extern uint16_t KEYS[KEY_COUNT];
extern uint16_t FACES[FACE_COUNT];
extern uint16_t FONT[FONT_COUNT];
extern uint16_t MINIMAP[64];
extern uint8_t *MAP_TILE_COLORS;

extern const char MAP_NAMES[MAP_COUNT][17];
extern const char MUSIC_FILENAMES[4][9];

extern const char CINEMA_TEXT[];

extern uint8_t CURSOR_SELECTED_COLOR[32];
extern const uint8_t PALETTE_TITLE_VGA[PALETTE_COUNT][3];
extern const uint8_t PALETTE_TITLE_EGA[PALETTE_COUNT];
extern const uint8_t PALETTE_TITLE_ETGA[PALETTE_COUNT];
extern uint8_t *VIDEO_PALETTE_TITLE;
extern const uint8_t PALETTE_GAMEOVER_VGA[PALETTE_COUNT][3];
extern const uint8_t PALETTE_GAMEOVER_EGA[PALETTE_COUNT];
extern const uint8_t PALETTE_GAMEOVER_ETGA[PALETTE_COUNT];
extern uint8_t *VIDEO_PALETTE_GAMEOVER;
extern const uint8_t PALETTE_GAME_VGA[PALETTE_COUNT][3];
extern const uint8_t PALETTE_GAME_EGA[PALETTE_COUNT];
extern const uint8_t PALETTE_GAME_ETGA[PALETTE_COUNT];
extern uint8_t *VIDEO_PALETTE_GAME;
extern const uint8_t PALETTE_DAMAGE_VGA[VGA_DAMAGE_FLASH_COUNT];
extern const uint8_t PALETTE_DAMAGE_CGA[CGA_DAMAGE_FLASH_COUNT];
extern const uint8_t PALETTE_DAMAGE_CGACOMP[CGA_DAMAGE_FLASH_COUNT];
extern const uint8_t PALETTE_EMP_VGA[VGA_EMP_FLASH_COUNT];
extern const uint8_t PALETTE_EMP_CGA[CGA_EMP_FLASH_COUNT];
extern const uint8_t PALETTE_EMP_CGACOMP[CGA_EMP_FLASH_COUNT];
extern const uint8_t PALETTE_HEALTH_VGA[12][3];
extern const uint8_t HEALTH_COLORS_ETGA[36];
extern const uint8_t HEALTH_COLORS_VGA[36];
extern const uint8_t HEALTH_COLORS_EGA[36];
extern const uint8_t HEALTH_COLORS_CGA[36];
extern const uint8_t HEALTH_COLORS_CGACOMP[36];
extern uint8_t *VIDEO_HEALTH_COLORS;
extern const uint8_t SCREEN_SHAKE_VAL[SCREEN_SHAKE_COUNT];

extern unsigned int WEAPON_AMMO[WEAPON_COUNT];
extern const char WEAPON_NAME[WEAPON_COUNT][11];

extern unsigned int ITEM_INVENTORY[ITEM_COUNT];
extern const char ITEM_NAME[ITEM_COUNT][11];

#endif //__GLOBALS_H_INCLUDED__
