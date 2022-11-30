#include "image_loader.h"
#include "SDL/SDL_image.h"
#include <iostream>

SDL_Surface* ImageLoader::GetImage(const char* path, SDL_Surface*)
{
    if (loaded_surfaces.find(path) != loaded_surfaces.end())
    {
        return loaded_surfaces[path];
    }
    else
    {
        SDL_Surface* raw_surface = IMG_Load(path);
        SDL_Surface* optimized_surface = nullptr;
        if (raw_surface == nullptr)
        {
            printf("Failed to create surface, path: %s, SDL_Error: %s\n", path, SDL_GetError());
        }
        else
        {
            optimized_surface = SDL_ConvertSurfaceFormat(raw_surface, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, 0);
            if (optimized_surface == nullptr)
            {
                printf("Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError());
            }

            //Get rid of old loaded surface
            SDL_FreeSurface(raw_surface);
            loaded_surfaces.emplace(path, optimized_surface);
        }
        return optimized_surface;
    }
}

void ImageLoader::UnloadAllImages()
{
    for (auto& pair : loaded_surfaces)
    {
        SDL_FreeSurface(pair.second);
    }
}

ImageLoader::~ImageLoader()
{
    UnloadAllImages();
}
