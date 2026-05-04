

#include <locale.h>

#include "sdl_main_window.h"
#include "setup.h"
#include "sdl3-imgui/sdl_renderer_imgui_init.h"

#define SDL_MAIN_NEEDED 1
#include <SDL3/SDL_main.h>

/*
* Animation part is composed of lots of textures, so it would be better to prioritise using dedicated GPU.
* It is trade-off between power-saving (iGPU) and memory-saving (dGPU).
*/
#if defined PRIORITISE_USING_DEDICATED_GPU
extern "C"
{
	_declspec(selectany) _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	_declspec(selectany) _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

struct SSdlInit
{
	SSdlInit()
	{
		isInitialised = ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		isInitialised |= ::TTF_Init();
		isInitialised |= ::MIX_Init();
		/* In SDL3, IMG_Init() has been removed. In SDL2, not required unless optional format be used. */
	}
	~SSdlInit()
	{
		::MIX_Quit();
		::TTF_Quit();
		::SDL_Quit();
	}

	bool isInitialised = false;
};

int SDL_main(int argc, char* argv[])
{
	::setlocale(LC_ALL, ".utf8");

	SSdlInit sdlInit;
	if (!sdlInit.isInitialised)return -1;

	CSdlMainWindow mainWindow(reinterpret_cast<const char*>(u8"戀姬大𢧐"));
	if (mainWindow.getRenderer() == nullptr)
	{
		::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to create SDL window", nullptr);
		return -1;
	}

	if (!setup::Initialise())
	{
		::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load setting file.", nullptr);
		return -1;
	}

	const setup::FontDatum& s = setup::GetFontDatum();

	SSdlRendererImGuiInit imguiInit(mainWindow.getWindow(), mainWindow.getRenderer(), s.filePath.data(), static_cast<float>(s.uiFontSize));
	if (!imguiInit.isInitialised)
	{
		::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to initialise Dear ImGui SDL renderer backend.", nullptr);
		return -1;
	}

	if (!mainWindow.loadFont(s.filePath.data(), s.bold, s.italic, static_cast<float>(s.messageFontSize), s.thickness))
	{
		::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load font to render message text.", nullptr);
		return -1;
	}

	mainWindow.display();

	return 0;
}