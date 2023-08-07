#ifndef __PCPLUS_H_INCLUDED__
#define __PCPLUS_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void PCP_videoInit(int key);
void PCP_videoSetMode(void);
void PCP_videoUnsetMode(void);
void PCP_videoClearScreen(void);
void PCP_videoDrawFullScreen(uint16_t segment);
void PCP_videoPrintTile(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void PCP_videoPrintSprite(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t tileseg);
void PCP_videoPrintSpriteAsColor(uint16_t x, uint16_t y, uint16_t spriteseg, uint16_t color);
void PCP_videoPrintFontAsColor(uint16_t x, uint16_t y, uint16_t fontseg, uint16_t color);
void PCP_videoCinemaAddChar(uint16_t letter_segment, uint16_t dest_segment);
void PCP_videoCinemaRotate(uint16_t tile1, uint16_t tile2, uint16_t tile3);
void PCP_videoFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void PCP_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
uint8_t PCP_videoGetPixel(uint16_t x, uint16_t y);
void PCP_minimapSet(uint8_t x, uint8_t y, uint8_t color);
void PCP_minimapPrepareLine(uint8_t *vidptr2, uint8_t y);
void PCP_minimapCopyLineToScreen(uint8_t *vidptr2, uint8_t y);

#endif //__PCPLUS_H_INCLUDED__
