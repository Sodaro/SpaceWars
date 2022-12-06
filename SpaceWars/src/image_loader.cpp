#include "image_loader.h"

#include <iostream>

#include "SDL/SDL_image.h"

SDL_Texture* ImageLoader::GetImage(const char* path,
                                   SDL_Renderer* screen_renderer) {
  if (loaded_textures.find(path) != loaded_textures.end()) {
    return loaded_textures[path];
  } else {
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = nullptr;
    if (surface == nullptr) {
      printf("Failed to create surface, path: %s, SDL_Error: %s\n", path,
             SDL_GetError());
    } else {
      texture = SDL_CreateTextureFromSurface(screen_renderer, surface);
      if (texture == nullptr) {
        printf("Unable to create texture from %s! SDL Error: %s\n", path,
               SDL_GetError());
      }

      // Get rid of old loaded surface
      SDL_FreeSurface(surface);
      loaded_textures.emplace(path, texture);
    }
    return texture;
  }
}

void ImageLoader::UnloadAllImages() {
  for (auto& pair : loaded_textures) {
    SDL_DestroyTexture(pair.second);
    pair.second = nullptr;
  }
}

ImageLoader::~ImageLoader() { UnloadAllImages(); }
