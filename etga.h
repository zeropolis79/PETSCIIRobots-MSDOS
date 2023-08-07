#ifndef __ETGA_H_INCLUDED__
#define __ETGA_H_INCLUDED__

#include <stdint.h>

// Function pointers for calling routine for correct video mode
void ETGA_videoInit(int key);
void ETGA_videoSetMode(void);
void ETGA_videoUnsetMode(void);
void ETGA_videoDrawFullScreen(uint16_t segment);
void ETGA_videoPrintTileMultibyte(uint16_t x, uint16_t y, uint16_t tileseg, uint16_t w, uint16_t h);
void ETGA_videoTranslate(uint8_t *source, uint8_t *dest, uint16_t w, uint8_t h, const uint8_t *palette, uint8_t multi_byte);
void ETGA_videoPaletteSet(const uint8_t *palette, uint8_t fade_bit_shift);

#endif //__ETGA_H_INCLUDED__
