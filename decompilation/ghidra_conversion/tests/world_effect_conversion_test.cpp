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

    std::array<WorldEffectGuardEntity, 13> guard_entities{};
    bool spawn_requested = false;
    if (expect(guard_world_effect_player_spawn(
                   false, guard_entities,
                   [&spawn_requested] { spawn_requested = true; }) ==
                   WorldEffectPlayerSpawnGuardAction::no_pending_effect &&
               !spawn_requested) != 0) {
        return 1;
    }

    guard_entities[4] = {0x01U, 0x80U};
    if (expect(guard_world_effect_player_spawn(
                   true, guard_entities,
                   [&spawn_requested] { spawn_requested = true; }) ==
                   WorldEffectPlayerSpawnGuardAction::existing_effect_found &&
               !spawn_requested) != 0) {
        return 2;
    }

    guard_entities[4] = {0x01U, 0x00U};
    if (expect(guard_world_effect_player_spawn(
                   true, guard_entities,
                   [&spawn_requested] { spawn_requested = true; }) ==
                   WorldEffectPlayerSpawnGuardAction::spawn_requested &&
               spawn_requested) != 0) {
        return 3;
    }

    WorldActivityTimeoutState guarded{0x0036U, false, 0, 0, false};
    if (expect(!schedule_world_activity_timeout(guarded) &&
               guarded.timer_marker == 0 && guarded.timeout == 0) != 0) {
        return 4;
    }

    WorldActivityTimeoutState short_timeout{0, true, 0, 0, false};
    if (expect(schedule_world_activity_timeout(short_timeout) &&
               short_timeout.timer_marker == -1 &&
               short_timeout.timeout == 0x0078U) != 0) {
        return 5;
    }

    WorldActivityTimeoutState long_timeout{0, true, 4, 0, true};
    if (expect(schedule_world_activity_timeout(long_timeout) &&
               long_timeout.timer_marker == 4 &&
               long_timeout.timeout == 0x0258U) != 0) {
        return 6;
    }

    std::array<std::uint8_t, 6> effect_flags{
        0xFFU, 0x00U, 0x01U, 0x02U, 0x05U, 0x07U};
    clear_entity_effect_pending_flags(effect_flags);
    if (expect(effect_flags ==
               std::array<std::uint8_t, 6>{0xFEU, 0x00U, 0x00U,
                                           0x02U, 0x04U, 0x06U}) != 0) {
        return 7;
    }

    promote_entity_effect_flags(effect_flags);
    if (expect(effect_flags ==
               std::array<std::uint8_t, 6>{0xFEU, 0x00U, 0x00U,
                                           0x06U, 0x04U, 0x06U}) != 0) {
        return 8;
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
        return 9;
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
        return 10;
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
        return 11;
    }

    WorldEffectTransitionState wrong_mode{0, 1U, 5U, false};
    WorldActivityTimeoutState wrong_mode_timeout{};
    if (expect(begin_world_effect_transition(wrong_mode, wrong_mode_timeout, {}, {}, {}) ==
                   WorldEffectTransitionAction::no_action &&
               wrong_mode.transition_timer == 5U) != 0) {
        return 12;
    }

    if (expect(find_first_available_world_effect_entity_slot(
                   std::array<std::uint8_t, 7>{1U, 1U, 0U, 1U, 1U, 1U, 1U}) ==
                   5U &&
               find_first_available_world_effect_entity_slot(
                   std::array<std::uint8_t, 7>{1U, 1U, 1U, 1U, 1U, 1U, 1U}) ==
                   10U) != 0) {
        return 13;
    }

    std::vector<std::uint16_t> effect_entity_slots{3U, 4U, 5U};
    std::vector<std::uint16_t> effect_slots{7U, 8U, 9U};
    std::vector<std::uint16_t> initialized_templates;
    const auto spawned_effects = spawn_world_effect_entities(
        {{1U, 0x1111U, 0x2222U, false, false},
         {2U, 0x3333U, 0x4444U, false, true},
         {3U, 0x5555U, 0x6666U, false, false},
         {0U, 0U, 0U, true, false}},
        [&effect_entity_slots]() -> std::optional<std::uint16_t> {
            if (effect_entity_slots.empty()) {
                return std::nullopt;
            }
            const auto slot = effect_entity_slots.front();
            effect_entity_slots.erase(effect_entity_slots.begin());
            return slot;
        },
        [&effect_slots]() -> std::optional<std::uint16_t> {
            if (effect_slots.empty()) {
                return std::nullopt;
            }
            const auto slot = effect_slots.front();
            effect_slots.erase(effect_slots.begin());
            return slot;
        },
        [&initialized_templates](
            const WorldEffectSpawnDescriptor& descriptor,
            const std::uint16_t entity_slot,
            const std::uint16_t effect_slot,
            const std::uint16_t vertical_origin,
            const std::optional<std::uint16_t>& parent_slot) {
            initialized_templates.push_back(
                static_cast<std::uint16_t>(descriptor.template_key));
            return WorldEffectEntityRecord{
                entity_slot,
                effect_slot,
                vertical_origin,
                parent_slot,
                0x0100U,
                0x0200U,
                0U,
                false,
                false,
                false};
        },
        [](WorldEffectEntityRecord& entity) {
            return entity.entity_slot != 4U;
        });
    if (expect(spawned_effects.action ==
                   WorldEffectEntitySpawnAction::completed &&
               initialized_templates == std::vector<std::uint16_t>{1U, 2U, 3U} &&
               spawned_effects.entities.size() == 3U &&
               spawned_effects.entities[0].vertical_origin == 0x00C0U &&
               !spawned_effects.entities[0].parent_entity_slot.has_value() &&
               spawned_effects.entities[0].effect_active &&
               spawned_effects.entities[0].visible &&
               spawned_effects.entities[0].transition_code == 4U &&
               spawned_effects.entities[0].world_x == 0x1111U &&
               spawned_effects.entities[1].vertical_origin == 0U &&
               spawned_effects.entities[1].overlay_flag &&
               !spawned_effects.entities[1].visible &&
               spawned_effects.entities[2].vertical_origin == 0x00C0U &&
               spawned_effects.entities[2].parent_entity_slot == 4U &&
               spawned_effects.final_vertical_origin == 0x00C0) != 0) {
        return 14;
    }

    WorldEffectCoordinateSequenceState coordinate_state{
        {{0U, 0U}, {0x1111U, 0x2222U}}, {0x9000U, 0x9100U}, 0, 0};
    std::vector<char> coordinate_events;
    const auto coordinate_spawn = spawn_world_effect_from_coordinate_sequence(
        coordinate_state,
        []() { return std::optional<std::uint16_t>{6U}; },
        []() { return std::optional<std::uint16_t>{3U}; },
        [&coordinate_events](const std::uint16_t entity_slot,
                             const std::uint16_t effect_slot,
                             const std::uint16_t world_x,
                             const std::uint16_t world_y,
                             const bool special_record) {
            coordinate_events.push_back(
                entity_slot == 6U && effect_slot == 3U &&
                        world_x == 0x9000U && world_y == 0x9100U &&
                        special_record
                    ? 'i'
                    : 'x');
        },
        [&coordinate_events]() { coordinate_events.push_back('t'); });
    if (expect(coordinate_spawn.action ==
                   WorldEffectSingleSpawnAction::spawned &&
               coordinate_spawn.world_x == 0x9000U &&
               coordinate_spawn.world_y == 0x9100U &&
               coordinate_spawn.active_count == 1U &&
               coordinate_events == std::vector<char>({'i', 't'})) != 0) {
        return 15;
    }

    std::vector<std::uint16_t> player_slots{8U};
    const auto player_spawn = spawn_world_effect_at_player_position(
        0x1000U,
        0x2000U,
        [&player_slots]() -> std::optional<std::uint16_t> {
            if (player_slots.empty()) {
                return std::nullopt;
            }
            const auto slot = player_slots.front();
            player_slots.clear();
            return slot;
        },
        []() { return std::optional<std::uint16_t>{4U}; },
        [](const std::uint16_t,
           const std::uint16_t,
           const std::uint16_t,
           const std::uint16_t,
           const bool special_record) {
            if (!special_record) {
                throw 1;
            }
        });
    if (expect(player_spawn.action == WorldEffectSingleSpawnAction::spawned &&
               player_spawn.world_x == 0x1080U &&
               player_spawn.world_y == 0x2000U) != 0) {
        return 16;
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
        return 17;
    }

    WorldEffectTriggerState gated_state{};
    gated_state.frame_counter = 0x000DU;
    gated_state.trigger_suppressed = true;
    if (expect(advance_world_effect_trigger(
                   gated_state, finishing_flags, {}, {}, wrong_mode_timeout,
                   {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::trigger_suppressed) != 0) {
        return 18;
    }

    WorldEffectTriggerState waiting_state{};
    waiting_state.frame_counter = 0x000CU;
    if (expect(advance_world_effect_trigger(
                   waiting_state, finishing_flags, {}, []() { return 0U; },
                   wrong_mode_timeout, {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::cadence_wait) != 0) {
        return 19;
    }

    WorldEffectTriggerState rejected_state{};
    rejected_state.frame_counter = 0x000DU;
    if (expect(advance_world_effect_trigger(
                   rejected_state, finishing_flags, {}, []() { return 0x8000U; },
                   wrong_mode_timeout, {}, {}, {}, {}) ==
                   WorldEffectTriggerAction::random_rejected) != 0) {
        return 20;
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
        return 21;
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
        return 19;
    }

    return 0;
}
