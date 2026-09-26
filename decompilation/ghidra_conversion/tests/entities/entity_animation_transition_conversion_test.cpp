#include "shadowrun_ghidra/entities/entity_animation_transition_conversion.hpp"

#include <cassert>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    EntityAnimationEventTable events;
    collect_entity_animation_events(
        {{2U, 1U}, {0U, 4U}, {0x0BU, 9U}, {7U, 6U}, {0x0BU, 10U}},
        events);
    assert(events.terminated && events.accepted_events.size() == 2U);
    assert(events.accepted_events[0].type == 0U &&
           events.accepted_events[1].type == 7U);
    assert(events.special_argument == 10U);

    EntityAnimationRecordState state{0U, 3, 4U, 2U, 0xFEU, 8U};
    assert(evaluate_entity_animation_rate(
               state, 0x80U,
               [](const std::int16_t argument, const std::int16_t code) {
                   assert(argument == -2 && code == -128);
                   return static_cast<std::int16_t>(2);
               })
               .source_carry);
    assert(!evaluate_entity_animation_rate(
                state, 2U,
                [](const std::int16_t, const std::int16_t) {
                    return static_cast<std::int16_t>(1);
                })
                .source_carry);
    state.rate_threshold = 1U;
    assert(evaluate_entity_animation_rate_from_phase(state, {}).source_carry);

    std::vector<int> service_order;
    EntityAnimationRecordState special_state{0U, 4, 1U, 2U, 0x12U, 5U};
    EntityAnimationTransitionCallbacks transition_callbacks{
        [](const std::int16_t, const std::int16_t) {
            return static_cast<std::int16_t>(0);
        },
        [&] {
            service_order.push_back(1);
            return true;
        },
        {},
        [&]() -> std::optional<std::uint8_t> {
            service_order.push_back(2);
            return 0x44U;
        },
        [&](const std::uint16_t mode) {
            assert(mode == 0U);
            service_order.push_back(3);
        },
        [&] { service_order.push_back(4); }};
    const auto special_result = run_entity_animation_transition(
        special_state,
        EntityAnimationEventTable{{}, static_cast<std::uint8_t>(7U), true},
        0U, 0U, transition_callbacks);
    assert(!special_result.source_carry && special_result.state_published);
    assert(special_state.frame == -1 && special_state.phase == 0x0CU &&
           special_state.event_argument == 7U &&
           special_state.auxiliary_marker == 0x44U);
    assert((service_order == std::vector<int>{1, 2, 3, 4}));

    EntityAnimationRecordState accepted_state{0U, 0, 0U, 2U, 4U, 1U};
    int selected_calls = 0;
    const auto accepted_result = run_entity_animation_transition(
        accepted_state,
        EntityAnimationEventTable{{{1U, 8U}}, std::nullopt, true},
        0U, 0U,
        {[](const std::int16_t, const std::int16_t) {
             return static_cast<std::int16_t>(0);
         },
         {},
         [&](const std::size_t count) -> std::optional<std::size_t> {
             ++selected_calls;
             assert(count == 1U);
             return 0U;
         },
         {},
         {},
         {}});
    assert(!accepted_result.source_carry && selected_calls == 1 &&
           accepted_state.phase == 2U && accepted_state.event_argument == 8U);

    EntityAnimationRecordState blocked_state{0U, 0, 0U, 2U, 4U, 1U};
    const auto blocked_result = run_entity_animation_transition(
        blocked_state,
        EntityAnimationEventTable{{{8U, 2U}}, std::nullopt, true},
        0U, 0U,
        {[](const std::int16_t, const std::int16_t) {
             return static_cast<std::int16_t>(2);
         },
         {},
         [](const std::size_t) -> std::optional<std::size_t> {
             return 0U;
         },
         {},
         {},
         {}});
    assert(blocked_result.source_carry && !blocked_result.state_published &&
           blocked_state.phase == 0U);

    EntityAnimationRecordState check_state{0U, -1, 1U, 0U, 0U, 0U};
    assert(check_entity_animation_transition(
        check_state,
        EntityAnimationEventTable{{}, std::nullopt, true},
        [] { return true; }));
    assert(!check_entity_animation_transition(
        check_state,
        EntityAnimationEventTable{{}, static_cast<std::uint8_t>(3U), true},
        [] { return true; }));
    assert(check_entity_animation_transition(
        check_state,
        EntityAnimationEventTable{{}, static_cast<std::uint8_t>(3U), true},
        [] { return false; }));
    check_state.phase = 0x0CU;
    assert(check_entity_animation_transition(
        check_state, EntityAnimationEventTable{}, [] { return true; }));
    assert(!check_entity_animation_transition(
        check_state, EntityAnimationEventTable{}, [] { return false; }));

    EntityAnimationRecordState branch_state{0U, -1, 1U, 0U, 0U, 0U};
    int advanced = 0;
    const auto branch_result = run_entity_animation_branch(
        branch_state, EntityAnimationEventTable{}, 0U, 0U,
        {{[](const std::int16_t, const std::int16_t) {
              return static_cast<std::int16_t>(0);
          },
          [] { return true; },
          {},
          {},
          {},
          {}},
         [&] { ++advanced; }});
    assert(branch_result.entered && branch_result.rate_checked &&
           branch_result.action_threshold_advanced && advanced == 1);

    EntityAnimationRecordState transition_branch{0U, 0, 0U, 5U, 0U, 0U};
    const auto transition_branch_result = run_entity_animation_branch(
        transition_branch,
        EntityAnimationEventTable{{{0U, 5U}}, std::nullopt, true}, 0U, 0U,
        {{[](const std::int16_t, const std::int16_t) {
              return static_cast<std::int16_t>(0);
          },
          {},
          [](const std::size_t) -> std::optional<std::size_t> {
              return 0U;
          },
          {},
          {},
          {}},
         [&] { ++advanced; }});
    assert(transition_branch_result.transition_called &&
           transition_branch_result.transition_published &&
           transition_branch.phase == 1U);

    EntityAnimationEntryState entry{1U, false, true, true, -1};
    std::vector<int> entry_order;
    const auto waiting = run_entity_animation_entry(
        entry, {[&] { entry_order.push_back(1); },
                [&] { entry_order.push_back(2); },
                [&] { entry_order.push_back(3); }});
    assert(waiting.action == EntityAnimationEntryAction::waiting_for_timer &&
           entry_order.empty());
    const auto resumed = run_entity_animation_entry(
        entry, {[&] { entry_order.push_back(1); },
                [&] { entry_order.push_back(2); },
                [&] { entry_order.push_back(3); }});
    assert(resumed.action == EntityAnimationEntryAction::continue_entity_update &&
           resumed.transition_requested && entry.timer == 0x14U &&
           (entry_order == std::vector<int>{2, 3}));
    entry.timer = 0U;
    const auto suppressed = run_entity_animation_entry(
        entry, {[&] { entry_order.push_back(4); }, {}, {}});
    assert(suppressed.action == EntityAnimationEntryAction::suppressed_by_frame_lock);

    EntityAnimationEntryState fallback{0U, false, false, true, 0};
    int fallback_calls = 0;
    const auto fallback_result = run_entity_animation_entry(
        fallback, {[&] { ++fallback_calls; }, {}, {}});
    assert(fallback_result.action == EntityAnimationEntryAction::fallback_update &&
           fallback_calls == 1 && fallback.timer == 0x14U);
    return 0;
}
