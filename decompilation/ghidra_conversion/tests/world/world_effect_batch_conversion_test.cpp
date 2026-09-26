#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    MenuOffsetPreparationState offset{};
    int initializations = 0;
    prepare_menu_offset_once(offset, [] { return 42; }, [&] { ++initializations; });
    prepare_menu_offset_once(offset, [] { return 99; }, [&] { ++initializations; });
    if (!offset.prepared || offset.offset != 42 || initializations != 1) {
        return 1;
    }

    std::vector<std::uint16_t> source(12, 7);
    std::vector<std::uint16_t> destination;
    copy_seven_words(source, destination);
    if (destination.size() != 7 || destination.back() != 7) {
        return 2;
    }
    copy_eleven_words(source, destination);
    if (destination.size() != 11) {
        return 3;
    }

    WorldEffectMotionState motion{};
    int audio = 0;
    initialize_world_effect_motion(motion, true, [&] { ++audio; });
    if (motion.horizontal != -18 || motion.origin_x != 0x1340 ||
        motion.origin_y != 0x1020 || audio != 1) {
        return 4;
    }

    std::array<std::uint32_t, 7> work{1, 2, 3, 4, 5, 6, 7};
    clear_world_effect_work(false, work);
    if (work[0] != 0 || work[6] != 0) {
        return 5;
    }
    if (advance_effect_vertical_position(0xF3) != 0xF5 ||
        retreat_effect_vertical_position(0xB0) != 0xAE ||
        advance_effect_horizontal_position(0x197) != 0x199 ||
        retreat_effect_horizontal_position(0xA8) != 0xA6) {
        return 6;
    }

    int forwarded = 0;
    forward_effect_sequence([&] { ++forwarded; });
    if (forwarded != 1) {
        return 7;
    }

    WorldActivityState activity{};
    if (!begin_world_activity_if_idle(activity, 12) || activity.timeout != 0x3C ||
        begin_world_activity_if_idle(activity, 13)) {
        return 8;
    }
    begin_world_activity(activity, 14);
    if (activity.source_value != 14) {
        return 9;
    }

    WorldActivityRequestState activity_request{};
    activity_request.request_pending = true;
    activity_request.reset_a = 4U;
    activity_request.reset_b = 5U;
    activity_request.reset_c = 6U;
    activity_request.reset_d = 7U;
    int activity_starts = 0;
    const std::vector<WorldActivityRequestRecord> request_records =
        {{false, false}, {true, false}, {true, true}};
    if (handle_world_activity_request(
            activity_request, request_records, [&] { ++activity_starts; }) !=
            WorldActivityRequestAction::activity_started ||
        activity_request.request_pending || activity_request.availability != 0 ||
        activity_request.activity_handler_token != 0x1F16AU ||
        activity_request.reset_a != 0U || activity_request.reset_b != 0U ||
        activity_request.reset_c != 0U || activity_request.reset_d != 0U ||
        activity_starts != 1) {
        return 10;
    }

    WorldEffectCandidateState candidates;
    build_world_effect_candidates(candidates, 10, 20, 30, 40, {});
    if (candidates.candidates.size() != 10 || candidates.candidates[0] !=
        std::pair<std::int16_t, std::int16_t>{10, 276}) {
        return 10;
    }
    std::vector<std::uint8_t> tiles(20, 9);
    if (lookup_world_effect_tile(tiles, 0, 0).value_or(0) != 9 ||
        lookup_world_effect_tile({}, 0, 0).has_value()) {
        return 11;
    }

    EffectSlotState effect{0x80, 0x08, true};
    int clears = 0;
    if (clear_effect_slot_state(effect, [&] { ++clears; }) || clears != 0) {
        return 12;
    }
    effect.entity_flags = 0;
    if (!clear_effect_slot_state(effect, [&] { ++clears; }) || clears != 1) {
        return 13;
    }

    EffectFadeMarkerState marker{};
    initialize_effect_fade_marker(marker);
    if (marker.marker != -1 || marker.countdown != 3) {
        return 14;
    }
    WorldEffectGlobals globals{3, 7};
    reset_world_effect_globals(globals);
    if (globals.selected_effect != -1 || globals.collision_flags != 0) {
        return 15;
    }

    WorldEffectDispatchState dispatch{4, 0, 2};
    int handled = 0;
    const std::array<std::function<void()>, 6> handlers{{
        {}, {}, [&] { ++handled; }, {}, {}, {}}};
    if (!dispatch_world_effect(
            dispatch,
            handlers) || handled != 1 ||
        dispatch.selected_effect != 0) {
        return 16;
    }
    dispatch.input_effect = 6;
    if (dispatch_world_effect(dispatch, {})) {
        return 17;
    }
    if (advance_eight_step_counter(7) != 8 ||
        advance_eight_step_counter(8) != 8) {
        return 18;
    }
    std::uint32_t mask = 0;
    mark_effect_bit(mask, 33);
    if (mask != 2U) {
        return 19;
    }
    return 0;
}
