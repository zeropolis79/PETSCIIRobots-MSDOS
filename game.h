#ifndef __GAME_H_INCLUDED__
#define __GAME_H_INCLUDED__

#include <stdint.h>
#include "globals.h"
#include "video.h"
#include "memory.h"

int gameInit();
void gameMapDraw();
void gameDisplayItem();
void gameDisplayHealth();
void gameMiniMapStats(uint8_t update);
void playerAnimate(uint8_t animation_offset, uint8_t reset);
void mapSet(uint8_t x, uint8_t y, uint8_t tile);
uint8_t checkForUnit(uint8_t x, uint8_t y);
uint8_t checkForHiddenUnit(uint8_t x, uint8_t y);
void unitAnimate(uint8_t unit);
int tileHasAttribute(uint8_t x, uint8_t y, uint8_t attrib);
void unitPos(uint8_t unit, uint8_t x, uint8_t y);
int requestWalkXY(uint8_t unit, uint8_t move_attrib, uint8_t x, uint8_t y);
int requestWalkLeft(uint8_t unit, uint8_t move_attrib);
int requestWalkRight(uint8_t unit, uint8_t move_attrib);
int requestWalkUp(uint8_t unit, uint8_t move_attrib);
int requestWalkDown(uint8_t unit, uint8_t move_attrib);

#endif //__GAME_H_INCLUDED__
