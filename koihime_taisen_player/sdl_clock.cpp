
#include "sdl_clock.h"

CSdlClock::CSdlClock()
{
	m_nFrequency = ::SDL_GetPerformanceFrequency();
	restart();
}

float CSdlClock::getElapsedTime()
{
	Uint64 nNow = getTicks();
	return (nNow - m_nLastCount) / static_cast<float>(m_nFrequency);
}

void CSdlClock::restart()
{
	m_nLastCount = getTicks();
}

Uint64 CSdlClock::getTicks()
{
	return ::SDL_GetPerformanceCounter();
}
