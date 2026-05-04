

#include "koihime_taisen_scene_player.h"
#include "koihime_taisen.h"

#include "sdl3-spine-cpp/spine_file_verifier.h"
#include "sdl_filesystem_utility.h"
#include "path_utility.h"

#include <SDL3_image/SDL_image.h>

CKoihimeTaisenScenePlayer::CKoihimeTaisenScenePlayer(SDL_Window* pWindow, SDL_Renderer* pRenderer)
	:m_window(pWindow), m_renderer(pRenderer)
{
	for (auto& spinePlayer : m_cutinManager.sdlSpinePlayers)
	{
		spinePlayer = std::make_unique<CSdlSpinePlayer>(pWindow, pRenderer);
	}

	m_textDrawer = std::make_unique<CSdlTextDrawer>(pRenderer);

	m_voicePlayer.setVolume(0.5f);
}

bool CKoihimeTaisenScenePlayer::setFont(const char* fontFileName, bool bold, bool italic, float messageFontSize, int thickness)
{
	bool bRet = m_textDrawer->setFont(fontFileName, bold, italic, messageFontSize, thickness);
	if (bRet && !m_formattedText.empty())
	{
		updateText();
	}

	return bRet;
}

bool CKoihimeTaisenScenePlayer::loadScenario(const std::string& scenarioFilePath)
{
	clearScenarioData();
	std::vector<adv::ImageDatum> imageData;
	bool bRet = koihime_taisen::ReadScenario(scenarioFilePath, m_textData, imageData, m_cutInData, m_sceneData, m_labelData);
	if (!bRet)return false;

	for (const auto& imageDatum : imageData)
	{
		ImageFrameDatum imageFrameDatum;
		for (const auto& filePath : imageDatum.filePaths)
		{
			SDL_Texture* pTexture = ::IMG_LoadTexture(m_renderer, filePath.data());
			if (pTexture == nullptr)
			{
				::SDL_Log(::SDL_GetError());
				continue;
			}
			::SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
			imageFrameDatum.textures.emplace_back(pTexture, ::SDL_DestroyTexture);
		}
		m_imageFrameData.push_back(std::move(imageFrameDatum));
	}

	bool isCutin1Loaded = false;
	bool isCutin2Loaded = false;
	for (const auto& cutInDatum : m_cutInData)
	{
		const auto& cutInFilePath = cutInDatum.filePath;
		uint8_t index = CutinManager::NameToIndex(cutInFilePath);
		if (index >= static_cast<uint8_t>(CutinManager::CutinIndex::kMax))continue;
		else if (index == static_cast<uint8_t>(CutinManager::CutinIndex::CutIn1) && isCutin1Loaded)continue;
		else if (index == static_cast<uint8_t>(CutinManager::CutinIndex::CutIn2) && isCutin2Loaded)continue;

		std::string atlasFilePath = cutInFilePath + ".atlas";
		std::string skelFilePath = cutInFilePath + ".bin";

		std::vector<std::string> atlasFileData;
		std::vector<std::string> skelFileData;

		atlasFileData.emplace_back(sdl_filesystem_utility::LoadFileAsString(atlasFilePath.data()));
		skelFileData.emplace_back(sdl_filesystem_utility::LoadFileAsString(skelFilePath.data()));

		std::string_view textureDirectory = path_utility::ExtractParentPath(cutInFilePath);
		std::vector<std::string> textureDirectories(atlasFileData.size(), std::string(textureDirectory.data(), textureDirectory.length()));

		const auto& metaData = spine_file_verifier::VerifySkeletonFileData(reinterpret_cast<unsigned char*>(skelFileData.back().data()), skelFileData.back().length());
		if (metaData.skeletonFormat == spine_file_verifier::SkeletonFormat::Neither)continue;
		if (!metaData.version.starts_with("3.8"))continue;

		bool bRet = m_cutinManager.sdlSpinePlayers[index]->loadSpineFromMemory(atlasFileData, textureDirectories, skelFileData);
		if (bRet)
		{
			if (index == static_cast<uint8_t>(CutinManager::CutinIndex::CutIn1))
			{
				isCutin1Loaded = true;
			}
			else if (index == static_cast<uint8_t>(CutinManager::CutinIndex::CutIn2))
			{
				isCutin2Loaded = true;
			}
		}
	}

	prepareScene();
	getCurrentImageSize(&m_sceneWidth, &m_sceneHeight);
	m_sceneTexture.reset(::SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_sceneWidth, m_sceneHeight));
	::SDL_SetTextureBlendMode(m_sceneTexture.get(), SDL_BLENDMODE_BLEND);

	workOutDefaultScale();
	resetScale();

	m_textClock.restart();

	return !m_sceneData.empty();
}

