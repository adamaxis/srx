#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void advance_entity_animation_accumulator(EntityAnimationState& state) {
    // 0x5FDC uses ADD.W followed by a signed CMP.W/BLT. Preserve the source
    // word wrap before the comparison; otherwise high-rate lists such as the
    // 0x270F weapon descriptor cadence keep advancing through a phase where
    // the 68000 sees a negative accumulator.
    const auto accumulated_word = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.accumulator) +
        static_cast<std::uint16_t>(state.increment));
    const auto accumulated = static_cast<std::int16_t>(accumulated_word);
    if (state.period <= accumulated) {
        state.frame = static_cast<std::uint16_t>(state.frame + 1U);
        state.redraw_requested = true;
        state.accumulator = static_cast<std::int16_t>(
            static_cast<std::uint16_t>(accumulated_word) -
            static_cast<std::uint16_t>(state.period));
        return;
    }

    state.accumulator = accumulated;
}

std::uint16_t select_field_entity_animation_selector(
    const std::uint8_t direction, const bool walking) {
    const auto slot = static_cast<std::size_t>(direction & 0x07U);
    return walking ? kFieldWalkAnimationSelectors[slot]
                   : kFieldIdleAnimationSelectors[slot];
}

bool field_entity_animation_flips_horizontally(
    const std::uint8_t direction) {
    return (direction & 0x07U) >= 5U;
}

std::uint32_t entity_kind_state_table(const std::uint16_t kind) {
    return kEntityKindStateTables[static_cast<std::size_t>(kind & 0x0FU)];
}

std::uint16_t bound_field_walk_animation_frame(const std::uint16_t frame) {
    return static_cast<std::uint16_t>(frame % kFieldWalkAnimationFrameCount);
}

bool advance_entity_sprite_descriptor(
    EntitySpriteDescriptorState& state,
    const EntitySpriteDescriptorList& list,
    const std::uint16_t descriptor_word) {
    auto frame = state.frame;
    if (list.marker < 0) {
        const auto count = static_cast<std::uint16_t>(list.marker & 0x7FFF);
        if (static_cast<std::int16_t>(count) <=
            static_cast<std::int16_t>(state.frame)) {
            state.selector = static_cast<std::uint16_t>(state.selector | 0x8000U);
            frame = static_cast<std::uint16_t>(count - 1U);
            state.frame = frame;
        }
        state.increment = list.auxiliary;
    } else {
        if (static_cast<std::int16_t>(list.marker) <=
            static_cast<std::int16_t>(state.frame)) {
            state.frame = 0;
        }
        frame = state.frame;
        if (state.kind == 0x000DU) {
            state.increment = frame == 2U ? 0x0090U : 0x0180U;
        } else if (state.timing_word < 0x0016U) {
            state.increment = static_cast<std::uint16_t>(
                static_cast<std::uint32_t>(state.timing_word) * 0x001CU);
        } else {
            state.increment = static_cast<std::uint16_t>(
                static_cast<std::uint32_t>(state.timing_word) * 0x0010U);
        }
    }

    const auto index = static_cast<std::size_t>(frame);
    const auto pointer =
        index < list.frame_pointers.size() ? list.frame_pointers[index] : 0U;
    auto append = false;
    if (descriptor_word == 0x8002U && (state.flags & 0x01U) != 0U) {
        state.flags = static_cast<std::uint8_t>(state.flags & ~0x01U);
        append = true;
    }
    state.descriptor = pointer;
    return append;
}

EntityMotionActionRowSelection select_entity_motion_action_row(
    const std::int8_t active_action_state,
    const std::uint8_t one_based_action_code) {
    const auto zero_based_code =
        static_cast<std::uint16_t>(one_based_action_code) - 1U;
    return EntityMotionActionRowSelection{
        active_action_state < 0 ? EntityMotionActionTable::MagicSpell
                                : EntityMotionActionTable::EquipmentItem,
        one_based_action_code,
        static_cast<std::uint16_t>(zero_based_code * 0x30U)};
}

} // namespace shadowrun::ghidra_conversion
