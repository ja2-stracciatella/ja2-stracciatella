#ifndef SGP_VISUALIZER_H
#define SGP_VISUALIZER_H

struct SDL_Renderer;

namespace Visualizer
{
	void Render(SDL_Renderer * const renderer);
	void ToggleOnOff();
}

#endif
