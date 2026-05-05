

#include "sdl_main_window.h"
#include "sdl_filesystem_utility.h"
#include "path_utility.h"
#include "setup.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

CSdlMainWindow::CSdlMainWindow(const char* windowName, EBackend eBackEnd, bool transparent)
{
	int backendFlag = 0;
	switch (eBackEnd)
	{
	case EBackend::kOpenGL:
		backendFlag = SDL_WINDOW_OPENGL;
		break;
	case EBackend::kVulkan:
		backendFlag = SDL_WINDOW_VULKAN;
		break;
	default:
		break;
	}

	m_window.reset(::SDL_CreateWindow(
		windowName, 960, 640,
		backendFlag | (transparent ? SDL_WINDOW_TRANSPARENT : 0))
	);
	if (m_window == nullptr)return;

	m_windowStyle.isTransparent = transparent;

	m_renderer.reset(::SDL_CreateRenderer(m_window.get(), nullptr));
	if (m_renderer == nullptr)return;

	::SDL_SetRenderVSync(m_renderer.get(), 1);
	::SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 0);

	m_scenePlayer = std::make_unique<CKoihimeTaisenScenePlayer>(m_window.get(), m_renderer.get());
	m_scenePlayer->onDpiChange(::SDL_GetWindowDisplayScale(m_window.get()));

	m_helpTextDrawer = std::make_unique<CSdlTextDrawer>(m_renderer.get());
}

SDL_Window* CSdlMainWindow::getWindow() const noexcept
{
	return m_window.get();
}

SDL_Renderer* CSdlMainWindow::getRenderer() const noexcept
{
	return m_renderer.get();
}

bool CSdlMainWindow::loadFont(const char* fontFilePath, bool bold, bool italic, float fontSize, int thickness)
{
	if (m_scenePlayer == nullptr)return false;

	m_helpTextDrawer->setFont(fontFilePath, false, false, fontSize / 2, thickness / 2);
	updateHelpText();

	return m_scenePlayer->setFont(fontFilePath, bold, italic, fontSize, thickness);
}

int CSdlMainWindow::display()
{
	m_playerClock.restart();
	while (!m_windowState.toBeQuit)
	{
		SDL_Event event;
		while (::SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				m_windowState.toBeQuit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				handleKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
				handleKeyUp(event.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				handleMouseButtonDown(event.button);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				handleMouseButtonUp(event.button);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				handleMouseMotion(event.motion);
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				handleMouseWheel(event.wheel);
				break;
			case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				m_scenePlayer->onDpiChange(::SDL_GetWindowDisplayScale(m_window.get()));
				break;
			default:
				break;
			}
		}

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		float deltaTime = m_playerClock.getElapsedTime();
		m_scenePlayer->update(deltaTime);
		m_playerClock.restart();

		::SDL_RenderClear(m_renderer.get());

		if (m_playerTexture != nullptr)
		{
			::SDL_SetRenderTarget(m_renderer.get(), m_playerTexture.get());
			::SDL_RenderClear(m_renderer.get());
			m_scenePlayer->draw(0, static_cast<float>(m_windowStyle.lastMenuBarHeight));
			::SDL_SetRenderTarget(m_renderer.get(), nullptr);

			::SDL_RenderTexture(m_renderer.get(), m_playerTexture.get(), nullptr, nullptr);
		}

		if (m_windowState.toShowHelp)
		{
			float helpTextHeight = 0.f;
			m_helpTextDrawer->getTextTextureSize(nullptr, &helpTextHeight);

			int windowHeight = 0;
			::SDL_GetWindowSize(m_window.get(), nullptr, &windowHeight);

			m_helpTextDrawer->renderText(0.f, windowHeight - helpTextHeight);
		}

		imguiMenuBar();
		imguiPopupMenu();
		imguiSettingDialogue();

		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer.get());

		::SDL_RenderPresent(m_renderer.get());

		updateWindowPosition();
		checkFileDialogueResult();
	}

	return 0;
}

void CSdlMainWindow::handleKeyDown(const SDL_KeyboardEvent& keyBoardEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureKeyboard)return;

	switch (keyBoardEvent.scancode)
	{
	case SDL_SCANCODE_LEFT:
		m_scenePlayer->shiftScene(false);
		break;
	case SDL_SCANCODE_RIGHT:
		if (!m_scenePlayer->hasReachedLastScene())
		{
			m_scenePlayer->shiftScene(true);
		}
		break;
	default:
		break;
	}
}

