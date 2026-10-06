#include "shadowrun/audio/audio_extract.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace shadowrun::audio {
namespace {

struct BlobDefinition {
    const char* relative_path;
    const char* role;
    std::size_t begin;
    std::size_t end;
};

// These are the literal operands used by the 68000 sound bootstrap at
// 0x531AA. The first range is copied to Z80 RAM by 0x52DE0. The following
// ranges are the four pointers passed to the driver initializer at 0x52EEC;
// the first three end at the next pointer, while the last is the 22-entry
// descriptor table immediately before its waveform payload.
constexpr BlobDefinition kBlobDefinitions[] = {
    {"driver/z80-driver.bin", "Z80 driver upload", 0x26C5E, 0x284E4},
    {"tables/driver-init-0.bin", "sound init pointer 0", 0x284E4, 0x29595},
    {"tables/driver-init-1.bin", "sound init pointer 1", 0x29595, 0x29609},
    {"tables/driver-init-2.bin", "sound init pointer 2", 0x29609, 0x311BF},
    {"tables/sound-resource-descriptors.bin", "sound init pointer 3 descriptor table",
     0x311BF, 0x312C7},
};

std::vector<std::uint8_t> copy_rom_range(
    const std::vector<std::uint8_t>& rom, std::size_t begin, std::size_t end) {
    if (begin > end || end > rom.size()) {
        throw std::out_of_range("audio resource range is outside the ROM");
    }
    return std::vector<std::uint8_t>(
        rom.begin() + static_cast<std::ptrdiff_t>(begin),
        rom.begin() + static_cast<std::ptrdiff_t>(end));
}

std::string hex_offset(std::size_t offset) {
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << offset;
    return output.str();
}

} // namespace

std::vector<GenesisAudioBlob> extract_genesis_audio_resources(
    const std::vector<std::uint8_t>& rom) {
    std::vector<GenesisAudioBlob> result;
    result.reserve(std::size(kBlobDefinitions));
    for (const auto& definition : kBlobDefinitions) {
        result.push_back({definition.relative_path,
                          definition.role,
                          definition.begin,
                          definition.end - definition.begin,
                          copy_rom_range(rom, definition.begin, definition.end)});
    }
    return result;
}

