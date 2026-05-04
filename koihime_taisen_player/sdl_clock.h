#ifndef SDL_CLOCK_H_
#define SDL_CLOCK_H_

#include <SDL3/SDL_timer.h>

/// @brief 経過時間測定
class CSdlClock
{
public:
    CSdlClock();
    ~CSdlClock() = default;

    /// @brief 秒単位の経過時間を取得
    float getElapsedTime();
    /// @brief 再計測開始
    void restart();
private:
    Uint64 m_nFrequency = 1;
    Uint64 m_nLastCount = 0;

    Uint64 getTicks();
};
#endif // !SDL_CLOCK_H_

