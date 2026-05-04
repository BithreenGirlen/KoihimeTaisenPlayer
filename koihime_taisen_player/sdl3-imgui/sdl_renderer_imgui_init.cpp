

#include "sdl_renderer_imgui_init.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

SSdlRendererImGuiInit::SSdlRendererImGuiInit(SDL_Window* pWindow, SDL_Renderer* pRenderer, const char* fontFilePath, float fontSize)
{
	if (pWindow == nullptr || pRenderer == nullptr)return;

	/* Setup Dear ImGui */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	io.ConfigViewportsNoAutoMerge = true;
	io.ConfigViewportsNoDefaultParent = false;

	ImGui::StyleColorsLight();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.875f;

	const auto& fontAtlas = io.Fonts;
	const ImWchar* glyph = fontAtlas->GetGlyphRangesChineseFull();
	fontAtlas->AddFontFromFileTTF(fontFilePath, fontSize, nullptr, glyph);

	isInitialised = ImGui_ImplSDL3_InitForSDLRenderer(pWindow, pRenderer);
	isInitialised &= ImGui_ImplSDLRenderer3_Init(pRenderer);
}

SSdlRendererImGuiInit::~SSdlRendererImGuiInit()
{
	if (isInitialised)
	{
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}
}
