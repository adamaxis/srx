#pragma once

// Id-keyed audio resources (doc 26 §4.2). The manifest is generated from
// enums.lua by gameplay/scripts/tools/gen_audio_manifest.lua; the runtime only
// plays the pre-rendered resources it names. No Qt dependency.

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class AudioResourceKind : std::uint8_t { music, sfx };

struct AudioResourceRow {
    std::uint16_t id = 0U;
    std::string enum_name;   // e.g. "MusicId.FIELD_06"
    AudioResourceKind kind = AudioResourceKind::sfx;
    std::string alias;       // asset-catalog alias
    std::optional<std::uint64_t> loop_start;  // sample frames
    std::optional<std::uint64_t> loop_end;
    bool persistent = false; // no proven end; plays until stopped
};

// Decoded 16-bit stereo PCM at kAudioSampleRate.
struct AudioPcm {
    std::vector<std::int16_t> samples; // interleaved L/R
    std::uint64_t frames = 0U;
};

class AudioResourceBank {
public:
    using PathResolver =
        std::function<std::filesystem::path(std::string_view alias)>;

    explicit AudioResourceBank(PathResolver resolver);

    // Parses audio_manifest.tsv. Returns false and sets error() on a
    // malformed row; rows whose alias does not resolve are kept (they play
    // nothing) so a missing file never silently renumbers ids.
    bool load_manifest(const std::filesystem::path& manifest);
    bool load_manifest_text(std::string_view text);

    const AudioResourceRow* row(std::uint16_t id) const;
    const std::map<std::uint16_t, AudioResourceRow>& rows() const {
        return rows_;
    }
    std::filesystem::path path_for(std::string_view alias) const;

    // Sample-frame length from the FLAC header (no decode). nullopt when the
    // file is missing or unreadable.
    std::optional<std::uint64_t> frame_count(std::uint16_t id);
    // Lazily decoded and cached PCM. Null when unavailable.
    std::shared_ptr<const AudioPcm> pcm(std::uint16_t id);

    const std::string& error() const { return error_; }

private:
    PathResolver resolver_;
    std::map<std::uint16_t, AudioResourceRow> rows_;
    std::map<std::uint16_t, std::optional<std::uint64_t>> frame_counts_;
    std::map<std::uint16_t, std::shared_ptr<const AudioPcm>> pcm_;
    std::string error_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
