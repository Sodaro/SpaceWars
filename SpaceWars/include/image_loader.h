#pragma once
#include "SDL/SDL.h"
#include <map>

class ImageLoader
{
	std::map<const char*, SDL_Surface*> loaded_surfaces;
public:
	~ImageLoader();
	ImageLoader() = default; 
	SDL_Surface* GetImage(const char* path, SDL_Surface* screen_surface);
	void UnloadAllImages();
};