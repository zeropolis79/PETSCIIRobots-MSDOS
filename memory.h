#ifndef __MEMORY_H_INCLUDED__
#define __MEMORY_H_INCLUDED__

#include <stdint.h>

void memoryInit();
uint16_t memoryAssign(uint16_t segment_size, uint8_t istemp);
uint8_t* memorySegToPtr(uint16_t segm);
uint8_t* memorySegOfsToPtr(uint16_t segm, uint16_t ofs);

#endif //__MEMORY_H_INCLUDED__
