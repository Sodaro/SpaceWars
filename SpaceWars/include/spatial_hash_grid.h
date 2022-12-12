#pragma once
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "common_math.h"
#include "constants.h"

namespace collision {
constexpr int grid_cols = 12;
constexpr int grid_rows = 9;
constexpr int grid_tile_width = constants::kGameWidth / grid_cols;
constexpr int grid_tile_height = constants::kGameHeight / grid_rows;
class SpatialGrid {
  std::map<std::string, std::vector<int>> entities{};

  std::string HashKey(int x, int y) {
    return std::to_string(x) + ";" + std::to_string(y);
  }
  std::vector<int> GetIndices(float x, float y, float w, float h) {
    // input: 0,0,16,16
    // output: 0,0,1,1
    float min_x = math::Clamp01(x / constants::kGameWidth);
    float min_y = math::Clamp01(y / constants::kGameHeight);
    float max_x = math::Clamp01((x + w) / constants::kGameWidth);
    float max_y = math::Clamp01((y + h) / constants::kGameHeight);
    std::vector<int> indices = {
        (int)(min_x * grid_tile_width), (int)(min_y * grid_tile_height),
        (int)(max_x * grid_tile_width), (int)(max_y * grid_tile_height)};
    return indices;
  }
  // insert the client into every cell that it occupies
  void InsertEntity(int entity, const std::vector<int>& indices) {
    for (int i = indices[0]; i <= indices[2]; i++) {
      for (int j = indices[1]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        if (entities.find(key) == entities.end()) {
          entities[key] = std::vector<int>{};
        }
        entities[key].emplace_back(entity);
      }
    }
  }

 public:
  std::vector<int> FindNearbyEntities(
      std::unordered_set<int> entities_to_exclude, float x, float y, float w,
      float h) {
    std::vector<int> entity_list = {};
    std::vector<int> indices = GetIndices(x, y, w, h);
    for (int i = indices[0]; i <= indices[2]; i++) {
      for (int j = indices[1]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        if (entities.find(key) == entities.end()) {
          continue;
        }
        for (int k = 0; k < entities.size(); k++) {
          if (entities_to_exclude.find(k) != entities_to_exclude.end()) {
            continue;
          }
          entity_list.emplace_back(k);
        }
      }
    }
    return entity_list;
  }

  //    check all the cells that the client occupies
  //    and return all the other clients that occupy the same
  //

  void Update(int entity, float x, float y, float w, float h) {
    std::vector<int> indices = GetIndices(x, y, w, h);
    RemoveEntity(entity, indices);
    InsertEntity(entity, indices);
  }

  void RemoveEntity(int entity, const std::vector<int>& indices) {
    for (int i = indices[0]; i <= indices[2]; i++) {
      for (int j = indices[1]; j <= indices[3]; j++) {
        auto key = HashKey(i, j);
        if (entities.find(key) == entities.end()) {
          continue;
        }
        auto entity_iterator =
            std::find(entities[key].begin(), entities[key].end(), entity);
        if (entity_iterator == entities[key].end()) {
          continue;
        }
        entities[key].erase(entity_iterator);
      }
    }
  }
};
}  // namespace collision
