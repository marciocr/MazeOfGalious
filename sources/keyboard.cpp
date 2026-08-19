#include <sys/time.h>
#include <time.h>

#include "SDL.h"

bool IsAltPressed()
{
	SDLMod modifiers;

	SDL_PumpEvents();
	modifiers=SDL_GetModState();
   
	return ((modifiers & KMOD_ALT) != 0);
} /* IsAltPressed */ 


bool IsAltPressed2()
{
	SDL_PumpEvents();
    return ( (SDL_GetModState() & KMOD_ALT) != 0);
}

