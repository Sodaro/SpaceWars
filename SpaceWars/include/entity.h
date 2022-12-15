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
class Hasher {
 public:
  // id is returned as hash function
  size_t operator()(const Entity& t) const { return t.id; }
};

}  // namespace entity
