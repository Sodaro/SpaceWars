#pragma once
#include "SDL/SDL.h"

struct Velocity {
  float x = 0;
  float y = 0;
};

struct Position {
  float x = 0;
  float y = 0;
};

struct RenderData {
  Position* position = nullptr;
  SDL_Texture* texture = nullptr;
  float size[2] = {0, 0};
  double angle = 0;
};