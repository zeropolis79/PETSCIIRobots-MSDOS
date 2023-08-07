#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "main.h"
#include "memory.h"

#define SEGMENT_COUNT				7

// Memory blocks
uint16_t SEGMENT_BLOCK[SEGMENT_COUNT];
uint16_t SEGMENT_AVAILABLE[SEGMENT_COUNT];
uint16_t SEGMENT_TOTAL_AVAILABLE;

// Data to remember the original memory block values when using temporary memory blocks
uint8_t TEMP_MEMORY_IN_USE;
uint16_t SAVED_SEGMENT_BLOCK[SEGMENT_COUNT];
uint16_t SAVED_SEGMENT_AVAILABLE[SEGMENT_COUNT];
uint16_t SAVED_SEGMENT_TOTAL_AVAILABLE;


void memoryInit(){
	uint8_t i;
	
	TEMP_MEMORY_IN_USE = 0;
	SEGMENT_TOTAL_AVAILABLE = 0;
	for (i = 0; i < SEGMENT_COUNT; i++){
		SEGMENT_BLOCK[i] = 0;
		SEGMENT_AVAILABLE[i] = 0;
	}
}

// Allocate large blocks of memory to store graphics/music/etc
void memoryAlloc(){
	uint8_t *data, i;
	uint16_t size, segm, off;

	for (i = 0; i < SEGMENT_COUNT; i++){
		if (SEGMENT_BLOCK[i] != 0){
			// This block already allocated, skip until we find one that isn't.
			continue;
		}
		// Max size that can be allocated
		size = 65504;
		while (size > 5000){
			data = malloc(size);
			if (data == NULL){
				size -= 4096;
				continue;
			}
			break;
		}
		if (data != NULL){
			// Fetch segment and offset of allocated ram in the variables 'segm' and 'off'
			_asm{
				push ds
				push si
				lds si, data
				mov off, si
				mov segm, ds
				pop si
				pop ds
			}
			// Align to the start of the first available segment in allocated block
			while (off > 0){
				if (off > 15){
					off -= 16;
				} else{
					size -= off;
					off = 0;
				}
				segm++;
			}
			size >>= 4;
			// Save the starting segment and how many available segments there are within this block
			SEGMENT_BLOCK[i] = segm;
			SEGMENT_AVAILABLE[i] = size;
			SEGMENT_TOTAL_AVAILABLE += size;
			break;
		}
	}
}

// Returns an unassigned block of memory of requested size. The segment is returned (offset is always 0)
// Segment size should be the size in segments (bytes / 16)
// 'istemp' means this is temporary memory, like for loading files. Temp memory is restored the next time non-temp memory is requested.
uint16_t memoryAssign(uint16_t segment_size, uint8_t istemp){
	uint8_t i, j, tries;
	uint16_t assigned_segment;
	
	// Restore saved memory settings if non-temporary memory is requested and previous request used tempoary memory
	if (!istemp && TEMP_MEMORY_IN_USE){
		TEMP_MEMORY_IN_USE = 0;
		SEGMENT_TOTAL_AVAILABLE = SAVED_SEGMENT_TOTAL_AVAILABLE;
		for (i = 0; i < SEGMENT_COUNT; i++){
			SEGMENT_BLOCK[i] = SAVED_SEGMENT_BLOCK[i];
			SEGMENT_AVAILABLE[i] = SAVED_SEGMENT_AVAILABLE[i];
		}
	}
	// Try to find a pre-allocated block of memory (aligned by segment), on second attempt try to allocate more memory
	for (tries = 0; tries < 2; tries++){
		assigned_segment = 0;
		for (i = 0; i < SEGMENT_COUNT; i++){
			if (SEGMENT_AVAILABLE[i] >= segment_size){
				assigned_segment = SEGMENT_BLOCK[i];
				// Save current memory settings if some temporary memory is requested
				if (istemp && !TEMP_MEMORY_IN_USE){
					TEMP_MEMORY_IN_USE = 1;
					SAVED_SEGMENT_TOTAL_AVAILABLE = SEGMENT_TOTAL_AVAILABLE;
					for (j = 0; j < SEGMENT_COUNT; j++){
						SAVED_SEGMENT_BLOCK[j] = SEGMENT_BLOCK[j];
						SAVED_SEGMENT_AVAILABLE[j] = SEGMENT_AVAILABLE[j];
					}
				}
				SEGMENT_BLOCK[i] += segment_size;
				SEGMENT_AVAILABLE[i] -= segment_size;
				SEGMENT_TOTAL_AVAILABLE -= segment_size;
			}
		}
		if (assigned_segment != 0 || tries == 1){
			break;
		}
		// Attempt to allocate more memory, since we didn't find enough pre-allocated
		memoryAlloc();
	}
	return assigned_segment;
}

// Convert a 16-bit int that holds a segment value to a pointer
uint8_t* memorySegToPtr(uint16_t segm){
	uint8_t *segptr;

	_asm{
		mov ax, segm
		mov word ptr [segptr + 2], ax
		mov word ptr [segptr], 0
	}
	return segptr;
}

// Convert a 16-bit int pair that holds segment and offset values to a pointer
uint8_t* memorySegOfsToPtr(uint16_t segm, uint16_t ofs){
	uint8_t *segptr;

	_asm{
		mov ax, segm
		mov word ptr [segptr + 2], ax
		mov ax, ofs
		mov word ptr [segptr], ax
	}
	return segptr;
}
