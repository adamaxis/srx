#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldSceneEntryState state{0x1111U, 0x2222U, 0x3333U, 0x1234U, 0x2345U,
                               0x3456U, 0x4567U};
    std::vector<char> order;
    const FieldSceneEntryCallbacks callbacks{
        [&order] { order.push_back('d'); },
        [&order] { order.push_back('p'); },
        [&order] { order.push_back('s'); },
        [&order] { order.push_back('a'); },
        [&order] { order.push_back('r'); },
        [&order] { order.push_back('u'); },
        [&order] { order.push_back('g'); }};
    initialize_field_scene_entry(state, callbacks);
    if (!expect(order == std::vector<char>{'d', 'p', 's', 'a', 'r', 'u', 'g'} &&
                state.field_transition_marker == 0U &&
                state.scene_refresh_marker == 0xFFFFU &&
                state.pending_scene_marker == 0xFFFFU &&
                state.camera_x == 0x1234U)) {
        return 1;
    }

    order.clear();
    state.camera_x = 0x12FFU;
    state.camera_y = 0x23FFU;
    state.camera_transition_x = 0x1111U;
    state.camera_transition_y = 0x2222U;
    reinitialize_field_scene_entry(state, callbacks);
    if (!expect(order == std::vector<char>{'s', 'g', 'd', 'r', 'u'} &&
                state.scene_refresh_marker == 0xFFFFU &&
                state.camera_x == 0x1280U && state.camera_y == 0x2380U &&
                state.camera_transition_x == 0xFFFFU &&
                state.camera_transition_y == 0xFFFFU)) {
        return 2;
    }

    order.clear();
    FieldSceneTransitionState transition{0U, 0x1111U, 0x2222U, 0x12FFU,
                                         0x23FFU, 0x3333U, 0x4444U, 2U,
                                         true};
    std::vector<std::uint8_t> event_counters{0U, 0xA5U, 0xE1U};
    std::vector<std::uint8_t> recorded_events;
    FieldSceneEntryCallbacks transition_callbacks = callbacks;
    transition_callbacks.step_palette_transition =
        [&order] { order.push_back('t'); };
    transition_callbacks.record_pending_event =
        [&recorded_events](const std::uint8_t event_index) {
            recorded_events.push_back(event_index);
        };
    advance_field_scene_transition(transition, event_counters,
                                   transition_callbacks);
    if (!expect(order == std::vector<char>{'t', 's', 'g', 'r', 'u'} &&
                transition.camera_x == 0x1280U &&
                transition.camera_y == 0x2380U &&
                transition.pending_event_index == 0U &&
                event_counters[2] == 0xE1U &&
                recorded_events == std::vector<std::uint8_t>{2U})) {
        return 3;
    }

    order.clear();
    transition = FieldSceneTransitionState{7U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                           false};
    advance_field_scene_transition(transition, event_counters,
                                   transition_callbacks);
    if (!expect(order == std::vector<char>{'t', 'p', 's', 'a', 'r', 'u', 'g'} &&
                transition.palette_transition_marker == 0U &&
                transition.scene_refresh_marker == 0xFFFFU &&
                transition.pending_scene_marker == 0xFFFFU)) {
        return 4;
    }
    return 0;
}
