#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct CyberdeckDataFile {
    std::int8_t id = -1;
    std::uint16_t size = 0U;
};

// Typed view of the US Genesis deck record at FFFBBE. Unknown fields remain
// absent when a restored save has not supplied its record bytes.
struct CyberdeckRecord {
    std::optional<std::uint8_t> mpcp;                       // +0x00
    std::optional<std::uint8_t> hardening;                   // +0x01
    std::optional<std::uint16_t> memory_megapulses;          // +0x02
    std::optional<std::uint16_t> storage_megapulses;         // +0x04
    std::optional<std::uint8_t> load_io_speed;               // +0x06
    std::optional<std::uint8_t> response;                    // +0x07
    std::optional<std::array<std::uint8_t, 4>> persona;      // +0x08..+0x0B
    std::optional<std::uint8_t> persona_condition;           // +0x0C
    std::optional<std::int8_t> model_index;                  // +0x0D
    std::optional<std::array<std::uint8_t, 12>> program_ratings; // +0x0E..+0x19
    std::optional<std::array<std::int8_t, 5>> active_programs; // +0x1A..+0x1E
    std::optional<std::array<CyberdeckDataFile, 5>> data_files; // +0x1F..+0x2D
};

} // namespace shadowrun::ghidra_conversion
