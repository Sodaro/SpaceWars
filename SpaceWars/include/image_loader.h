#pragma once
#include <map>

#include "SDL/SDL.h"

class ImageLoader {
  std::map<const char*, SDL_Texture*> loaded_textures;
  std::map<const char*, SDL_FRect*> source_rects;

 public:
  ~ImageLoader();
  ImageLoader() = default;
  SDL_Texture* GetImage(const char* path, SDL_Renderer* screen_renderer);
  void UnloadAllImages();
};