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

    std::array<std::uint8_t, 6> finishing_flags{
        0x02U, 0x00U, 0x06U, 0x01U, 0x04U, 0x00U};
    WorldEffectTriggerState finishing_state{};
    finishing_state.transition.transition_latched = true;
    finishing_state.transition.transition_timer = 0;
    bool finish_sound = false;
    const auto finished = advance_world_effect_trigger(
        finishing_state,
        finishing_flags,
        {},
        {},
        wrong_mode_timeout,
        {},
        [&finish_sound]() { finish_sound = true; },
        {},
        {});
    if (expect(finished == WorldEffectTriggerAction::transition_finished &&
               !finishing_state.transition.transition_latched && finish_sound &&
               finishing_flags ==
                   std::array<std::uint8_t, 6>{0x06U, 0x00U, 0x06U,
                                               0x01U, 0x04U, 0x00U}) != 0) {
        return 10;
    }

    WorldEffectTriggerState gated_state{};
    gated_state.frame_counter = 0x000DU;
    gated_state.trigger_suppressed = true;
    if (expect(advance_world_effect_trigger(
                   gated_state, finishing_flags, {}, {}, wrong_mode_timeout,
                   {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::trigger_suppressed) != 0) {
        return 11;
    }

    WorldEffectTriggerState waiting_state{};
    waiting_state.frame_counter = 0x000CU;
    if (expect(advance_world_effect_trigger(
                   waiting_state, finishing_flags, {}, []() { return 0U; },
                   wrong_mode_timeout, {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::cadence_wait) != 0) {
        return 12;
    }

    WorldEffectTriggerState rejected_state{};
    rejected_state.frame_counter = 0x000DU;
    if (expect(advance_world_effect_trigger(
                   rejected_state, finishing_flags, {}, []() { return 0x8000U; },
                   wrong_mode_timeout, {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::random_rejected) != 0) {
        return 13;
    }

    WorldEffectTriggerState matching_state{};
    matching_state.transition.world_mode = 2U;
    matching_state.frame_counter = 0x000DU;
    matching_state.player_fixed_x = 0x0120U;
    matching_state.player_fixed_y = 0x0220U;
    WorldActivityTimeoutState matching_timeout{};
    std::vector<char> trigger_events;
    const auto started = advance_world_effect_trigger(
        matching_state,
        finishing_flags,
        {{0x0008, 0x0014, 0x0010, 0x0028, false},
         {0, 0, 0, 0, true}},
        []() { return 0x0100U; },
        matching_timeout,
        [&trigger_events](const std::uint16_t sound_id) {
            trigger_events.push_back(sound_id == 0x30U ? 's' : 'x');
        },
        {},
        [&trigger_events]() { trigger_events.push_back('c'); },
        [&trigger_events]() { trigger_events.push_back('e'); });
    if (expect(started == WorldEffectTriggerAction::transition_started &&
               matching_state.transition.transition_latched &&
               matching_state.transition.transition_timer == 0x0960U &&
               trigger_events == std::vector<char>({'s', 'c', 'e'})) != 0) {
        return 14;
    }

    WorldEffectTriggerState outside_state = matching_state;
    outside_state.transition.transition_latched = false;
    outside_state.frame_counter = 0x000DU;
    outside_state.player_fixed_x = 0x0500U;
    if (expect(advance_world_effect_trigger(
                   outside_state, finishing_flags,
                   {{0x0008, 0x0014, 0x0010, 0x0028, false}},
                   []() { return 0x0100U; }, matching_timeout, {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::no_matching_region) != 0) {
        return 15;
    }

    return 0;
}
