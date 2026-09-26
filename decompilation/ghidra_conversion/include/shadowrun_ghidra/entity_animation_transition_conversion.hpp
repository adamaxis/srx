#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationEvent {
    std::uint8_t type = 0;
    std::uint8_t argument = 0;
};

struct EntityAnimationEventTable {
    std::vector<EntityAnimationEvent> accepted_events;
    std::optional<std::uint8_t> special_argument;
    bool terminated = false;
};

// ROM 0x13C74: retain event types 0, 1, 7, and 8 for random selection;
// retain the last type-0x0B argument as the special event value. The source
// closes the temporary table with two 0xFFFF words; `terminated` records
// that boundary without exposing the original scratch address.
void collect_entity_animation_events(
    const std::vector<EntityAnimationEvent>& source,
    EntityAnimationEventTable& destination);

struct EntityAnimationRecordState {
    std::uint8_t branch_mode = 0;      // record +0x9B
    std::int8_t frame = 0;             // record +0x56
    std::uint8_t phase = 0;            // record +0x57
    std::uint8_t rate_threshold = 0;   // record +0x9C
    std::uint8_t event_argument = 0;   // record +0x9D
    std::uint8_t auxiliary_marker = 0; // record +0x4E
};

using EntityAnimationRateCalculator =
    std::function<std::int16_t(std::int16_t event_argument,
                               std::int16_t event_code)>;

struct EntityAnimationRateDecision {
    // This is the source CCR carry from 0x13CDE: true means the calculated
    // rate reached the record threshold (or phase one took the bypass path).
    bool source_carry = false;
    std::int16_t residual = 0;
};

// ROM 0x13CDE: compare the signed +0x9D event argument and signed event code
// through the existing rate calculator, preserving the source's phase-one
// bypass and signed residual test.
EntityAnimationRateDecision evaluate_entity_animation_rate(
    const EntityAnimationRecordState& state,
    std::uint8_t event_type,
    const EntityAnimationRateCalculator& calculate_rate);

// ROM 0x13CD4: the same gate entered with (record +0x57) - 1 as its event
// code, used by the negative-frame branch at 0x13A92.
EntityAnimationRateDecision evaluate_entity_animation_rate_from_phase(
    const EntityAnimationRecordState& state,
    const EntityAnimationRateCalculator& calculate_rate);

using EntityAnimationLaneSelector = std::function<bool()>;
using EntityAnimationEventSelector =
    std::function<std::optional<std::size_t>(std::size_t event_count)>;
using EntityAnimationAuxiliaryClearer =
    std::function<std::optional<std::uint8_t>()>;
using EntityAnimationDelayReset = std::function<void(std::uint16_t mode)>;
using EntityAnimationAssetRefresh = std::function<void()>;

struct EntityAnimationTransitionCallbacks {
    EntityAnimationRateCalculator calculate_rate;
    EntityAnimationLaneSelector select_lane;
    EntityAnimationEventSelector select_event;
    EntityAnimationAuxiliaryClearer clear_auxiliary_marker;
    EntityAnimationDelayReset reset_delay;
    EntityAnimationAssetRefresh refresh_assets;
};

struct EntityAnimationTransitionResult {
    // 0x13B12 leaves with carry set when it did not publish a new event.
    bool source_carry = true;
    bool special_event_considered = false;
    bool accepted_event_considered = false;
    bool state_published = false;
    std::optional<EntityAnimationEvent> selected_event;
};

// ROM 0x13B12: prefer the special type-0x0B event when its rate is below the
// threshold and a lane is available; otherwise choose one accepted event,
// apply its rate gate, and publish the phase/frame/argument fields.
EntityAnimationTransitionResult run_entity_animation_transition(
    EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    std::uint8_t inherited_event_type,
    std::uint8_t inherited_event_argument,
    const EntityAnimationTransitionCallbacks& callbacks);

// ROM 0x13BBA: return the source carry decision for the current phase. The
// result is true when the caller proceeds to the phase-rate check or phase-12
// transition path, and false when a currently available special lane blocks
// that path.
bool check_entity_animation_transition(
    const EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    const EntityAnimationLaneSelector& select_lane);

struct EntityAnimationBranchCallbacks {
    EntityAnimationTransitionCallbacks transition;
    std::function<void()> advance_action_threshold;
};

struct EntityAnimationBranchResult {
    bool entered = false;
    bool transition_called = false;
    bool rate_checked = false;
    bool action_threshold_advanced = false;
    bool transition_published = false;
};

// ROM 0x13A92: gate the entity animation branch by +0x9B, handle the
// negative-frame lane/rate path, then run 0x13B12 for thresholds at least five.
EntityAnimationBranchResult run_entity_animation_branch(
    EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    std::uint8_t inherited_event_type,
    std::uint8_t inherited_event_argument,
    const EntityAnimationBranchCallbacks& callbacks);

struct EntityAnimationEntryState {
    std::uint16_t timer = 0;                 // record +0xB6
    bool global_frame_lock = false;          // FF E175
    bool record_animation_enabled = false;   // record +0x4D
    bool global_animation_enabled = false;   // FF DF84
    std::int16_t transition_marker = 0;      // record +0xDC
};

enum class EntityAnimationEntryAction {
    waiting_for_timer,
    suppressed_by_frame_lock,
    fallback_update,
    continue_entity_update,
};

struct EntityAnimationEntryCallbacks {
    std::function<void()> fallback_update;
    std::function<void()> animation_transition;
    std::function<void()> continue_entity_update;
};

struct EntityAnimationEntryResult {
    EntityAnimationEntryAction action =
        EntityAnimationEntryAction::waiting_for_timer;
    bool timer_expired = false;
    bool timer_rearmed = false;
    bool transition_requested = false;
};

// ROM 0x138B8: decrement the animation timer, take the global frame lock,
// route unprepared records to 0x14508, and otherwise enter 0x13A92 only for a
// negative transition marker before continuing through the surrounding update.
EntityAnimationEntryResult run_entity_animation_entry(
    EntityAnimationEntryState& state,
    const EntityAnimationEntryCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
