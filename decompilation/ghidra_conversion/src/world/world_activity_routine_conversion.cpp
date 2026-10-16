#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool marker_matches(const std::uint8_t marker,
                    const std::int8_t runner_slot) {
    const auto expected = static_cast<std::uint8_t>(
        static_cast<std::int16_t>(runner_slot) + 1);
    return marker == expected;
}

void clear_effect_words(WorldActivityRoutineState& state) {
    state.world_activity_effect_words_cleared = true;
}

WorldActivityMode classify_source_destination(
    const std::uint16_t scene_id) {
    constexpr std::uint16_t kLastFieldSceneId = 20U;
    constexpr std::uint16_t kLastResourceActivitySceneId = 43U;
    constexpr std::uint16_t kLastScriptedActivitySceneId = 55U;
    if (scene_id <= kLastFieldSceneId) {
        return WorldActivityMode::field;
    }
    if (scene_id <= kLastResourceActivitySceneId) {
        return WorldActivityMode::resource_activity;
    }
    if (scene_id <= kLastScriptedActivitySceneId) {
        return WorldActivityMode::scripted_activity;
    }
    return WorldActivityMode::halted;
}

} // namespace

std::optional<std::uint16_t> resolve_world_activity_scene_id(
    const std::uint8_t mode_event_code,
    const std::array<std::uint8_t, 32>& event_mode_table) {
    if (mode_event_code >= event_mode_table.size()) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(event_mode_table[mode_event_code]);
}

