#ifndef SDL_AUDIO_PLAYER_H_
#define SDL_AUDIO_PLAYER_H_

#include <memory>

#include <SDL3_mixer/SDL_mixer.h>

class CSdlAudioPlayer
{
public:
	CSdlAudioPlayer();
	~CSdlAudioPlayer() = default;

	bool load(const char* filePath, bool toPredecode = false, bool autoPlay = true);
	bool play();

	/// @brief Set loop count; default argument to loop infinitely
	bool setLoop(int loopCount = -1);
	bool isLooped();
	/// @return Remained loop count; -1 if infinite loop
	int getLoopCount();

	bool pause();
	bool isPaused();
	bool resume();

	/// @brief Stop playing; default argument to stop immediately
	bool end(Sint64 fadeOutFrame = 0);
	bool isEnded();

	bool setPlaybackRate(float playbackRate);
	float getPlaybackRate();

	/// @brief Set volume or amplification.
	/// @param volume 0.0f to be muted, 1.0f to be default volume; amplify if it were greater than 1.0f
	bool setVolume(float volume);
	float getVolume();

	bool setCurrentTimeInMilliSeconds(Sint64 time);
	Sint64 getCurrentTimeInMilliSeconds();
private:
	std::unique_ptr<MIX_Mixer, decltype(&::MIX_DestroyMixer)> m_mixer{ nullptr, ::MIX_DestroyMixer };
	std::unique_ptr<MIX_Track, decltype(&::MIX_DestroyTrack)> m_track{ nullptr, ::MIX_DestroyTrack };
	std::unique_ptr<MIX_Audio, decltype(&::MIX_DestroyAudio)> m_audio{ nullptr, ::MIX_DestroyAudio };
};

#endif // !SDL_AUDIO_PLAYER_H_
