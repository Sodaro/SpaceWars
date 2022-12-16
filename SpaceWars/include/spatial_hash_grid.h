#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common_math.h"
#include "constants.h"
#include "entity.h"
#include "hasher.h"

namespace collision {
constexpr int grid_cols = 20;
constexpr int grid_rows = 15;
constexpr float width_offset = 16.f / constants::kGameWidth;
constexpr float height_offset = 16.f / constants::kGameHeight;
constexpr float grid_tile_width = constants::kGameWidth / grid_cols;
constexpr float grid_tile_height = constants::kGameHeight / grid_rows;
class SpatialGrid {
  std::unordered_map<std::string, std::unordered_set<entity::Entity, Hasher>>
      cells{};

  std::string HashKey(int x, int y) {
    // return std::format("{};{}", x, y);
    return std::to_string(x) + ";" + std::to_string(y);
  }
  std::vector<int> GetIndices(float x, float y, float x2, float y2) {
    // input: 0,0,16,16
    // output: 0,0,1,1

    float min_x = math::Clamp01(x / constants::kGameWidth);
    float min_y = math::Clamp01(y / constants::kGameHeight);
    float max_x = math::Clamp01(x2 / constants::kGameWidth);
    float max_y = math::Clamp01(y2 / constants::kGameHeight);
    std::vector<int> indices = {
        (int)(min_x * grid_cols), (int)(max_x * grid_cols),
        (int)(min_y * grid_rows), (int)(max_y * grid_rows)};
    return indices;
  }
  // insert the client into every cell that it occupies
  void Insert(const entity::Entity& entity, const std::vector<int>& indices) {
    for (int i = indices[0]; i <= indices[1]; i++) {
      for (int j = indices[2]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        cells[key].insert(entity);
      }
    }
  }

 public:
  std::unordered_set<entity::Entity, Hasher> FindNearbyEntities(
      std::vector<entity::Type> types_to_exclude, float x, float y, float w,
      float h) {
    std::unordered_set<entity::Entity, Hasher> entity_set = {};
    std::vector<int> indices = GetIndices(x, y, w, h);
    for (int i = indices[0]; i <= indices[1]; i++) {
      for (int j = indices[2]; j <= indices[3]; j++) {
        const auto key = HashKey(i, j);
        if (cells[key].empty()) {
          continue;
        }
        for (const auto& entity : cells[key]) {
          if (std::find(types_to_exclude.begin(), types_to_exclude.end(),
                        entity.type) != types_to_exclude.end()) {
            continue;
          }
          entity_set.insert(entity);
        }
      }
    }
    return entity_set;
  }

  std::unordered_set<entity::Entity, Hasher> FindNearbyEntitiesOfType(
      entity::Type type, float x, float y, float w, float h) {
    std::unordered_set<entity::Entity, Hasher> entity_set = {};
    std::vector<int> indices = GetIndices(x, y, w, h);
    for (int i = indices[0]; i <= indices[1]; i++) {
      for (int j = indices[2]; j <= indices[3]; j++) {
        const auto key = HashKey(i, j);
        if (cells[key].empty()) {
          continue;
        }
        for (const auto& entity : cells[key]) {
          if (entity.type != type) {
            continue;
          }
          entity_set.insert(entity);
        }
      }
    }
    return entity_set;
  }

  //    check all the cells that the client occupies
  //    and return all the other clients that occupy the same
  //

  void Update(const entity::Entity& entity, float x, float y, float w,
              float h) {
    Remove(entity,
           GetIndices(x - w * 1.1f, y - h * 1.1f, x + w * 1.1f, y + h * 1.1f));
    Insert(entity, GetIndices(x - w, y - h, x + w, y + h));
  }

  void Remove(const entity::Entity& entity, const std::vector<int>& indices) {
    for (int i = indices[0]; i <= indices[1]; i++) {
      for (int j = indices[2]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        if (cells.find(key) == cells.end()) {
          continue;
        }
        cells[key].erase(entity);
      }
    }
  }
  void Remove(const entity::Entity& entity, float x, float y, float w,
              float h) {
    const std::vector<int>& indices =
        GetIndices(x - w * 1.1f, y - h * 1.1f, x + w * 1.1f, y + h * 1.1f);
    for (int i = indices[0]; i <= indices[1]; i++) {
      for (int j = indices[2]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        if (cells.find(key) == cells.end()) {
          continue;
        }
        cells[key].erase(entity);
      }
    }
  }
};
}  // namespace collision
