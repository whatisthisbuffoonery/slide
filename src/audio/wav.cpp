#include "wav.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace slide
{
static uint16_t read_u16(const std::vector<unsigned char> &bytes, size_t offset)
{
    return uint16_t(bytes.at(offset)) | uint16_t(bytes.at(offset + 1) << 8);
}
static uint32_t read_u32(const std::vector<unsigned char> &bytes, size_t offset)
{
    return uint32_t(read_u16(bytes, offset)) | (uint32_t(read_u16(bytes, offset + 2)) << 16);
}
Clip load_wav(const std::filesystem::path &file)
{
    if (file.empty())
        return {};
    try
    {
        std::ifstream input(file, std::ios::binary | std::ios::ate);
        if (!input)
            throw std::runtime_error("cannot open WAV");
        auto size = input.tellg();
        if (size < 12 || size > 128 * 1024 * 1024)
            throw std::runtime_error("WAV must be between 12 bytes and 128 MiB");
        std::vector<unsigned char> bytes(static_cast<size_t>(size));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char *>(bytes.data()), size))
            throw std::runtime_error("cannot read WAV");
        if (std::memcmp(bytes.data(), "RIFF", 4) || std::memcmp(bytes.data() + 8, "WAVE", 4))
            throw std::runtime_error("expected a RIFF/WAVE file");
        size_t riffEnd = size_t(read_u32(bytes, 4)) + 8;
        if (riffEnd > bytes.size() || riffEnd < 12)
            throw std::runtime_error("truncated RIFF data");
        size_t format = 0, data = 0, dataSize = 0;
        for (size_t offset = 12; offset + 8 <= riffEnd;)
        {
            size_t chunkSize = read_u32(bytes, offset + 4);
            size_t start = offset + 8;
            if (chunkSize > riffEnd - start)
                throw std::runtime_error("truncated WAV chunk");
            if (!std::memcmp(bytes.data() + offset, "fmt ", 4))
            {
                if (chunkSize < 16)
                    throw std::runtime_error("short fmt chunk");
                format = start;
            }
            if (!std::memcmp(bytes.data() + offset, "data", 4))
            {
                data = start;
                dataSize = chunkSize;
            }
            offset = start + chunkSize + (chunkSize & 1);
        }
        if (!format || !data || !dataSize)
            throw std::runtime_error("missing format or sample data");
        unsigned channels = read_u16(bytes, format + 2);
        unsigned rate = read_u32(bytes, format + 4);
        if (read_u16(bytes, format) != 1 || read_u16(bytes, format + 14) != 16 ||
            (channels != 1 && channels != 2))
            throw std::runtime_error("export as 16-bit PCM WAV, mono or stereo");
        if (rate < 8000 || rate > 192000)
            throw std::runtime_error("unsupported sample rate");
        unsigned block = channels * 2;
        if (read_u16(bytes, format + 12) != block || dataSize % block)
            throw std::runtime_error("invalid PCM frame alignment");
        size_t frames = dataSize / block;
        size_t outputFrames = std::max<size_t>(1, frames * 44100ull / rate);
        if (outputFrames > 16 * 1024 * 1024)
            throw std::runtime_error("decoded track exceeds six minutes; use a shorter loop");
        auto result = std::make_shared<AudioClip>();
        result->samples.resize(outputFrames * 2);
        auto sample = [&](size_t frame, unsigned channel)
        {
            uint16_t bits =
                read_u16(bytes, data + frame * block + (channels == 1 ? 0 : channel * 2));
            int value = bits < 32768 ? int(bits) : int(bits) - 65536;
            return float(value) / 32768.f;
        };
        for (size_t frame = 0; frame < outputFrames; ++frame)
        {
            double source = double(frame) * rate / 44100.;
            size_t first = std::min(size_t(source), frames - 1);
            size_t second = std::min(first + 1, frames - 1);
            float fraction = float(source - double(first));
            for (unsigned channel = 0; channel < 2; ++channel)
                result->samples[frame * 2 + channel] =
                    sample(first, channel) * (1 - fraction) + sample(second, channel) * fraction;
        }
        return result;
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error(file.string() + ": " + error.what());
    }
}
std::shared_ptr<const SoundBank> load_sound_bank(const AudioFiles &files)
{
    auto bank = std::make_shared<SoundBank>();
    bank->music = load_wav(files.music);
    bank->effects = {load_wav(files.jump), load_wav(files.checkpoint), load_wav(files.defeat),
                     load_wav(files.victory)};
    bank->musicVolume = files.musicVolume;
    bank->effectsVolume = files.effectsVolume;
    return bank;
}
} // namespace slide
