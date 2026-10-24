#pragma once
#include "../platform/windows.hpp"
#include "../game/game.hpp"
#include "wav.hpp"
#include <mmsystem.h>
#include <atomic>
#include <mutex>
#include <thread>

namespace slide
{
// Owns the playback device and thread. It never reads mutable Game state.
class Audio
{
  public:
    ~Audio();
    void start();
    void set_bank(std::shared_ptr<const SoundBank> sounds);
    void play(SoundEvent event);
    bool available() const
    {
        return device != nullptr;
    }
    std::atomic<bool> muted{false};

  private:
    void run();
    HWAVEOUT device = nullptr;
    std::atomic<bool> running{false};
    std::thread worker;
    std::mutex mutex;
    std::shared_ptr<const SoundBank> requestedBank;
    std::vector<SoundEvent> pending;
};
} // namespace slide
