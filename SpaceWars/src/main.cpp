// SpaceWars.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

#include "SDL/SDL_image.h"
#include "common_math.h"
#include "components.h"
#include "constants.h"
#include "entity.h"
#include "image_loader.h"
#include "input.h"
#include "spatial_hash_grid.h"

struct Application {
  SDL_Window* window = nullptr;
  SDL_Renderer* window_renderer = nullptr;
};

struct CollisionData {
  int collider_id = 0;
};

class IDManager {
  static int id;

 public:
  static int GetNextID() { return id++; };
};

Position position_components[constants::kEntityCount]{};
Velocity velocity_components[constants::kEntityCount]{};
RenderData render_data_components[constants::kEntityCount]{};
std::vector<entity::Entity> entities;
std::vector<entity::Entity> active_entities;
std::unordered_set<entity::Entity, entity::Hasher> dead_entities;
collision::SpatialGrid spatial_grid{};
int IDManager::id = 0;

bool DEBUG_ENABLED = false;

std::vector<int> GetActiveEntitiesOfTypeIndices(entity::Type type) {
  std::vector<int> matching_entity_indices;
  for (int i = 0; i < active_entities.size(); i++) {
    if (active_entities[i].type == type) {
      matching_entity_indices.emplace_back(i);
    }
  }
  return matching_entity_indices;
}

std::vector<entity::Entity> GetActiveEntitiesOfType(entity::Type type) {
  std::vector<entity::Entity> matching_entities;
  for (int i = 0; i < active_entities.size(); i++) {
    if (active_entities[i].type == type) {
      matching_entities.emplace_back(active_entities[i]);
    }
  }
  return matching_entities;
}

void RemoveDeadEntities() {
  for (int i = static_cast<int>(active_entities.size() - 1); i >= 0; i--) {
    if (dead_entities.contains(active_entities[i])) {
      auto& entity = active_entities[i];
      auto pos = position_components[entity.id];
      active_entities.erase(active_entities.begin() + i);
      spatial_grid.Remove(entity, pos.x, pos.y, 16.f, 16.f);
    }
  }
  dead_entities.clear();
}

bool InitializeApplication(Application& app) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    printf("Failed to initialize SDL! Error: % s\n", SDL_GetError());
    return false;
  }
  SDL_Window* window = SDL_CreateWindow(
      "SpaceWars", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      constants::kScreenWidth, constants::kScreenHeight, SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_Renderer* window_renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (window_renderer == nullptr) {
    printf("Failed to create renderer! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  // Initialize PNG and JPEG loading
  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    printf("SDL_image could not initialize! SDL_image Error: %s\n",
           IMG_GetError());
    return false;
  }
  SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  app.window_renderer = window_renderer;
  app.window = window;
  return true;
}

bool IsOutsideView(float x, float y, float w, float h) {
  return x + w < 0 || x > constants::kGameWidth || y + h < 0 ||
         y > constants::kGameHeight;
}

void RenderCollisionGrid(SDL_Renderer* renderer) {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x55, 0x55, 0xFF);
  for (int i = 0; i < collision::grid_rows; i++) {
    for (int j = 0; j < collision::grid_cols; j++) {
      SDL_FRect outlineRect = {
          j * collision::grid_tile_width, i * collision::grid_tile_height,
          collision::grid_tile_width, collision::grid_tile_height};
      SDL_RenderDrawRectF(renderer, &outlineRect);
    }
  }
}

// movement system
void AddVelocitiesToPositions(const float delta_time) {
  for (const auto& entity : active_entities) {
    auto id = entity.id;
    position_components[id].x += velocity_components[id].x * delta_time;
    position_components[id].y += velocity_components[id].y * delta_time;
  }
}

// enemy rotation system
void AngleTowardsVelocity(const std::vector<entity::Entity>& enemies) {
  for (const auto& enemy : enemies) {
    auto id = enemy.id;
    const auto& velocity = velocity_components[id];
    float angle = math::RadToDeg(atan2f(velocity.y, velocity.x));
    render_data_components[id].angle = angle + 90.f;
  }
}

// updates enemy velocity so it will move towards target position
void UpdateEnemyVelocities(const std::vector<entity::Entity>& enemies,
                           const Position* target_pos) {
  for (const auto& enemy : enemies) {
    auto id = enemy.id;
    float x = target_pos->x - position_components[id].x;
    float y = target_pos->y - position_components[id].y;
    float l = sqrtf(x * x + y * y);
    if (l == 0) {
      continue;
    }
    x /= l;
    y /= l;
    velocity_components[id].x = x * 100.f;
    velocity_components[id].y = y * 100.f;
  }
}

inline float GetUpdatedTimeDelta(Uint64& prev_time) {
  Uint64 time = SDL_GetPerformanceCounter();
  float delta = (float)(time - prev_time) / SDL_GetPerformanceFrequency();
  prev_time = time;
  return delta;
}

