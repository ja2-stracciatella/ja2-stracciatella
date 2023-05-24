#include "Font.h"
#include "FPS.h"
#include "SDL.h"
#include <chrono>
#include <memory>
#include <numeric>
#include <vector>
#include <string_theory/format>


using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace FPS
{

struct SDLDeleter
{
	void operator()(SDL_Surface *s) { SDL_FreeSurface(s); }
	void operator()(SDL_Texture *t) { SDL_DestroyTexture(t); }
};


GameLoopFunc_t  ActualGameLoop; // the real game loop function
GameLoopFunc_t  GameLoopPtr;    // the function MainLoop will call
RenderPresent_t RenderPresentPtr{ SDL_RenderPresent };

std::unique_ptr<SDL_Surface, SDLDeleter> Surface;
std::unique_ptr<SDL_Texture, SDLDeleter> Texture;
SGPVObject * DisplayFont;

std::vector<Clock::duration> LastGameLoopDurations;
Clock::time_point TimeLastDisplayed;

unsigned FramesSinceLastDisplay;


void UpdateTexture(SDL_Renderer * const renderer)
{
	SetFontAttributes(DisplayFont, FONT_FCOLOR_WHITE);

	SDL_FillRect(Surface.get(), nullptr, 0);

	auto * const pixels = static_cast<UINT16*>(Surface->pixels);

	MPrintBuffer(pixels, Surface->pitch, 0, 0,
		ST::format("{} FPS", FramesSinceLastDisplay));

	if (!LastGameLoopDurations.empty())
	{
		auto const averageLoopDuration =
			std::accumulate(
				LastGameLoopDurations.begin(),
				LastGameLoopDurations.end(),
				Clock::duration{0})
			/ LastGameLoopDurations.size();

		MPrintBuffer(pixels, Surface->pitch, 0, 12,
			ST::format("Game Loop: {} micros",
				std::chrono::duration_cast<std::chrono::microseconds>(averageLoopDuration).count()));
	}

	Texture.reset(SDL_CreateTextureFromSurface(renderer, Surface.get()));
}


void RenderPresentHook(SDL_Renderer * const renderer)
{
	++FramesSinceLastDisplay;

	auto const now = Clock::now();

	// We must copy the texture each frame otherwise it would flicker,
	// but we only update its content once per second to make it less noisy.
	if (now > TimeLastDisplayed + 1s)
	{
		UpdateTexture(renderer);
		TimeLastDisplayed = now;
		FramesSinceLastDisplay = 0;
		LastGameLoopDurations.clear();
	}

	SDL_Rect const dest{ 11, 23, Surface->w, Surface->h };
	SDL_RenderCopy(renderer, Texture.get(), nullptr, &dest);
	SDL_RenderPresent(renderer);
}


void GameLoopHook()
{
	auto const before = Clock::now();
	ActualGameLoop();
	auto const elapsed = Clock::now() - before;

	// Only store "interesting" game loops
	if (elapsed > 150us)
	{
		LastGameLoopDurations.push_back(elapsed);
	}
}


void Init(GameLoopFunc_t const gameLoop, SGPVObject * const displayFont)
{
	GameLoopPtr = ActualGameLoop = gameLoop;
	DisplayFont = displayFont;
}


void ToggleOnOff()
{
	if (GameLoopPtr == ActualGameLoop)
	{
		// Currently disabled
		RenderPresentPtr = RenderPresentHook;
		GameLoopPtr = GameLoopHook;

		Surface.reset(SDL_CreateRGBSurfaceWithFormat(0, 320, 26, 0, SDL_PIXELFORMAT_RGB565));
		SDL_SetColorKey(Surface.get(), true, 0);
	}
	else
	{
		// Currently enabled
		RenderPresentPtr = SDL_RenderPresent;
		GameLoopPtr = ActualGameLoop;

		Surface.reset();
		Texture.reset();
	}
}

}
