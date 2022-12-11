// SpaceWars.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Input.h"
#include "SDL/SDL_image.h"
#include "common_math.h"
#include "image_loader.h"
#include "vector2.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 960;
constexpr int GAME_WIDTH = 640;
constexpr int GAME_HEIGHT = 480;
constexpr int JOYSTICK_DEAD_ZONE = 8000;  // Analog joystick dead zone

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

void SpinSprites(RenderData arr[], const int start, const int end,
                 const float dt) {
  for (int i = start; i < end; i++) {
    arr[i].angle += (float)dt * 100.f;
  }
}

void DrawAllSprites(SDL_Renderer* renderer, const RenderData arr[],
                    const int count) {
  for (int i = 0; i < count; i++) {
    SDL_FRect frect{arr[i].position->x, arr[i].position->y, arr[i].size[0],
                    arr[i].size[1]};
    SDL_RenderCopyExF(renderer, arr[i].texture, NULL, &frect, arr[i].angle,
                      NULL, SDL_RendererFlip::SDL_FLIP_NONE);
  }
}

void AddVelocitiesToPositions(Position position_arr[],
                              const Velocity velocity_arr[], const int start,
                              const int count, const float dt) {
  for (int i = start; i < count; i++) {
    position_arr[i].x += velocity_arr[i].x * dt;
    position_arr[i].y += velocity_arr[i].y * dt;
  }
}
void AngleTowardsVelocity(RenderData arr[], const Velocity velocity_arr[],
                          const int start, const int count) {
  for (int i = start; i < count; i++) {
    const auto& position = arr[i].position;
    const auto& velocity = velocity_arr[i];
    float angle =
        AngleBetween(position->x, position->y, position->x + velocity.x,
                     position->y + velocity.y);
    arr[i].angle = angle;
  }
}

void AdjustVelocityTowardsPosition(Velocity velocity_arr[],
                                   const Position* target_pos,
                                   const Position positions[], const int start,
                                   const int count) {
  for (int i = start; i < count; i++) {
    float x = target_pos->x - positions[i].x;
    float y = target_pos->y - positions[i].y;
    float l = sqrtf(x * x + y * y);
    if (l == 0) continue;
    x /= l;
    y /= l;
    velocity_arr[i].x = x * 50.f;
    velocity_arr[i].y = y * 50.f;
  }
}

// void AngleTowardsPosition(Position* pos, RenderData arr[], int start, int
// count)
//{
//     for (int i = start; i < count; i++)
//     {
//         float angle = AngleBetween(pos->x, pos->y, arr[i].position->x,
//         arr[i].position->y); arr[i].angle = angle; printf("angle %f\n",
//         angle);
//     }
// }
//  bool CheckOverlap(const Collider& a, const Collider& b) {
//    return a.x <= b.x + b.w && a.y <= b.y + b.h && a.x + a.w > b.x &&
//           a.y + a.h > b.y;
//  }

