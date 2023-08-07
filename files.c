#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "main.h"
#include "video.h"
#include "memory.h"

int fileErrorRead(FILE *fp, const char *filename){
	cleanup();
	puts("\n\nAborting: Unable to read file");
	puts(filename);
	if (fp != NULL){
		fclose(fp);
	}
	PROGRAM_ABORT = 1;
	return 0;
}

int fileErrorDecompress(FILE *fp, const char *filename){
	cleanup();
	puts("\n\nAborting: Unable to decompress file");
	puts(filename);
	if (fp != NULL){
		fclose(fp);
	}
	PROGRAM_ABORT = 1;
	return 0;
}

int fileErrorMemory(const char *filename){
	cleanup();
	puts("\n\nAborting: Out of memory reading file");
	puts(filename);
	PROGRAM_ABORT = 1;
	return 0;
}

// Read in a file 'name.ext', allocating enough RAM to store it.
// Return pointer on success, 0 for fail
uint8_t fileRead(uint8_t *data, const char *name, const char *ext, unsigned long size){
	FILE *fp;
	char filename[13];

	// Create filename string
	strcpy(filename, name);
	strcat(filename, ext);
	// Open the file for binary reading
	if ((fp = fopen(filename, "rb")) == NULL) return fileErrorRead(fp, filename);
	if (size == 0){
		// File size not provided, seek to the end of the file to get the size
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);
	}
	// Read the file into the alloctated ram
	if (fread(data, size, 1, fp) != 1) return fileErrorRead(fp, filename);

	// Close the file and return success
	fclose(fp);
	return 1;
}

// Read in a file 'name.ext', allocating an array of segment locations to each tile's graphics
// Return 1 on success, 0 for fail
int fileReadTiles(const char *name, const char *ext, unsigned int width, unsigned int height, uint16_t data[], unsigned int count){
	FILE *fp, *maskfp;
	char filename[13];
	uint16_t i, size, filesize, masksize, maskfilesize, tempram;

	// Create filename string
	strcpy(filename, name);
	strcat(filename, ext);
	// Calculate the needed RAM for each tile
	size = (width * height) << VIDEO_PLANESHIFT >> VIDEO_BITSHIFT;
	// Calculate the needed temporary RAM to load the data into before graphic translation
	filesize = (width * height) >> VIDEO_FILESHIFT;
	// For sprites or font, read mask file (1 bit per pixel)
	maskfp = NULL;
	masksize = 0;
	if (data == SPRITES || strcmp(ext, ".gfx") == 0){
		maskfilesize = (width * height) >> 3;
		masksize = maskfilesize;
		if (data == SPRITES){
			if ((maskfp = fopen("mask.gfx", "rb")) == NULL) return fileErrorRead(maskfp, "mask.gfx");
		} else{
			if ((maskfp = fopen(filename, "rb")) == NULL) return fileErrorRead(maskfp, filename);
			size = 0;
		}
	}
	// Open the file for binary reading
	fp = NULL;
	if (size > 0){
		if ((fp = fopen(filename, "rb")) == NULL) return fileErrorRead(fp, filename);
	}
	// Read each tile into temporary ram and translate as needed into the propery memory location
	for (i = 0; i < count; i++){
		// Process mask data if sprites, which will give us the final amount of ram needed for each sprite as well
		if (maskfp != NULL){
			// Use minimap as temporary ram for font/mask data (due to not wanting to allocate temporary ram, and then allocate actual ram, since an overlap can occur)
			if (fread(memorySegToPtr(MINIMAP[0]), maskfilesize, 1, maskfp) != 1) return fileErrorRead(fp, filename);
			if (videoMaskTranslate != NULL){
				masksize = videoMaskTranslate(memorySegToPtr(MINIMAP[0]), maskfilesize);
			}
			if ((data[i] = memoryAssign((size + masksize + 15) >> 4, 0)) == 0) return fileErrorMemory(filename);
			memcpy(memorySegToPtr(data[i]) + size, memorySegToPtr(MINIMAP[0]), masksize);
		} else{
			if ((data[i] = memoryAssign((size + 15) >> 4, 0)) == 0) return fileErrorMemory(filename);
		}
		if (fp != NULL){
			if (videoTranslate != NULL){
				if ((tempram = memoryAssign((filesize + 15) >> 4, 1)) == 0) return fileErrorMemory(filename);
				if (fread(memorySegToPtr(tempram), filesize, 1, fp) != 1) return fileErrorRead(fp, filename);
				if (data == FACES){
					videoTranslate(memorySegToPtr(tempram), memorySegToPtr(data[i]), width, height, VIDEO_PALETTE_TITLE, 0);
				} else{
					videoTranslate(memorySegToPtr(tempram), memorySegToPtr(data[i]), width, height, VIDEO_PALETTE_GAME, 0);
				}
			} else{
				if (fread(memorySegToPtr(data[i]), filesize, 1, fp) != 1) return fileErrorRead(fp, filename);
			}
		}
	}
	// Close the file and return
	if (fp != NULL){
		fclose(fp);
	}
	if (maskfp != NULL){
		fclose(maskfp);
	}
	return 1;
}

// Read in a map file 'name', allocating UNIT and MAP data
// Return 1 on success, 0 for fail
int fileReadMap(int levelnum){
	FILE *fp;
	
	uint8_t *data;
	char filename[13];
	char ext[1];

	// Create filename string. Add level number to 97 ('a')
	strcpy(filename, "level-");
	filename[6] = levelnum + 97;
	filename[7] = 0;
	// Open the file for binary reading
	if ((fp = fopen(filename, "rb")) == NULL) fileErrorRead(fp, filename);
	// Read in unit type
	if (fread(UNIT_TYPE, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit x location
	if (fread(UNIT_X, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit y location
	if (fread(UNIT_Y, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit 'a' data
	if (fread(UNIT_A, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit 'b' data
	if (fread(UNIT_B, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit 'c' data
	if (fread(UNIT_C, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit 'd' data
	if (fread(UNIT_D, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in unit health
	if (fread(UNIT_HEALTH, MAP_UNIT_COUNT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in non-used map data
	if (fread(&MAP, MAP_UNIT_COUNT * 4, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Read in map data
	if (fread(&MAP, MAP_WIDTH * MAP_HEIGHT, 1, fp) != 1) return fileErrorRead(fp, filename);
	// Close the file and return
	fclose(fp);
	return 1;
}
