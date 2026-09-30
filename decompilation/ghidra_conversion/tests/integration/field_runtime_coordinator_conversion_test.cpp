#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion::integration;

struct Observations {
    std::vector<std::string> order;
    std::uint16_t sampled_word = 0;
};

FieldRuntimeCoordinatorCallbacks make_callbacks(
    Observations& observations) {
    FieldRuntimeCoordinatorCallbacks callbacks{};
    callbacks.wait_for_vblank = [&] {
        observations.order.emplace_back("vblank");
    };
    callbacks.handle_field_transition_request = [&](auto&) {
        observations.order.emplace_back("field_transition");
    };
    callbacks.handle_world_activity_request = [&](auto&) {
        observations.order.emplace_back("world_activity");
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
        assert(command == 8U);
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

void test_normal_frame_order_and_dirty_bits() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;

    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::frame_waiting);
    const std::vector<std::string> expected{
        "vblank", "latch", "sample", "behavior", "camera", "render",
        "palette", "world_frame"};
    assert(observations.order == expected);
    assert(state.display_dirty_flags == 0x0EU);
}

void test_activity_gate_blocks_input_dispatch() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.field_activity_lock = true;
    observations.sampled_word = 0xA000U;

    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::frame_waiting);
    assert(!state.action_event_dispatched &&
           !state.command_overview_dispatched);
    assert(observations.order.size() == 8U);
}

void test_action_precedes_command_overview_without_excluding_it() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    observations.sampled_word = 0xA000U;

    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::command_overview_dispatched);
    assert(state.action_event_dispatched &&
           state.command_overview_dispatched);
    assert((observations.order[8] == "action") &&
           (observations.order[9] == "audio_fixed") &&
           (observations.order[10] == "command_overview") &&
           (observations.order[11] == "audio_tick") &&
           (observations.order[12] == "select_entity") &&
           (observations.order[13] == "scene_transition"));
}

void test_request_branches_precede_normal_frame() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.field_transition_requested = true;
    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::field_transition_dispatched);
    assert(!state.field_transition_requested);
    assert(observations.order.size() == 2U);
    assert(observations.order[1] == "field_transition");

    observations.order.clear();
    state.world_activity_requested = true;
    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::world_activity_dispatched);
    assert(!state.world_activity_requested);
    assert(observations.order.size() == 2U);
    assert(observations.order[1] == "world_activity");

    observations.order.clear();
    state.field_transition_requested = true;
    callbacks.handle_field_transition_request = {};
    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::field_transition_pending);
    assert(state.field_transition_requested);
    assert(observations.order.size() == 1U);
}

void test_pending_handler_can_return_to_title_or_reenter_field() {
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldRuntimeCoordinatorState state;
    state.pending_field_scene_handler = true;
    callbacks.run_pending_field_scene_handler = [&] {
        observations.order.emplace_back("pending_handler");
        return PendingFieldSceneHandlerResult::returned_to_title;
    };
    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::returned_to_title);
    assert(!state.pending_field_scene_handler);

    observations.order.clear();
    state.pending_field_scene_handler = true;
    state.display_service_word = 0U;
    callbacks.run_pending_field_scene_handler = [&] {
        observations.order.emplace_back("pending_handler");
        return PendingFieldSceneHandlerResult::resumed_field;
    };
    assert(advance_field_runtime_frame(state, callbacks) ==
           FieldRuntimeFrameResult::field_scene_reentry_requested);
    assert(!state.pending_field_scene_handler);
    assert(observations.order.back() == "scene_transition");
}

} // namespace

int main() {
    test_normal_frame_order_and_dirty_bits();
    test_activity_gate_blocks_input_dispatch();
    test_action_precedes_command_overview_without_excluding_it();
    test_request_branches_precede_normal_frame();
    test_pending_handler_can_return_to_title_or_reenter_field();
    return 0;
}