bool CKoihimeTaisenScenePlayer::hasScenarioData() const
{
	return !m_sceneData.empty();
}

void CKoihimeTaisenScenePlayer::update(float fDelta)
{
	for (const auto& sdlSpinePlayer : m_cutinManager.sdlSpinePlayers)
	{
		sdlSpinePlayer->update(fDelta * m_timeScale);
	}

	m_elapsedTime += fDelta * m_timeScale;
	if (m_elapsedTime >= 1.f / m_fps)
	{
		if (m_nSceneIndex >= m_sceneData.size())return;
		size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		if (nImageIndex >= m_imageFrameData.size())return;

		m_imageFrameData[nImageIndex].StepFrame();
		m_elapsedTime = 0;
	}

	checkTextClock();
}

void CKoihimeTaisenScenePlayer::draw(float textPosX, float textPosY)
{
	if (m_nSceneIndex >= m_sceneData.size())return;

	size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
	if (nImageIndex >= m_imageFrameData.size())return;

	const auto* pCurrentFrameTexture = m_imageFrameData[nImageIndex].getCurrntFrameTexture();
	if (pCurrentFrameTexture == nullptr)return;

	if (m_sceneTexture != nullptr)
	{
		SDL_Texture* pPreviousRenderTarget = ::SDL_GetRenderTarget(m_renderer);

		::SDL_SetRenderTarget(m_renderer, m_sceneTexture.get());
		::SDL_RenderClear(m_renderer);
		::SDL_RenderTexture(m_renderer, pCurrentFrameTexture->get(), nullptr, nullptr);

		if (m_sceneData[m_nSceneIndex].hasCutIn && m_isCutInVisible)
		{
			size_t cutInIndex = m_sceneData[m_nSceneIndex].nCutInIndex;
			if (cutInIndex < m_cutInData.size())
			{
				const auto& cutInDatum = m_cutInData[cutInIndex];
				uint8_t index = CutinManager::NameToIndex(cutInDatum.filePath);
				if (index < static_cast<uint8_t>(CutinManager::CutinIndex::kMax))
				{
					m_cutinManager.sdlSpinePlayers[index]->redraw();
				}
			}
		}

		::SDL_SetRenderTarget(m_renderer, pPreviousRenderTarget);
	}

	SDL_FPoint targetSize = getRenderTargetSize();
	SDL_FRect dstRect
	{
		.x = -(m_sceneWidth * m_fScale - targetSize.x) / 2.f - m_fOffset.x / 2.f,
		.y = -(m_sceneHeight * m_fScale - targetSize.y) / 2.f - m_fOffset.y / 2.f,
		.w = m_sceneWidth * m_fScale,
		.h = m_sceneHeight * m_fScale
	};
	::SDL_RenderTexture(m_renderer, m_sceneTexture.get(), nullptr, &dstRect);

	m_textDrawer->renderText(textPosX, textPosY);
}

SDL_Point CKoihimeTaisenScenePlayer::getSceneSize() const
{
	SDL_Point sceneSize
	{
		.x = static_cast<int>(m_sceneWidth * m_fScale * kDefaultZoom),
		.y = static_cast<int>(m_sceneHeight * m_fScale * kDefaultZoom)
	};

	return sceneSize;
}

