#include "audio_resources.hpp"

#include "audio_mixer.hpp"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#include <fstream>
#include <sstream>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

std::vector<std::string> split_tabs(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream stream(line);
    while (std::getline(stream, field, '\t')) {
        fields.push_back(field);
    }
    if (!line.empty() && line.back() == '\t') {
        fields.emplace_back();
    }
    return fields;
}

std::optional<std::uint64_t> optional_number(const std::string& text) {
    if (text.empty() || text == "-") {
        return std::nullopt;
    }
    return std::stoull(text, nullptr, 0);
}

} // namespace

AudioResourceBank::AudioResourceBank(PathResolver resolver)
    : resolver_(std::move(resolver)) {}

bool AudioResourceBank::load_manifest(const std::filesystem::path& manifest) {
    std::ifstream file(manifest, std::ios::binary);
    if (!file) {
        error_ = "cannot open " + manifest.string();
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    return load_manifest_text(text.str());
}

bool AudioResourceBank::load_manifest_text(const std::string_view text) {
    rows_.clear();
    frame_counts_.clear();
    pcm_.clear();
    error_.clear();
    std::istringstream stream{std::string(text)};
    std::string line;
    bool header = false;
    std::size_t line_number = 0U;
    while (std::getline(stream, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const auto fields = split_tabs(line);
        if (!header) {
            header = true;
            if (fields.size() != 7U || fields[0] != "id" ||
                fields[3] != "alias") {
                error_ = "invalid audio manifest header";
                return false;
            }
            continue;
        }
        try {
            if (fields.size() != 7U) {
                throw std::invalid_argument("field count");
            }
            AudioResourceRow row;
            row.id = static_cast<std::uint16_t>(std::stoul(fields[0], nullptr, 0));
            row.enum_name = fields[1];
            if (fields[2] == "music") {
                row.kind = AudioResourceKind::music;
            } else if (fields[2] == "sfx") {
                row.kind = AudioResourceKind::sfx;
            } else {
                throw std::invalid_argument("kind");
            }
            row.alias = fields[3];
            row.loop_start = optional_number(fields[4]);
            row.loop_end = optional_number(fields[5]);
            row.persistent = fields[6] == "1";
            if (row.loop_start.has_value() != row.loop_end.has_value() ||
                (row.loop_start && *row.loop_start >= *row.loop_end) ||
                !rows_.emplace(row.id, row).second) {
                throw std::invalid_argument("loop/duplicate");
            }
        } catch (const std::exception&) {
            error_ = "invalid audio manifest row at line " +
                     std::to_string(line_number);
            return false;
        }
    }
    if (!header) {
        error_ = "audio manifest is empty";
        return false;
    }
    return true;
}

const AudioResourceRow* AudioResourceBank::row(const std::uint16_t id) const {
    const auto found = rows_.find(id);
    return found == rows_.end() ? nullptr : &found->second;
}

std::filesystem::path AudioResourceBank::path_for(
    const std::string_view alias) const {
    return resolver_ ? resolver_(alias) : std::filesystem::path{};
}

std::optional<std::uint64_t> AudioResourceBank::frame_count(
    const std::uint16_t id) {
    const auto cached = frame_counts_.find(id);
    if (cached != frame_counts_.end()) {
        return cached->second;
    }
    std::optional<std::uint64_t> frames;
    if (const auto* entry = row(id)) {
        const auto path = path_for(entry->alias);
        if (!path.empty()) {
            if (auto* flac = drflac_open_file(path.string().c_str(), nullptr)) {
                if (flac->sampleRate == kAudioSampleRate) {
                    frames = flac->totalPCMFrameCount;
                }
                drflac_close(flac);
            }
        }
    }
    frame_counts_[id] = frames;
    return frames;
}

std::shared_ptr<const AudioPcm> AudioResourceBank::pcm(const std::uint16_t id) {
    const auto cached = pcm_.find(id);
    if (cached != pcm_.end()) {
        return cached->second;
    }
    std::shared_ptr<const AudioPcm> result;
    if (const auto* entry = row(id)) {
        const auto path = path_for(entry->alias);
        unsigned int channels = 0U;
        unsigned int sample_rate = 0U;
        drflac_uint64 frames = 0U;
        drflac_int16* decoded = path.empty()
            ? nullptr
            : drflac_open_file_and_read_pcm_frames_s16(
                  path.string().c_str(), &channels, &sample_rate, &frames,
                  nullptr);
        if (decoded != nullptr) {
            if (sample_rate == kAudioSampleRate &&
                (channels == 1U || channels == 2U)) {
                auto pcm = std::make_shared<AudioPcm>();
                pcm->frames = frames;
                pcm->samples.resize(static_cast<std::size_t>(frames) * 2U);
                for (drflac_uint64 i = 0U; i < frames; ++i) {
                    pcm->samples[i * 2U] = decoded[i * channels];
                    pcm->samples[i * 2U + 1U] =
                        decoded[i * channels + (channels - 1U)];
                }
                result = std::move(pcm);
            }
            drflac_free(decoded, nullptr);
        }
    }
    pcm_[id] = result;
    if (result) {
        frame_counts_[id] = result->frames;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