void CSdlMainWindow::handleKeyUp(const SDL_KeyboardEvent& keyBoardEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureKeyboard)return;

	switch (keyBoardEvent.scancode)
	{
	case SDL_SCANCODE_C:
		toggleTextColour();
		break;
	case SDL_SCANCODE_H:
		m_windowState.toShowHelp ^= true;
		break;
	case SDL_SCANCODE_T:
		m_scenePlayer->setTextVisibility(!m_scenePlayer->isTextVisible());
		break;
	case SDL_SCANCODE_ESCAPE:
		m_windowState.toBeQuit = true;
		break;
	case SDL_SCANCODE_UP:
		if (!m_scriptFilePaths.empty())
		{
			if (--m_nScriptPathIndex >= m_scriptFilePaths.size())
			{
				m_nScriptPathIndex = m_scriptFilePaths.size() - 1;
			}

			loadScenario(m_scriptFilePaths[m_nScriptPathIndex]);
		}
		break;
	case SDL_SCANCODE_DOWN:
		if (!m_scriptFilePaths.empty())
		{
			if (++m_nScriptPathIndex >= m_scriptFilePaths.size())
			{
				m_nScriptPathIndex = 0;
			}

			loadScenario(m_scriptFilePaths[m_nScriptPathIndex]);
		}
		break;
	}
}

void CSdlMainWindow::handleMouseButtonDown(const SDL_MouseButtonEvent& mouseButtonEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse)return;

	switch (mouseButtonEvent.button)
	{
	case SDL_BUTTON_LEFT:
		::SDL_GetMouseState(&m_mouseState.lastMousePos.x, &m_mouseState.lastMousePos.y);
		m_mouseState.wasLeftPressed = true;
		break;
	}
}

void CSdlMainWindow::handleMouseButtonUp(const SDL_MouseButtonEvent& mouseButtonEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse)return;
	if (!m_scenePlayer->hasScenarioData())return;

	if (mouseButtonEvent.button == SDL_BUTTON_LEFT)
	{
		{
			if (!m_mouseState.wasLeftCombined)
			{
				Uint32 mouseButtonState = ::SDL_GetMouseState(nullptr, nullptr);
				/* Right-button pressed + left-click to start moving window; left-click again to stop moving. */
				if (m_windowState.isUnderWindowMove || (mouseButtonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)))
				{
					if (!m_windowState.isUnderWindowMove)
					{
						m_mouseState.wasRightCombined = true;
					}
					m_windowState.isUnderWindowMove ^= true;
				}
			}

			m_mouseState.wasLeftCombined = false;
			m_mouseState.wasLeftPressed = false;
		}
	}
	else if (mouseButtonEvent.button == SDL_BUTTON_MIDDLE)
	{
		Uint32 mouseButtonState = ::SDL_GetMouseState(nullptr, nullptr);
		if (mouseButtonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
		{
			/* Hide/show the border of window. */
			m_windowStyle.isBorderless ^= true;
			::SDL_SetWindowBordered(m_window.get(), !m_windowStyle.isBorderless);
			if (m_windowStyle.isBorderless)
			{
				SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_window.get());
				if (displayId == 0)return;

				/* Move to the origin of the monitor where the window is on. */
				SDL_Rect displayRect{};
				bool bRet = ::SDL_GetDisplayBounds(displayId, &displayRect);
				if (bRet)
				{
					::SDL_SetWindowPosition(m_window.get(), displayRect.x, displayRect.y);
				}
			}
			else
			{
				/* Move downwards by the height of title bar. */
				int topBorder = 0;
				::SDL_GetWindowBordersSize(m_window.get(), &topBorder, nullptr, nullptr, nullptr);
				SDL_Rect windowRect{};
				::SDL_GetWindowPosition(m_window.get(), &windowRect.x, &windowRect.y);
				::SDL_SetWindowPosition(m_window.get(), windowRect.x, windowRect.y + topBorder);
			}

			m_mouseState.wasRightCombined = true;

			resizeWindow();
		}
		else if(mouseButtonState == 0)
		{
			m_scenePlayer->resetScale();
			resizeWindow();
		}
	}
	else if (mouseButtonEvent.button == SDL_BUTTON_RIGHT)
	{
		if (!m_mouseState.wasRightCombined)
		{
			m_windowState.toShowPopupMenu = true;
		}

		m_mouseState.wasRightCombined = false;
	}
}

void CSdlMainWindow::handleMouseMotion(const SDL_MouseMotionEvent& mouseMotionEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse)return;

	SDL_FPoint mousePos{};
	Uint32 mouseButtonState = ::SDL_GetMouseState(&mousePos.x, &mousePos.y);
	if (mouseButtonState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
	{
		if (m_mouseState.wasLeftPressed)
		{
			int iX = static_cast<int>(m_mouseState.lastMousePos.x - mousePos.x);
			int iY = static_cast<int>(m_mouseState.lastMousePos.y - mousePos.y);
			m_scenePlayer->addOffset(iX, iY);
			m_mouseState.lastMousePos = mousePos;

			m_mouseState.wasLeftCombined = true;
		}
	}
}

