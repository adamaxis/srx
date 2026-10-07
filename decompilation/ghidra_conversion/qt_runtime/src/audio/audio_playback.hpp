#pragma once

#include <QObject>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// Host playback for extracted Windows audio assets. The converted service
// boundaries map source event ids to named FLAC resources. QMediaPlayer is
// used because the runtime assets are lossless FLAC; QSoundEffect rejects
// these files on the supported Qt Windows build. Unknown events stay
// observable through the presentation boundary log instead of receiving an
// invented sound.
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

    // The resolver turns a catalog alias into an absolute file path (the
    // conversion catalog owns the audio rows); a null result is a no-op.
    using PathResolver =
        std::function<std::filesystem::path(std::string_view alias)>;

    AudioPlaybackService(PathResolver resolver, QObject* parent = nullptr);
    ~AudioPlaybackService() override;

    // Play the extracted resource for a source audio event id. No-op (with
    // a logged boundary by the caller) when the event has no verified alias.
    void play_event(std::uint16_t event);

    // Stop every channel playing this source resource. This is distinct from
    // the numeric command id: a ROM command may start or stop the same id
    // depending on whether it uses the FF 10 or FF 12 service path.
    void stop_resource(std::uint16_t resource_id);

    // Reproduce a ROM FF 10 retrigger for the same table-2 resource, without
    // leaving overlapping host channels when the source starts it again.
    void play_event_restarting(std::uint16_t event);

    // Play a catalog alias directly (audio.menu.command-024 and friends).
    void play_alias(std::string_view alias);

    // ROM 0x53126 ends with FF 16; the Z80 handler clears every active sound
    // channel. Mirror that source handoff in the host playback pool.
    void stop_all_channels();

    std::string_view alias_for_event(std::uint16_t event) const;
    PlaybackStatus playback_status_for_event(std::uint16_t event) const;
    std::size_t playing_channel_count_for_event(std::uint16_t event) const;

private:
    struct PlaybackChannel;
    PlaybackChannel& acquire_channel();
    void start_alias(std::string_view alias,
                     std::optional<std::uint16_t> event);

    PathResolver resolver_;
    std::vector<std::unique_ptr<PlaybackChannel>> channels_;
    std::size_t next_channel_ = 0U;
    std::uint64_t request_sequence_ = 0U;
    std::unordered_map<std::uint16_t, std::string> event_aliases_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