void RenderGame(const Application& app, const RenderData render_data_comps[],
                SDL_Texture* background_texture, SDL_Texture* render_texture) {
  SDL_SetRenderTarget(app.window_renderer, render_texture);
  SDL_RenderClear(app.window_renderer);

  // Render texture to screen
  SDL_RenderCopy(app.window_renderer, background_texture, NULL, NULL);
  SDL_FRect frect{};
  for (const auto& entity : active_entities) {
    auto id = entity.id;
    frect.x = render_data_comps[id].position->x;
    frect.y = render_data_comps[id].position->y;
    frect.w = render_data_comps[id].size[0];
    frect.h = render_data_comps[id].size[1];
    SDL_RenderCopyExF(app.window_renderer, render_data_comps[id].texture, NULL,
                      &frect, render_data_comps[id].angle, NULL,
                      SDL_RendererFlip::SDL_FLIP_NONE);
  }
  if (DEBUG_ENABLED) {
    RenderCollisionGrid(app.window_renderer);
  }
  SDL_SetRenderTarget(app.window_renderer, NULL);
  SDL_RenderCopy(app.window_renderer, render_texture, NULL, NULL);
  SDL_RenderPresent(app.window_renderer);
}

void PrintFPS(Uint64 time_start_of_frame) {
  static float tracked_frame_time = 0.f;
  static float frame_time_count = 0.f;
  float frame_time =
      (float)(SDL_GetPerformanceCounter() - time_start_of_frame) /
      (float)SDL_GetPerformanceFrequency();
  tracked_frame_time += frame_time;
  frame_time_count++;
  if (tracked_frame_time >= 1) {
    float avg_elapsed = tracked_frame_time / frame_time_count;
    printf("avg fps: %f\n", 1 / avg_elapsed);
    tracked_frame_time = 0.f;
    frame_time_count = 0;
  }
}

void UpdateCollisionGrid() {
  const auto& positions = position_components;
  spatial_grid.Update(entities[0], positions[0].x, positions[0].y, 16, 16);

  for (const auto& entity : active_entities) {
    if (IsOutsideView(position_components[entity.id].x,
                      position_components[entity.id].y, 16.f, 16.f)) {
      continue;
    }
    spatial_grid.Update(entity, position_components[entity.id].x,
                        position_components[entity.id].y, 16, 16);
  }
}

void HandleCollisions() {
  SDL_FRect bullet_rect{};
  SDL_FRect enemy_rect{};
  CollisionData collision_data;

  const auto bullets = GetActiveEntitiesOfType(entity::Type::kBullet);
  for (const auto& bullet : bullets) {
    auto bullet_id = bullet.id;
    bullet_rect.x = position_components[bullet_id].x;
    bullet_rect.y = position_components[bullet_id].y;
    bullet_rect.w = 16.f;
    bullet_rect.h = 16.f;

    const auto nearby_enemies = spatial_grid.FindNearbyEntitiesOfType(
        entity::Type::kEnemy, bullet_rect.x, bullet_rect.y,
        bullet_rect.x + bullet_rect.w, bullet_rect.y + bullet_rect.h);

    for (const auto& enemy : nearby_enemies) {
      auto id = enemy.id;
      enemy_rect.x = position_components[id].x;
      enemy_rect.y = position_components[id].y;
      enemy_rect.w = 16.f;
      enemy_rect.h = 16.f;
      if (SDL_HasIntersectionF(&bullet_rect, &enemy_rect)) {
        dead_entities.insert(enemy);
      }
    }
  }
}

void HandlePlayerLogic(float delta_time) {
  static int current_bullet_index = constants::kLastEnemyIndex + 1;
  static float shoot_cooldown = 0.1f;
  static float shoot_timer = 0.f;
  shoot_timer -= delta_time;
  float horizontal = input::Handler::GetAxis(input::Axis::kHorizontal);
  if (horizontal != 0) {
    float angle_delta = (float)(delta_time * 60.f * horizontal);
    render_data_components[0].angle += angle_delta;
  }

  float vertical = input::Handler::GetAxis(input::Axis::kVertical);
  float radians = math::RadToDeg((float)render_data_components[0].angle);
  float facing_x = (float)std::cos(radians);
  float facing_y = (float)std::sin(radians);
  if (vertical != 0) {
    float new_x = (float)(60 * delta_time * facing_x * vertical);
    float new_y = (float)(60 * delta_time * facing_y * vertical);
    velocity_components[0].x += new_x;
    velocity_components[0].y += new_y;
  } else {
    auto velocity_x = velocity_components[0].x;
    auto velocity_y = velocity_components[0].y;
    float new_x = (float)(30 * delta_time * math::Sign(velocity_x) * -1);
    float new_y = (float)(30 * delta_time * math::Sign(velocity_y) * -1);
    velocity_components[0].x += new_x;
    velocity_components[0].y += new_y;
  }
  if (input::Handler::IsKeyDown(SDL_SCANCODE_SPACE) && shoot_timer <= 0) {
    active_entities.emplace_back(entities[current_bullet_index]);
    position_components[current_bullet_index].x = position_components[0].x;
    position_components[current_bullet_index].y = position_components[0].y;
    velocity_components[current_bullet_index] = {facing_x * 200.f,
                                                 facing_y * 200.f};

    render_data_components[current_bullet_index].angle =
        (float)render_data_components[0].angle;
    if (++current_bullet_index >= constants::kEntityCount) {
      current_bullet_index = constants::kLastEnemyIndex + 1;
    }
    shoot_timer = shoot_cooldown;
  }
}

