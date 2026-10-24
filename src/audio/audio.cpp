#include "audio.hpp"
#include <algorithm>

namespace slide
{
Audio::~Audio()
{
    running = false;
    if (worker.joinable())
        worker.join();
    if (device)
        waveOutClose(device);
}
void Audio::start()
{
    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 44100;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 4;
    format.nAvgBytesPerSec = 176400;
    if (waveOutOpen(&device, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
    {
        device = nullptr;
        return;
    }
    running = true;
    worker = std::thread(&Audio::run, this);
}
void Audio::set_bank(std::shared_ptr<const SoundBank> sounds)
{
    std::lock_guard<std::mutex> lock(mutex);
    requestedBank = std::move(sounds);
    pending.clear();
}
void Audio::play(SoundEvent event)
{
    std::lock_guard<std::mutex> lock(mutex);
    pending.push_back(event);
}
void Audio::run()
{
    constexpr size_t blockFrames = 1024;
    std::array<std::array<short, blockFrames * 2>, 4> samples{};
    std::array<WAVEHDR, 4> headers{};
    for (size_t i = 0; i < headers.size(); ++i)
    {
        headers[i].lpData = reinterpret_cast<LPSTR>(samples[i].data());
        headers[i].dwBufferLength = DWORD(samples[i].size() * sizeof(short));
        if (waveOutPrepareHeader(device, &headers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
            running = false;
    }
    struct Voice
    {
        Clip clip;
        size_t cursor = 0;
    };
    std::vector<Voice> voices;
    std::shared_ptr<const SoundBank> bank;
    size_t musicCursor = 0;
    while (running)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (bank != requestedBank)
            {
                bank = requestedBank;
                musicCursor = 0;
                voices.clear();
            }
            if (bank)
            {
                for (SoundEvent event : pending)
                {
                    Clip clip = bank->effects.at(size_t(event));
                    if (!clip)
                        continue;
                    if (voices.size() >= 16)
                        voices.erase(voices.begin());
                    voices.push_back({clip, 0});
                }
            }
            pending.clear();
        }
        for (size_t block = 0; block < headers.size() && running; ++block)
        {
            auto &header = headers[block];
            if (!(header.dwFlags & WHDR_PREPARED))
                continue;
            if ((header.dwFlags & WHDR_INQUEUE) && !(header.dwFlags & WHDR_DONE))
                continue;
            for (size_t sample = 0; sample < samples[block].size(); ++sample)
            {
                float value = 0;
                if (bank && bank->music)
                {
                    value = bank->music->samples[musicCursor] * bank->musicVolume;
                    musicCursor = (musicCursor + 1) % bank->music->samples.size();
                }
                for (Voice &voice : voices)
                    if (voice.cursor < voice.clip->samples.size())
                        value += voice.clip->samples[voice.cursor++] * bank->effectsVolume;
                samples[block][sample] = muted ? 0 : short(std::clamp(value, -1.f, 1.f) * 32767);
            }
            voices.erase(std::remove_if(voices.begin(), voices.end(), [](const Voice &voice)
                                        { return voice.cursor >= voice.clip->samples.size(); }),
                         voices.end());
            if (waveOutWrite(device, &header, sizeof(header)) != MMSYSERR_NOERROR)
                running = false;
        }
        Sleep(3);
    }
    waveOutReset(device);
    for (auto &header : headers)
        if (header.dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(device, &header, sizeof(header));
}
} // namespace slide