void CKoihimeTaisenScenePlayer::rescale(bool upscale)
{
	m_fScale += kScaleDelta * (upscale ? 1.f : -1.f);
	m_fScale = (std::max)(m_fScale, kMinScale);
}

void CKoihimeTaisenScenePlayer::addOffset(int iX, int iY)
{
	if (!m_toMoveCutIn)
	{
		m_fOffset.x += iX * m_fScale;
		m_fOffset.y += iY * m_fScale;

		adjustOffset();
	}
	else
	{
		for (const auto& spinePlayer : m_cutinManager.sdlSpinePlayers)
		{
			spinePlayer->addOffset(iX, iY);
		}
	}
}

void CKoihimeTaisenScenePlayer::resetScale()
{
	m_fScale = m_fDefaultScale;
	m_fOffset = {};

	m_timeScale = kDefaultTimeScale;
	m_fps = kDefaultFps;

	for (const auto& pSdlSpinePlayer : m_cutinManager.sdlSpinePlayers)
	{
		pSdlSpinePlayer->resetScale();
	}

	m_cutinManager.sdlSpinePlayers[static_cast<uint8_t>(CutinManager::CutinIndex::CutIn1)]->setTimeScale(CutinManager::kDefaultCutIn1TimeScale);
	m_cutinManager.sdlSpinePlayers[static_cast<uint8_t>(CutinManager::CutinIndex::CutIn2)]->setTimeScale(CutinManager::kDefaultCutIn2TimeScale);
}

float CKoihimeTaisenScenePlayer::getTimeScale() const
{
	return m_timeScale;
}

void CKoihimeTaisenScenePlayer::setTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

void CKoihimeTaisenScenePlayer::shiftScene(bool forward)
{
	if (m_sceneData.empty())return;

	if (forward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}

	prepareScene();
}

bool CKoihimeTaisenScenePlayer::hasReachedLastScene() const
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}

void CKoihimeTaisenScenePlayer::setTextVisibility(bool visible)
{
	m_textDrawer->setTextVisibility(visible);
}

bool CKoihimeTaisenScenePlayer::isTextVisible() const
{
	return m_textDrawer->isTextVisible();
}

void CKoihimeTaisenScenePlayer::toggleTextColour()
{
	m_textDrawer->toggleTextColour();
	if (!m_formattedText.empty())
	{
		updateText();
	}
}

void CKoihimeTaisenScenePlayer::setCutInVisibility(bool visible)
{
	m_isCutInVisible = visible;
}

bool CKoihimeTaisenScenePlayer::isCutInVisible() const
{
	return m_isCutInVisible;
}

void CKoihimeTaisenScenePlayer::setCutInMoveMode(bool toMoveCutIn)
{
	m_toMoveCutIn = toMoveCutIn;
}

bool CKoihimeTaisenScenePlayer::isCutInMoveMode() const
{
	return m_toMoveCutIn;
}

const std::vector<adv::LabelDatum>& CKoihimeTaisenScenePlayer::getLabelData() const
{
	return m_labelData;
}

bool CKoihimeTaisenScenePlayer::jumpScene(size_t nSceneIndex)
{
	if (nSceneIndex >= m_sceneData.size())return false;

	m_nSceneIndex = nSceneIndex;
	prepareScene();

	return true;
}

void CKoihimeTaisenScenePlayer::onDpiChange(float dpiScale)
{
	m_textDrawer->onDpiChange(dpiScale);
}

void CKoihimeTaisenScenePlayer::onResize()
{
	SDL_FPoint targetSize = getRenderTargetSize();
	m_textDrawer->updateText(m_formattedText.data(), m_formattedText.length(), targetSize.x);
	adjustOffset();
}

CSdlAudioPlayer& CKoihimeTaisenScenePlayer::getVoicePlayer()
{
	return m_voicePlayer;
}

void CKoihimeTaisenScenePlayer::clearScenarioData()
{
	m_textData.clear();
	m_imageFrameData.clear();
	m_cutInData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_labelData.clear();

	m_formattedText.clear();
}