void CSdlMainWindow::handleMouseWheel(const SDL_MouseWheelEvent& mouseMotionEvent)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse)return;

	const float scrollSign = (mouseMotionEvent.y < 0 ? 1.f : -1.f);
	Uint32 buttonState = ::SDL_GetMouseState(nullptr, nullptr);
	if (buttonState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
	{
		constexpr float kTimeScaleDelta = 0.05f;

		float timeScale = m_scenePlayer->getTimeScale() + kTimeScaleDelta * scrollSign;
		if (timeScale < 0.f)timeScale = 0.f;
		m_scenePlayer->setTimeScale(timeScale);

		m_mouseState.wasLeftCombined = true;
	}
	else if (buttonState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
	{
		m_scenePlayer->shiftScene(mouseMotionEvent.y < 0);

		m_mouseState.wasRightCombined = true;
	}
	else
	{
		m_scenePlayer->rescale(mouseMotionEvent.y < 0);

		int keyCount = 0;
		const bool* pKeyboardState = ::SDL_GetKeyboardState(&keyCount);
		if (keyCount > SDL_SCANCODE_LCTRL && pKeyboardState[SDL_SCANCODE_LCTRL] == false)
		{
			resizeWindow();
		}
	}
}

void CSdlMainWindow::menuOnOpenFile()
{
	/*
	* It is desirable to filter files by "char_*2.json" as is supported by Win32 API.
	* But SDL provides filtering only by extension, and returns error if a filter like this were passed.
	* So it is up to the user to select a script file whose name matches the expected pattern.
	*/
	SDL_DialogFileCallback fileDialogueCallback = [](void* userdata, const char* const* filelist, int filter)
		-> void
		{
			CSdlMainWindow* pThis = static_cast<CSdlMainWindow*>(userdata);
			if (pThis == nullptr)return;

			if (filelist == nullptr || *filelist == nullptr)
			{
				::SDL_Log(::SDL_GetError());
				return;
			}

			pThis->m_dialogueContext.selectedFilePath = filelist[0];
			::SDL_SetAtomicU32(&pThis->m_dialogueContext.toOpenScriptFile, 1U);
		};

	SDL_PropertiesID sdlPropertyId = ::SDL_CreateProperties();
	::SDL_SetStringProperty(sdlPropertyId, SDL_PROP_FILE_DIALOG_TITLE_STRING, "Select char_*2.json");

	static constexpr SDL_DialogFileFilter filters[] =
	{
		{
			.name = "script file",
			.pattern = "json"
		}
	};
	static constexpr size_t filterCount = sizeof(filters) / sizeof(SDL_DialogFileFilter);

	::SDL_SetPointerProperty(sdlPropertyId, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, m_window.get());
	::SDL_SetPointerProperty(sdlPropertyId, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, const_cast<SDL_DialogFileFilter*>(filters));
	::SDL_SetNumberProperty(sdlPropertyId, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, filterCount);
	::SDL_ShowFileDialogWithProperties(SDL_FileDialogType::SDL_FILEDIALOG_OPENFILE, fileDialogueCallback, this, sdlPropertyId);
	::SDL_DestroyProperties(sdlPropertyId);
}

void CSdlMainWindow::resizeWindow()
{
	if (!m_scenePlayer->hasScenarioData())return;

	SDL_Point sceneSize = m_scenePlayer->getSceneSize();

	SDL_DisplayID displayId = ::SDL_GetDisplayForWindow(m_window.get());
	if (displayId == 0)return;

	const SDL_DisplayMode* pDisplayMode = ::SDL_GetCurrentDisplayMode(displayId);
	if (pDisplayMode == nullptr)return;

	int windowWidth = (std::min)(sceneSize.x, pDisplayMode->w);
	int windowHeight = (std::min)(sceneSize.y + (m_windowStyle.isBorderless ? 0 : m_windowStyle.lastMenuBarHeight), pDisplayMode->h);

	int textureWidth = (std::min)(sceneSize.x, pDisplayMode->w);
	int textureHeight = (std::min)(sceneSize.y, pDisplayMode->h);

	::SDL_SetWindowSize(m_window.get(), windowWidth, windowHeight);

	m_playerTexture.reset(::SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureWidth, textureHeight));

	m_scenePlayer->onResize();

	updateHelpText();
}

