#pragma once

#include "shadowrun_ghidra/world_resource_batch_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleRevealWordWrite {
    std::uint16_t destination_address = 0;
    std::uint32_t source_buffer_token = 0;
    std::uint16_t source_byte_offset = 0;
    std::uint32_t mask = 0;
    std::uint32_t value = 0;
};

struct TitleRevealTransferState {
    std::int16_t countdown = 0;          // FFE008
    std::uint16_t profile_selector = 0;  // FFDF88
    std::uint8_t behavior_flags = 0;     // behavior record +0x12
    bool world_sequence_requested = false; // FFE16B

    std::uint16_t destination_base = 0;  // FFDF76
    std::uint16_t random_bound = 0;      // FFE006
    std::uint32_t source_buffer_token = 0; // FFDF7E
    WorldSequenceState world_sequence{};
};

struct TitleRevealTransferCallbacks {
    // 0x1D1DC/0x0F5C: refresh the source block at the host asset boundary.
    std::function<void()> transfer_source_block;
    // 0x0D3A: return the bounded source-PRNG sample used for one write.
    std::function<std::uint16_t(std::uint16_t bound)> sample_random;
    // 0x1D3F4: resolve the eight-nibble reveal mask. The source always passes
    // selector 5 from this service.
    std::function<std::uint32_t(std::uint16_t selector)> expand_mask;
    // Read one source longword from the named decoded source asset.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_buffer_token,
        std::uint16_t source_byte_offset)>
        read_source_longword;
    // Host presentation receives the masked write directly; it does not
    // reproduce the Genesis VDP command/data port.
    std::function<void(const TitleRevealWordWrite&)> write_reveal_word;
    std::function<void()> publish_menu_selection_then_text; // 0x1A01E
};

enum class TitleRevealTransferAction {
    rendered,
    started_world_sequence,
    handed_to_menu_text,
    source_read_unresolved,
    transfer_dependency_unavailable,
};

// ROM 0x1D1F4: advance the title/world reveal transfer selected by FF1592
// bit 0x10. Active frames emit 0x47 masked source writes; the terminal frame
// clears the behavior flag and chooses the world-sequence or menu/text handoff.
TitleRevealTransferAction advance_title_reveal_transfer_service(
    TitleRevealTransferState& state,
    const TitleRevealTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
