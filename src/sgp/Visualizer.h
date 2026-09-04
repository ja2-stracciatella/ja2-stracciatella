#ifndef SGP_VISUALIZER_H
#define SGP_VISUALIZER_H

struct SDL_Renderer;

namespace Visualizer
{
	bool IsActivated();

	void Render(SDL_Renderer * const renderer);
	void ToggleOnOff();
}

#endif
