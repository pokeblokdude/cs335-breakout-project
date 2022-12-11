#pragma once

#include "GFX_Math.h"

struct Bounds { 
  vec2i min;
  vec2i max;
};

struct Ball {
  vec2i position = vec2i{120, 160};
  vec2i lastPos;
  vec2i velocity;
  Bounds bounds;
  int radius = 4;
  uint16_t color = 0xFFFF;
};

struct Block {
  vec2i position;
  uint8_t health;
};