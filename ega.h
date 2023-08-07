#ifndef __EGA_H_INCLUDED__
#define __EGA_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void EGA_videoInit(int key);
void EGA_videoSetMode(void);
void EGA_videoUnsetMode(void);
void EGA_videoClearScreen(void);
void EGA_videoDrawFullScreen(uint16_t segment);
void EGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void EGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void EGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
void EGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
void EGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void EGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment);
void EGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3);
void EGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void EGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
uint8_t EGA_videoGetPixel(uint16_t x, uint16_t y);
uint8_t EGA_videoGetUnitColorForMinimap(uint8_t unit);
void EGA_minimapSet(uint8_t x, uint8_t y, uint8_t color);
void EGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y);
void EGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y);

#endif //__EGA_H_INCLUDED__
