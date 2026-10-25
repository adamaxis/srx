#include "audio_playback.hpp"

#include <QtGlobal>

#include <fstream>

#include "qt_audio_sink.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

AudioPlaybackService::~AudioPlaybackService() {
    finish_output();
    // Debug aid: SRX_AUDIO_EVENT_LOG=<path> writes the mixer's command log
    // (frame, op, id) so A/V traces can line up song starts and fades.
    const auto log_path = qEnvironmentVariable("SRX_AUDIO_EVENT_LOG");
    if (!log_path.isEmpty() && mixer_) {
        std::ofstream log(log_path.toStdString());
        log << "frame\top\tid\n";
        for (const auto& event : mixer_->events()) {
            log << event.frame << '\t' << static_cast<int>(event.op) << '\t'
                << event.id << '\n';
        }
    }
}

AudioPlaybackService::AudioPlaybackService(PathResolver resolver,
                                           const Output output,
                                           std::filesystem::path wav_path,
                                           QObject* parent)
    : QObject(parent), resolver_(std::move(resolver)), output_(output),
      wav_path_(std::move(wav_path)) {}

void AudioPlaybackService::ensure_loaded() {
    if (loaded_) {
        return;
    }
    loaded_ = true;
    // SDL_AUDIODRIVER does not select Qt's QAudioSink device. Keep headless
    // QA independent of a host PulseAudio/PipeWire server as well.
    if (output_ == Output::device &&
        qEnvironmentVariableIntValue("SRX_HEADLESS") != 0) {
        output_ = Output::null;
    }
    bank_ = std::make_unique<AudioResourceBank>(resolver_);
    const auto manifest = resolver_ ? resolver_("audio.manifest")
                                    : std::filesystem::path{};
    if (manifest.empty() || !bank_->load_manifest(manifest)) {
        manifest_error_ = manifest.empty() ? "audio.manifest alias is missing"
                                           : bank_->error();
        bank_->load_manifest_text("id\tenum_name\tkind\talias\tloop_start\tloop_end\tpersistent\n");
    }
    if (output_ == Output::device) {
        auto device = std::make_unique<QtAudioSink>();
        if (device->ok()) {
            sink_ = std::move(device);
        } else {
            output_ = Output::null;
        }
    }
    if (output_ == Output::wav) {
        sink_ = std::make_unique<WavAudioSink>(wav_path_);
    } else if (output_ == Output::null) {
        sink_ = std::make_unique<NullAudioSink>();
    }
    mixer_ = std::make_unique<AudioMixer>(*bank_, output_ != Output::null);
    commands_ = std::make_unique<GemsCommandLayer>(*mixer_);
}

void AudioPlaybackService::play_sfx(const std::uint16_t id) {
    ensure_loaded();
    commands_->play_sfx(id);
}

void AudioPlaybackService::play_music(const std::uint16_t id) {
    ensure_loaded();
    commands_->play_music(id);
}

void AudioPlaybackService::stop_sfx(const std::uint16_t id) {
    ensure_loaded();
    commands_->stop_sfx(id);
}

void AudioPlaybackService::stop_all() {
    ensure_loaded();
    commands_->stop_all();
}

void AudioPlaybackService::fade_out() {
    ensure_loaded();
    commands_->fade_out();
}

void AudioPlaybackService::wait_frames(const std::uint16_t frames) {
    ensure_loaded();
    commands_->wait_frames(frames);
}

void AudioPlaybackService::attenuate(const std::uint8_t level) {
    ensure_loaded();
    commands_->attenuate(level);
}

void AudioPlaybackService::set_music_enabled(const bool enabled) {
    ensure_loaded();
    commands_->set_music_enabled(enabled);
}

void AudioPlaybackService::set_sfx_enabled(const bool enabled) {
    ensure_loaded();
    commands_->set_sfx_enabled(enabled);
}

void AudioPlaybackService::play_alias(const std::string_view alias) {
    ensure_loaded();
    for (const auto& [id, row] : bank_->rows()) {
        if (row.alias == alias) {
            commands_->play_sfx(id);
            return;
        }
    }
}

void AudioPlaybackService::begin_source_sequence() {
    sequence_one_shot_events_.clear();
}

void AudioPlaybackService::play_event_once_in_sequence(
    const std::uint16_t event) {
    if (sequence_one_shot_events_.insert(event).second) {
        play_sfx(event);
    }
}

void AudioPlaybackService::advance_frame() {
    ensure_loaded();
    commands_->advance_frame(sink_.get());
}

std::string_view AudioPlaybackService::alias_for_event(
    const std::uint16_t event) {
    ensure_loaded();
    const auto* row = bank_->row(event);
    return row == nullptr ? std::string_view{} : std::string_view(row->alias);
}

AudioPlaybackService::PlaybackStatus
AudioPlaybackService::playback_status_for_event(const std::uint16_t event) {
    ensure_loaded();
    PlaybackStatus status{};
    if (bank_->row(event) == nullptr) {
        return status;
    }
    status.mapped = true;
    const auto frames = bank_->frame_count(event);
    status.source_ready = frames.has_value();
    status.audio_available = frames.has_value() && *frames != 0U;
    status.error = !frames.has_value();
    status.playing = mixer_->playing(event);
    status.duration_ms = frames.has_value()
        ? static_cast<std::int64_t>(*frames * 1000U / kAudioSampleRate)
        : 0;
    return status;
}

std::size_t AudioPlaybackService::playing_channel_count_for_event(
    const std::uint16_t event) {
    ensure_loaded();
    return mixer_->playing_voice_count(event);
}

const std::vector<AudioEvent>& AudioPlaybackService::events() {
    ensure_loaded();
    return mixer_->events();
}

AudioMixer& AudioPlaybackService::mixer() {
    ensure_loaded();
    return *mixer_;
}

GemsCommandLayer& AudioPlaybackService::commands() {
    ensure_loaded();
    return *commands_;
}

const std::string& AudioPlaybackService::manifest_error() {
    ensure_loaded();
    return manifest_error_;
}

void AudioPlaybackService::finish_output() {
    if (auto* wav = dynamic_cast<WavAudioSink*>(sink_.get())) {
        wav->finish();
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
