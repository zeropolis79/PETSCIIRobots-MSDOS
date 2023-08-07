#ifndef __VGA_H_INCLUDED__
#define __VGA_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void VGA_videoInit(int key);
void VGA_videoSetMode(void);
void VGA_videoUnsetMode(void);
void VGA_videoPaletteSet(const uint8_t *palette, uint8_t fade_bit_shift);
void VGA_videoClearScreen(void);
void VGA_videoDrawFullScreen(uint16_t segment);
void VGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void VGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void VGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
void VGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
void VGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void VGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment);
void VGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3);
void VGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
uint8_t VGA_videoGetPixel(uint16_t x, uint16_t y);
uint16_t VGA_videoMaskTranslate(uint8_t *data, uint16_t masksize);
uint8_t VGA_videoGetUnitColorForMinimap(uint8_t unit);
void VGA_videoDamageFlashStep(void);
void VGA_videoEMPFlashStep(void);
void VGA_videoAnimateCursorAndRobotColors(void);
void VGA_videoFadeIn(const uint8_t *palette, uint8_t timer);
void VGA_videoFadeOut(const uint8_t *palette, uint8_t timer);
void VGA_minimapSet(uint8_t x, uint8_t y, uint8_t color);
void VGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y);
void VGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y);

#endif //__VGA_H_INCLUDED__
