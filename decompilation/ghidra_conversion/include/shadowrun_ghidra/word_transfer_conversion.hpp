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
    std::function<void(std::uint16_t)> write_control_word;
};

// ROM 0xC3B2: emit one transfer command followed by one data word.
void write_single_word(
    std::uint16_t destination_address,
    std::uint16_t word,
    const WordTransferCallbacks& callbacks);

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

// ROM 0x0F98: emit a bounded even-byte source block as words after one
// transfer command. The source uses longword writes internally; the host
// callback sees the equivalent ordered word stream.
void write_vdp_word_block(
    std::uint16_t destination_address,
    const std::vector<std::uint16_t>& words,
    std::size_t byte_count,
    const WordTransferCallbacks& callbacks);

// ROM 0x0F5C: split a source block at its 64K source-address boundary before
// forwarding each portion to the bounded word-block writer.
void write_vdp_word_block_with_source_wrap(
    std::uint32_t source_address,
    std::uint16_t destination_address,
    const std::vector<std::uint16_t>& words,
    std::size_t byte_count,
    const WordTransferCallbacks& callbacks);

// ROM 0x33BE: repeat the wrapped transfer leaf for a fixed four-by-four
// source batch, preserving the source's 16-call order.
void write_four_by_four_transfer_batch(
    const std::function<void()>& write_wrapped_block);

// ROM 0x0301E: upload the fixed 0x140-byte scene-profile block at its source
// destination using the shared bounded transfer operation.
void upload_scene_profile_block(
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks);

// ROM 0x1232: write a row-major word grid, advancing the destination by
// 0x80 bytes between rows and applying the source word mask.
void write_vdp_grid(
    std::uint16_t destination_address,
    std::size_t width,
    std::size_t height,
    std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks);

// ROM 0x01184: write one masked row, optionally bracketing it with the
// source's display-increment control words.
void write_masked_vdp_row(
    std::uint16_t destination_address,
    std::size_t width,
    std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    bool configure_display_increment,
    const WordTransferCallbacks& callbacks);

// ROM 0x011C0: repeat one masked source row at 0x80-byte destination strides.
void write_repeated_vdp_grid(
    std::uint16_t destination_address,
    std::size_t width,
    std::size_t height,
    std::uint16_t word_mask,
    const std::vector<std::uint16_t>& row_words,
    const WordTransferCallbacks& callbacks);

// ROM 0x011F6: consume a row-major source stream while advancing each row's
// destination by two bytes.
void write_linear_vdp_grid(
    std::uint16_t destination_address,
    std::size_t width,
    std::size_t height,
    std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