bool InitializeApplication(Application& app) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    printf("Failed to initialize SDL! Error: % s\n", SDL_GetError());
    return false;
  }
  SDL_Window* window = SDL_CreateWindow("SpaceWars", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_Renderer* window_renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
  // SDL_SetRenderDrawColor(texture_target_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  app.window_renderer = window_renderer;
  // app.texture_target_renderer = texture_target_renderer;
  app.window = window;
  return true;
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
  Input::Initialize();

  // unoptimized: 5000    colliders   2-3 fps
  // optimized:   5000    colliders   250+ fps
  // optimized:   10000   colliders   120+ fps

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

  constexpr unsigned int entity_count = 1000;
  // Collider collider_components[entity_count];
  Position position_components[entity_count] = {};
  Velocity velocity_components[entity_count] = {};
  // Health health_components[sizeof(Velocity) * entity_count];
  RenderData render_data_components[entity_count] = {};

  render_texture =
      SDL_CreateTexture(app.window_renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, GAME_WIDTH, GAME_HEIGHT);
  SDL_Texture* player_texture =
      image_loader.GetImage("./assets/player.png", app.window_renderer);
  SDL_Texture* background_texture =
      image_loader.GetImage("./assets/background.png", app.window_renderer);
  SDL_Texture* enemy_texture =
      image_loader.GetImage("./assets/enemy.png", app.window_renderer);
  // SDL_Texture* bullet_texture = image_loader.GetImage("./assets/bullet.png",
  // app.window_renderer);

  render_data_components[0].texture = player_texture;
  render_data_components[0].position = &position_components[0];
  render_data_components[0].size[0] = 16.f;
  render_data_components[0].size[1] = 16.f;
  for (int i = 1; i < entity_count; i++) {
    int x = i % 10;
    int y = i / 10;
    position_components[i] = {x * 16.f, y * 8.f};
    render_data_components[i] = {
        &position_components[i], enemy_texture, {16.f, 16.f}, 0};
    velocity_components[i] = {100, 0};
  }

  Uint64 previous_time = SDL_GetPerformanceCounter();
  double delta_time = 0;
  float tracked_elapsed = 0.f;
  int elapsed_count = 0;

  // for (int i = 0; i < collider_count; i++) {
  //   int x = i % 1000;
  //   int y = i / 1000;
  //   colliders.emplace_back(Collider(x, y));
  // }

  bool is_running = true;
  while (is_running) {
    Input::Update();

    if (Input::IsKeyDown(SDL_SCANCODE_ESCAPE)) {
      is_running = false;
      break;
    }

    Uint64 current_time = SDL_GetPerformanceCounter();
    delta_time =
        (float)(current_time - previous_time) / SDL_GetPerformanceFrequency();
    previous_time = current_time;

    float horizontal = Input::GetAxis(InputAxis::Horizontal);
    float vertical = Input::GetAxis(InputAxis::Vertical);

    // auto& player_data = render_data_components[0];
    if (horizontal != 0) {
      render_data_components[0].angle +=
          (float)(delta_time * 60.f * horizontal);
    }
    if (vertical != 0) {
      float radians =
          (float)render_data_components[0].angle * (float)M_PI / 180.f;
      float facing_x = (float)std::cos(radians);
      float facing_y = (float)std::sin(radians);
      velocity_components[0].x +=
          (float)(60 * delta_time * facing_x * vertical);
      velocity_components[0].y +=
          (float)(60 * delta_time * facing_y * vertical);
    } else {
      velocity_components[0].x -=
          (float)(30 * delta_time * Sign(velocity_components[0].x));
      velocity_components[0].y -=
          (float)(30 * delta_time * Sign(velocity_components[0].y));
    }
    AdjustVelocityTowardsPosition(velocity_components, &position_components[0],
                                  position_components, 1, entity_count);
    AddVelocitiesToPositions(position_components, velocity_components, 0,
                             entity_count, (float)delta_time);
    AngleTowardsVelocity(render_data_components, velocity_components, 1,
                         entity_count);
    // SpinSprites(1, entity_count, render_data_components, (float)delta_time);

    SDL_SetRenderTarget(app.window_renderer, render_texture);
    SDL_RenderClear(app.window_renderer);

    // Render texture to screen
    SDL_RenderCopy(app.window_renderer, background_texture, NULL, NULL);
    DrawAllSprites(app.window_renderer, render_data_components, entity_count);
    SDL_SetRenderTarget(app.window_renderer, NULL);
    SDL_RenderCopy(app.window_renderer, render_texture, NULL, NULL);
    SDL_RenderPresent(app.window_renderer);
    float elapsed = (float)(SDL_GetPerformanceCounter() - current_time) /
                    (float)SDL_GetPerformanceFrequency();
    tracked_elapsed += elapsed;
    elapsed_count++;
    if (tracked_elapsed >= 1) {
      float avg_elapsed = tracked_elapsed / elapsed_count;
      printf("avg fps: %f\n", 1 / avg_elapsed);
      tracked_elapsed = 0.f;
      elapsed_count = 0;
    }
    // printf("fps: %f\n", 1.0f / elapsed);

    // printf("%i joysticks were found.\n\n", SDL_NumJoysticks());
    // SDL_Delay(16);
  }

  image_loader.UnloadAllImages();

  SDL_DestroyRenderer(app.window_renderer);
  SDL_DestroyWindow(app.window);
  app.window = nullptr;

  IMG_Quit();
  SDL_Quit();

  return 0;
}
