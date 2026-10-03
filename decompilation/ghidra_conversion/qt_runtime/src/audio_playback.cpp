#include "audio_playback.hpp"

#include <QSoundEffect>
#include <QString>
#include <QUrl>

namespace shadowrun::ghidra_conversion::qt_runtime {

AudioPlaybackService::~AudioPlaybackService() = default;

AudioPlaybackService::AudioPlaybackService(PathResolver resolver,
                                           QObject* parent)
    : QObject(parent), resolver_(std::move(resolver)) {
    constexpr std::size_t kChannelCount = 4U;
    channels_.reserve(kChannelCount);
    for (std::size_t index = 0; index < kChannelCount; ++index) {
        auto effect = std::make_unique<QSoundEffect>(this);
        effect->setVolume(0.6f);
        channels_.push_back(std::move(effect));
    }
    // ROM audio command ids observed in the converted service boundaries,
    // mapped to the extracted aliases whose names carry the same command
    // evidence (menu command 0x24, field effect 0x10, title stages 1/3).
    event_aliases_ = {
        {0x10U, "audio.field.effect-016"},
        {0x11U, "audio.intro.title.stage-1"},
        {0x12U, "audio.intro.title.stage-3"},
        {0x24U, "audio.menu.command-024"},
        {0x29U, "audio.menu.command-029"},
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
    play_alias(alias);
}

void AudioPlaybackService::play_alias(const std::string_view alias) {
    if (!resolver_) {
        return;
    }
    const auto path = resolver_(alias);
    if (path.empty() || !std::filesystem::is_regular_file(path)) {
        return;
    }
    auto& effect = acquire_channel();
    effect.stop();
    effect.setSource(
        QUrl::fromLocalFile(QString::fromStdWString(path.wstring())));
    effect.play();
}

QSoundEffect& AudioPlaybackService::acquire_channel() {
    auto& channel = channels_[next_channel_];
    next_channel_ = (next_channel_ + 1U) % channels_.size();
    return *channel;
}

void AudioPlaybackService::stop_effects() {
    for (const auto& channel : channels_) {
        channel->stop();
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
