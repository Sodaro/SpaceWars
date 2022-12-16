#pragma once
#include <SDL/SDL.h>

#include <cmath>

namespace math {
inline float RadToDeg(float radians) { return radians * (float)M_PI / 180.f; }

inline float GetMagnitude(const float vec[2]) {
  return sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
}
inline float GetMagnitude(const float x, const float y) {
  return sqrtf(x * x + y * y);
};
inline float GetDistance(const float x, const float y, const float x2,
                         const float y2) {
  return sqrtf(x * x2 + y * y2);
}

inline bool SafeNormalize(float (&vec)[2]) {
  float length = GetMagnitude(vec);
  if (length == 0) return false;

  vec[0] /= length;
  vec[1] /= length;
  return true;
}

inline float Sign(float a) {
  if (a > 0) return 1;
  if (a < 0) return -1;
  return 0;
}

inline float Dot(float x, float y, float x2, float y2) {
  return x * x2 + y * y2;
}

inline float Clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }

inline float AngleBetween(float x, float y, float x2, float y2) {
  if (x == 0 && y == 0 || x2 == 0 && y2 == 0) {
    return 0;
  }

  float l1 = sqrtf(x * x + y * y);
  float l2 = sqrtf(x2 * x2 + y2 * y2);

  float angle = Dot(x, y, x2, y2) / (l1 * l2);
  return acosf(angle) * (180.f / (float)M_PI);
}
}  // namespace math