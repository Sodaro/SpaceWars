#pragma once
namespace entity {
enum class Type { kPlayer, kEnemy, kBullet };
struct Entity {
  int id = 0;
  Type type = Type::kPlayer;
  bool operator==(const Entity& t) const {
    return (this->id == t.id && this->type == t.type);
  }
};
}  // namespace entity
