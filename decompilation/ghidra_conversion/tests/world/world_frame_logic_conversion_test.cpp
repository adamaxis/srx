#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"

#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldFrameLogicState state{};
    state.scene_state = 2U;
    state.transition_marker = -1;
    state.region_or_mode = 9;
    state.activity_countdown = 1U;
    state.resource_countdown = 2U;
    state.world_active = 1U;
    state.event_lock = 1U;
    state.world_mode = 2U;
    state.current_target_present = true;
    state.current_target_active = true;
    state.current_entity_special = true;
    state.current_entity_mode = 0x0BU;
    state.current_target_effect_locked = true;
    std::vector<int> order;
    advance_world_frame_logic(
        state,
        { [&] { order.push_back(1); },
          [&] { order.push_back(2); },
          [&] { order.push_back(3); },
          [&] { order.push_back(4); },
          [&] { order.push_back(5); },
          [&] { order.push_back(6); },
          [&] { order.push_back(7); },
          [&] { order.push_back(8); },
          [&] { order.push_back(9); } });
    if (state.scene_state != 3U || state.transition_marker != 0 ||
        state.resource_countdown != 0x3BU || state.activity_handler_token != 0xC64CU ||
        state.activity_countdown != 0U || !state.activity_started ||
        !state.target_selection_requested || state.service_records_reset ||
        order != std::vector<int>{1, 2, 3, 5, 6, 7, 8, 9}) {
        return 1;
    }

    // The low-mode cleanup branch does not select a target; it decrements the
    // cleanup timer and resets service records only after it crosses below 0.
    state = {};
    state.world_active = 1U;
    state.scene_state = 0U;
    state.world_mode = 0U;
    state.cleanup_countdown = 0;
    order.clear();
    advance_world_frame_logic(
        state,
        { {}, {}, [&] { order.push_back(3); }, [&] { order.push_back(4); },
          [&] { order.push_back(5); }, {}, {}, [&] { order.push_back(8); },
          [&] { order.push_back(9); } });
    if (!state.service_records_reset || state.target_selection_requested ||
        order != std::vector<int>{4, 5, 8, 9}) {
        return 2;
    }

    // A non-active world skips target/cleanup selection but still services the
    // downstream effect sequence in order.
    state = {};
    order.clear();
    advance_world_frame_logic(
        state,
        { {}, {}, {}, {}, [&] { order.push_back(5); }, {}, {},
          [&] { order.push_back(8); }, [&] { order.push_back(9); } });
    if (!order.empty() && order != std::vector<int>{5, 8, 9}) {
        return 3;
    }
    return 0;
}
