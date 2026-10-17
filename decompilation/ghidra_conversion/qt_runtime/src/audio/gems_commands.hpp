#pragma once

// The ROM's game-level sound API (doc 10 §3, doc 26 §1) as typed calls over
// the frame-locked mixer. Every routine here mirrors one 68000 entry point:
//   play_sfx    0x530DE  gated by FFFC0A, startsong (op 0x10)
//   play_music  0x530EA  gated by FFFC08, stopsong(same id) then startsong
//   stop_sfx    0x530D2  gated by FFFC0A, stopsong (op 0x12)
//   stop_all    0x52FA0  op 0x16, ungated
//   attenuate   0x5315E  op 0x20 with d0; 0x53156 sends 0
//   fade_out    0x53126  op 0x20 0x14,0x28..0x78 three frames apart, then
//                        op 0x16 and op 0x20 0 (18 frames) [C]
// The ROM blocks inside the fade (jsr 0xC24 waits); the host is non-blocking,
// so operations requested while a fade runs are queued and replayed in order
// on the frame the fade completes.

#include "audio_mixer.hpp"

#include <cstdint>
#include <deque>
#include <functional>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline constexpr std::uint8_t kGemsFadeStep = 0x14U;       // 0x53130 [C]
inline constexpr std::uint8_t kGemsFadeSteps = 6U;         // d6 = 5, dbra [C]
// 0x53138 d5 = 2; 0xC24 loops bsr 0xBD2 (one VBlank) with dbra, so d5 + 1
// = 3 VBlanks per step [C]. The fade is 6 x 3 = 18 frames.
inline constexpr std::uint8_t kGemsFadeStepFrames = 3U;
inline constexpr std::uint8_t kGemsStartMenuDuck = 8U;     // 0xB62 [C]

class GemsCommandLayer {
public:
    explicit GemsCommandLayer(AudioMixer& mixer);

    void set_music_enabled(bool enabled) { music_enabled_ = enabled; } // FFFC08
    void set_sfx_enabled(bool enabled) { sfx_enabled_ = enabled; }     // FFFC0A
    bool music_enabled() const { return music_enabled_; }
    bool sfx_enabled() const { return sfx_enabled_; }

    void play_sfx(std::uint16_t id);
    void play_music(std::uint16_t id);
    void stop_sfx(std::uint16_t id);
    void stop_all();
    void attenuate(std::uint8_t level);
    void restore_volume() { attenuate(0U); }
    void fade_out();

    bool fading() const { return fade_frame_.has_value(); }

    // Run due fade steps for the current frame, then render it.
    void advance_frame(AudioSink* sink);

private:
    void run(std::function<void()> operation);
    void step_fade();

    AudioMixer& mixer_;
    bool music_enabled_ = true;
    bool sfx_enabled_ = true;
    std::optional<std::uint32_t> fade_frame_; // frames since the fade began
    std::deque<std::function<void()>> deferred_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