WorldActivityResult run_world_activity_routine(
    const WorldActivityRoutineInput& input,
    WorldActivityRoutineState& state,
    const WorldActivityRoutineCallbacks& callbacks) {
    state = WorldActivityRoutineState{};
    state.scene_id = input.initial_scene_id;
    state.world_flags = input.world_flags;

    if (input.timeout_override != 0U) {
        state.timeout_requested = true;
        if (callbacks.schedule_world_activity_timeout) {
            callbacks.schedule_world_activity_timeout();
        }
        state.scene_id = input.timeout_override == 1U ? 0x36U : 0x37U;
    }
    if (input.descriptors_pending) {
        state.descriptors_published = true;
        if (callbacks.publish_world_activity_descriptors) {
            callbacks.publish_world_activity_descriptors();
        }
        state.scene_id = 0x0FU;
    }

    state.scene_snapshot_taken = true;
    if (callbacks.snapshot_scene_state) {
        callbacks.snapshot_scene_state();
    }
    state.field_scene_prepared = true;
    if (callbacks.prepare_field_scene_buffers) {
        callbacks.prepare_field_scene_buffers();
    }

    const bool source_scene_overridden = input.timeout_override != 0U ||
                                         input.descriptors_pending;
    state.mode = !source_scene_overridden &&
                         input.routed_destination_mode.has_value()
                     ? *input.routed_destination_mode
                     : classify_source_destination(state.scene_id);

    if (state.mode == WorldActivityMode::resource_activity) {
        state.default_resource_word = input.default_resource_word;
        if (!input.resource_random_index_bound.has_value() ||
            *input.resource_random_index_bound == 0U ||
            *input.resource_random_index_bound > 6U) {
            return WorldActivityResult::resource_activity_bound_unavailable;
        }
        state.resource_random_index_bound =
            *input.resource_random_index_bound;
        if (!callbacks.sample_source_random_bound ||
            !callbacks.advance_source_prng) {
            return WorldActivityResult::
                resource_activity_random_input_unavailable;
        }
        const auto sampled_index = callbacks.sample_source_random_bound(
            state.resource_random_index_bound);
        const auto marker_random_word = callbacks.advance_source_prng();
        if (!sampled_index.has_value() || !marker_random_word.has_value()) {
            return WorldActivityResult::
                resource_activity_random_input_unavailable;
        }
        if (*sampled_index >= state.resource_random_index_bound) {
            return WorldActivityResult::resource_activity_sample_out_of_range;
        }
        state.selected_resource_random_index = *sampled_index;
        state.resource_marker_ordinal = static_cast<std::uint8_t>(
            *marker_random_word & 0x0007U);
        if (input.resource_overrides.force_index_five_marker_zero) {
            state.selected_resource_random_index = 5U;
            state.resource_marker_ordinal = 0U;
        }
        if (input.resource_overrides.force_index_two_marker_three) {
            state.selected_resource_random_index = 2U;
            state.resource_marker_ordinal = 3U;
        }
        if (input.resource_overrides.force_index_four) {
            state.selected_resource_random_index = 4U;
        }
        copy_scene_resource_cursor(state.resource_cursor,
                                   input.resource_cursor_source);
        state.mode_word_upload_requested = true;
        if (callbacks.upload_mode_word_block) {
            callbacks.upload_mode_word_block();
        }
        clear_effect_words(state);
        return WorldActivityResult::resource_activity_ready;
    }
    if (state.mode == WorldActivityMode::scripted_activity) {
        clear_effect_words(state);
        return WorldActivityResult::scripted_activity_ready;
    }
    if (state.mode == WorldActivityMode::halted) {
        clear_effect_words(state);
        return WorldActivityResult::halted;
    }

    if (input.existing_activity) {
        clear_effect_words(state);
        return WorldActivityResult::existing_activity_still_active;
    }

    if (state.scene_id == 0x14U) {
        if ((state.world_flags & (std::uint32_t{1U} << 26U)) == 0U) {
            clear_effect_words(state);
            return WorldActivityResult::blocked_by_scene_gate;
        }
        state.world_flags &= ~(std::uint32_t{1U} << 26U);
        state.timeout_requested = true;
        if (callbacks.schedule_world_activity_timeout) {
            callbacks.schedule_world_activity_timeout();
        }
        clear_effect_words(state);
        return WorldActivityResult::timeout_scheduled;
    }

    if (state.scene_id == 0x0DU &&
        (input.mode_flags & (std::uint32_t{1U} << 19U)) == 0U) {
        clear_effect_words(state);
        return WorldActivityResult::blocked_by_scene_gate;
    }
    if (state.scene_id == 0x0FU &&
        (input.mode_flags & (std::uint32_t{1U} << 29U)) == 0U) {
        clear_effect_words(state);
        return WorldActivityResult::blocked_by_scene_gate;
    }

    std::int8_t runner_slot = 0;
    if (state.scene_id == 0x0AU) {
        runner_slot = input.scene_variant == 6U ? 2 : 4;
    } else if (state.scene_id >= 0x0AU) {
        const auto index = static_cast<std::size_t>(state.scene_id - 0x0AU);
        if (index >= input.runner_slot_by_scene.size()) {
            clear_effect_words(state);
            return WorldActivityResult::blocked_by_scene_gate;
        }
        runner_slot = input.runner_slot_by_scene[index];
    }

    if (marker_matches(input.primary_runner_marker, runner_slot) ||
        marker_matches(input.secondary_runner_marker, runner_slot)) {
        clear_effect_words(state);
        return WorldActivityResult::field_ready;
    }

    if (state.scene_id == 0x0AU) {
        state.selected_scene_ten_variant_script = input.scene_variant == 6U;
        state.selected_world_script = input.scene_variant == 6U ? 0U : 1U;
        clear_effect_words(state);
        return WorldActivityResult::world_script_selected;
    }

    const auto script_index = static_cast<std::size_t>(state.scene_id - 0x0AU);
    if (script_index >= input.world_script_available.size() ||
        !input.world_script_available[script_index]) {
        clear_effect_words(state);
        return WorldActivityResult::blocked_by_scene_gate;
    }
    state.selected_world_script = script_index;
    clear_effect_words(state);
    return WorldActivityResult::world_script_selected;
}

void begin_world_activity(
    WorldActivityStartState& state,
    const std::uint32_t descriptor) {
    state.timer_frames = 0x3CU;
    state.activity_pointer = descriptor;
}

bool schedule_world_activity_timeout(WorldActivityTimeoutState& state) {
    if (state.scene_id == 0x36U && state.scene_36_gate == 0U) {
        return false;
    }
    if (state.timeout_marker == 0) {
        state.timeout_marker = -1;
    }
    state.timeout_frames = state.extended_timeout != 0U ? 0x0258U : 0x0078U;
    return true;
}

bool handle_world_activity_request(
    WorldActivityRoutineRequestState& state,
    const std::array<WorldActivityRequestSlot, kWorldActivityRequestSlotCount>&
        slots,
    WorldActivityStartState& activity) {
    state.clear_80 = 0U;
    state.clear_82 = 0U;
    state.clear_84 = 0U;
    state.clear_87 = 0U;
    if (state.request_flag == 0U) {
        return false;
    }
    state.request_flag = 0U;
    state.found_marker = 0xFFU;
    for (const auto& slot : slots) {
        if ((slot.flags_byte & 1U) != 0U &&
            (slot.route_flags & 0x80U) != 0U) {
            state.found_marker = 0U;
            break;
        }
    }
    begin_world_activity(activity, kWorldActivityRequestDescriptor);
    return true;
}

} // namespace shadowrun::ghidra_conversion
