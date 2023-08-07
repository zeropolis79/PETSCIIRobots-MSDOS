#ifndef __VIDEO_H_INCLUDED__
#define __VIDEO_H_INCLUDED__

#include <stdint.h>

#define TEXT	0
#define BITMAP	1

#define CGA			0x04
#define CGACOMP 	0x06
#define EGA			0x0d
#define VGA			0x13
#define TDY_LOW		0x08
#define NO_MODE		0xff

#define FADE_INSTANT	0
#define FADE_SPEED		4

extern uint8_t VIDEO_MODE;
extern uint8_t VIDEO_MODE_ON_STARTUP;
extern uint16_t VIDEO_SCREENSEG;
extern uint16_t VIDEO_SCREENOFFSET[200];
extern uint8_t VIDEO_TILEW_OFFSET[11];
extern uint8_t VIDEO_TILEH_OFFSET[7];
extern uint8_t VIDEO_COMPOSITE;
extern uint8_t VIDEO_PLANESHIFT;
extern uint8_t VIDEO_BITSHIFT;
extern uint8_t VIDEO_FILESHIFT;
extern uint8_t VIDEO_MULTIBYTE_BITSHIFT;
extern uint8_t VIDEO_FILE_MULTIBYTE;
extern uint8_t VIDEO_SKIP_TRANSLATE_MULTIBYTE;
extern uint8_t VIDEO_FONT_COLOR_MENU;
extern uint8_t VIDEO_FONT_COLOR_MENU_SELECTED;
extern uint8_t VIDEO_FONT_COLOR_INFO;
extern uint8_t VIDEO_FONT_COLOR_WHITE;
extern uint8_t VIDEO_FONT_COLOR_GAMEOVER;
extern uint16_t VIDEO_SCREENSIZE;
extern uint16_t VIDEO_GAMEOVERSIZE;
extern uint16_t VIDEO_HUDSIZE;
extern uint16_t VIDEO_TILE_W;
extern uint16_t VIDEO_TILE_H;
extern uint16_t VIDEO_ITEM_W;
extern uint16_t VIDEO_ITEM_H;
extern uint16_t VIDEO_KEY_W;
extern uint16_t VIDEO_KEY_H;
extern uint16_t VIDEO_HEALTH_W;
extern uint16_t VIDEO_HEALTH_H;
extern uint16_t VIDEO_FACE_X;
extern uint16_t VIDEO_FACE_Y;
extern uint16_t VIDEO_FACE_W;
extern uint16_t VIDEO_FACE_H;
extern uint16_t VIDEO_FONT_W;
extern uint16_t VIDEO_FONT_H;
extern char VIDEO_FILEEXT[5];
extern uint8_t INFOLINE;
extern uint8_t *VIDEO_ACTIVE_PALETTE;
extern uint8_t VIDEO_USE_PALETTE_ANIMATION;

// Function pointers for calling routine for correct video mode
extern void (*videoSetMode)(void);
extern void (*videoUnsetMode)(void);
extern void (*videoClearScreen)(void);
extern void (*videoDrawFullScreen)(uint16_t segment);
extern void (*videoPrintTile)(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
extern void (*videoPrintTileMultibyte)(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
extern void (*videoPrintSprite)(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
extern void (*videoPrintSpriteAsColor)(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
extern void (*videoPrintFontAsColor)(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
extern void (*videoCinemaAddChar)(uint16_t letter_segment, uint16_t dest_segment);
extern void (*videoCinemaRotate)(uint16_t tile1, uint16_t tile2, uint16_t tile3);
extern void (*videoFillRect)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
extern void (*videoTranslate)(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
extern uint16_t (*videoMaskTranslate)(uint8_t *source, uint16_t masksize);
extern uint8_t (*videoGetPixel)(uint16_t x, uint16_t y);
extern void (*videoPaletteSet)(const uint8_t *palette, uint8_t fade_bit_shift);
extern uint8_t (*videoGetUnitColorForMinimap)(uint8_t unit);
extern void (*videoDamageFlashStep)(void);
extern void (*videoEMPFlashStep)(void);
extern void (*videoScreenShakeStep)(void);
extern void (*videoFadeIn)(const uint8_t *palette, uint8_t timer);
extern void (*videoFadeOut)(const uint8_t *palette, uint8_t timer);
extern void (*minimapSet)(uint8_t x, uint8_t y, uint8_t color);
extern void (*minimapPrepareLine)(uint8_t *vidptr2, uint8_t y);
extern void (*minimapCopyLineToScreen)(uint8_t *vidptr2, uint8_t y);

extern void dummy_videoSetPalette(const uint8_t *palette, uint8_t fade_bit_shift);
extern void dummy_videoFadeIn(const uint8_t *palette, uint8_t timer);
extern void dummy_videoFadeOut(const uint8_t *palette, uint8_t timer);

int videoSelect(const char *arg);
void videoPrintString(uint16_t x, uint16_t y, uint8_t text, const char *str, uint16_t color);
void videoInfoClear(void);
void videoInfoPrint(const char *str);

#endif //__VIDEO_H_INCLUDED__
