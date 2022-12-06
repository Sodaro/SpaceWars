#pragma once
struct Vector2 {
  float x = 0;
  float y = 0;

  Vector2 operator*(float f) { return {this->x * f, this->y * f}; }
  Vector2 operator/(float f) { return {this->x / f, this->y / f}; }

  void operator+=(const Vector2& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
  }
  void operator-=(const Vector2& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
  }
};