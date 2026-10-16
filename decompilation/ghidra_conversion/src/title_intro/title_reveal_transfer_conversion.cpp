#include "shadowrun_ghidra/title_intro/title_reveal_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleRevealTransferAction advance_title_reveal_transfer_service(
    TitleRevealTransferState& state,
    const TitleRevealTransferCallbacks& callbacks) {
    state.countdown = static_cast<std::int16_t>(state.countdown - 1);
    if (state.countdown < 0) {
        state.behavior_flags = static_cast<std::uint8_t>(
            state.behavior_flags & ~0x10U);
        if (callbacks.transfer_source_block) {
            callbacks.transfer_source_block({state.source_buffer_token,
                                             state.destination_base,
                                             state.random_bound});
        }
        if (state.world_sequence_requested) {
            initialize_world_sequence(state.world_sequence);
            return TitleRevealTransferAction::started_world_sequence;
        }
        if (callbacks.publish_menu_selection_then_text) {
            callbacks.publish_menu_selection_then_text();
            return TitleRevealTransferAction::handed_to_menu_text;
        }
        return TitleRevealTransferAction::transfer_dependency_unavailable;
    }

    if ((state.profile_selector & 0x0003U) == 0U &&
        callbacks.transfer_source_block) {
        callbacks.transfer_source_block({state.source_buffer_token,
                                         state.destination_base,
                                         state.random_bound});
    }
    if (!callbacks.sample_random || !callbacks.expand_mask ||
        !callbacks.read_source_longword || !callbacks.write_reveal_word) {
        return TitleRevealTransferAction::transfer_dependency_unavailable;
    }

    // D6 starts at 0x46 and DBF executes the body 0x47 times.
    for (std::size_t iteration = 0; iteration < 0x47U; ++iteration) {
        const auto random_word = callbacks.sample_random(state.random_bound);
        const auto source_offset = static_cast<std::uint16_t>(
            random_word & 0xFFFCU);
        const auto destination_address = static_cast<std::uint16_t>(
            state.destination_base + random_word);
        // ROM 0x1D266 calls 0x1D3F4 for every DBF iteration. The expander
        // performs its own bounded PRNG sample from mask bank 5; it is not a
        // frame-constant mask.
        const auto mask = callbacks.expand_mask(5U);
        const auto source_word = callbacks.read_source_longword(
            state.source_buffer_token, source_offset);
        if (!source_word.has_value()) {
            return TitleRevealTransferAction::source_read_unresolved;
        }
        callbacks.write_reveal_word(TitleRevealWordWrite{
            destination_address, state.source_buffer_token, source_offset,
            mask, *source_word & mask});
    }
    return TitleRevealTransferAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
