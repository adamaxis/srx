#include "shadowrun_ghidra/world/world_effect_animation_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectAnimationState normal{};
    normal.animation_active = true;
    normal.work.x = 0x0120U;
    normal.work.y = 0x00D0U;
    normal.work.frame_counter = 0U;
    normal.work.descriptor_state = 4U;
    normal.work.descriptor_phase = 2U;
    normal.work.descriptor_pointer = 0x000F1B32U;
    int delta_reads = 0;
    int append_count = 0;
    int build_count = 0;
    int overlay_count = 0;
    WorldEffectAnimationCallbacks callbacks{};
    callbacks.read_runner_animation_delta = [&]() -> std::optional<std::int16_t> {
        ++delta_reads;
        return -2;
    };
    callbacks.resolve_sprite_data =
        [](const bool transition, const std::uint32_t table,
           const std::uint8_t phase) -> std::optional<std::uint32_t> {
        return table + static_cast<std::uint32_t>(phase) +
               (transition ? 0x10000000U : 0U);
    };
    callbacks.append_sprite_record = [&](const std::uint16_t key) {
        append_count += key == 0x0694U ? 1 : 100;
    };
    callbacks.build_sprite = [&](const WorldEffectAnimationSpriteBuild& build) {
        build_count += build.x == 0x0120U && build.y == 0x00D0U &&
                               build.source_table_token == 0x000F1B32U
                           ? 1
                           : 100;
    };
    callbacks.advance_primary_overlay = [&] { ++overlay_count; };
    callbacks.advance_secondary_overlay = [&] { ++overlay_count; };
    callbacks.advance_background_overlay = [&] { ++overlay_count; };
    if (tick_world_effect_animation(normal, callbacks) !=
            WorldEffectAnimationAction::rendered ||
        delta_reads != 1 || normal.work.frame_counter != 2U ||
        normal.work.phase_or_descriptor_index != 1U || append_count != 1 ||
        build_count != 1 || overlay_count != 3) {
        return 1;
    }

    // The ROM adds the runner delta with ADD.B and then applies BPL to the
    // wrapped byte. A widened host addition would incorrectly keep 1 + 127
    // as 128 instead of taking the source negative-byte path to zero.
    WorldEffectAnimationState byte_wrapped{};
    byte_wrapped.animation_active = true;
    byte_wrapped.work.frame_counter = 0U;
    byte_wrapped.work.descriptor_state = 1U;
    byte_wrapped.work.descriptor_phase = 1U;
    byte_wrapped.work.descriptor_pointer = 0x3333U;
    auto byte_callbacks = callbacks;
    byte_callbacks.read_runner_animation_delta =
        []() -> std::optional<std::int16_t> { return 127; };
    if (tick_world_effect_animation(byte_wrapped, byte_callbacks) !=
            WorldEffectAnimationAction::rendered ||
        byte_wrapped.work.frame_counter != 0U) {
        return 6;
    }

    WorldEffectAnimationState simple{};
    simple.animation_active = true;
    simple.ffdf8c_nonzero = true;
    simple.selected_mode = 0;
    simple.work.frame_counter = 0U;
    simple.work.descriptor_state = 8U;
    simple.work.descriptor_phase = 1U;
    simple.work.descriptor_pointer = 0x2222U;
    if (tick_world_effect_animation(simple, callbacks) !=
            WorldEffectAnimationAction::rendered ||
        simple.work.frame_counter != 8U ||
        simple.work.phase_or_descriptor_index != 1U) {
        return 2;
    }

    WorldEffectAnimationState transition{};
    transition.animation_active = true;
    transition.descriptor_loading = true;
    transition.transition_active = true;
    transition.work.frame_counter = 0U;
    transition.work.phase_or_descriptor_index = 5U;
    int resets = 0;
    callbacks.reset_effect_entity = [&] { ++resets; };
    if (tick_world_effect_animation(transition, callbacks) !=
            WorldEffectAnimationAction::effect_entity_reset || resets != 1 ||
        transition.work.phase_or_descriptor_index != 6U) {
        return 3;
    }

    transition.work.frame_counter = 1U;
    transition.work.phase_or_descriptor_index = 0U;
    transition.work.x = 1U;
    transition.work.y = 2U;
    if (tick_world_effect_animation(transition, callbacks) !=
            WorldEffectAnimationAction::rendered ||
        transition.work.frame_counter != 0U) {
        return 4;
    }

    // FFE082 alone is not the transition-table selector. The source still
    // builds the normal descriptor sprite, skips only the append/timing
    // branch, and advances all three overlays while FFE0F2 is clear.
    WorldEffectAnimationState completion_gate{};
    completion_gate.animation_active = true;
    completion_gate.transition_active = true;
    completion_gate.work.descriptor_pointer = 0x4444U;
    bool used_transition_table = true;
    int gate_appends = 0;
    int gate_overlays = 0;
    auto gate_callbacks = callbacks;
    gate_callbacks.reset_effect_entity = {};
    gate_callbacks.resolve_sprite_data =
        [&](const bool transition_table, const std::uint32_t table,
            const std::uint8_t) -> std::optional<std::uint32_t> {
        used_transition_table = transition_table;
        return table;
    };
    gate_callbacks.append_sprite_record =
        [&](const std::uint16_t) { ++gate_appends; };
    gate_callbacks.advance_primary_overlay = [&] { ++gate_overlays; };
    gate_callbacks.advance_secondary_overlay = [&] { ++gate_overlays; };
    gate_callbacks.advance_background_overlay = [&] { ++gate_overlays; };
    if (tick_world_effect_animation(completion_gate, gate_callbacks) !=
            WorldEffectAnimationAction::rendered ||
        used_transition_table || gate_appends != 0 || gate_overlays != 3) {
        return 7;
    }

    WorldEffectAnimationState inactive{};
    if (tick_world_effect_animation(inactive, {}) !=
        WorldEffectAnimationAction::inactive) {
        return 5;
    }
    return 0;
}
