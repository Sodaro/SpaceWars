#pragma once
#include <SDL/SDL.h>

#include <cmath>

float GetMagnitude(const float vec[2]) {
  return sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
}

bool SafeNormalize(float (&vec)[2]) {
  float length = GetMagnitude(vec);
  if (length == 0) return false;

  vec[0] /= length;
  vec[1] /= length;
  return true;
}

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

float Sign(float a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

float Dot(float x, float y, float x2, float y2) { return x * x2 + y * y2; }
float AngleBetween(float x, float y, float x2, float y2) {
  if (x == 0 && y == 0 || x2 == 0 && y2 == 0) {
    return 0;
  }

  float l1 = sqrtf(x * x + y * y);
  float l2 = sqrtf(x2 * x2 + y2 * y2);

  float angle = Dot(x, y, x2, y2) / (l1 * l2);
  return acosf(angle) * (180.f / (float)M_PI);
}