#ifndef __FILES_H_INCLUDED__
#define __FILES_H_INCLUDED__

#include <stdint.h>

uint8_t fileRead(uint8_t *data, const char *name, const char *ext, unsigned long maxlen);
uint8_t fileReadTiles(const char *name, const char *ext, unsigned int width, unsigned int height, uint16_t data[], unsigned int count);
int fileReadMap(int levelnum);

#endif //__FILES_H_INCLUDED__