void InitializeEnemies(SDL_Texture* texture1, SDL_Texture* texture2) {
  for (int i = 1; i <= constants::kLastEnemyIndex; i++) {
    int group = (i - 1) / constants::kEnemyGroupSize;
    int group_x = group % 5;
    int group_y = group / 5;
    int x = i % 10;
    int y = i / 10;
    position_components[i] = {group_x * 75.f + x * 20.f,
                              group_y * 75.f + y * 20.f};
    render_data_components[i] = {&position_components[i],
                                 i % 2 == 0 ? texture1 : texture2,
                                 {16.f, 16.f},
                                 0};
  }
}

void InitializeBullets(SDL_Texture* texture) {
  int start = constants::kLastEnemyIndex + 1;
  for (int i = start; i < constants::kEntityCount; i++) {
    render_data_components[i] = {
        &position_components[i], texture, {16.f, 16.f}, 0};
  }
}

void FlagStrayBullets() {
  const auto& bullets = GetActiveEntitiesOfType(entity::Type::kBullet);
  for (const auto& bullet : bullets) {
    auto id = bullet.id;
    auto pos = position_components[id];
    if (!IsOutsideView(pos.x, pos.y, 16.f, 16.f)) {
      continue;
    }
    dead_entities.insert(bullet);
  }
}

int main(int, char*[]) {
  Application app;
  ImageLoader image_loader;
  SDL_Texture* render_texture;

  if (!InitializeApplication(app)) {
    printf("Failed to initalize application!");
    return 1;
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  input::Handler::Initialize();

  render_texture = SDL_CreateTexture(
      app.window_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      constants::kGameWidth, constants::kGameHeight);
  SDL_Texture* player_texture =
      image_loader.GetImage("./assets/player.png", app.window_renderer);
  SDL_Texture* background_texture =
      image_loader.GetImage("./assets/background.png", app.window_renderer);
  SDL_Texture* enemy_texture =
      image_loader.GetImage("./assets/enemy.png", app.window_renderer);
  SDL_Texture* enemy_texture2 =
      image_loader.GetImage("./assets/enemy2.png", app.window_renderer);
  SDL_Texture* bullet_texture =
      image_loader.GetImage("./assets/bullet.png", app.window_renderer);
  position_components[0].x = 100.f;
  position_components[0].y = 100.f;
  render_data_components[0].texture = player_texture;
  render_data_components[0].position = &position_components[0];
  render_data_components[0].size[0] = 16.f;
  render_data_components[0].size[1] = 16.f;

  entities.reserve(constants::kEntityCount);
  active_entities.reserve(constants::kEntityCount);
  entities.emplace_back(
      entity::Entity{IDManager::GetNextID(), entity::Type::kPlayer});
  for (int i = 0; i < constants::kEnemyShipCount; i++) {
    entities.emplace_back(
        entity::Entity{IDManager::GetNextID(), entity::Type::kEnemy});
  }
  for (int i = 0; i < constants::kBulletCount; i++) {
    entities.emplace_back(
        entity::Entity{IDManager::GetNextID(), entity::Type::kBullet});
  }
  for (int i = 0; i < constants::kEnemyShipCount + 1; i++) {
    active_entities.push_back(entities[i]);
  }

  InitializeEnemies(enemy_texture, enemy_texture2);
  InitializeBullets(bullet_texture);

  Uint64 previous_time = SDL_GetPerformanceCounter();

  bool is_running = true;

  while (is_running) {
    input::Handler::Update();

    if (input::Handler::IsKeyDown(SDL_SCANCODE_ESCAPE)) {
      is_running = false;
      break;
    }
    if (input::Handler::GetKeyPressed(SDL_SCANCODE_F1)) {
      DEBUG_ENABLED = !DEBUG_ENABLED;
    }

    float delta_time = GetUpdatedTimeDelta(previous_time);
    const auto enemies = GetActiveEntitiesOfType(entity::Type::kEnemy);
    HandlePlayerLogic((float)delta_time);

    UpdateEnemyVelocities(enemies, &position_components[0]);
    AngleTowardsVelocity(enemies);

    AddVelocitiesToPositions((float)delta_time);
    UpdateCollisionGrid();
    HandleCollisions();

    FlagStrayBullets();
    RemoveDeadEntities();

    RenderGame(app, render_data_components, background_texture, render_texture);

    PrintFPS(previous_time);
  }

  image_loader.UnloadAllImages();

  SDL_DestroyRenderer(app.window_renderer);
  SDL_DestroyWindow(app.window);
  app.window = nullptr;

  IMG_Quit();
  SDL_Quit();

  return 0;
}
