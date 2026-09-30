#include "shadowrun_ghidra/title_intro/title_effect_animation_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/title_world_action_selection_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    int service = 0;

    // ROM 0x1A030 enters the title/world mode branch when FFDF84 is already
    // nonzero.  It publishes D0=9/D1=0x0B, renders text, and tail-calls the
    // one-time menu-offset preparation in that order.
    std::vector<int> title_world_order;
    TitleWorldActionSelectionState world_active{true, 0};
    const auto world_result = resolve_title_world_action_selection(
        world_active,
        {[&]() -> std::optional<std::int8_t> {
             assert(false && "select_hud_mode must be skipped");
             return 0;
         },
         [&] { assert(false && "action result must be skipped"); }, {},
         [&](const std::uint8_t code, const std::uint8_t value) {
             assert(code == 9U && value == 0x0BU);
             title_world_order.push_back(1);
         },
         [&] { title_world_order.push_back(2); },
         [&] { title_world_order.push_back(3); }});
    assert(world_result.action ==
               TitleWorldActionSelectionAction::selected_world_mode &&
           world_result.return_code == 9 &&
           title_world_order == std::vector<int>({1, 2, 3}));

    // Modes 1 and 8 take the same title/world publication branch after the
    // active runner HUD mode lookup.
    for (const auto mode : {static_cast<std::int8_t>(1),
                            static_cast<std::int8_t>(8)}) {
        std::uint8_t selection_value = 0;
        int action_rolls = 0;
        TitleWorldActionSelectionState state{};
        const auto result = resolve_title_world_action_selection(
            state,
            {[&] { return std::optional<std::int8_t>(mode); },
             [&] { ++action_rolls; }, {},
             [&](const std::uint8_t code, const std::uint8_t value) {
                 assert(code == 9U);
                 selection_value = value;
             },
             [&] { ++service; }, [&] { ++service; }});
        assert(result.action ==
                   TitleWorldActionSelectionAction::selected_world_mode &&
               result.return_code == 9 && selection_value == 0x0CU &&
               action_rolls == 0);
    }

    // A negative selected HUD mode is preserved across 0x55FD8, then routes
    // to prepare_menu_offset_once and returns -1.
    int first_roll = 0;
    int prepared = 0;
    TitleWorldActionSelectionState rejected_state{};
    const auto rejected = resolve_title_world_action_selection(
        rejected_state,
        {[&] { return std::optional<std::int8_t>(-1); },
         [&] { ++first_roll; }, {}, {}, {}, [&] { ++prepared; }});
    assert(rejected.action ==
               TitleWorldActionSelectionAction::action_roll_rejected &&
           rejected.return_code == -1 && first_roll == 1 && prepared == 1);

    // A nonnegative, non-1/8 mode runs both action-result continuations and
    // publishes the raw D4 marker to the typed FFF0AA-equivalent state.
    TitleWorldActionSelectionState resolved_state{};
    int second_roll = 0;
    const auto resolved = resolve_title_world_action_selection(
        resolved_state,
        {[&] { return std::optional<std::int8_t>(0); },
         [&] { ++first_roll; },
         [&] {
             ++second_roll;
             return std::optional<std::uint8_t>(0xFEU);
         },
         {}, {}, {}});
    assert(resolved.action ==
               TitleWorldActionSelectionAction::action_marker_resolved &&
           resolved.return_code == 0 && first_roll == 2 && second_roll == 1 &&
           resolved_state.pending_action_marker == 0xFEU);

    // ROM 0x56050 clamps its difficulty and preserves the raw underflow byte
    // while clearing the live action result on BPL.
    TitleWorldActionMarkerState marker_state{1U, 1U, 0U};
    std::uint16_t trial_count_seen = 0;
    std::uint16_t difficulty_seen = 0;
    const auto marker = resolve_title_world_action_marker(
        marker_state,
        {[&] { return std::optional<std::uint16_t>(4U); },
         [&](const std::uint16_t trials, const std::uint16_t difficulty) {
             trial_count_seen = trials;
             difficulty_seen = difficulty;
             return std::optional<std::uint8_t>(2U);
         }});
    assert(marker.outcome == TitleWorldActionMarkerOutcome::resolved &&
           marker.pending_marker == 0xFFU && marker.active_result_cleared &&
           marker_state.active_runner_action_result == 0U &&
           marker_state.thresholded_rolls == 2U && trial_count_seen == 4U &&
           difficulty_seen == 2U);

    int clear = 0;
    refresh_menu_selection_after_clear(true, [&] { ++clear; }, [&] { ++service; },
                                       [&] { ++service; });
    refresh_menu_selection_or_prepare(false, [&] { ++service; }, [&] { ++clear; },
                                      [&] { ++service; }, [&] { ++service; });
    assert(clear == 1);

    std::uint8_t selection_code = 0;
    std::uint8_t selection_value = 0;
    int exact_clear = 0;
    int exact_prepare = 0;
    MenuSelectionRefreshExactCallbacks exact_callbacks{};
    exact_callbacks.clear_selected_menu_record = [&] { ++exact_clear; };
    exact_callbacks.set_hud_selection = [&](const std::uint8_t code,
                                            const std::uint8_t value) {
        selection_code = code;
        selection_value = value;
    };
    exact_callbacks.render_menu_text_followup = [&] { ++service; };
    exact_callbacks.prepare_menu_offset = [&] { ++exact_prepare; };
    refresh_menu_selection_after_clear(MenuSelectionRefreshState{1},
                                       exact_callbacks);
    assert(exact_clear == 1 && exact_prepare == 0 && selection_code == 9U &&
           selection_value == 6U);
    refresh_menu_selection_or_prepare(MenuSelectionRefreshState{0},
                                      exact_callbacks);
    assert(exact_clear == 1 && exact_prepare == 1 && selection_code == 0x0AU &&
           selection_value == 0x0AU);

    AudioFrameState audio{};
    initialize_audio_frame(audio, [&] { ++service; });
    assert(audio.field_a == 0 && audio.field_b == 2 && audio.field_c == 1);

    TitleAnimationCycleState animation{0, 0, true};
    int appends = 0;
    assert(advance_title_animation_cycle(animation, [&] { ++appends; }, [&] { ++service; }));
    assert(animation.phase == 1 && appends == 1);
    build_title_sprites_twice([&] { ++service; });
    TitleVerticalBuildState vertical{0x1200};
    assert(append_title_vertical_sprite(vertical, [&] { ++appends; }, [&] { ++service; }));
    assert(vertical.y == 0x1260);

    WorldEffectSeedState effect{};
    initialize_world_effect_seed(effect, -1, 0x1020, [&] { ++service; }, [&] { ++service; });
    assert(effect.source_x == 0x10C0 && effect.horizontal_velocity == -8 && effect.lifetime == -1);
    TitleTransitionState transition{};
    reset_title_transition(transition);
    assert(transition.timer == 1 && transition.dirty);

    TitlePhaseCycleState phase{5, 0};
    advance_title_phase_cycle(phase, [&] { ++appends; }, [&] { ++service; });
    assert(phase.phase == 0 && phase.countdown == 3);

    TitleJitterState jitter{};
    int random_calls = 0;
    assert(advance_title_jitter(jitter, [&] {
               ++random_calls;
               return static_cast<std::uint16_t>(0x0101);
           },
           [&] { ++service; }, [&] { ++clear; }));
    assert(random_calls == 0x58 && (jitter.dirty_flags & 0x0C) != 0U);
    jitter.frame = 0x3B;
    assert(!advance_title_jitter(jitter, [] { return static_cast<std::uint16_t>(0); },
                                 {}, [&] { ++clear; }));

    int handler = 0;
    assert(dispatch_title_phase_handler(0, {[&] { ++handler; }}, [&] { ++service; }));
    assert(handler == 1);

    std::array<std::uint16_t, 7> profile{1, 2, 3, 4, 5, 6, 7};
    std::array<std::uint16_t, 7> copied{};
    std::uint32_t dirty = 0;
    copy_title_profile(profile, copied, dirty);
    assert(copied == profile && dirty == 1);
    AlternatingProfileState alternating{2, profile, {7, 6, 5, 4, 3, 2, 1}, {}, 0};
    assert(advance_alternating_profile(alternating, [&] { ++service; }));
    assert(alternating.destination == alternating.odd_profile);

    AudioMenuCountdownState countdown{0, 4, 1, 0xFF};
    assert(advance_audio_menu_countdown(countdown, [&] { ++service; }, [&] { ++clear; }));
    assert((countdown.entity_flags & 2U) == 0U);

    initialize_effect_motion_seed(effect, true, [&] { ++service; });
    assert(effect.source_y == 0x1020 && effect.horizontal_velocity == -18);

    InteractionState interaction{};
    update_interaction_state(interaction, true, false, 0, {});
    assert(interaction.latch == 0xFF && interaction.countdown == 0x3C);
    interaction = {};
    update_interaction_state(interaction, false, false, -1,
                             { {}, {}, {}, [&] { ++service; } });
    assert(interaction.x == 0x10C && interaction.y == 0x102);

    std::uint16_t phase_value = 3;
    int refreshes = 0;
    update_persistent_title_effect(true, 0, phase_value, [&] { ++service; }, {},
                                   [&] { ++service; }, [&] { ++clear; },
                                   [&] { ++refreshes; });
    assert(phase_value == 0 && refreshes == 1);

    std::array<std::int16_t, 4> positions{10, 20, 30, 40};
    advance_title_effect_position(positions, [&] { ++service; });
    assert((positions == std::array<std::int16_t, 4>{19, -6, 16, 26}));
    build_title_sprites([&] { ++service; });
    return 0;
}
