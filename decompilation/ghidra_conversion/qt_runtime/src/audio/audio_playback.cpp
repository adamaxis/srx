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
    std::uint16_t resource_id = 0U;
    std::uint64_t request_sequence = 0U;
    bool has_event = false;
    bool has_resource = false;

    PlaybackChannel() {
        output.setVolume(0.6f);
        player.setAudioOutput(&output);
    }
};

AudioPlaybackService::~AudioPlaybackService() = default;

AudioPlaybackService::AudioPlaybackService(PathResolver resolver,
                                           QObject* parent)
    : QObject(parent), resolver_(std::move(resolver)) {
    // Six host channels support overlapping independent effects. Repeated
    // ROM starts for the pre-title and logo cues are rendered offline as one
    // source-cadenced stereo sequence each; the first request starts that
    // sequence asset rather than layering complete sequence renders.
    constexpr std::size_t kChannelCount = 6U;
    channels_.reserve(kChannelCount);
    for (std::size_t index = 0; index < kChannelCount; ++index) {
        channels_.push_back(std::make_unique<PlaybackChannel>());
    }

    // ROM 0x59C72/0x59C92/0x59CB8 send stage ids 0x10/0x11/0x12 through the
    // gated 0x530EA command pair: FF 12 <id>, then FF 10 <id>. Independent
    // audio matches the extracted 0x10 and 0x12 tracks. A ROM-driven probe of
    // the exact 0x11 pair has no YM key-on, so it remains intentionally
    // unmapped; source track-stop/lifetime behavior is still open.
    event_aliases_ = {
        {0x00U, "audio.music.resource.000"},
        {0x10U, "audio.intro.title.stage-1"},
        {0x12U, "audio.intro.title.stage-3"},
        // Event 0x16 starts table-2 resource 22 on ordinary FF 10 paths.
        // The pre-title caller uses FF 12 with the same id to stop it.
        {0x16U, "audio.field.effect-016"},
        // ROM 0x1514E selects spawn sounds from 0x15318/0x15336 and submits
        // the selected event to 0x530DE. The regular action table selects
        // resources 28, 81, 30, and 29; its alternate table selects 63 or 74.
        {0x1CU, "audio.effects.resource.028"},
        {0x1DU, "audio.effects.resource.029"},
        {0x1EU, "audio.effects.resource.030"},
        {0x3FU, "audio.effects.resource.063"},
        {0x51U, "audio.effects.resource.081"},
        {0x24U, "audio.menu.command-024"},
        // ROM command 0x25 is table-2 index 37. The importer emits that
        // zero-based entry as audio.effects.resource.037.
        {0x25U, "audio.effects.resource.037"},
        {0x29U, "audio.menu.command-029"},
        {0x4AU, "audio.effects.resource.074"},
        // At the SEGA logo, queued command 0x5C starts table-2 resource 92.
        // The mapped asset contains all 60 starts at the ROM's two-frame
        // cadence, rendered together through the Z80 driver and Genesis chips.
        // The adjacent pre-title command 0x16 is FF 12 (stop resource 22),
        // despite that id's valid FF 10 start alias above.
        {0x5CU, "audio.intro.sega-logo-shock-sequence"},
        // ROM 0x1E4E submits 0x5D on alternating logo columns. The asset
        // contains all 19 resource-93 starts at the four-frame cadence.
        {0x5DU, "audio.intro.shadowrun-logo-reveal-sequence"},
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

void AudioPlaybackService::begin_source_sequence() {
    sequence_one_shot_events_.clear();
}

void AudioPlaybackService::play_event_once_in_sequence(
    const std::uint16_t event) {
    if (sequence_one_shot_events_.insert(event).second) {
        play_event(event);
    }
}

void AudioPlaybackService::stop_resource(const std::uint16_t resource_id) {
    for (const auto& channel : channels_) {
        if (channel->has_resource && channel->resource_id == resource_id) {
            channel->player.stop();
        }
    }
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
    channel.resource_id = event.value_or(0U);
    channel.has_resource = event.has_value();
    channel.request_sequence = ++request_sequence_;
    channel.player.play();
}

AudioPlaybackService::PlaybackChannel&
AudioPlaybackService::acquire_channel() {
    auto& channel = channels_[next_channel_];
    next_channel_ = (next_channel_ + 1U) % channels_.size();
    return *channel;
}

void AudioPlaybackService::stop_all_channels() {
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

std::size_t AudioPlaybackService::playing_channel_count_for_event(
    const std::uint16_t event) const {
    return static_cast<std::size_t>(std::count_if(
        channels_.cbegin(), channels_.cend(), [event](const auto& channel) {
            return channel->has_event && channel->event == event &&
                   channel->player.playbackState() ==
                       QMediaPlayer::PlayingState;
        }));
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
