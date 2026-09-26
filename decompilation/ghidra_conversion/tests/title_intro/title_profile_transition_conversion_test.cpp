#include "shadowrun_ghidra/title_intro/title_profile_transition_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleProfileTransitionState transition{};
    transition.countdown = 1U;
    transition.behavior_flags = 0xFFU;
    int builds = 0;
    TitleProfileTransitionCallbacks transition_callbacks{};
    transition_callbacks.resolve_sprite_data =
        [](const std::uint32_t table, const std::uint16_t phase)
        -> std::optional<std::uint32_t> { return table + phase + 8U; };
    transition_callbacks.build_sprite =
        [&](const TitleProfileTransitionSpriteBuild& build) {
            if (build.source_table_token != 0x1C136U || build.phase != 0U ||
                build.y != 0xEEU || build.build_flags != 0x2000U ||
                build.sprite_data_token != 0x1C13EU ||
                (build.x != 0x15EU && build.x != 0xE0U)) {
                builds += 100;
            } else {
                ++builds;
            }
        };
    if (advance_title_profile_transition(transition, transition_callbacks) !=
            TitleProfileTransitionAction::rendered ||
        transition.countdown != 0U || transition.dirty_flags != 1U ||
        builds != 2) {
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
        (transition.behavior_flags & 0x80U) != 0U || finalized != 4) {
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
        [] { return std::optional<std::uint16_t>(1U); };
    timer_callbacks.copy_profile_words = [&](const std::int32_t offset) {
        copied += offset == 14 ? 1 : 100;
    };
    timer_callbacks.mark_profile_dirty = [&] { ++dirty; };
    if (advance_profile_copy_timer(timer, timer_callbacks) !=
            ProfileCopyTimerAction::copied || copied != 1 || dirty != 1 ||
        timer.countdown != -1) {
        return 4;
    }
    return 0;
}