void write_genesis_audio_resources(
    const std::vector<std::uint8_t>& rom,
    const std::filesystem::path& output_root) {
    if (output_root.filename() == "assets") {
        throw std::invalid_argument(
            "audio source extraction cannot write into the runtime assets directory; "
            "use a separate analysis directory");
    }
    const auto resources = extract_genesis_audio_resources(rom);
    // This is deliberately an analysis artifact, not a runtime asset tree.
    // The ROM contains a custom Z80 event format; until that driver is traced,
    // emitting these slices under assets/audio would falsely present them as
    // playable Windows audio.
    const auto audio_root = output_root / "audio_raw";
    std::filesystem::create_directories(audio_root);
    std::ofstream catalog(audio_root / "catalog.tsv", std::ios::binary);
    if (!catalog) {
        throw std::runtime_error("cannot create audio resource catalog");
    }
    catalog << "asset\trole\tsource_offset\tbyte_count\tformat\tstatus\n";
    std::size_t total_bytes = 0;
    std::size_t written_files = 0;
    const auto write_resource = [&](const std::string& relative_path,
                                    const std::string& role,
                                    std::size_t source_offset,
                                    const std::vector<std::uint8_t>& bytes,
                                    const std::string& format) {
        const auto path = audio_root / relative_path;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("cannot create audio resource: " +
                                     path.string());
        }
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        if (!output) {
            throw std::runtime_error("cannot write audio resource: " +
                                     path.string());
        }
        catalog << relative_path << '\t' << role << '\t'
                << hex_offset(source_offset) << '\t' << bytes.size() << '\t'
                << format << "\traw-extracted\n";
        total_bytes += bytes.size();
        ++written_files;
    };
    for (const auto& resource : resources) {
        write_resource(resource.relative_path, resource.role,
                       resource.source_offset, resource.bytes, "custom-z80");
    }

    const auto read_le16 = [](const std::vector<std::uint8_t>& bytes,
                              std::size_t offset) {
        if (offset + 1U >= bytes.size()) {
            throw std::out_of_range("Z80 sequence pointer is truncated");
        }
        return static_cast<std::size_t>(bytes[offset]) |
            (static_cast<std::size_t>(bytes[offset + 1U]) << 8U);
    };
    const auto write_indexed_sequences = [&](const GenesisAudioBlob& table,
                                             std::size_t pointer_table_bytes,
                                             const std::string& table_name) {
        if ((pointer_table_bytes & 1U) != 0U ||
            pointer_table_bytes > table.bytes.size()) {
            throw std::runtime_error("invalid Z80 sequence pointer table size");
        }
        std::vector<std::size_t> offsets;
        for (std::size_t offset = 0; offset < pointer_table_bytes; offset += 2U) {
            offsets.push_back(read_le16(table.bytes, offset));
        }
        if (!std::is_sorted(offsets.begin(), offsets.end()) ||
            offsets.empty() || offsets.front() < pointer_table_bytes ||
            offsets.back() > table.bytes.size()) {
            throw std::runtime_error("Z80 sequence pointers are not an ordered in-blob table");
        }
        for (std::size_t index = 0; index < offsets.size(); ++index) {
            const auto begin = offsets[index];
            const auto end = index + 1U < offsets.size()
                ? offsets[index + 1U]
                : table.bytes.size();
            if (begin > end) {
                throw std::runtime_error("Z80 sequence pointer order is invalid");
            }
            std::ostringstream name;
            name << "sequences/" << table_name << "/entry-"
                 << std::setfill('0') << std::setw(3) << index << ".bin";
            std::vector<std::uint8_t> bytes(
                table.bytes.begin() + static_cast<std::ptrdiff_t>(begin),
                table.bytes.begin() + static_cast<std::ptrdiff_t>(end));
            write_resource(name.str(), "indexed Z80 event stream",
                           table.source_offset + begin, bytes,
                           "custom-z80-sequence");
        }
    };
    write_indexed_sequences(resources[1], 0x100U, "table-0");
    write_indexed_sequences(resources[3], 0xBCU, "table-2");

    // The fourth initializer starts with 22 fixed 12-byte descriptors. Keep
    // those records individual as well; their following waveform payload is
    // not yet assigned a format by the Z80 driver audit.
    const auto& descriptors = resources[4];
    constexpr std::size_t kDescriptorBytes = 12U;
    for (std::size_t index = 0;
         index + kDescriptorBytes <= descriptors.bytes.size();
         index += kDescriptorBytes) {
        std::ostringstream name;
        name << "descriptors/sound-" << std::setfill('0') << std::setw(2)
             << (index / kDescriptorBytes) << ".bin";
        std::vector<std::uint8_t> bytes(
            descriptors.bytes.begin() + static_cast<std::ptrdiff_t>(index),
            descriptors.bytes.begin() +
                static_cast<std::ptrdiff_t>(index + kDescriptorBytes));
        write_resource(name.str(), "Z80 sound descriptor",
                       descriptors.source_offset + index, bytes,
                       "custom-z80-descriptor");
    }
    catalog.flush();
    std::ofstream readme(audio_root / "README.md", std::ios::binary);
    if (!readme) {
        throw std::runtime_error("cannot create audio resource README");
    }
    readme << "# Genesis sound source analysis\n\n"
           << "This directory is an analysis-only holding area, not a native "
              "runtime asset directory. Do not copy it into decompilation/assets.\n\n"
           << "These files are exact byte slices from the US Genesis ROM. "
              "The game does not contain standard MIDI files: the 68000 "
              "uploads a custom Z80 sound driver at ROM 0x26C5E and passes "
              "four ROM pointers to its initializer at ROM 0x531AA.\n\n"
           << "`catalog.tsv` records the source offsets and lengths. The "
              "two indexed tables are split into 128 and 94 individual "
              "event streams, and the descriptor table is split into 22 "
              "records. These include music and effect event data, "
              "but their event encoding is interpreted by the Z80 driver. "
              "They are not WAV, MIDI, or other playable Windows audio. "
              "A decoder/render step must produce complete Windows-playable "
              "files before any game runtime can consume them.\n\n"
           << "Extracted " << written_files << " files from " << resources.size()
           << " source blobs ("
           << total_bytes << " bytes).\n";
}

} // namespace shadowrun::audio
