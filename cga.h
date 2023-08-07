#ifndef __CGA_H_INCLUDED__
#define __CGA_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void CGA_videoInit(int key);
void CGA_videoSetMode(void);
void CGA_videoUnsetMode(void);
void CGA_videoClearScreen(void);
void CGA_videoDrawFullScreen(uint16_t segment);
void CGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void CGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void CGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
void CGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
void CGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void TDY_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void CGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment);
void CGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3);
void CGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
// void CGA_videoChangeColor(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t bitplane);
void CGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
uint16_t CGA_videoMaskTranslate(uint8_t *data, uint16_t masksize);
uint8_t CGA_videoGetPixel(uint16_t x, uint16_t y);
uint8_t CGA_videoGetUnitColorForMinimap(uint8_t unit);
void CGA_videoDamageFlashStep(void);
void CGA_videoEMPFlashStep(void);
void CGA_videoScreenShakeStep(void);
void CGA_minimapSet(uint8_t x, uint8_t y, uint8_t color);
void CGA_minimapPrepareLine(uint8_t *vidptr2, uint8_t y);
void CGACOMP_minimapPrepareLine(uint8_t *vidptr2, uint8_t y);
void CGA_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y);

#endif //__CGA_H_INCLUDED__
