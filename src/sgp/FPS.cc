#include "FPS.h"
#include "SDL.h"

namespace FPS
{

GameLoopFunc_t  actualGameLoop; // the real game loop function
GameLoopFunc_t  GameLoopPtr;    // the function MainLoop will call
RenderPresent_t RenderPresentPtr{ SDL_RenderPresent };


void RenderPresentHook(SDL_Renderer * const renderer)
{
	SDL_RenderPresent(renderer);
}


void GameLoopHook()
{
	actualGameLoop();
}


void Init(GameLoopFunc_t const gameLoop)
{
	GameLoopPtr = actualGameLoop = gameLoop;
}


void ToggleOnOff()
{
	if (GameLoopPtr == actualGameLoop)
	{
		// Currently disabled
		RenderPresentPtr = RenderPresentHook;
		GameLoopPtr = GameLoopHook;
	}
	else
	{
		// Currently enabled
		RenderPresentPtr = SDL_RenderPresent;
		GameLoopPtr = actualGameLoop;
	}
}

}
