#pragma once

#include <QObject>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class QSoundEffect;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Host playback for the extracted Windows audio assets. The source audio
// event ids recorded by the converted service boundaries map to the named
// FLAC resources through the ROM's audio command ids. Only the mappings
// with source evidence in the alias names are wired here; unknown events
// stay observable through the presentation boundary log instead of being
// assigned an invented sound.
class AudioPlaybackService final : public QObject {
public:
    // The resolver turns a catalog alias into an absolute file path (the
    // conversion catalog owns the audio rows); a null result is a no-op.
    using PathResolver =
        std::function<std::filesystem::path(std::string_view alias)>;

    AudioPlaybackService(PathResolver resolver, QObject* parent = nullptr);
    ~AudioPlaybackService() override;

    // Play the extracted resource for a source audio event id. No-op (with
    // a logged boundary by the caller) when the event has no verified
    // alias.
    void play_event(std::uint16_t event);

    // Play a catalog alias directly (audio.menu.command-024 and friends).
    void play_alias(std::string_view alias);

    // Stop every active one-shot channel.
    void stop_effects();

    std::string_view alias_for_event(std::uint16_t event) const;

private:
    QSoundEffect& acquire_channel();

    PathResolver resolver_;
    std::vector<std::unique_ptr<QSoundEffect>> channels_;
    std::size_t next_channel_ = 0U;
    std::unordered_map<std::uint16_t, std::string> event_aliases_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
