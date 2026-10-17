#include "audio_mixer.hpp"

#include <algorithm>
#include <cstring>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

void write_u32(std::ofstream& file, const std::uint32_t value) {
    const char bytes[4] = {static_cast<char>(value & 0xFFU),
                           static_cast<char>((value >> 8U) & 0xFFU),
                           static_cast<char>((value >> 16U) & 0xFFU),
                           static_cast<char>((value >> 24U) & 0xFFU)};
    file.write(bytes, 4);
}

void write_u16(std::ofstream& file, const std::uint16_t value) {
    const char bytes[2] = {static_cast<char>(value & 0xFFU),
                           static_cast<char>((value >> 8U) & 0xFFU)};
    file.write(bytes, 2);
}

void write_wav_header(std::ofstream& file, const std::uint64_t frames) {
    const auto data_bytes = static_cast<std::uint32_t>(frames * 4U);
    file.write("RIFF", 4);
    write_u32(file, 36U + data_bytes);
    file.write("WAVEfmt ", 8);
    write_u32(file, 16U);
    write_u16(file, 1U);                    // PCM
    write_u16(file, 2U);                    // stereo
    write_u32(file, kAudioSampleRate);
    write_u32(file, kAudioSampleRate * 4U); // byte rate
    write_u16(file, 4U);                    // block align
    write_u16(file, 16U);                   // bits
    file.write("data", 4);
    write_u32(file, data_bytes);
}

} // namespace

WavAudioSink::WavAudioSink(const std::filesystem::path& path)
    : file_(path, std::ios::binary | std::ios::trunc) {
    if (file_) {
        write_wav_header(file_, 0U);
    }
}

WavAudioSink::~WavAudioSink() { finish(); }

void WavAudioSink::submit(const std::int16_t* stereo, const std::size_t frames) {
    if (!file_ || finished_) {
        return;
    }
    // Little-endian host assumed (x86/ARM); WAV is little-endian.
    file_.write(reinterpret_cast<const char*>(stereo),
                static_cast<std::streamsize>(frames * 4U));
    frames_ += frames;
}

void WavAudioSink::finish() {
    if (!file_ || finished_) {
        return;
    }
    finished_ = true;
    file_.seekp(0);
    write_wav_header(file_, frames_);
    file_.close();
}

AudioMixer::AudioMixer(AudioResourceBank& bank, const bool render_pcm)
    : bank_(bank), render_pcm_(render_pcm) {}

bool AudioMixer::prepare(Voice& voice, const std::uint16_t id) {
    const auto* row = bank_.row(id);
    if (row == nullptr) {
        return false;
    }
    std::shared_ptr<const AudioPcm> pcm;
    std::optional<std::uint64_t> length;
    if (render_pcm_) {
        pcm = bank_.pcm(id);
        if (pcm) {
            length = pcm->frames;
        }
    } else {
        length = bank_.frame_count(id);
    }
    if (!length.has_value() || *length == 0U) {
        return false;
    }
    voice = Voice{};
    voice.active = true;
    voice.id = id;
    voice.pcm = std::move(pcm);
    voice.length = *length;
    if (row->loop_start && row->loop_end && *row->loop_end <= *length) {
        voice.loop_start = row->loop_start;
        voice.loop_end = row->loop_end;
    }
    voice.started = ++start_counter_;
    return true;
}

void AudioMixer::start(const std::uint16_t id) {
    events_.push_back({frame_, AudioOp::start, id});
    const auto* row = bank_.row(id);
    if (row == nullptr) {
        return;
    }
    if (row->kind == AudioResourceKind::music) {
        // One music slot: a new song replaces the current one. In GEMS two
        // sequences could share the chips; the ROM always fades or stops
        // before changing songs on the paths traced so far [H].
        Voice next{};
        if (prepare(next, id)) {
            music_ = std::move(next);
        }
        return;
    }
    // SFX: retrigger a voice already playing this id (GEMS restarts a
    // sequence on startsong [H]); else a free voice; else steal the oldest.
    Voice* target = nullptr;
    for (auto& voice : sfx_) {
        if (voice.active && voice.id == id && voice.release_left == 0U) {
            target = &voice;
            break;
        }
    }
    if (target == nullptr) {
        for (auto& voice : sfx_) {
            if (!voice.active) {
                target = &voice;
                break;
            }
        }
    }
    if (target == nullptr) {
        target = &*std::min_element(
            sfx_.begin(), sfx_.end(), [](const Voice& a, const Voice& b) {
                return a.started < b.started;
            });
    }
    Voice next{};
    if (prepare(next, id)) {
        *target = std::move(next);
    }
}

