#include "shadowrun_ghidra/title_intro/title_effect_motion_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (resolve_title_effect_motion_sprite_data(0U, 0U, 0x0100U) !=
            0x1D7EAU ||
        resolve_title_effect_motion_sprite_data(0U, 0U, 0x0120U) !=
            0x1D772U ||
        resolve_title_effect_motion_sprite_data(1U, 0U, 0x0100U) !=
            0x1D772U ||
        resolve_title_effect_motion_sprite_data(1U, 3U, 0x0120U) !=
            0x1D838U ||
        resolve_title_effect_motion_sprite_data(1U, 12U, 0x0100U)
            .has_value()) {
        return 1;
    }

    TitleEffectMotionState state{};
    state.frame_delay = 0U;
    state.position_x = 0x100U;
    state.position_y = 0x200U;
    state.effect_mode = 1U;
    int resolves = 0;
    int builds = 0;
    TitleEffectMotionCallbacks callbacks{};
    callbacks.resolve_sprite_data =
        [&](const std::uint16_t mode, const std::uint16_t frame,
            const std::uint16_t x) -> std::optional<std::uint32_t> {
        if (mode != 1U || frame != 1U || x != 0x100U) {
            resolves += 100;
        }
        ++resolves;
        return 0x4321U;
    };
    callbacks.build_sprite =
        [&](const std::uint32_t token, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
        if (token != 0x4321U || x != 0x100U || y != 0x200U ||
            flags != 0x2000U) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_effect_motion(state, callbacks) !=
            TitleEffectMotionAction::rendered ||
        state.frame_delay != 3U || state.frame != 1U || resolves != 1 ||
        builds != 1) {
        return 2;
    }

    TitleEffectMotionState finish{};
    finish.frame_delay = 0U;
    finish.frame = 0x0BU;
    finish.position_x = 0x011FU;
    finish.transition_state = 0x1234U;
    finish.menu_state = 0x5678U;
    int published = 0;
    int serviced = 0;
    callbacks = {};
    callbacks.publish_behavior_descriptor =
        [&](const TitleEffectMotionRecordSlot slot,
            const std::uint32_t descriptor) {
        published += slot == TitleEffectMotionRecordSlot::secondary &&
                             descriptor == 0x1C288U
                         ? 1
                         : 100;
    };
    callbacks.run_menu_selection_service = [&] { ++serviced; };
    if (advance_title_effect_motion(finish, callbacks) !=
            TitleEffectMotionAction::finalized ||
        finish.secondary_behavior_descriptor != 0x1C288U ||
        finish.secondary_stage != 0U ||
        finish.transition_state != 0U || finish.menu_state != 0U ||
        published != 1 || serviced != 1) {
        return 3;
    }

    TitleEffectMotionState extracted{};
    extracted.frame_delay = 0U;
    extracted.position_x = 0x0100U;
    extracted.position_y = 0x0200U;
    extracted.effect_mode = 0U;
    int extracted_builds = 0;
    TitleEffectMotionCallbacks extracted_callbacks{};
    extracted_callbacks.build_sprite =
        [&](const std::uint32_t token, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
            if (token != 0x1D804U || x != 0x0100U || y != 0x0200U ||
            flags != 0x2000U) {
            extracted_builds += 100;
        }
        ++extracted_builds;
    };
    if (advance_title_effect_motion(extracted, extracted_callbacks) !=
            TitleEffectMotionAction::rendered ||
        extracted.frame != 1U || extracted_builds != 1) {
        return 4;
    }

    TitleEffectMotionState right_finish{};
    right_finish.frame_delay = 0U;
    right_finish.frame = 0x0BU;
    right_finish.position_x = 0x0120U;
    callbacks = {};
    callbacks.publish_behavior_descriptor =
        [&](const TitleEffectMotionRecordSlot slot,
            const std::uint32_t descriptor) {
        published += slot == TitleEffectMotionRecordSlot::primary &&
                             descriptor == 0x1C28EU
                         ? 1
                         : 100;
    };
    if (advance_title_effect_motion(right_finish, callbacks) !=
            TitleEffectMotionAction::finalized ||
        right_finish.primary_behavior_descriptor != 0x1C28EU ||
        right_finish.primary_stage != 0U || published != 2) {
        return 5;
    }
    return 0;
}
