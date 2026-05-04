

#include "sdl_text_drawer.h"

CSdlTextDrawer::CSdlTextDrawer(SDL_Renderer* pRenderer)
	:m_renderer(pRenderer)
{

}

bool CSdlTextDrawer::setFont(const char* fontFilePath, bool bold, bool italic, float fontSize, int thickness)
{
	m_fillFont.reset(::TTF_OpenFont(fontFilePath, fontSize * m_dpiScale));
	if (m_fillFont == nullptr)return false;

	m_outlineFont.reset(::TTF_OpenFont(fontFilePath, fontSize * m_dpiScale));
	if (m_outlineFont == nullptr)return false;

	::TTF_SetFontStyle(m_fillFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontStyle(m_outlineFont.get(), (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0));
	::TTF_SetFontOutline(m_outlineFont.get(), thickness);

	return true;
}

bool CSdlTextDrawer::hasFont() const
{
	return m_fillFont != nullptr && m_outlineFont != nullptr;
}

void CSdlTextDrawer::updateText(const char* text, size_t length, int wrapWidth)
{
	if (!hasFont())return;

	static constexpr SDL_Color kWhite = SDL_Color{ 0xff, 0xff, 0xff, 0xff };
	static constexpr SDL_Color kBlack = SDL_Color{ 0x00, 0x00, 0x00, 0xff };

	auto pFillSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_fillFont.get(), text, length, m_isTextColourReversed ? kWhite : kBlack, wrapWidth),
			::SDL_DestroySurface
		);

	auto pOutlineSurface = std::unique_ptr<SDL_Surface, decltype (&::SDL_DestroySurface)>
		(
			::TTF_RenderText_Blended_Wrapped(m_outlineFont.get(), text, length, m_isTextColourReversed ? kBlack : kWhite, wrapWidth + ::TTF_GetFontOutline(m_outlineFont.get()) * 2),
			::SDL_DestroySurface
		);

	if (pFillSurface == nullptr || pOutlineSurface == nullptr)return;

	m_fillTexture.reset(::SDL_CreateTextureFromSurface(m_renderer, pFillSurface.get()));
	m_outlineTexture.reset(::SDL_CreateTextureFromSurface(m_renderer, pOutlineSurface.get()));
}

void CSdlTextDrawer::renderText(float posX, float posY)
{
	if (!m_isTextVisible)return;
	if (m_renderer == nullptr || m_fillTexture == nullptr || m_outlineTexture == nullptr)return;

	SDL_FRect outlineRect{ .x = posX, .y = posY };
	::SDL_GetTextureSize(m_outlineTexture.get(), &outlineRect.w, &outlineRect.h);

	int thickness = ::TTF_GetFontOutline(m_outlineFont.get());
	SDL_FRect fillRect{ .x = posX + thickness, .y = posY + thickness };
	::SDL_GetTextureSize(m_fillTexture.get(), &fillRect.w, &fillRect.h);

	::SDL_RenderTexture(m_renderer, m_outlineTexture.get(), nullptr, &outlineRect);
	::SDL_RenderTexture(m_renderer, m_fillTexture.get(), nullptr, &fillRect);
}

bool CSdlTextDrawer::getTextTextureSize(float* width, float* height)
{
	if (m_outlineFont == nullptr)return false;

	return ::SDL_GetTextureSize(m_outlineTexture.get(), width, height);;
}

void CSdlTextDrawer::toggleTextColour()
{
	m_isTextColourReversed ^= true;
}

void CSdlTextDrawer::setTextVisibility(bool visible)
{
	m_isTextVisible = visible;
}

bool CSdlTextDrawer::isTextVisible()
{
	return m_isTextVisible;
}

void CSdlTextDrawer::onDpiChange(float dpiScale)
{
	m_dpiScale = dpiScale;
}