void CSdlMainWindow::updateWindowPosition()
{
	if (m_windowState.isUnderWindowMove)
	{
		SDL_FPoint currentMousePos{};
		::SDL_GetGlobalMouseState(&currentMousePos.x, &currentMousePos.y);

		SDL_Rect windowRect{};
		::SDL_GetWindowPosition(m_window.get(), &windowRect.x, &windowRect.y);
		::SDL_GetWindowSize(m_window.get(), &windowRect.w, &windowRect.h);

		SDL_Point windowPosToBe{};
		windowPosToBe.x = static_cast<int>(currentMousePos.x - (windowRect.w / 2.f));
		windowPosToBe.y = static_cast<int>(currentMousePos.y - (windowRect.h / 2.f));

		::SDL_SetWindowPosition(m_window.get(), windowPosToBe.x, windowPosToBe.y);
	}
}

void CSdlMainWindow::checkFileDialogueResult()
{
	Uint32 toLoadFile = ::SDL_GetAtomicU32(&m_dialogueContext.toOpenScriptFile);
	if (toLoadFile)
	{
		::SDL_SetAtomicU32(&m_dialogueContext.toOpenScriptFile, 0U);
		/* Avoid clearing the filepath just in case. */
		const auto& selectedFilePath = m_dialogueContext.selectedFilePath;
		std::string_view fileName = path_utility::TruncateFilePath(selectedFilePath);
		if (!fileName.starts_with("char_") || !fileName.ends_with("2.json"))
		{
			::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
				"Selected file does not match the expected pattern.", m_window.get());
		}
		else
		{
			bool bRet = loadScenario(selectedFilePath);
			if (bRet)
			{
				m_scriptFilePaths.clear();
				m_nScriptPathIndex = 0;

				std::string_view parentDirectory = path_utility::ExtractParentPath(selectedFilePath);
				m_scriptFilePaths = sdl_filesystem_utility::CreateFilePaths(parentDirectory, s_ScriptFilePattern.data(), sdl_filesystem_utility::EPathType::File);
				if (!m_scriptFilePaths.empty())
				{
					const auto& iter = std::find(m_scriptFilePaths.begin(), m_scriptFilePaths.end(), selectedFilePath);
					if (iter != m_scriptFilePaths.cend())
					{
						m_nScriptPathIndex = std::distance(m_scriptFilePaths.begin(), iter);
					}
				}
			}
		}
	}
}

bool CSdlMainWindow::loadScenario(const std::string& scenarioFilePath)
{
	bool bRet = m_scenePlayer->loadScenario(scenarioFilePath);
	if (bRet)
	{
		std::string_view filename = path_utility::ExtractFileNameWithoutExtension(scenarioFilePath);
		char windowTitleBuffer[256]{};
		if (filename.length() < sizeof(windowTitleBuffer) - 1)
		{
			memcpy(windowTitleBuffer, filename.data(), filename.length());
			windowTitleBuffer[filename.length()] = '\0';
			::SDL_SetWindowTitle(m_window.get(), windowTitleBuffer);
		}

		resizeWindow();
		m_playerClock.restart();
	}

	return bRet;
}

void CSdlMainWindow::updateHelpText()
{
	int windowWidth = 0;
	::SDL_GetWindowSize(m_window.get(), &windowWidth, nullptr);

	static constexpr const char8_t help[] =
	{
		u8"[H] Hide/show help\n"
		u8"[T] Hide/show message\n"
		u8"[C] Toggle text colour\n"
		u8"[Scroll] Scale up/down\n"
		u8"[Ctrl + scroll] Zoom in/out\n"
		u8"[L-drag] Move view-point\n"
		u8"[L-pressed + scroll] Speed up/down the animation\n"
		u8"[M-click] Reset scale, animation speed, and view-point\n"
		u8"[R-click] Show context menu to jump scene\n"
		u8"[R-pressed + M-click] Hide/show the border of window\n"
		u8"[R-pressed + L-click] Move window\n"
		u8"[← | →; R-pressed + scroll] Rewind/fast-forward the message\n"
		u8"[↑ | ↓] Open the previous/next script\n"
	};
	m_helpTextDrawer->updateText(reinterpret_cast<const char*>(help), sizeof(help) - 1, windowWidth);
}

void CSdlMainWindow::toggleTextColour()
{
	m_scenePlayer->toggleTextColour();

	m_helpTextDrawer->toggleTextColour();
	updateHelpText();
}