void CKoihimeTaisenScenePlayer::prepareScene()
{
	prepareText();
}

void CKoihimeTaisenScenePlayer::prepareText()
{
	if (m_nSceneIndex >= m_sceneData.size())return;

	size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
	if (nTextIndex >= m_textData.size())return;

	char indexBuffer[64]{};
	sprintf_s(indexBuffer, "\n%zu/%zu", nTextIndex + 1, m_textData.size());
	m_formattedText += indexBuffer;

	const auto& t = m_textData[nTextIndex];
	m_formattedText.assign(t.message).append(indexBuffer);
	updateText();

	if (!t.voicePath.empty())
	{
		m_voicePlayer.load(t.voicePath.data());
	}
}

void CKoihimeTaisenScenePlayer::updateText()
{
	SDL_FPoint targetSize = getRenderTargetSize();
	m_textDrawer->updateText(m_formattedText.data(), m_formattedText.length(), targetSize.x);
}

void CKoihimeTaisenScenePlayer::checkTextClock()
{
	float fElapsed = m_textClock.getElapsedTime();
	if (::isgreaterequal(fElapsed, 3.f))
	{
		if (m_voicePlayer.isEnded())
		{
			m_textClock.restart();

			if (!hasReachedLastScene())
			{
				shiftScene(true);
			}
		}
	}
}

void CKoihimeTaisenScenePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_window);
	if (displayId == 0)return;

	const SDL_DisplayMode* pDisplayMode = ::SDL_GetCurrentDisplayMode(displayId);
	if (pDisplayMode == nullptr)return;

	int desktopWidth = pDisplayMode->w;
	int desktopHeight = pDisplayMode->h;

	if (m_sceneWidth > desktopWidth || m_sceneHeight > desktopHeight)
	{
		float fScaleX = static_cast<float>(desktopWidth) / m_sceneWidth;
		float fScaleY = static_cast<float>(desktopHeight) / m_sceneHeight;

		m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	}
}

void CKoihimeTaisenScenePlayer::getCurrentImageSize(int* width, int* height)
{
	if (m_nSceneIndex >= m_sceneData.size())return;

	size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
	if (nImageIndex >= m_imageFrameData.size())return;

	const auto* currentFrameTexture = m_imageFrameData[nImageIndex].getCurrntFrameTexture();
	if (currentFrameTexture == nullptr)return;

	SDL_FPoint frameSize{};
	::SDL_GetTextureSize(currentFrameTexture->get(), &frameSize.x, &frameSize.y);
	if (width != nullptr)*width = static_cast<int>(frameSize.x);
	if (height != nullptr)*height = static_cast<int>(frameSize.y);
}

void CKoihimeTaisenScenePlayer::adjustOffset()
{
	SDL_FPoint targetSize = getRenderTargetSize();

	float fMaxOffsetX = m_sceneWidth * m_fScale - targetSize.x;
	float fMaxOffsetY = m_sceneHeight * m_fScale - targetSize.y;

	m_fOffset.x = (std::max)(-fMaxOffsetX, m_fOffset.x);
	m_fOffset.y = (std::max)(-fMaxOffsetY, m_fOffset.y);

	m_fOffset.x = (std::min)(fMaxOffsetX, m_fOffset.x);
	m_fOffset.y = (std::min)(fMaxOffsetY, m_fOffset.y);
}

SDL_FPoint CKoihimeTaisenScenePlayer::getRenderTargetSize()
{
	SDL_FPoint targetSize{};
	SDL_Texture* pSdlRenderTexture = ::SDL_GetRenderTarget(m_renderer);
	if (pSdlRenderTexture == nullptr) /* Render target is window. */
	{
		int windowWidth = 0, windowHeight = 0;
		::SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
		targetSize.x = static_cast<float>(windowWidth);
		targetSize.y = static_cast<float>(windowHeight);
	}
	else /* Render trarget is texture */
	{
		::SDL_GetTextureSize(pSdlRenderTexture, &targetSize.x, &targetSize.y);
	}

	return targetSize;
}
