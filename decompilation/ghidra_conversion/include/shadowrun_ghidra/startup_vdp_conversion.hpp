#pragma once

#include "shadowrun_ghidra/word_transfer_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct RawVdpWriteCallbacks {
    std::function<void(std::uint16_t)> write_control_word;
    std::function<void(std::uint32_t)> write_control_long;
    std::function<void(std::uint16_t)> write_data_word;
};

// ROM 0x0EC0: upload the fixed 64-word startup palette table. The source
// rewrites the destination control word for each entry; the host callback
// receives that presentation detail without exposing MMIO to game logic.
void upload_startup_palette(
    const std::array<std::uint16_t, 64>& words,
    const RawVdpWriteCallbacks& callbacks);

// ROM 0x0F1C: issue a source transfer command and write its two-word header.
void write_transfer_header(
    std::uint16_t source_address,
    std::uint16_t first_word,
    std::uint16_t second_word,
    const WordTransferCallbacks& callbacks);

// ROM 0x0F3C: issue the fixed incrementing-write command and emit two words.
void write_fixed_transfer_pair(
    std::uint16_t first_word,
    std::uint16_t second_word,
    const RawVdpWriteCallbacks& callbacks);

// ROM 0x0FFE / 0x102A: clear the preceding sixteen words, then fill one
// 0x800-word plane with the repeated 0x07FF tile value.
void clear_and_fill_vdp_plane(
    std::uint16_t plane_address,
    const WordTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
