#ifndef SPINE_PLAYER_H_
#define SPINE_PLAYER_H_

/* Base-type spine player regardless of rendering library. */

#include <string>
#include <vector>
#include <memory>

#include "sdl_spine.h"
#include "sdl_spine_loader.h"
using FPoint2 = SDL_FPoint;
using CSpineDrawable = CSdlSpineDrawable;
using CTextureLoader = CSdlTextureLoader;
namespace spine_loader = sdl_spine_loader;


class CSpinePlayer
{
public:
	CSpinePlayer();
	virtual ~CSpinePlayer();

	bool loadSpineFromFile(const std::vector<std::string>& atlasFilePaths, const std::vector<std::string>& skeletonFilePaths);
	bool loadSpineFromMemory(const std::vector<std::string>& atlasFileData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& SkeletonFileData);

	size_t getNumberOfSpines() const;
	bool hasSpineBeenLoaded() const;

	void update(float fDelta);

	/// @brief 速度・尺度・位置初期化
	void resetScale();

	/// @brief 尺度を考慮した位置座標の加減算
	void addOffset(int iX, int iY);

	void shiftAnimation();
	void shiftSkin();

	void setAnimationByIndex(size_t nIndex);
	void setAnimationByName(const char* animationName);
	void restartAnimation();

	void setSkinByIndex(size_t nIndex);
	void setSkinByName(const char* skinName);
	void setupSkin();

	void togglePma();
	void toggleBlendMode();
	void togglePause();
	void toggleVisibility();

	bool premultiplyAlpha(bool toBePremultiplied, size_t nDrawableIndex = 0);
	bool isAlphaPremultiplied(size_t nDrawableIndex = 0) const;

	bool forceBlendModeNormal(bool toForce, size_t nDrawableIndex = 0);
	bool isBlendModeNormalForced(size_t nDrawableIndex = 0) const;

	bool setPause(bool paused, size_t nDrawableIndex = 0);
	bool isPaused(size_t nDrawableIndex = 0);

	bool setVisibility(bool visible, size_t nDrawableIndex = 0);
	bool isVisible(size_t nDrawableIndex = 0);

	void setDrawOrder(bool toBeReversed);
	bool isDrawOrderReversed() const;

	const std::vector<std::string>& getSlotNames() const;
	const std::vector<std::string>& getSkinNames() const;
	const std::vector<std::string>& GetAnimationNames() const;

	/// @brief 現在再生している動作の名称を取得。空の場合nullptr
	const char* getCurrentAnimationName();
	/// @brief 現在再生されている動作の時間情報を取得
	/// @param fTrack 再生が始まってからの総経過時間
	/// @param fLast 再生区間に於ける、現在の再生位置
	/// @param fStart 再生区間の開始位置
	/// @param fEnd 再生区間の終了位置
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);
	/// @brief 再生位置の変更
	void setCurrentAnimationTime(float animationTime);
	/// @brief 動作の再生時間を取得
	float getAnimationDuration(const char* animationName);
	/// @brief 動作予約
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false);
	/// @brief 或る動作から別の或る動作への遷移時間を設定
	void mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime);
	/// @brief 全ての動作間の遷移時間を初期化。Spine4.0以前ではこの機能は無効
	/// @remark Spine4.0以前では代替手段としてmixAnimationsに0.fを渡す
	void clearMixedAnimation();

	/// @brief 現在設定されているスキンを上書きしてスキン合成。上書きなので再読み込みしないと元に戻せなくなります
	void mixSkins(const std::vector<std::string>& skinNames);

	void setSlotsToExclude(const std::vector<std::string>& slotNames);
	void setSlotExclusionCallback(bool (*pFunc)(const char*, size_t));
#if 0
	/// @return スロット名を見出し語、装着品名を値とした辞書
	std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments();
	/// @brief 現在の装着品を強制差し替え
	bool replaceAttachment(const char* slotName, const char* attachmentName);
#endif

	FPoint2 getBaseSize() const;
	void setBaseSize(float fWidth, float fHeight);
	void resetBaseSize();

	FPoint2 getOffset() const;
	void setOffset(float fWidth, float fHeight);

	float getSkeletonScale() const;
	void setSkeletonScale(float fScale);

	float getCanvasScale() const;
	void setCanvasScale(float fScale);

	float getTimeScale() const;
	void setTimeScale(float fTimeScale);
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	CTextureLoader m_textureLoader;
	std::vector<std::shared_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::unique_ptr<CSpineDrawable>> m_drawables;

	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	bool m_isDrawOrderReversed = false;

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	std::vector<std::string> m_slotNames;

	void clearDrawables();
	bool setupDrawer();

	void workOutDefaultSize();
	virtual void workOutDefaultScale() = 0;
	virtual void workOutDefaultOffset() = 0;

	void updatePosition();

	void clearAnimationTracks();
};

#endif // !SPINE_PLAYER_H_
