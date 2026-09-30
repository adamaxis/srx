#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion::integration;

struct Observations {
    std::vector<std::string> order;
    std::uint16_t sampled_word = 0;
    bool exit_requires_anchor_refresh = false;
    bool fixed_audio_command_valid = true;
};

int expect(const bool condition) {
    return condition ? 0 : 1;
}

FieldRuntimeCoordinatorCallbacks make_callbacks(Observations& observations) {
    FieldRuntimeCoordinatorCallbacks callbacks{};
    callbacks.wait_for_vblank = [&] {
        observations.order.emplace_back("vblank");
    };
    callbacks.prepare_mode_entry_service = [&] {
        observations.order.emplace_back("prepare_mode_entry");
    };
    callbacks.toggle_field_scene_variant = [&](auto&) {
        observations.order.emplace_back("toggle_variant");
        return false;
    };
    callbacks.run_field_exit_orchestrator = [&](auto&) {
        observations.order.emplace_back("field_exit");
        return observations.exit_requires_anchor_refresh;
    };
    callbacks.reset_field_entity_transition_words = [&](auto&) {
        observations.order.emplace_back("reset_anchor_words");
    };
    callbacks.snapshot_entity_anchors = [&](auto&) {
        observations.order.emplace_back("snapshot_anchors");
    };
    callbacks.advance_field_mode_transition = [&](auto&) {
        observations.order.emplace_back("advance_mode_transition");
    };
    callbacks.initialize_world_activity_mode = [&](auto&) {
        observations.order.emplace_back("initialize_world_activity");
    };
    callbacks.begin_scene_37_world_activity = [&](auto&) {
        observations.order.emplace_back("begin_scene_37_activity");
    };
    callbacks.handle_world_activity_request = [&](auto&) {
        observations.order.emplace_back("world_activity_request");
    };
    callbacks.initialize_field_scene_state = [&](auto&) {
        observations.order.emplace_back("initialize_field_scene");
    };
    callbacks.latch_controller_events = [&] {
        observations.order.emplace_back("latch");
    };
    callbacks.sample_controller_input = [&] {
        observations.order.emplace_back("sample");
        return observations.sampled_word;
    };
    callbacks.service_entity_behavior_frames = [&] {
        observations.order.emplace_back("behavior");
    };
    callbacks.update_field_camera = [&] {
        observations.order.emplace_back("camera");
    };
    callbacks.update_entity_render_pass = [&] {
        observations.order.emplace_back("render");
    };
    callbacks.advance_palette_fade = [&] {
        observations.order.emplace_back("palette");
    };
    callbacks.advance_world_frame_logic = [&] {
        observations.order.emplace_back("world_frame");
    };
    callbacks.handle_mode_action_event = [&] {
        observations.order.emplace_back("action");
    };
    callbacks.run_fixed_audio_transaction = [&](const std::uint16_t command) {
        observations.fixed_audio_command_valid &= command == 8U;
        observations.order.emplace_back("audio_fixed");
    };
    callbacks.run_command_overview = [&] {
        observations.order.emplace_back("command_overview");
    };
    callbacks.service_audio_tick = [&] {
        observations.order.emplace_back("audio_tick");
    };
    callbacks.select_next_entity = [&] {
        observations.order.emplace_back("select_entity");
    };
    callbacks.restart_field_session = [&] {
        observations.order.emplace_back("restart_field");
    };
    callbacks.advance_field_scene_transition = [&] {
        observations.order.emplace_back("scene_transition");
    };
    callbacks.run_pending_field_scene_handler = [&] {
        observations.order.emplace_back("pending_handler");
        return PendingFieldSceneHandlerResult::resumed_field;
    };
    return callbacks;
}

int test_normal_frame_order_and_dirty_bits() {
    Observations observations;
    const auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;

    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::frame_waiting) != 0) {
        return 1;
    }
    const std::vector<std::string> expected{
        "vblank", "latch", "sample", "behavior", "camera", "render",
        "palette", "world_frame"};
    return expect(observations.order == expected &&
                  state.display_dirty_flags == 0x0EU);
}

int test_activity_gate_blocks_input_dispatch() {
    Observations observations;
    const auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.field_activity_lock = true;
    observations.sampled_word = 0xA000U;

    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::frame_waiting) != 0) {
        return 1;
    }
    return expect(!state.action_event_dispatched &&
                  !state.command_overview_dispatched &&
                  observations.order.size() == 8U);
}

int test_action_precedes_command_overview_without_excluding_it() {
    Observations observations;
    const auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    observations.sampled_word = 0xA000U;

    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::command_overview_dispatched) != 0) {
        return 1;
    }
    return expect(state.action_event_dispatched &&
                  state.command_overview_dispatched &&
                  observations.fixed_audio_command_valid &&
                  observations.order[8] == "action" &&
                  observations.order[9] == "audio_fixed" &&
                  observations.order[10] == "command_overview" &&
                  observations.order[11] == "audio_tick" &&
                  observations.order[12] == "select_entity" &&
                  observations.order[13] == "scene_transition");
}

