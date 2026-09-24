#include "shadowrun_ghidra/entity_timing_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t advance_entity_primary_timer(EntityTimingState& state,
                                            std::uint32_t frame_counter,
                                            std::uint16_t inactive_result) {
    if ((state.flags & 0x40U) != 0U) {
        return inactive_result;
    }

    const auto frame_phase = static_cast<std::uint16_t>(frame_counter & 0x3FU);
    if (frame_phase != 0U) {
        return frame_phase;
    }

    state.primary_timer = static_cast<std::int8_t>(state.primary_timer - 1);
    if (state.primary_timer < 0) {
        state.flags = static_cast<std::uint8_t>(state.flags & 0x7FU);
    }
    return 0U;
}

std::uint16_t advance_entity_secondary_timer(EntityTimingState& state,
                                              std::uint32_t frame_counter) {
    const auto frame_phase = static_cast<std::uint16_t>(frame_counter & 0x3FU);
    if (frame_phase != 0U) {
        return frame_phase;
    }

    state.secondary_timer = static_cast<std::int8_t>(state.secondary_timer - 1);
    if (state.secondary_timer < 0) {
        state.flags = static_cast<std::uint8_t>(state.flags & 0xDFU);
    }
    return 0U;
}

} // namespace shadowrun::ghidra_conversion
