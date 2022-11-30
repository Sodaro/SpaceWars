// SpaceWars.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "image_loader.h"
#include "SDL/SDL_image.h"
#include <iostream>

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

struct Application
{
    SDL_Window* window = nullptr;
    SDL_Surface* screen_surface = nullptr;
};

bool InitializeApplication(Application& app)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
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

    //Initialize PNG and JPEG loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));
    app.window = window;
    app.screen_surface = surface;
    return true;
}

int main(int, char*[])
{
    Application app;
    ImageLoader image_loader;
    if (!InitializeApplication(app))
    {
        printf("Failed to initalize application!");
        return 1;
    }
    
    SDL_Surface* image_surface = image_loader.GetImage("./assets/cool.bmp", app.screen_surface);
    SDL_Surface* square_surface = image_loader.GetImage("./assets/square.jpg", app.screen_surface);
    SDL_Surface* triangle_surface = image_loader.GetImage("./assets/triangle.png", app.screen_surface);
    SDL_Event e;
    SDL_Rect triangle_dst{ 16,16,32,32 };
    SDL_Rect triangle_src{ 0,0,32,32 };
    bool is_running = true;
    while (is_running)
    {
        //Poll all the events in the event queue
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                is_running = false;
                break;
            }
        }

        SDL_BlitSurface(image_surface, NULL, app.screen_surface, NULL);
        SDL_BlitSurface(square_surface, NULL, app.screen_surface, NULL);
        SDL_BlitSurface(triangle_surface, &triangle_src, app.screen_surface, &triangle_dst);
        SDL_UpdateWindowSurface(app.window);
    }

    image_loader.UnloadAllImages();
    SDL_DestroyWindow(app.window);
    app.window = nullptr;

    SDL_Quit();
    
    return 0;
}