int test_transition_branch_source_order() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.field_transition_requested = true;

    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 1;
    }
    const std::vector<std::string> expected{
        "vblank", "prepare_mode_entry", "toggle_variant", "field_exit",
        "initialize_world_activity", "scene_transition"};
    if (expect(observations.order == expected &&
               !state.field_transition_requested &&
               state.mode_entry_marker == -1 &&
               state.scene_refresh_marker == -1) != 0) {
        return 2;
    }

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    state.scene_runtime_state = 1U;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 3;
    }
    if (expect(state.scene_runtime_state == 2U &&
               observations.order == std::vector<std::string>{
                   "vblank", "prepare_mode_entry", "initialize_world_activity",
                   "scene_transition"}) != 0) {
        return 4;
    }

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    state.field_scene_gate = 1U;
    state.world_activity_transition_marker = 1U;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::world_activity_scene_reentry_requested) !=
        0) {
        return 5;
    }
    if (expect(state.world_activity_scene_reset_bit &&
               !state.world_activity_requested &&
               observations.order == std::vector<std::string>{
                   "vblank", "prepare_mode_entry", "world_activity_request",
                   "initialize_field_scene"}) != 0) {
        return 6;
    }

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    observations.exit_requires_anchor_refresh = true;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 7;
    }
    if (expect(observations.order == std::vector<std::string>{
                   "vblank", "prepare_mode_entry", "toggle_variant", "field_exit",
                   "reset_anchor_words", "snapshot_anchors", "scene_transition"}) !=
        0) {
        return 8;
    }
    observations.exit_requires_anchor_refresh = false;

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    state.scene_id = 4U;
    state.field_mode = 6U;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 9;
    }
    return expect(observations.order == std::vector<std::string>{
                      "vblank", "prepare_mode_entry", "toggle_variant",
                      "field_exit", "scene_transition"});
}

int test_mode_transition_and_scene_37_activity_paths() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.field_transition_requested = true;
    state.mode_transition_gate = 1U;
    state.field_mode = 8U;

    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 1;
    }
    if (expect(observations.order == std::vector<std::string>{
                   "vblank", "prepare_mode_entry", "toggle_variant", "field_exit",
                   "advance_mode_transition", "scene_transition"}) != 0) {
        return 2;
    }

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    state.scene_id = 0x37U;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_dispatched) != 0) {
        return 3;
    }
    return expect(observations.order == std::vector<std::string>{
                      "vblank", "prepare_mode_entry", "toggle_variant", "field_exit",
                      "initialize_world_activity", "begin_scene_37_activity",
                      "scene_transition"});
}

int test_request_branches_and_unresolved_transition() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.world_activity_requested = true;
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::world_activity_scene_reentry_requested) !=
        0) {
        return 1;
    }
    if (expect(!state.world_activity_requested &&
               observations.order == std::vector<std::string>{
                   "vblank", "world_activity_request", "initialize_field_scene"}) !=
        0) {
        return 2;
    }

    observations.order.clear();
    state = FieldRuntimeCoordinatorState{};
    state.field_transition_requested = true;
    callbacks.prepare_mode_entry_service = {};
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_transition_pending) != 0) {
        return 3;
    }
    return expect(state.field_transition_requested &&
                  observations.order == std::vector<std::string>{"vblank"});
}

int test_pending_handler_can_return_to_title_or_reenter_field() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.pending_field_scene_handler = true;
    callbacks.run_pending_field_scene_handler = [&] {
        observations.order.emplace_back("pending_handler");
        return PendingFieldSceneHandlerResult::returned_to_title;
    };
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::returned_to_title) != 0) {
        return 1;
    }
    if (expect(!state.pending_field_scene_handler) != 0) {
        return 2;
    }

    observations.order.clear();
    state.pending_field_scene_handler = true;
    state.display_service_word = 0U;
    callbacks.run_pending_field_scene_handler = [&] {
        observations.order.emplace_back("pending_handler");
        return PendingFieldSceneHandlerResult::resumed_field;
    };
    if (expect(advance_field_runtime_frame(state, callbacks) ==
               FieldRuntimeFrameResult::field_scene_reentry_requested) != 0) {
        return 3;
    }
    return expect(!state.pending_field_scene_handler &&
                  observations.order.back() == "scene_transition");
}

} // namespace

int main() {
    return test_normal_frame_order_and_dirty_bits() != 0 ||
                   test_activity_gate_blocks_input_dispatch() != 0 ||
                   test_action_precedes_command_overview_without_excluding_it() !=
                       0 ||
                   test_transition_branch_source_order() != 0 ||
                   test_mode_transition_and_scene_37_activity_paths() != 0 ||
                   test_request_branches_and_unresolved_transition() != 0 ||
                   test_pending_handler_can_return_to_title_or_reenter_field() !=
                       0
               ? 1
               : 0;
}
