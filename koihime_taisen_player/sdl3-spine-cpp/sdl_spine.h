#ifndef SDL_SPINE_H_
#define SDL_SPINE_H_

#include <spine/spine.h>
#include <SDL3/SDL.h>

class CSdlSpineDrawable
{
public:
	CSdlSpineDrawable(spine::SkeletonData* pSkeletonData);
	~CSdlSpineDrawable();

	spine::Skeleton* skeleton() const;
	spine::AnimationState* animationState() const;

	/// @brief 乗算済みアルファ適用有無。Spine 3.8より古い場合のみ有効で、4.0からはAtlasPageの同値を参照するため手動変更不可
	void premultiplyAlpha(bool toPremultiply);
	bool isAlphaPremultiplied() const;

	/// @brief スロット指定の混色法を無視して通常混色法を適用するか否か
	void forceBlendModeNormal(bool toForce);
	bool isBlendModeNormalForced() const;

	/// @brief 時間更新を行うか否か
	void setPause(bool paused) noexcept;
	bool isPaused() const noexcept;

	/// @brief 描画を行うか否か
	void setVisibility(bool visible) noexcept;
	bool isVisible() const noexcept;

	/// @brief 時間の加算並びワールド座標の更新
	/// @param fDelta 加算すべき時間(秒単位)
	/// @remark 停止中や0.0fの時間加算であっても座標更新は行う
	void update(float fDelta);
	void draw(float fScale, float fOffsetX = 0.f, float fOffsetY = 0.f);

	/// @brief 描画対象から除外するスロットを設定
	/// @remark 登録関数による判定の方が優先
	void setLeaveOutList(spine::Vector<spine::String>& list);
	/// @brief 描画対象から除外するか否か判定する関数を登録。nullptrを渡すことで登録解除
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t));

	/// @brief 全体の境界矩形を算出
	SDL_FRect getBoundingBox() const;
	/// @brief スロットの境界矩形を算出
	SDL_FRect getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
private:
	bool m_isAlphaPremultiplied = true;
	bool m_toForceBlendModeNormal = false;
	bool m_isVisible = true;
	bool m_isPaused = false;

	spine::Skeleton* m_skeleton = nullptr;
	spine::AnimationState* m_animationState = nullptr;

	spine::SkeletonClipping m_clipper;

	spine::Vector<SDL_Vertex> m_sdlVertices;
	spine::Vector<int> m_sdlIndices;
	spine::Vector<float> m_worldVertices;
	spine::Vector<unsigned short> m_quadIndices;

	spine::Vector<spine::String> m_leaveOutList;

	bool isSlotToBeLeftOut(const spine::String& slotName);
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;
};

class CSdlTextureLoader : public spine::TextureLoader
{
public:
	CSdlTextureLoader() {};
	virtual ~CSdlTextureLoader() {};

	void setRenderer(SDL_Renderer* pSdlRenderer);

	void load(spine::AtlasPage& atlasPage, const spine::String& path) override;
	void unload(void* texture) override;
private:
	SDL_Renderer* m_pSdlRenderer = nullptr;
};

#endif //!SDL_SPINE_H_
