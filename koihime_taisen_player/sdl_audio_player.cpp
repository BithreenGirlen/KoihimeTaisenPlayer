

#include "sdl_audio_player.h"

CSdlAudioPlayer::CSdlAudioPlayer()
{
	m_mixer.reset(::MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr));
	m_track.reset(::MIX_CreateTrack(m_mixer.get()));
}

bool CSdlAudioPlayer::load(const char* filePath, bool toPredecode, bool autoPlay)
{
	if (m_mixer == nullptr)return false;

	m_audio.reset(::MIX_LoadAudio(m_mixer.get(), filePath, toPredecode));
	if (autoPlay) return play();

	return m_audio != nullptr;
}

bool CSdlAudioPlayer::play()
{
	if (m_track == nullptr || m_audio == nullptr)return false;

	::MIX_SetTrackAudio(m_track.get(), m_audio.get());

	return ::MIX_PlayTrack(m_track.get(), 0);
}

bool CSdlAudioPlayer::setLoop(int loopCount)
{
	return ::MIX_SetTrackLoops(m_track.get(), loopCount);
}

bool CSdlAudioPlayer::isLooped()
{
	return getLoopCount() != 0;
}

int CSdlAudioPlayer::getLoopCount()
{
	return ::MIX_GetTrackLoops(m_track.get());
}

bool CSdlAudioPlayer::pause()
{
	return ::MIX_PauseTrack(m_track.get());
}

bool CSdlAudioPlayer::isPaused()
{
	return ::MIX_TrackPaused(m_track.get());
}

bool CSdlAudioPlayer::resume()
{
	return ::MIX_ResumeTrack(m_track.get());
}

bool CSdlAudioPlayer::end(Sint64 fadeOutFrame)
{
	return ::MIX_StopTrack(m_track.get(), fadeOutFrame);
}

bool CSdlAudioPlayer::isEnded()
{
	return !::MIX_TrackPlaying(m_track.get());
}

bool CSdlAudioPlayer::setPlaybackRate(float playbackRate)
{
	return ::MIX_SetTrackFrequencyRatio(m_track.get(), playbackRate);
}

float CSdlAudioPlayer::getPlaybackRate()
{
	return ::MIX_GetTrackFrequencyRatio(m_track.get());
}

bool CSdlAudioPlayer::setVolume(float volume)
{
	return ::MIX_SetTrackGain(m_track.get(), volume);
}
float CSdlAudioPlayer::getVolume()
{
	return ::MIX_GetTrackGain(m_track.get());
}

bool CSdlAudioPlayer::setCurrentTimeInMilliSeconds(Sint64 time)
{
	Sint64 frames = ::MIX_TrackMSToFrames(m_track.get(), time);

	return ::MIX_SetTrackPlaybackPosition(m_track.get(), frames);
}

Sint64 CSdlAudioPlayer::getCurrentTimeInMilliSeconds()
{
	Sint64 frames = ::MIX_GetTrackPlaybackPosition(m_track.get());

	return ::MIX_TrackFramesToMS(m_track.get(), frames);
}
