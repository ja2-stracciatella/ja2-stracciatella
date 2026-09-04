#ifndef SDL_HELPERS_H
#define SDL_HELPERS_H

#include "SDL3/SDL.h"

struct SDLDeleter
{
	void operator()(SDL_Surface* s) { SDL_DestroySurface(s); }
	void operator()(SDL_Texture* t) { SDL_DestroyTexture(t); }
};

#endif
