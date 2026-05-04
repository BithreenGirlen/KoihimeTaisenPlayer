#ifndef SDL_TEXT_DRAWER_H_
#define SDL_TEXT_DRAWER_H_

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

class CSdlTextDrawer
{
public:
	CSdlTextDrawer(SDL_Renderer* pRenderer);
	~CSdlTextDrawer() = default;

	/// @brief Set font to be used to render text
	bool setFont(const char* fontFilePath, bool bold = true, bool italic = true, float fontSize = kFillSize, int thiciness = kOutLineSize);
	bool hasFont() const;

	void updateText(const char* text, size_t length, int wrapWidth = 0);
	void renderText(float posX, float posY);

	bool getTextTextureSize(float* width, float* height);

	/// @brief Toggle text colour between black and white
	/// @remark There is no knowing the current state
	void toggleTextColour();

	void setTextVisibility(bool visible);
	bool isTextVisible();

	/// @brief Pass the result of SDL_GetWindowDisplayScale().
	void onDpiChange(float dpiScale);

	static constexpr int kOutLineSize = 4;
	static constexpr float kFillSize = 32.f;
private:
	SDL_Renderer* m_renderer = nullptr;
	float m_dpiScale = 1.f;

	std::unique_ptr<TTF_Font, decltype(&::TTF_CloseFont)> m_fillFont{ nullptr, ::TTF_CloseFont };
	std::unique_ptr<TTF_Font, decltype(&::TTF_CloseFont)> m_outlineFont{ nullptr, ::TTF_CloseFont };;
	std::unique_ptr<SDL_Texture, decltype (&::SDL_DestroyTexture)> m_fillTexture{ nullptr, ::SDL_DestroyTexture };
	std::unique_ptr<SDL_Texture, decltype (&::SDL_DestroyTexture)> m_outlineTexture{ nullptr, ::SDL_DestroyTexture };

	bool m_isTextColourReversed = false;
	bool m_isTextVisible = true;
};

#endif // !SDL_TEXT_DRAWER_H_
