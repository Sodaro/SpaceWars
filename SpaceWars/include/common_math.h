#pragma once
#include <SDL/SDL.h>

#include <cmath>

float GetPointMagnitude(const SDL_FPoint& point) {
  return sqrtf(point.x * point.x + point.y * point.y);
}

SDL_FPoint GetNormalizedPoint(const SDL_FPoint& point) {
  float length = GetPointMagnitude(point);
  return {point.x / length, point.y / length};
}
bool SafeNormalizePoint(SDL_FPoint& point) {
  float length = GetPointMagnitude(point);
  if (length == 0) return false;

  point.x /= length;
  point.y /= length;
  return true;
}