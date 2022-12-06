// SpaceWars.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "image_loader.h"
#include "SDL/SDL_image.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;
constexpr int JOYSTICK_DEAD_ZONE = 8000; //Analog joystick dead zone

struct Application
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

struct Collider
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    Collider(int x, int y) : x(x), y(y), w(16), h(16) {};
};

enum class InputAxis
{
    Horizontal,
    Vertical
};

constexpr SDL_Scancode USED_SCANCODES[] = {
    SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D,
    SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_SPACE, SDL_SCANCODE_X,
    SDL_SCANCODE_ESCAPE
};


class Input
{
    std::map<InputAxis, std::vector<SDL_Scancode>> axis_mappings;
    std::map<SDL_Scancode, bool> key_states = {};
public:
    void Initialize()
    {
        for (int i = 0; i < sizeof(USED_SCANCODES)/sizeof(SDL_Scancode); i++)
            key_states[USED_SCANCODES[i]] = false;

        axis_mappings =
        {
           {InputAxis::Horizontal, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D, SDL_SCANCODE_LEFT, SDL_SCANCODE_A}},
           {InputAxis::Vertical, {SDL_SCANCODE_DOWN, SDL_SCANCODE_S, SDL_SCANCODE_UP, SDL_SCANCODE_W}}
        };
    }

    void Update()
    {
        SDL_Event e;
        //Poll all the events in the event queue
        while (SDL_PollEvent(&e) != 0)
        {
            key_states[e.key.keysym.scancode] = e.key.state;
        }
    }
    int GetAxis(InputAxis axis)
    {
        const auto& a = axis_mappings[axis];
        return (key_states[a[0]] | key_states[a[1]]) - (key_states[a[2]] | key_states[a[3]]);
    };

    bool IsKeyUp(SDL_Scancode scan_code)
    {
        return !key_states[scan_code];
    }

    bool IsKeyDown(SDL_Scancode scan_code)
    {
        return key_states[scan_code];
    }
};

bool CheckOverlap(const Collider& a, const Collider& b)
{
    return a.x <= b.x + b.w && a.y <= b.y + b.h && a.x + a.w > b.x && a.y + a.h > b.y;
}

bool InitializeApplication(Application& app)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        printf("Failed to initialize SDL! Error: % s\n", SDL_GetError());
        return false;
    }
    SDL_Window* window = SDL_CreateWindow("SpaceWars", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        printf("Failed to create renderer! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    //Initialize PNG and JPEG loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    app.renderer = renderer;
    app.window = window;
    return true;
}

int main(int, char*[])
{
    Application app;
    ImageLoader image_loader;
    Input input;
    
    if (!InitializeApplication(app))
    {
        printf("Failed to initalize application!");
        return 1;
    }

    input.Initialize();
    
    // unoptimized: 5000    colliders   2-3 fps
    // optimized:   5000    colliders   250+ fps
    // optimized:   10000   colliders   120+ fps
    constexpr unsigned int collider_count = 5000;
    std::vector<Collider> colliders;
    colliders.reserve(sizeof(Collider) * collider_count);

    SDL_Texture* background_texture = image_loader.GetImage("./assets/cool.bmp", app.renderer);
    SDL_Texture* square_texture = image_loader.GetImage("./assets/square.jpg", app.renderer);
    SDL_Texture* triangle_texture = image_loader.GetImage("./assets/triangle.png", app.renderer);
    
    SDL_FRect triangle_dst{ 0,16,32,32 };
    SDL_FRect square_dst{ 16,16,32,32 };
    SDL_Rect triangle_src{ 0,0,32,32 };
    SDL_Rect square_src{ 0,0,32,32 };
    bool is_running = true;

    for (int i = 0; i < collider_count; i++)
    {
        int x = i % 1000;
        int y = i / 1000;
        colliders.emplace_back(Collider(x, y));
    }

    
    Uint64 previous_time = SDL_GetPerformanceCounter();
    double delta_time = 0;
    
    while (is_running)
    {
        input.Update();
        Uint64 current_time = SDL_GetPerformanceCounter();
        delta_time = (float)(current_time - previous_time) / SDL_GetPerformanceFrequency();
        previous_time = current_time;

        //printf("%f\n", delta_time);

        auto horizontal = input.GetAxis(InputAxis::Horizontal);
        auto vertical = input.GetAxis(InputAxis::Vertical);
        triangle_dst.x += (float)(100 * delta_time * horizontal);
        triangle_dst.y += (float)(100 * delta_time * vertical);
        //auto sort_pred = [](const Collider& col_a, const Collider& col_b)
        //{
        //    return col_a.x < col_b.x;
        //};

        //std::sort(colliders.begin(), colliders.end(), sort_pred);

        //auto search_pred = [](const Collider& col_a, const Collider& col_b)
        //{
        //    return CheckOverlap(col_a, col_b);
        //};
        //for (size_t i = 0; i < colliders.size(); i++)
        //{
        //    size_t found_index = std::binary_search(colliders.begin(), colliders.end(), colliders[i], search_pred);
        //    for (size_t j = found_index; j < colliders.size(); j++)
        //    {
        //        if (!CheckOverlap(colliders[i], colliders[j]))
        //        {
        //            //printf("%d, %d, %d, %d", colliders[i]->x, colliders[i]->y, colliders[j]->x, colliders[j]->y);
        //            break;
        //        }
        //    }
        //}

        SDL_RenderClear(app.renderer);

        //Render texture to screen
        SDL_RenderCopy(app.renderer, background_texture, NULL, NULL);
        SDL_RenderCopyF(app.renderer, square_texture, &square_src, &square_dst);
        SDL_RenderCopyF(app.renderer, triangle_texture, &triangle_src, &triangle_dst);

        //Update screen
        SDL_RenderPresent(app.renderer);
        //float elapsed = (float)(SDL_GetPerformanceCounter() - current_time) / (float)SDL_GetPerformanceFrequency();
        //printf("fps: %f\n", 1.0f / elapsed);

        //printf("%i joysticks were found.\n\n", SDL_NumJoysticks());
        //SDL_Delay(16);
    }

    image_loader.UnloadAllImages();

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    app.window = nullptr;

    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
