#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace shadowrun::audio {

// The Genesis sound system is a Z80 driver with ROM-resident tables. These
// blobs remain byte-for-byte ROM slices; decoding them into MIDI is a separate
// step because the driver, not the 68000, interprets their event format.
struct GenesisAudioBlob {
    std::string relative_path;
    std::string role;
    std::size_t source_offset = 0;
    std::size_t byte_count = 0;
    std::vector<std::uint8_t> bytes;
};

std::vector<GenesisAudioBlob> extract_genesis_audio_resources(
    const std::vector<std::uint8_t>& rom);

void write_genesis_audio_resources(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& output_root);

} // namespace shadowrun::audio