void CSdlMainWindow::imguiMenuBar()
{
	int menuBarHeight = 0;
	if (!m_windowStyle.isBorderless)
	{
		if (ImGui::BeginMainMenuBar())
		{
			menuBarHeight = static_cast<int>(ImGui::GetWindowSize().y);
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					menuOnOpenFile();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Option"))
			{
				bool isTextVisible = m_scenePlayer->isTextVisible();
				bool isCutInVisible = m_scenePlayer->isCutInVisible();
				bool isToMoveCutIn = m_scenePlayer->isCutInMoveMode();
				if (ImGui::MenuItem("Show text", nullptr, &isTextVisible))
				{
					m_scenePlayer->setTextVisibility(isTextVisible);
				}
				if (ImGui::MenuItem("Show cut-in", nullptr, &isCutInVisible))
				{
					m_scenePlayer->setCutInVisibility(isCutInVisible);
				}
				if (ImGui::MenuItem("Move cut-in", nullptr, &isToMoveCutIn))
				{
					m_scenePlayer->setCutInMoveMode(isToMoveCutIn);
				}

				ImGui::MenuItem("Show help", nullptr, &m_windowState.toShowHelp);
				if (ImGui::MenuItem("Setting", nullptr, &m_windowState.toShowSettingDialogue));

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	m_windowStyle.lastMenuBarHeight = menuBarHeight;
}

void CSdlMainWindow::imguiPopupMenu()
{
	if (const auto& labelData = m_scenePlayer->getLabelData(); !labelData.empty())
	{
		if (ImGui::BeginPopup("Jump##ContextMenu"))
		{
			for (const auto& labelDatum : labelData)
			{
				if (ImGui::MenuItem(labelDatum.caption.data()))
				{
					m_scenePlayer->jumpScene(labelDatum.nSceneIndex);
				}
			}
			ImGui::EndPopup();
		}
	}

	if (m_windowState.toShowPopupMenu)
	{
		ImGui::OpenPopup("Jump##ContextMenu");
		m_windowState.toShowPopupMenu = false;
	}
}

void CSdlMainWindow::imguiSettingDialogue()
{
	/* ホイール回転で増減可能な浮動小数点数スライダ */
	const auto ScrollableSliderFloat = [](const char* label, float* v, float v_min, float v_max, float v_step, const char* format = "%.0f", ImGuiSliderFlags flags = 0)
		-> bool
		{
			bool result = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
			if (result)return result;
			else
			{
				ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
				if (ImGui::IsItemHovered())
				{
					float wheel = ImGui::GetIO().MouseWheel;
					if (wheel > 0.f && (*v + v_step) < v_max)
					{
						*v += v_step;
					}
					else if (wheel < 0.f && (*v - v_step) > v_min)
					{
						*v -= v_step;
					}

					return wheel != 0.f;
				}
			}

			return result;
		};

	/* ホイール回転で増減可能な整数スライダ */
	const auto ScrollableSliderInt = [](const char* label, int* v, int v_min, int v_max)
		-> bool
		{
			bool result = ImGui::SliderInt(label, v, v_min, v_max);
			if (result)return result;
			else
			{
				ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
				if (ImGui::IsItemHovered())
				{
					float wheel = ImGui::GetIO().MouseWheel;
					if (wheel > 0 && *v < v_max)
					{
						++(*v);
					}
					else if (wheel < 0 && *v > v_min)
					{
						--(*v);
					}

					return wheel != 0.f;
				}
			}

			return result;
		};

	if (m_windowState.toShowSettingDialogue)
	{
		ImGui::Begin("Setting", &m_windowState.toShowSettingDialogue, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::SeparatorText("Voice");
		CSdlAudioPlayer& voicePlayer = m_scenePlayer->getVoicePlayer();
		float voiceVolume = voicePlayer.getVolume();
		if (ScrollableSliderFloat("Volume", &voiceVolume, 0.f, 1.0f, 0.01f, "%.2f"))
		{
			voicePlayer.setVolume(voiceVolume);
		}

		ImGui::SeparatorText("Font");

		const setup::FontDatum& fontDatum = setup::GetFontDatum();
		static int fontSize = static_cast<int>(fontDatum.messageFontSize);
		static int thickness = static_cast<int>(fontDatum.thickness);
		static bool bold = fontDatum.bold;
		static bool italic = fontDatum.italic;

		ScrollableSliderInt("Size", &fontSize, 8, 64);
		ScrollableSliderInt("Thickness", &thickness, 0, 8);
		ImGui::Checkbox("Bold", &bold);
		ImGui::SameLine();
		ImGui::Checkbox("Italic", &italic);

		if (ImGui::Button("Apply##ApplyFontSetting"))
		{
			m_scenePlayer->setFont(fontDatum.filePath.data(), bold, italic, fontSize, thickness);
		}

		ImGui::End();
	}
}
