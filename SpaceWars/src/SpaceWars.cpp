// SpaceWars.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "SDL/SDL.h"
#include <iostream>

struct Application
{
    SDL_Window* window = NULL;
    SDL_Surface* screen = NULL;
};

bool Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize SDL! Error: % s\n", SDL_GetError());
        return false;
    }

}

bool LoadMedia();

void Close();

int main(int, char*[])
{
    SDL_Surface* image_surface = NULL;
    return 0;
}
