#ifndef KOIHIME_TAISEN_SCENE_PLAYER_H_
#define KOIHIME_TAISEN_SCENE_PLAYER_H_

#include <memory>

#include "adv.h"
#include "sdl3-spine-cpp/sdl_spine_player.h"
#include "sdl_clock.h"
#include "sdl_text_drawer.h"
#include "sdl_audio_player.h"

#include <SDL3/SDL.h>

class CKoihimeTaisenScenePlayer
{
public:
	CKoihimeTaisenScenePlayer(SDL_Window* pWindow, SDL_Renderer* pRenderer);
	~CKoihimeTaisenScenePlayer() = default;

	bool setFont(const char* fontFilePath, bool bold = true, bool italic = true, float fontSize = CSdlTextDrawer::kFillSize, int thiciness = CSdlTextDrawer::kOutLineSize);

	/// @brief 台本の読み取りと記載データ取り込み
	bool loadScenario(const std::string& scenarioFilePath);
	bool hasScenarioData() const;

	/// @brief 経過時間更新
	void update(float fDelta);
	/// @brief 描画
	void draw(float textPosX = 0, float textPosY = 0);

	/// @brief 表示枠の大きさ取得
	SDL_Point getSceneSize() const;
	/// @brief 拡縮
	void rescale(bool upscale);
	/// @brief 表示範囲移動
	void addOffset(int iX, int iY);
	/// @brief 尺度・表示範囲・速度初期化
	void resetScale();

	/// @brief 時間尺度取得
	float getTimeScale() const;
	/// @brief 時間尺度設定
	void setTimeScale(float timeScale);

	/// @brief 文章送り・戻し
	void shiftScene(bool forward);
	/// @brief 最終場面是否
	bool hasReachedLastScene() const;

	void setTextVisibility(bool visible);
	bool isTextVisible() const;
	void toggleTextColour();

	void setCutInVisibility(bool visible);
	bool isCutInVisible() const;

	/// @brief カットイン表示位置を移動させるか、視点移動にするか
	void setCutInMoveMode(bool toMoveCutIn);
	bool isCutInMoveMode() const;

	/// @brief 静止画切り替わり場面の名称と並び番号取得
	const std::vector<adv::LabelDatum>& getLabelData() const;
	/// @brief 特定場面に遷移
	bool jumpScene(size_t nSceneIndex);

	/// @brief Pass the result of SDL_GetWindowDisplayScale().
	void onDpiChange(float dpiScale);
	void onResize();

	CSdlAudioPlayer& getVoicePlayer();

private:
	using SdlUniqueTexture = std::unique_ptr<SDL_Texture, decltype(&::SDL_DestroyTexture)>;

	static constexpr int kDefaultWidth = 1680;
	static constexpr int kDefaultHeight = 960;

	static constexpr float kScaleDelta = 0.025f;
	static constexpr float kMinScale = 0.15f;
	static constexpr float kDefaultZoom = 1.f;

	static constexpr float kDefaultTimeScale = 1.f;
	static constexpr int kDefaultFps = 40;

	float m_fDefaultScale = 1.f;

	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;

	int m_sceneWidth = kDefaultWidth;
	int m_sceneHeight = kDefaultHeight;
	float m_fScale = 1.f;
	SDL_FPoint m_fOffset{};

	float m_timeScale = 1.f;
	int m_fps = 40;
	float m_elapsedTime = 0.f;

	/// @brief 静止画・Spine描画先
	SdlUniqueTexture m_sceneTexture{ nullptr, ::SDL_DestroyTexture };

	/// @brief GPU上の静止画・パラパラ漫画データ
	struct ImageFrameDatum
	{
		std::vector<SdlUniqueTexture> textures;
		size_t textureIndex = 0;

		/// @brief 現在のコマ取得
		const SdlUniqueTexture* getCurrntFrameTexture()
		{
			if (textureIndex >= textures.size())
			{
				return nullptr;
			}

			return &textures[textureIndex];
		}
		/// @brief 1コマ進め・戻し
		void StepFrame(bool forward = true)
		{
			if (textures.empty())return;

			if (forward)
			{
				if (++textureIndex >= textures.size())
				{
					textureIndex = 0;
				}
			}
			else
			{
				if (--textureIndex >= textures.size())
				{
					textureIndex = textures.size() - 1;
				}
			}
		}
	};

	/// @brief カットイン再生管理器
	struct CutinManager
	{
		/* 増えることはないだろうから固定で */
		enum class CutinIndex : uint8_t
		{
			CutIn1 = 0,
			CutIn2,
			kMax,
			Unknown = static_cast<uint8_t>(-1U)
		};
		static constexpr size_t kCapacity = static_cast<size_t>(CutinIndex::kMax);
		static constexpr float kDefaultCutIn1TimeScale = 1.f;
		static constexpr float kDefaultCutIn2TimeScale = 2.f;

		std::unique_ptr<CSdlSpinePlayer> sdlSpinePlayers[kCapacity];
		size_t lastAnimationIndices[kCapacity]{};

		static uint8_t NameToIndex(std::string_view s)
		{
			if (s.ends_with("cutin1"))
			{
				return static_cast<uint8_t>(CutinIndex::CutIn1);
			}
			else if (s.ends_with("cutin2"))
			{
				return static_cast<uint8_t>(CutinIndex::CutIn2);
			}

			return static_cast<uint8_t>(-1U);
		}
	};

	std::vector<adv::TextDatum> m_textData;
	std::vector<adv::CutInDatum> m_cutInData;
	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;
	std::vector<adv::LabelDatum> m_labelData;

	std::vector<ImageFrameDatum> m_imageFrameData;
	CutinManager m_cutinManager;

	bool m_isCutInVisible = true;
	bool m_toMoveCutIn = false;

	std::string m_formattedText;
	CSdlClock m_textClock;
	CSdlAudioPlayer m_voicePlayer;

	std::unique_ptr<CSdlTextDrawer> m_textDrawer;

	void clearScenarioData();

	void prepareScene();
	void prepareText();

	void updateText();
	void checkTextClock();

	/// @brief ウィンドウが存在するモニタの解像度内に収まるよう尺度算出
	void workOutDefaultScale();
	/// @brief 現在表示している静画寸法取得
	void getCurrentImageSize(int* width, int* height);
	/// @brief 表示範囲限度に收める
	void adjustOffset();

	SDL_FPoint getRenderTargetSize();
};

#endif // !KOIHIME_TAISEN_SCENE_PLAYER_H_
