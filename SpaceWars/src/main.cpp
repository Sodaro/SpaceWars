// SpaceWars.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "SDL/SDL_image.h"
#include "common_math.h"
#include "constants.h"
#include "image_loader.h"
#include "input.h"
#include "spatial_hash_grid.h"
#include "vector2.h"

struct Application {
  SDL_Window* window = nullptr;
  SDL_Renderer* window_renderer = nullptr;
};

struct Health {
  float amount = 0.f;
};

struct Collider {
  float size[2];
};

struct Velocity {
  float x = 0;
  float y = 0;
};

struct Position {
  float x = 0;
  float y = 0;
};

struct RenderData {
  Position* position = nullptr;
  SDL_Texture* texture = nullptr;
  float size[2] = {0, 0};
  double angle = 0;
};

struct Entity {
  RenderData* render_data = nullptr;
  Position* position = nullptr;
  Velocity* velocity = nullptr;
  Collider* collider = nullptr;
  Health* health = nullptr;
};

// Collider collider_components[entity_count];
Position position_components[constants::kEntityCount]{};
Velocity velocity_components[constants::kEntityCount]{};
// Health health_components[sizeof(Velocity) * entity_count];
RenderData render_data_components[constants::kEntityCount]{};
collision::SpatialGrid spatial_grid{};

void AddVelocitiesToPositions(Position position_arr[],
                              const Velocity velocity_arr[], const int start,
                              const int count, const float dt) {
  for (int i = start; i < count; i++) {
    position_arr[i].x += velocity_arr[i].x * dt;
    position_arr[i].y += velocity_arr[i].y * dt;
  }
}

void AngleTowardsVelocity(RenderData arr[], const Velocity velocity_arr[],
                          const int start, const int end) {
  for (int i = start; i < end; i++) {
    const auto& position = arr[i].position;
    const auto& velocity = velocity_arr[i];
    float angle =
        math::AngleBetween(position->x, position->y, position->x + velocity.x,
                           position->y + velocity.y);
    arr[i].angle = angle;
  }
}

void AdjustVelocityTowardsPosition(Velocity velocity_arr[],
                                   const Position* target_pos,
                                   const Position positions[], const int start,
                                   const int end) {
  for (int i = start; i < end; i++) {
    float x = target_pos->x - positions[i].x;
    float y = target_pos->y - positions[i].y;
    float l = sqrtf(x * x + y * y);
    if (l == 0) {
      continue;
    }
    x /= l;
    y /= l;
    velocity_arr[i].x = x * 50.f;
    velocity_arr[i].y = y * 50.f;
  }
}

//  bool CheckOverlap(const Collider& a, const Collider& b) {
//    return a.x <= b.x + b.w && a.y <= b.y + b.h && a.x + a.w > b.x &&
//           a.y + a.h > b.y;
//  }

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

inline float GetUpdatedTimeDelta(Uint64& prev_time) {
  Uint64 time = SDL_GetPerformanceCounter();
  float delta = (float)(time - prev_time) / SDL_GetPerformanceFrequency();
  prev_time = time;
  return delta;
}

void RenderGame(const Application& app, SDL_Texture* background_texture,
                SDL_Texture* render_texture) {
  SDL_SetRenderTarget(app.window_renderer, render_texture);
  SDL_RenderClear(app.window_renderer);

  // Render texture to screen
  SDL_RenderCopy(app.window_renderer, background_texture, NULL, NULL);
  const auto& arr = render_data_components;
  SDL_FRect frect{};
  for (int i = 0; i < constants::kEntityCount; i++) {
    frect.x = arr[i].position->x;
    frect.y = arr[i].position->y;
    frect.w = arr[i].size[0];
    frect.h = arr[i].size[1];
    SDL_RenderCopyExF(app.window_renderer, arr[i].texture, NULL, &frect,
                      arr[i].angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
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
  for (int i = 0; i < constants::kEntityCount; i++) {
    auto pos = position_components[i];
    spatial_grid.Update(i, pos.x, pos.y, 16, 16);
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

  // TODO: use frect for tracking position and size, use frectintersect for
  // collision

  /*
   * Components:
   * * Position
   * * Velocity
   * * Renderer
   * * Health
   *
   * Systems:
   * * PositionUpdating
   * * TargetTracking
   * * BulletSpawning
   * * CollisionChecking
   * * Rendering
   */

  render_texture = SDL_CreateTexture(
      app.window_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      constants::kGameWidth, constants::kGameHeight);
  SDL_Texture* player_texture =
      image_loader.GetImage("./assets/player.png", app.window_renderer);
  SDL_Texture* background_texture =
      image_loader.GetImage("./assets/background.png", app.window_renderer);
  SDL_Texture* enemy_texture =
      image_loader.GetImage("./assets/enemy.png", app.window_renderer);
  // SDL_Texture* bullet_texture =
  // image_loader.GetImage("./assets/bullet.png", app.window_renderer);

  render_data_components[0].texture = player_texture;
  render_data_components[0].position = &position_components[0];
  render_data_components[0].size[0] = 16.f;
  render_data_components[0].size[1] = 16.f;
  for (int i = 1; i < constants::kEntityCount; i++) {
    int x = i % 20;
    int y = i / 20;
    position_components[i] = {x * 16.f, y * 8.f};
    render_data_components[i] = {
        &position_components[i], enemy_texture, {16.f, 16.f}, 0};
    velocity_components[i] = {100, 0};
  }

  Uint64 previous_time = SDL_GetPerformanceCounter();

  // for (int i = 0; i < collider_count; i++) {
  //   int x = i % 1000;
  //   int y = i / 1000;
  //   colliders.emplace_back(Collider(x, y));
  // }

  bool is_running = true;
  while (is_running) {
    input::Handler::Update();

    if (input::Handler::IsKeyDown(SDL_SCANCODE_ESCAPE)) {
      is_running = false;
      break;
    }

    float delta_time = GetUpdatedTimeDelta(previous_time);
    float horizontal = input::Handler::GetAxis(input::Axis::kHorizontal);
    if (horizontal != 0) {
      float angle_delta = (float)(delta_time * 60.f * horizontal);
      render_data_components[0].angle += angle_delta;
    }

    float vertical = input::Handler::GetAxis(input::Axis::kVertical);
    if (vertical != 0) {
      float radians = math::RadToDeg((float)render_data_components[0].angle);
      float facing_x = (float)std::cos(radians);
      float facing_y = (float)std::sin(radians);
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
    AdjustVelocityTowardsPosition(velocity_components, &position_components[0],
                                  position_components, 1,
                                  constants::kEntityCount);
    AngleTowardsVelocity(render_data_components, velocity_components, 1,
                         constants::kEntityCount);

    AddVelocitiesToPositions(position_components, velocity_components, 0,
                             constants::kEntityCount, (float)delta_time);

    UpdateCollisionGrid();
    //   SpinSprites(1, entity_count, render_data_components,
    //   (float)delta_time);

    RenderGame(app, background_texture, render_texture);
    PrintFPS(previous_time);
    //  SDL_Delay(16);
  }

  image_loader.UnloadAllImages();

  SDL_DestroyRenderer(app.window_renderer);
  SDL_DestroyWindow(app.window);
  app.window = nullptr;

  IMG_Quit();
  SDL_Quit();

  return 0;
}
