#include "shadowrun_ghidra/title_intro/title_effect_animation_batch_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    int service = 0;
    MenuEntryCallbacks menu{
        [] { return static_cast<std::uint8_t>(0); }, [&] { ++service; },
        [] { return static_cast<std::uint8_t>(4); }, {}, {}, [&] { ++service; }};
    assert(select_menu_entry(0, menu) == 0 && service == 1);
    menu.select_hud_mode = [] { return static_cast<std::uint8_t>(9); };
    assert(select_menu_entry(0, menu) == 0x45);

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
