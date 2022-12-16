#pragma once
#include "components.h"
#include "entity.h"
struct Hasher {
  size_t operator()(const Position& a) const { return (int)a.x ^ (int)a.y; };
  size_t operator()(const entity::Entity& t) const { return t.id; }
};
