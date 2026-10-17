#pragma once

// Host audio service: the window/FSR boundary over the resource pipeline
// (doc 26 §4). Ids resolve through assets/audio/audio_manifest.tsv (generated
// from enums.lua); the GemsCommandLayer applies the ROM entry-point
// semantics; the AudioMixer renders exactly one game frame per
// advance_frame() into the selected sink. Unknown ids are logged as
// operations but play nothing (no invented sound).

#include "audio_mixer.hpp"
#include "audio_resources.hpp"
#include "gems_commands.hpp"

#include <QObject>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

class AudioPlaybackService final : public QObject {
public:
    struct PlaybackStatus {
        bool mapped = false;
        bool source_ready = false;
        bool audio_available = false;
        bool playing = false;
        bool error = false;
        std::int64_t duration_ms = 0;
    };

    enum class Output : std::uint8_t {
        device, // QAudioSink; falls back to null when no device exists
        null,   // headless: no decode, deterministic voice state + event log
        wav,    // headless render to wav_path
    };

    // The resolver turns a catalog alias into an absolute file path. The
    // manifest itself is catalog alias "audio.manifest".
    using PathResolver = AudioResourceBank::PathResolver;

    explicit AudioPlaybackService(PathResolver resolver,
                                  Output output = Output::device,
                                  std::filesystem::path wav_path = {},
                                  QObject* parent = nullptr);
    ~AudioPlaybackService() override;

    // ROM entry points (gems_commands.hpp).
    void play_sfx(std::uint16_t id);    // 0x530DE
    void play_music(std::uint16_t id);  // 0x530EA
    void stop_sfx(std::uint16_t id);    // 0x530D2
    void stop_all();                    // 0x52FA0 (op 0x16)
    void fade_out();                    // 0x53126
    void attenuate(std::uint8_t level); // 0x5315E (0x53156 = level 0)
    void set_music_enabled(bool enabled);  // FFFC08
    void set_sfx_enabled(bool enabled);    // FFFC0A

    // Compatibility names used by the existing host callbacks.
    void play_event(std::uint16_t event) { play_sfx(event); }
    void stop_resource(std::uint16_t resource_id) { stop_sfx(resource_id); }
    void stop_all_channels() { stop_all(); }
    void play_alias(std::string_view alias);

    // One-shot latch for the two composite intro renders (0x5C/0x5D): the
    // asset already contains every ROM-cadenced start.
    void begin_source_sequence();
    void play_event_once_in_sequence(std::uint16_t event);

    // Render one game frame. Call exactly once per source frame.
    void advance_frame();

    std::string_view alias_for_event(std::uint16_t event);
    PlaybackStatus playback_status_for_event(std::uint16_t event);
    std::size_t playing_channel_count_for_event(std::uint16_t event);

    const std::vector<AudioEvent>& events();
    AudioMixer& mixer();
    GemsCommandLayer& commands();
    const std::string& manifest_error();
    void finish_output(); // flush the WAV header

private:
    void ensure_loaded();

    PathResolver resolver_;
    Output output_;
    std::filesystem::path wav_path_;
    bool loaded_ = false;
    std::string manifest_error_;
    std::unique_ptr<AudioResourceBank> bank_;
    std::unique_ptr<AudioMixer> mixer_;
    std::unique_ptr<GemsCommandLayer> commands_;
    std::unique_ptr<AudioSink> sink_;
    std::unordered_set<std::uint16_t> sequence_one_shot_events_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
