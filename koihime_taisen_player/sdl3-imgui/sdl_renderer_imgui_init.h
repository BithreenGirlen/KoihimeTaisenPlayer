#ifndef SDL_RENDERER_IMGUI_INIT_H_
#define SDL_RENDERER_IMGUI_INIT_H_

#include <SDL3/SDL.h>

struct SSdlRendererImGuiInit
{
	SSdlRendererImGuiInit(SDL_Window* pWindow, SDL_Renderer* pRenderer, const char* fontFilePath = nullptr, float fontSize = 20.f);
	~SSdlRendererImGuiInit();

	bool isInitialised = false;
};

#endif // !SDL_RENDERER_IMGUI_INIT_H_
