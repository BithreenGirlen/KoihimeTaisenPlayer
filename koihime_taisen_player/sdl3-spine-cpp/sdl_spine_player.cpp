

#include "sdl_spine_player.h"

CSdlSpinePlayer::CSdlSpinePlayer(SDL_Window* pSdlWindow, SDL_Renderer* pSdlRenderer)
	:m_pSdlWindow(pSdlWindow), m_pSdlRenderer(pSdlRenderer)
{
	/* 
	* In spine-c, SDL_Renderer can be stored in spAtlas::rendererObject when to create spAtlas.
	* But in spine-cpp, spine::Atlas does not have member variable to store user data.
	* So SDL_Renderer should be passed to derived class from spine::TextureLoader instead.
	*/
#ifdef SDL_SPINE_CPP
	m_textureLoader.setRenderer(pSdlRenderer);
#endif
}

CSdlSpinePlayer::~CSdlSpinePlayer()
{

}

void CSdlSpinePlayer::redraw()
{
	float fTargetWidth = 0.f, fTargetHeight = 0.f;
	SDL_Texture* pSdlTexture = ::SDL_GetRenderTarget(m_pSdlRenderer);
	if (pSdlTexture == nullptr) /* Render target is window. */
	{
		int iWindowWidth = 0, iWindowHeight = 0;
		::SDL_GetWindowSize(m_pSdlWindow, &iWindowWidth, &iWindowHeight);
		fTargetWidth = static_cast<float>(iWindowWidth);
		fTargetHeight = static_cast<float>(iWindowHeight);
	}
	else /* Render trarget is texture */
	{
		bool bRet = ::SDL_GetTextureSize(pSdlTexture, &fTargetWidth, &fTargetHeight);
		if (!bRet)return;
	}

	float fX = (m_fBaseSize.x * m_fSkeletonScale - fTargetWidth) / 2.f;
	float fY = (m_fBaseSize.y * m_fSkeletonScale - fTargetHeight) / 2.f;

	if (!m_isDrawOrderReversed)
	{
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables[i]->draw(m_fSkeletonScale, -fX, -fY);
		}
	}
	else
	{
		for (long long i = m_drawables.size() - 1; i >= 0; --i)
		{
			m_drawables[i]->draw(m_fSkeletonScale, -fX, -fY);
		}
	}
}

SDL_FRect CSdlSpinePlayer::getBoundingBox() const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	for (const auto& drawable : m_drawables)
	{
		const SDL_FRect rect = drawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
		fMaxX = (std::max)(fMaxX, rect.x + rect.w);
		fMaxY = (std::max)(fMaxY, rect.y + rect.h);
	}

	return { fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

SDL_FRect CSdlSpinePlayer::getCurrentBoundingOfSlot(const char* slotName, size_t nameLength) const
{
	bool found = false;
	for (const auto& drawable : m_drawables)
	{
		const auto& rect = drawable->getBoundingBoxOfSlot(slotName, nameLength, &found);
		if (found)
		{
			return rect;
		}
	}
	return {};
}

void CSdlSpinePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	int skeletonWidth = static_cast<int>(m_fBaseSize.x);
	int skeletonHeight = static_cast<int>(m_fBaseSize.y);

	SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_pSdlWindow);
	if (displayId == 0)return;

	const SDL_DisplayMode* pDisplayMode = ::SDL_GetCurrentDisplayMode(displayId);
	if (pDisplayMode == nullptr)return;

	int desktopWidth = pDisplayMode->w;
	int desktopHeight = pDisplayMode->h;

	if (skeletonWidth > desktopWidth || skeletonHeight > desktopHeight)
	{
		float fScaleX = static_cast<float>(desktopWidth) / skeletonWidth;
		float fScaleY = static_cast<float>(desktopHeight) / skeletonHeight;

		m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	}
}

void CSdlSpinePlayer::workOutDefaultOffset()
{
	m_fDefaultOffset = {};

	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;

	for (const auto& pDrawable : m_drawables)
	{
		const auto& rect = pDrawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
	}

	m_fDefaultOffset.x = fMinX == FLT_MAX ? 0 : fMinX;
	m_fDefaultOffset.y = fMinY == FLT_MAX ? 0 : fMinY;
}

