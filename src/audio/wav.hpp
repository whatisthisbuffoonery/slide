#pragma once
#include "../content/course.hpp"
#include <array>
#include <memory>
#include <vector>

namespace slide
{
struct AudioClip
{
    // Interleaved stereo floating-point samples, resampled to 44.1 kHz at load time.
    std::vector<float> samples;
};
using Clip = std::shared_ptr<const AudioClip>;
struct SoundBank
{
    Clip music;
    std::array<Clip, 4> effects;
    float musicVolume = 0.25f;
    float effectsVolume = 0.75f;
};
Clip load_wav(const std::filesystem::path &file);
std::shared_ptr<const SoundBank> load_sound_bank(const AudioFiles &files);
} // namespace slide
