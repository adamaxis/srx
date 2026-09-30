#include "shadowrun_ghidra/title_intro/title_profile_transition_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleProfileTransitionState transition{};
    transition.profile_selector = 2U;
    transition.countdown = 1U;
    transition.behavior_flags = 0xFFU;
    int builds = 0;
    TitleProfileTransitionCallbacks transition_callbacks{};
    transition_callbacks.build_sprite =
        [&](const TitleProfileTransitionSpriteBuild& build) {
            if (build.source_table_token != 0x1C136U || build.phase != 0U ||
                build.y != 0xEEU || build.build_flags != 0x2000U ||
                build.sprite_data_token != 0x1D8C6U ||
                (build.x != 0x15EU && build.x != 0xE0U)) {
                builds += 100;
            } else {
                ++builds;
            }
        };
    if (advance_title_profile_transition(transition, transition_callbacks) !=
            TitleProfileTransitionAction::rendered ||
        transition.countdown != 0U || transition.dirty_flags != 1U ||
        transition.profile_destination_words[0] != 0x0444U ||
        transition.profile_destination_words[6] != 0x0222U || builds != 2) {
        return 1;
    }

    transition.countdown = 0U;
    int finalized = 0;
    transition_callbacks.copy_title_profile = [&] { ++finalized; };
    transition_callbacks.attach_behavior_descriptor_b = [&] { ++finalized; };
    transition_callbacks.run_menu_slot_service = [&] { ++finalized; };
    transition_callbacks.initialize_world_effect_mode = [&] { ++finalized; };
    if (advance_title_profile_transition(transition, transition_callbacks) !=
            TitleProfileTransitionAction::finalized ||
        (transition.behavior_flags & 0x80U) != 0U ||
        transition.profile_destination_words[0] != 0x0000U ||
        transition.profile_destination_words[6] != 0x0EEEU ||
        finalized != 4) {
        return 2;
    }

    ProfileCopyTimerState timer{};
    timer.countdown = -8;
    timer.current_record_timing_value = -2;
    ProfileCopyTimerCallbacks timer_callbacks{};
    timer_callbacks.resolve_reload_countdown =
        [](const std::int8_t value) -> std::optional<std::int16_t> {
        return static_cast<std::int16_t>(value * 3);
    };
    if (advance_profile_copy_timer(timer, timer_callbacks) !=
            ProfileCopyTimerAction::reloaded || timer.countdown != -6) {
        return 3;
    }

    timer.countdown = 0;
    timer_callbacks.resolve_reload_countdown = {};
    int copied = 0;
    int dirty = 0;
    timer_callbacks.read_copy_eligibility =
        [] { return std::optional<std::int16_t>(1); };
    timer_callbacks.copy_profile_words = [&](const std::int32_t offset) {
        copied += offset == 14 ? 1 : 100;
    };
    timer_callbacks.mark_profile_dirty = [&] { ++dirty; };
    if (advance_profile_copy_timer(timer, timer_callbacks) !=
            ProfileCopyTimerAction::copied || copied != 1 || dirty != 1 ||
        timer.countdown != -1) {
        return 4;
    }
    timer.countdown = 0;
    timer_callbacks.read_copy_eligibility =
        [] { return std::optional<std::int16_t>(-1); };
    if (advance_profile_copy_timer(timer, timer_callbacks) !=
        ProfileCopyTimerAction::copy_ineligible || dirty != 1) {
        return 5;
    }
    if (!resolve_title_profile_words(7U).has_value() ||
        resolve_title_profile_words(8U).has_value() ||
        resolve_title_profile_transition_sprite_data(0U) != 0x1D8C6U ||
        resolve_title_profile_transition_sprite_data(3U) != 0x1D8E4U ||
        resolve_title_profile_transition_sprite_data(4U).has_value() ||
        resolve_title_profile_reload_countdown(0) != 0x00F0 ||
        resolve_title_profile_reload_countdown(10) != 0x0005 ||
        resolve_title_profile_reload_countdown(-1).has_value()) {
        return 6;
    }
    return 0;
}
