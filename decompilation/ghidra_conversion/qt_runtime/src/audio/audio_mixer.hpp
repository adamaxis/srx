#pragma once

// Deterministic, frame-locked software mixer (doc 26 §4.3). The game thread
// issues GEMS-level operations and calls advance_frame() once per source
// frame; each frame renders exactly kAudioSamplesPerFrame stereo samples.
// The timeline is a pure function of (frame, operation list), so headless
// runs are reproducible and every operation is logged as (frame, op, id).

#include "audio_resources.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline constexpr std::uint32_t kAudioSampleRate = 44100U;
// 44,100 / 60. The NTSC VBlank is 59.92 Hz [C]; the host frame clock is the
// game's 60 Hz pump, so one game frame owns exactly 735 output samples.
inline constexpr std::size_t kAudioSamplesPerFrame = 735U;
inline constexpr std::size_t kAudioSfxVoices = 8U;
// Key-off release approximation for GEMS stopsong (doc 26 §4.4) [H]: 30 ms.
inline constexpr std::size_t kAudioReleaseSamples = 1323U;
// Op 0x20 level at or above which output is silent [H mapping].
inline constexpr std::uint8_t kAudioAttenuationSilent = 0x7FU;

// GEMS ops as sent by the ROM (FF op args via 0x52E96) [C].
enum class AudioOp : std::uint8_t {
    start = 0x10,
    stop = 0x12,
    stop_all = 0x16,
    attenuate = 0x20,
};

struct AudioEvent {
    std::uint64_t frame = 0U;
    AudioOp op = AudioOp::start;
    std::uint16_t id = 0U;   // sequence id (start/stop) or level (attenuate)
    bool operator==(const AudioEvent& other) const {
        return frame == other.frame && op == other.op && id == other.id;
    }
    bool operator!=(const AudioEvent& other) const { return !(*this == other); }
};

class AudioSink {
public:
    virtual ~AudioSink() = default;
    // False: the mixer advances voice positions without decoding or mixing.
    virtual bool wants_pcm() const { return true; }
    virtual void submit(const std::int16_t* stereo, std::size_t frames) = 0;
};

// Headless: discards samples. The mixer still advances deterministically.
class NullAudioSink final : public AudioSink {
public:
    bool wants_pcm() const override { return false; }
    void submit(const std::int16_t*, std::size_t frames) override {
        frames_ += frames;
    }
    std::uint64_t frames() const { return frames_; }

private:
    std::uint64_t frames_ = 0U;
};

// Renders the session to a 16-bit stereo 44.1 kHz WAV file.
class WavAudioSink final : public AudioSink {
public:
    explicit WavAudioSink(const std::filesystem::path& path);
    ~WavAudioSink() override;
    bool ok() const { return static_cast<bool>(file_); }
    void submit(const std::int16_t* stereo, std::size_t frames) override;
    void finish();

private:
    std::ofstream file_;
    std::uint64_t frames_ = 0U;
    bool finished_ = false;
};

class AudioMixer {
public:
    // render_pcm=false (headless null sink): voices advance by length only;
    // no FLAC is decoded.
    AudioMixer(AudioResourceBank& bank, bool render_pcm = true);

    // Raw driver ops. Gating (FFFC08/FFFC0A) and the fade live in
    // GemsCommandLayer; these always take effect at the current frame.
    void start(std::uint16_t id);
    void stop(std::uint16_t id);
    void stop_all();
    void attenuate(std::uint8_t level);

    // Render one game frame (735 samples) into sink (may be null) and step
    // the frame counter.
    void advance_frame(AudioSink* sink);

    std::uint64_t frame() const { return frame_; }
    std::uint8_t attenuation() const { return attenuation_; }
    const std::vector<AudioEvent>& events() const { return events_; }
    void clear_events() { events_.clear(); }

    bool playing(std::uint16_t id) const;
    std::size_t playing_voice_count(std::uint16_t id) const;
    std::optional<std::uint16_t> music_id() const;
    std::size_t active_sfx_voices() const;
    // Sample-frame position of the newest voice playing id.
    std::optional<std::uint64_t> position(std::uint16_t id) const;

private:
    struct Voice {
        bool active = false;
        std::uint16_t id = 0U;
        std::shared_ptr<const AudioPcm> pcm;
        std::uint64_t length = 0U;
        std::uint64_t position = 0U;
        std::optional<std::uint64_t> loop_start;
        std::optional<std::uint64_t> loop_end;
        std::size_t release_left = 0U; // nonzero: releasing
        std::uint64_t started = 0U;    // start order, for stealing
    };

    bool prepare(Voice& voice, std::uint16_t id);
    void advance_voice(Voice& voice, std::size_t samples, std::int32_t* mix);
    void release(Voice& voice);

    AudioResourceBank& bank_;
    Voice music_{};
    std::array<Voice, kAudioSfxVoices> sfx_{};
    std::uint64_t frame_ = 0U;
    std::uint64_t start_counter_ = 0U;
    std::uint8_t attenuation_ = 0U;
    bool render_pcm_ = true;
    std::vector<AudioEvent> events_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
