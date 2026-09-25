#include "shadowrun_ghidra/world_activity_routine_conversion.hpp"

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

} // namespace

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

    if (state.scene_id > 0x14U) {
        if (state.scene_id < 0x2CU) {
            state.mode = WorldActivityMode::resource_activity;
            state.default_resource_word = input.default_resource_word;
            state.random_bound = input.sampled_random_bound;
            state.random_slot = static_cast<std::uint8_t>(
                input.advanced_prng_value & 0x07U);
            if (input.force_random_pair_five_zero) {
                state.random_bound = 5U;
                state.random_slot = 0U;
            }
            if (input.force_random_pair_two_three) {
                state.random_bound = 2U;
                state.random_slot = 3U;
            }
            if (input.force_random_bound_four) {
                state.random_bound = 4U;
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
        if (state.scene_id < 0x38U) {
            state.mode = WorldActivityMode::scripted_activity;
            clear_effect_words(state);
            return WorldActivityResult::scripted_activity_ready;
        }

        state.mode = WorldActivityMode::halted;
        clear_effect_words(state);
        return WorldActivityResult::halted;
    }

    state.mode = WorldActivityMode::field;
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

} // namespace shadowrun::ghidra_conversion
