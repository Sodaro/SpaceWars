// SpaceWars.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "SDL/SDL_image.h"
#include "common_math.h"
#include "image_loader.h"
#include "vector2.h"
#include "Input.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 960;
constexpr int GAME_WIDTH = 320;
constexpr int GAME_HEIGHT = 240;
constexpr int JOYSTICK_DEAD_ZONE = 8000;  // Analog joystick dead zone

struct Application {
  SDL_Window* window = nullptr;
  SDL_Renderer* window_renderer = nullptr;
};

struct Collider {
    float size[2];
};
struct Health {
    float amount = 0.f;
};
struct Velocity
{
    float values[2];
};
struct Position
{
    float values[2];
};
struct RenderData
{
    SDL_FRect destination_rect{};
    SDL_Texture* texture = nullptr;
    double angle = 0;
};


void SpinSprites(int start, int end, RenderData* arr, float dt)
{
    for (int i = start; i < end; i++)
    {
        arr[i].angle += (float)dt * 100.f;
    }
}

void DrawSprites(SDL_Renderer* renderer, const RenderData* arr, int count)
{
    for (int i = 0; i < count; i++)
    {
        SDL_RenderCopyExF(renderer, arr[i].texture, NULL, &arr[i].destination_rect, arr[i].angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
    }
}
// bool CheckOverlap(const Collider& a, const Collider& b) {
//   return a.x <= b.x + b.w && a.y <= b.y + b.h && a.x + a.w > b.x &&
//          a.y + a.h > b.y;
// }

bool InitializeApplication(Application& app) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    printf("Failed to initialize SDL! Error: % s\n", SDL_GetError());
    return false;
  }
  SDL_Window* window =
      SDL_CreateWindow("SpaceWars", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_Renderer* window_renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED);
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
  //SDL_SetRenderDrawColor(texture_target_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  app.window_renderer = window_renderer;
  //app.texture_target_renderer = texture_target_renderer;
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

  constexpr unsigned int entity_count = 100;
  //Collider collider_components[sizeof(Collider) * entity_count];
  //Position position_components[sizeof(Position) * entity_count];
  //Velocity velocity_components[sizeof(Velocity) * entity_count];
  //Health health_components[sizeof(Velocity) * entity_count];
  RenderData render_data_components[entity_count];

  render_texture = SDL_CreateTexture(app.window_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GAME_WIDTH, GAME_HEIGHT);
  SDL_Texture* player_texture = image_loader.GetImage("./assets/player.png", app.window_renderer);
  SDL_Texture* background_texture = image_loader.GetImage("./assets/background.png", app.window_renderer);
  SDL_Texture* enemy_texture = image_loader.GetImage("./assets/enemy.png", app.window_renderer);
  //SDL_Texture* bullet_texture = image_loader.GetImage("./assets/bullet.png", app.window_renderer);

  render_data_components[0] = { {0,0,16,16}, player_texture, 0 };
  for (int i = 1; i < entity_count; i++)
  {
      int x = i % 10;
      int y = i / 10;
      render_data_components[i] = { {x * 16.f, y * 8.f,16,16}, enemy_texture, 0};
  }
  
  Uint64 previous_time = SDL_GetPerformanceCounter();
  double delta_time = 0;


  //for (int i = 0; i < collider_count; i++) {
  //  int x = i % 1000;
  //  int y = i / 1000;
  //  colliders.emplace_back(Collider(x, y));
  //}

  bool is_running = true;
  while (is_running) {
    Input::Update();

    if (Input::IsKeyDown(SDL_SCANCODE_ESCAPE)) {
      is_running = false;
      break;
    }

    Uint64 current_time = SDL_GetPerformanceCounter();
    delta_time = (float)(current_time - previous_time) / SDL_GetPerformanceFrequency();
    previous_time = current_time;

    float horizontal = Input::GetAxis(InputAxis::Horizontal);
    float vertical = Input::GetAxis(InputAxis::Vertical);

    auto& player_data = render_data_components[0];
    if (horizontal != 0)
    {
        player_data.angle += (float)(delta_time * 180.f * horizontal);
    }
    if (vertical != 0)
    {
        float radians = (float)player_data.angle * (float)M_PI / 180.f;
        float facing_x = (float)std::cos(radians);
        float facing_y = (float)std::sin(radians);
        player_data.destination_rect.x += (float)(100 * delta_time * facing_x * vertical);
        player_data.destination_rect.y += (float)(100 * delta_time * facing_y * vertical);
    }

    SpinSprites(1, entity_count, render_data_components, (float)delta_time);

    // auto sort_pred = [](const Collider& col_a, const Collider& col_b)
    //{
    //     return col_a.x < col_b.x;
    // };

    // std::sort(colliders.begin(), colliders.end(), sort_pred);

    // auto search_pred = [](const Collider& col_a, const Collider& col_b)
    //{
    //     return CheckOverlap(col_a, col_b);
    // };
    // for (size_t i = 0; i < colliders.size(); i++)
    //{
    //     size_t found_index = std::binary_search(colliders.begin(),
    //     colliders.end(), colliders[i], search_pred); for (size_t j =
    //     found_index; j < colliders.size(); j++)
    //     {
    //         if (!CheckOverlap(colliders[i], colliders[j]))
    //         {
    //             //printf("%d, %d, %d, %d", colliders[i]->x, colliders[i]->y,
    //             colliders[j]->x, colliders[j]->y); break;
    //         }
    //     }
    // }
    SDL_SetRenderTarget(app.window_renderer, render_texture);
    SDL_RenderClear(app.window_renderer);

    // Render texture to screen
    SDL_RenderCopy(app.window_renderer, background_texture, NULL, NULL);
    
    DrawSprites(app.window_renderer, render_data_components, entity_count);

    SDL_SetRenderTarget(app.window_renderer, NULL);
    SDL_RenderCopy(app.window_renderer, render_texture, NULL, NULL);
    // Update screen
    SDL_RenderPresent(app.window_renderer);
    //float elapsed = (float)(SDL_GetPerformanceCounter() - current_time) / (float)SDL_GetPerformanceFrequency();
    //printf("fps: %f\n", 1.0f / elapsed);

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
