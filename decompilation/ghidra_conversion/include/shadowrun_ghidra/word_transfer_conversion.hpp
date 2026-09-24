#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct VdpTransferCommand {
    std::uint16_t low_word = 0;
    std::uint16_t high_word = 0;
};

// ROM 0x0E9C: split a source transfer address into the two command words
// emitted to the display-control boundary.
VdpTransferCommand make_vdp_transfer_command(std::uint16_t source_address);

struct WordTransferCallbacks {
    std::function<void(const VdpTransferCommand&)> begin_transfer;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x0DFC: emit one source word repeatedly. word_count is the host number
// of writes, replacing the source DBF register convention.
void write_repeated_words(
    std::uint16_t source_address,
    std::uint16_t word,
    std::size_t word_count,
    const WordTransferCallbacks& callbacks);

// ROM 0x0E18: emit a bounded source word stream in order.
void write_word_stream(
    std::uint16_t source_address,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks);

// ROM 0x1056: clear exactly sixteen words at the source transfer address.
void clear_vdp_words(
    std::uint16_t source_address,
    const WordTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
