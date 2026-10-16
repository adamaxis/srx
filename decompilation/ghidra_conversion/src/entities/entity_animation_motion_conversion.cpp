#include "shadowrun_ghidra/entities/entity_animation_motion_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void route_to_finish(EntityAnimationMotionState& state,
                     EntityAnimationMotionResult& result,
                     const EntityAnimationMotionCallbacks& callbacks) {
    result.routed_to_finish = true;
    if (static_cast<std::int16_t>(state.frame_counter) <= 0x003C &&
        callbacks.finish_animation) {
        result.handler_installed = callbacks.finish_animation();
    }
}

bool run_position_gate(EntityAnimationMotionState& state,
                       EntityAnimationMotionResult& result,
                       const EntityAnimationMotionCallbacks& callbacks) {
    result.position_gate_called = true;
    if (!callbacks.evaluate_position_gate) {
        result.position_gate_unresolved = true;
        return false;
    }
    const auto carry = callbacks.evaluate_position_gate();
    if (!carry.has_value()) {
        result.position_gate_unresolved = true;
        return false;
    }
    result.position_gate_carry = *carry;
    if (!*carry) {
        if (callbacks.continue_to_target_update) {
            callbacks.continue_to_target_update();
        }
        result.routed_to_target_update = true;
        return true;
    }
    state.delay = 0U;
    route_to_finish(state, result, callbacks);
    return true;
}

} // namespace

EntityAnimationMotionResult run_entity_animation_motion(
    EntityAnimationMotionState& state,
    const EntityAnimationMotionCallbacks& callbacks) {
    EntityAnimationMotionResult result{};

    if (state.global_phase_active &&
        (state.entity_flags & 0x08U) == 0U &&
        state.entity_type == 0x28U) {
        route_to_finish(state, result, callbacks);
        return result;
    }

    if (state.animation_mode != 0U) {
        if (state.animation_mode != 2U) {
            result.blocked_by_animation_mode = true;
            return result;
        }
        if (!state.linked_position.has_value()) {
            return result;
        }
        result.linked_record_present = true;
        state.target_position = *state.linked_position;
        result.linked_position_copied = true;
        if (callbacks.continue_to_target_update) {
            callbacks.continue_to_target_update();
        }
        result.routed_to_target_update = true;
        return result;
    }

    if (!state.linked_position.has_value()) {
        return result;
    }
    result.linked_record_present = true;
    if (!callbacks.measure_heading_distance) {
        result.distance_table_unresolved = true;
        return result;
    }
    const auto measurement = callbacks.measure_heading_distance(
        state.current_position, *state.linked_position);
    if (!measurement.has_value()) {
        result.distance_table_unresolved = true;
        return result;
    }
    result.direction = measurement->direction;
    result.distance = measurement->distance;

    state.route_flags = static_cast<std::uint8_t>(
        state.route_flags & static_cast<std::uint8_t>(~0x10U));

    std::optional<EntityAnimationDistanceBounds> bounds;
    if ((state.entity_flags & 0x08U) != 0U) {
        result.distance_table_used = true;
        result.distance_table_index = state.distance_type;
        if (!callbacks.distance_bounds) {
            result.distance_table_unresolved = true;
            return result;
        }
        bounds = callbacks.distance_bounds(
            EntityAnimationDistanceTable::by_entity_type,
            state.distance_type);
    } else if (state.delay != 0U) {
        state.delay = static_cast<std::uint8_t>(state.delay - 1U);
        result.distance_table_used = true;
        std::optional<std::uint8_t> distance_index;
        if (callbacks.item_catalog != nullptr &&
            callbacks.magic_spell_catalog != nullptr) {
            const auto selection = select_entity_motion_action_row(
                state.animation_frame, state.animation_phase);
            result.phase_source_row = selection.row_byte_offset;
            const auto row_index = static_cast<std::size_t>(
                static_cast<std::uint16_t>(state.animation_phase) - 1U);
            if (selection.table == EntityMotionActionTable::EquipmentItem) {
                if (row_index >= callbacks.item_catalog->size() ||
                    (*callbacks.item_catalog)[row_index].item_code !=
                        state.animation_phase) {
                    result.source_data_missing = true;
                    result.distance_table_unresolved = true;
                    return result;
                }
                distance_index = (*callbacks.item_catalog)[row_index]
                                     .source_action_parameter_tail;
            } else {
                if (row_index >= callbacks.magic_spell_catalog->size() ||
                    (*callbacks.magic_spell_catalog)[row_index].spell_code !=
                        static_cast<std::uint8_t>(
                            state.animation_phase - 1U)) {
                    result.source_data_missing = true;
                    result.distance_table_unresolved = true;
                    return result;
                }
                distance_index = static_cast<std::uint8_t>(
                    (*callbacks.magic_spell_catalog)[row_index]
                        .attribute_selector);
            }
        } else {
            if (!callbacks.select_phase_distance) {
                result.distance_table_unresolved = true;
                return result;
            }
            const auto selection = callbacks.select_phase_distance(
                state.animation_frame < 0, state.animation_phase);
            if (!selection.has_value()) {
                result.distance_table_unresolved = true;
                return result;
            }
            distance_index = selection->distance_index;
            result.phase_source_row = selection->source_row;
        }
        result.distance_table_index = *distance_index;
        if (!callbacks.distance_bounds) {
            result.distance_table_unresolved = true;
            return result;
        }
        bounds = callbacks.distance_bounds(
            EntityAnimationDistanceTable::by_animation_phase,
            *distance_index);
    }

    if (bounds.has_value()) {
        if (result.distance >= bounds->lower &&
            result.distance < bounds->upper) {
            state.delay = 0U;
        } else if (result.distance >= bounds->upper) {
            state.target_position = *state.linked_position;
            result.target_position_copied = true;
            run_position_gate(state, result, callbacks);
            return result;
        }
    }

    state.route_flags = static_cast<std::uint8_t>(
        state.route_flags | 0x10U);
    if (state.terrain_gate != 0U) {
        route_to_finish(state, result, callbacks);
        return result;
    }
    if (!callbacks.transform_vector) {
        result.distance_table_unresolved = true;
        return result;
    }
    const auto vector = callbacks.transform_vector(
        static_cast<std::uint16_t>(result.direction + 0x4000U),
        static_cast<std::uint16_t>(result.distance << 1U));
    if (!vector.has_value()) {
        result.distance_table_unresolved = true;
        return result;
    }
    state.target_position.x = static_cast<std::uint16_t>(
        state.current_position.x + vector->x);
    state.target_position.y = static_cast<std::uint16_t>(
        state.current_position.y + vector->y);
    result.target_position_built = true;
    run_position_gate(state, result, callbacks);
    return result;
}

} // namespace shadowrun::ghidra_conversion
