#include "shadowrun_ghidra/world_effect_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldActivityTimeoutState guarded{0x0036U, false, 0, 0, false};
    if (expect(!schedule_world_activity_timeout(guarded) &&
               guarded.timer_marker == 0 && guarded.timeout == 0) != 0) {
        return 1;
    }

    WorldActivityTimeoutState short_timeout{0, true, 0, 0, false};
    if (expect(schedule_world_activity_timeout(short_timeout) &&
               short_timeout.timer_marker == -1 &&
               short_timeout.timeout == 0x0078U) != 0) {
        return 2;
    }

    WorldActivityTimeoutState long_timeout{0, true, 4, 0, true};
    if (expect(schedule_world_activity_timeout(long_timeout) &&
               long_timeout.timer_marker == 4 &&
               long_timeout.timeout == 0x0258U) != 0) {
        return 3;
    }

    std::array<std::uint8_t, 6> effect_flags{
        0xFFU, 0x00U, 0x01U, 0x02U, 0x05U, 0x07U};
    clear_entity_effect_pending_flags(effect_flags);
    if (expect(effect_flags ==
               std::array<std::uint8_t, 6>{0xFEU, 0x00U, 0x00U,
                                           0x02U, 0x04U, 0x06U}) != 0) {
        return 4;
    }

    promote_entity_effect_flags(effect_flags);
    if (expect(effect_flags ==
               std::array<std::uint8_t, 6>{0xFEU, 0x00U, 0x00U,
                                           0x06U, 0x04U, 0x06U}) != 0) {
        return 5;
    }

    WorldEffectTransitionState scene_timeout{0x0014U, 2U, 7U, false};
    WorldActivityTimeoutState scene_timeout_state{0, true, 0, 0, false};
    std::vector<char> scene_events;
    const auto timeout_action = begin_world_effect_transition(
        scene_timeout,
        scene_timeout_state,
        [&scene_events](const std::uint16_t) { scene_events.push_back('s'); },
        [&scene_events]() { scene_events.push_back('c'); },
        [&scene_events]() { scene_events.push_back('e'); });
    if (expect(timeout_action ==
               WorldEffectTransitionAction::timeout_scheduled &&
               scene_timeout.transition_timer == 7U &&
               scene_timeout_state.timer_marker == -1 &&
               scene_timeout_state.timeout == 0x0078U &&
               scene_events.empty()) != 0) {
        return 6;
    }

    WorldEffectTransitionState start_transition{0, 2U, 0, false};
    WorldActivityTimeoutState start_timeout_state{};
    scene_events.clear();
    const auto start_action = begin_world_effect_transition(
        start_transition,
        start_timeout_state,
        [&scene_events](const std::uint16_t sound_id) {
            scene_events.push_back(sound_id == 0x30U ? 's' : 'x');
        },
        [&scene_events]() { scene_events.push_back('c'); },
        [&scene_events]() { scene_events.push_back('e'); });
    if (expect(start_action == WorldEffectTransitionAction::started &&
               start_transition.transition_timer == 0x0960U &&
               start_transition.transition_latched &&
               scene_events == std::vector<char>({'s', 'c', 'e'})) != 0) {
        return 7;
    }

    scene_events.clear();
    const auto latched_action = begin_world_effect_transition(
        start_transition,
        start_timeout_state,
        [&scene_events](const std::uint16_t) { scene_events.push_back('s'); },
        [&scene_events]() { scene_events.push_back('c'); },
        [&scene_events]() { scene_events.push_back('e'); });
    if (expect(latched_action == WorldEffectTransitionAction::already_latched &&
               scene_events.empty()) != 0) {
        return 8;
    }

    WorldEffectTransitionState wrong_mode{0, 1U, 5U, false};
    WorldActivityTimeoutState wrong_mode_timeout{};
    if (expect(begin_world_effect_transition(wrong_mode, wrong_mode_timeout, {}, {}, {}) ==
                   WorldEffectTransitionAction::no_action &&
               wrong_mode.transition_timer == 5U) != 0) {
        return 9;
    }

    return 0;
}
