#include "audio_playback.hpp"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QString>
#include <QUrl>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

struct AudioPlaybackService::PlaybackChannel {
    QAudioOutput output;
    QMediaPlayer player;
    std::filesystem::path source_path;
    std::uint16_t event = 0U;
    std::uint64_t request_sequence = 0U;
    bool has_event = false;

    PlaybackChannel() {
        output.setVolume(0.6f);
        player.setAudioOutput(&output);
    }
};

AudioPlaybackService::~AudioPlaybackService() = default;

AudioPlaybackService::AudioPlaybackService(PathResolver resolver,
                                           QObject* parent)
    : QObject(parent), resolver_(std::move(resolver)) {
    constexpr std::size_t kChannelCount = 4U;
    channels_.reserve(kChannelCount);
    for (std::size_t index = 0; index < kChannelCount; ++index) {
        channels_.push_back(std::make_unique<PlaybackChannel>());
    }

    // ROM audio command ids observed in the converted service boundaries,
    // mapped to extracted FLAC aliases with source evidence.
    event_aliases_ = {
        {0x10U, "audio.field.effect-016"},
        {0x11U, "audio.intro.title.stage-1"},
        {0x12U, "audio.intro.title.stage-3"},
        {0x24U, "audio.menu.command-024"},
        // ROM command 0x25 is table-2 index 37. The importer emits that
        // zero-based entry as audio.effects.resource.037.
        {0x25U, "audio.effects.resource.037"},
        {0x29U, "audio.menu.command-029"},
        // The maglock unlock routine sends command 0x4B (table-2 resource 75).
        {0x4BU, "audio.effects.resource.075"},
        // 0x2C/resource 44 is a verified silent table slot and has no asset.
        // 0x30/resource 48 remains unmapped until its source stop lifetime is
        // understood.
    };
}

std::string_view AudioPlaybackService::alias_for_event(
    const std::uint16_t event) const {
    const auto found = event_aliases_.find(event);
    return found == event_aliases_.end() ? std::string_view{}
                                         : std::string_view(found->second);
}

void AudioPlaybackService::play_event(const std::uint16_t event) {
    const auto alias = alias_for_event(event);
    if (alias.empty()) {
        return;
    }
    start_alias(alias, event);
}

void AudioPlaybackService::play_alias(const std::string_view alias) {
    start_alias(alias, std::nullopt);
}

void AudioPlaybackService::start_alias(
    const std::string_view alias,
    const std::optional<std::uint16_t> event) {
    if (!resolver_) {
        return;
    }
    const auto path = resolver_(alias);
    if (path.empty() || !std::filesystem::is_regular_file(path)) {
        return;
    }

    auto& channel = acquire_channel();
    channel.player.stop();
    if (channel.source_path != path ||
        channel.player.error() != QMediaPlayer::NoError) {
        channel.source_path = path;
        channel.player.setSource(
            QUrl::fromLocalFile(QString::fromStdWString(path.wstring())));
    }
    channel.player.setPosition(0);
    channel.event = event.value_or(0U);
    channel.has_event = event.has_value();
    channel.request_sequence = ++request_sequence_;
    channel.player.play();
}

AudioPlaybackService::PlaybackChannel&
AudioPlaybackService::acquire_channel() {
    auto& channel = channels_[next_channel_];
    next_channel_ = (next_channel_ + 1U) % channels_.size();
    return *channel;
}

void AudioPlaybackService::stop_effects() {
    for (const auto& channel : channels_) {
        channel->player.stop();
    }
}

AudioPlaybackService::PlaybackStatus
AudioPlaybackService::playback_status_for_event(
    const std::uint16_t event) const {
    const auto latest = std::max_element(
        channels_.cbegin(), channels_.cend(),
        [event](const auto& left, const auto& right) {
            const bool left_matches = left->has_event && left->event == event;
            const bool right_matches = right->has_event && right->event == event;
            if (left_matches != right_matches) {
                return !left_matches;
            }
            return left->request_sequence < right->request_sequence;
        });
    if (latest == channels_.cend() || !(*latest)->has_event ||
        (*latest)->event != event) {
        return {};
    }

    const auto& player = (*latest)->player;
    const auto status = player.mediaStatus();
    const bool source_ready =
        status == QMediaPlayer::LoadedMedia ||
        status == QMediaPlayer::BufferedMedia ||
        status == QMediaPlayer::StalledMedia ||
        status == QMediaPlayer::EndOfMedia;
    return PlaybackStatus{
        true,
        source_ready,
        player.hasAudio(),
        player.playbackState() == QMediaPlayer::PlayingState,
        player.error() != QMediaPlayer::NoError ||
            status == QMediaPlayer::InvalidMedia,
        static_cast<std::int64_t>(player.duration())};
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
