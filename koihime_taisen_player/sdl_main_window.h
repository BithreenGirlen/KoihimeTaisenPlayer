#ifndef SDL_MAIN_WINDOW_H_
#define SDL_MAIN_WINDOW_H_

#include <memory>

#include "sdl_clock.h"
#include "koihime_taisen_scene_player.h"

class CSdlMainWindow
{
public:
	enum class EBackend
	{
		kUnspecified = 0,
		kOpenGL,
		kVulkan
	};
	CSdlMainWindow(const char* windowName, EBackend backEnd = EBackend::kUnspecified, bool transparent = false);
	~CSdlMainWindow() = default;

	SDL_Window* getWindow() const noexcept;
	SDL_Renderer* getRenderer() const noexcept;

	bool loadFont(const char* fontFilePath, bool bold = true, bool italic = true, float fontSize = CSdlTextDrawer::kFillSize, int thickness = CSdlTextDrawer::kOutLineSize);

	int display();
private:
	static constexpr std::string_view s_ScriptFilePattern = "char_*2.json";
	struct WindowStyle
	{
		bool isBorderless = false;
		bool isTransparent = false;

		/// @brief 直前のメニュー欄表示処理時のメニュー欄の高さ
		int lastMenuBarHeight = 0;
	};
	struct WindowState
	{
		/// @brief 終了是否
		bool toBeQuit = false;
		bool isUnderWindowMove = false;
		bool toShowPopupMenu = false;
		bool toShowSettingDialogue = false;
		bool toShowHelp = false;
	};
	struct MouseState
	{
		bool wasLeftPressed = false;
		/// @brief 左釦押下を要する操作を行ったか
		bool wasLeftCombined = false;
		/// @brief 右釦押下を要する操作を行ったか
		bool wasRightCombined = false;
		/// @brief 最後に取得したクライアント基準のマウス位置
		SDL_FPoint lastMousePos{};
	};

	struct DialogueContext
	{
		/// @brief 選択されたファイルの読み込み処理を行うか
		SDL_AtomicU32 toOpenScriptFile{};
		std::string selectedFilePath;
	};

	WindowStyle m_windowStyle;
	WindowState m_windowState;
	MouseState m_mouseState;
	DialogueContext m_dialogueContext;

	std::unique_ptr<SDL_Window, decltype(&::SDL_DestroyWindow)> m_window{ nullptr, ::SDL_DestroyWindow };
	std::unique_ptr<SDL_Renderer, decltype(&::SDL_DestroyRenderer)> m_renderer{ nullptr, ::SDL_DestroyRenderer };

	CSdlClock m_playerClock;
	std::unique_ptr<CKoihimeTaisenScenePlayer> m_scenePlayer;
	std::unique_ptr<SDL_Texture, decltype(&::SDL_DestroyTexture)> m_playerTexture{ nullptr, ::SDL_DestroyTexture };

	std::vector<std::string> m_scriptFilePaths;
	size_t m_nScriptPathIndex = 0;

	std::unique_ptr<CSdlTextDrawer> m_helpTextDrawer;

	void handleKeyDown(SDL_KeyboardEvent keyBoardEvent);
	void handleKeyUp(SDL_KeyboardEvent keyBoardEvent);
	void handleMouseButtonDown(SDL_MouseButtonEvent mouseButtonEvent);
	void handleMouseButtonUp(SDL_MouseButtonEvent mouseButtonEvent);
	void handleMouseMotion(SDL_MouseMotionEvent mouseMotionEvent);
	void handleMouseWheel(SDL_MouseWheelEvent mouseMotionEvent);

	/// @brief Open non-blocking file-select-dialogue
	void menuOnOpenFile();

	void resizeWindow();
	/// @brief Move borderless window
	void updateWindowPosition();

	/// @brief Read the result of file dialogue which is written by another thread
	void checkFileDialogueResult();
	bool loadScenario(const std::string& scenarioFilePath);

	void updateHelpText();
	void toggleTextColour();

	void imguiMenuBar();
	void imguiPopupMenu();
	void imguiSettingDialogue();
};
#endif // !SDL_MAIN_WINDOW_H_