void AudioMixer::release(Voice& voice) {
    if (voice.active && voice.release_left == 0U) {
        voice.release_left = kAudioReleaseSamples;
    }
}

void AudioMixer::stop(const std::uint16_t id) {
    events_.push_back({frame_, AudioOp::stop, id});
    if (music_.active && music_.id == id) {
        release(music_);
    }
    for (auto& voice : sfx_) {
        if (voice.active && voice.id == id) {
            release(voice);
        }
    }
}

void AudioMixer::stop_all() {
    events_.push_back({frame_, AudioOp::stop_all, 0U});
    release(music_);
    for (auto& voice : sfx_) {
        release(voice);
    }
}

void AudioMixer::attenuate(const std::uint8_t level) {
    events_.push_back({frame_, AudioOp::attenuate, level});
    attenuation_ = level;
}

void AudioMixer::advance_voice(Voice& voice, const std::size_t samples,
                               std::int32_t* mix) {
    for (std::size_t i = 0U; i < samples && voice.active; ++i) {
        if (voice.loop_end && voice.position >= *voice.loop_end) {
            voice.position = *voice.loop_start;
        }
        if (voice.position >= voice.length) {
            voice.active = false;
            break;
        }
        if (mix != nullptr && voice.pcm) {
            std::int32_t left = voice.pcm->samples[voice.position * 2U];
            std::int32_t right = voice.pcm->samples[voice.position * 2U + 1U];
            if (voice.release_left != 0U) {
                const auto ramp = static_cast<std::int32_t>(voice.release_left);
                left = left * ramp / static_cast<std::int32_t>(kAudioReleaseSamples);
                right = right * ramp / static_cast<std::int32_t>(kAudioReleaseSamples);
            }
            mix[i * 2U] += left;
            mix[i * 2U + 1U] += right;
        }
        ++voice.position;
        if (voice.release_left != 0U && --voice.release_left == 0U) {
            voice.active = false;
        }
    }
}

void AudioMixer::advance_frame(AudioSink* sink) {
    const bool want_pcm = render_pcm_ && sink != nullptr && sink->wants_pcm();
    std::array<std::int32_t, kAudioSamplesPerFrame * 2U> mix{};
    std::int32_t* target = want_pcm ? mix.data() : nullptr;
    advance_voice(music_, kAudioSamplesPerFrame, target);
    for (auto& voice : sfx_) {
        advance_voice(voice, kAudioSamplesPerFrame, target);
    }
    if (sink != nullptr) {
        std::array<std::int16_t, kAudioSamplesPerFrame * 2U> out{};
        if (want_pcm) {
            const auto level = std::min<std::int32_t>(attenuation_,
                                                      kAudioAttenuationSilent);
            const auto gain = kAudioAttenuationSilent - level;
            for (std::size_t i = 0U; i < out.size(); ++i) {
                const auto scaled = mix[i] * gain / kAudioAttenuationSilent;
                out[i] = static_cast<std::int16_t>(
                    std::clamp<std::int32_t>(scaled, -32768, 32767));
            }
        }
        sink->submit(out.data(), kAudioSamplesPerFrame);
    }
    ++frame_;
}

bool AudioMixer::playing(const std::uint16_t id) const {
    return playing_voice_count(id) != 0U;
}

std::size_t AudioMixer::playing_voice_count(const std::uint16_t id) const {
    std::size_t count = music_.active && music_.id == id ? 1U : 0U;
    for (const auto& voice : sfx_) {
        if (voice.active && voice.id == id) {
            ++count;
        }
    }
    return count;
}

std::optional<std::uint16_t> AudioMixer::music_id() const {
    return music_.active ? std::optional<std::uint16_t>(music_.id)
                         : std::nullopt;
}

std::size_t AudioMixer::active_sfx_voices() const {
    return static_cast<std::size_t>(std::count_if(
        sfx_.begin(), sfx_.end(), [](const Voice& v) { return v.active; }));
}

std::optional<std::uint64_t> AudioMixer::position(const std::uint16_t id) const {
    const Voice* newest = nullptr;
    const auto consider = [&](const Voice& voice) {
        if (voice.active && voice.id == id &&
            (newest == nullptr || voice.started > newest->started)) {
            newest = &voice;
        }
    };
    consider(music_);
    for (const auto& voice : sfx_) {
        consider(voice);
    }
    return newest ? std::optional<std::uint64_t>(newest->position)
                  : std::nullopt;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
