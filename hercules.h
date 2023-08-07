#ifndef __HERCULES_H_INCLUDED__
#define __HERCULES_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void HGC_videoInit(int key);
void HGC_videoSetMode(void);
void HGC_videoUnsetMode(void);
// void CGA_videoClearScreen();
// void CGA_videoDrawFullScreen(uint16_t segment);
// void CGA_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
// void CGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
// void CGA_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
// void CGA_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
// void CGA_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
// void CGA_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment);
// void CGA_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3);
// void CGA_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
// // void CGA_videoChangeColor(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t bitplane);
// void CGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
// uint16_t CGA_videoMaskTranslate(uint8_t *data, uint16_t masksize);
// uint8_t CGA_videoGetPixel(uint16_t x, uint16_t y);
void HGC_videoScreenShakeStep(void);

#endif //__HERCULES_H_INCLUDED__
