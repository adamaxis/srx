#include "shadowrun_ghidra/menus/menu_hud_service_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    MenuEntryRenderState pending{};
    pending.entry_marker = -1;
    int pending_merges = 0;
    const auto pending_action = render_menu_entry(
        pending, false, false, false,
        {[&] { ++pending_merges; }, {}, {}, {}, {}});
    if (pending_action != MenuEntryRenderAction::pending ||
        pending.pending_flags != 0x40U || pending_merges != 1) {
        return 100;
    }

    MenuEntryRenderState positive{};
    positive.entry_status = 1;
    positive.available_count = 3;
    std::vector<int> entry_calls;
    const auto positive_action = render_menu_entry(
        positive, true, true, true,
        {[&] { entry_calls.push_back(1); },
         [&] { entry_calls.push_back(2); },
         [&] { entry_calls.push_back(3); },
         [&] { entry_calls.push_back(4); },
         [&] { entry_calls.push_back(5); }});
    if (positive_action != MenuEntryRenderAction::text_and_number ||
        entry_calls != std::vector<int>{3, 2, 4, 5, 4, 5}) {
        return 101;
    }

    MenuEntryRenderState zero_status{};
    int zero_text_calls = 0;
    if (render_menu_entry(
            zero_status, true, false, false,
            {{}, {}, {}, [&] { ++zero_text_calls; }, {}}) !=
            MenuEntryRenderAction::text_only ||
        zero_text_calls != 1) {
        return 102;
    }
    const std::vector<std::uint8_t> eligibility_table{0x00U, 0x10U, 0xF0U,
                                                       0x2FU};
    if (read_menu_entry_profile_eligibility(1U, eligibility_table) != 1U ||
        read_menu_entry_profile_eligibility(2U, eligibility_table) !=
            0x0FFFU ||
        read_menu_entry_profile_eligibility(3U, eligibility_table) != 2U ||
        read_menu_entry_profile_eligibility(4U, eligibility_table).has_value()) {
        return 103;
    }

    MenuModeResourceState mode{};
    std::vector<int> events;
    prepare_menu_mode_resources(
        mode, true, 2U, [&events] { events.push_back(1); },
        [&events] { events.push_back(2); });
    if (mode.mode_variant != 1 || mode.transfer_marker != 0U ||
        mode.prompt_marker != 0U || !mode.transition_pending ||
        events != std::vector<int>{1, 2}) {
        return 1;
    }

    int uploads = 0;
    upload_menu_resource_pair([&uploads] { ++uploads; });
    if (uploads != 2) {
        return 2;
    }

    events.clear();
    prepare_menu_hud_sequence({
        [&events] { events.push_back(1); },
        [&events] { events.push_back(2); },
        [&events] { events.push_back(3); },
        [&events] { events.push_back(4); },
        [&events] { events.push_back(5); }});
    if (events.size() != 14U || events[0] != 1 || events[4] != 1 ||
        events[9] != 1 || events[10] != 2 || events[13] != 5) {
        return 3;
    }

    int grid_writes = 0;
    write_menu_hud_grid_batch([&grid_writes] { ++grid_writes; });
    if (grid_writes != 5) {
        return 4;
    }

    MenuHudRowsState rows{true, {-1, 0, 1, -1, 2}};
    std::vector<std::int16_t> prepared_rows;
    std::vector<std::uint32_t> prepared_offsets;
    std::vector<std::int16_t> written_rows;
    int row_services = 0;
    render_menu_hud_rows(
        rows,
        {[&](const std::int16_t row, const std::uint32_t offset) {
             prepared_rows.push_back(row);
             prepared_offsets.push_back(offset);
         },
         [&row_services] { ++row_services; },
         [&written_rows](const std::int16_t row, const std::uint32_t) {
             written_rows.push_back(row);
         }});
    if (prepared_rows != std::vector<std::int16_t>{4, 2, 1} ||
        prepared_offsets != std::vector<std::uint32_t>{0xEBC0U, 0xEBA8U,
                                                        0xEB9CU} ||
        written_rows != std::vector<std::int16_t>{4, 3, 2, 1, 0} ||
        row_services != 3) {
        return 5;
    }

    MenuNumberOrGridState number_state{1U, {-1, 0, -1}};
    events.clear();
    if (!refresh_menu_number_or_grid(
            number_state,
            {[&events] { events.push_back(6); },
             [&events] { events.push_back(7); },
             [&events] { events.push_back(8); },
             [&events] { events.push_back(9); }}) ||
        events != std::vector<int>{6, 7, 8}) {
        return 6;
    }
    number_state.selected_row = 2U;
    if (refresh_menu_number_or_grid(
            number_state,
            {{}, {}, {}, [&events] { events.push_back(9); }}) ||
        events != std::vector<int>{6, 7, 8, 9}) {
        return 7;
    }

    int text_calls = 0;
    render_menu_text_followup([&text_calls] { ++text_calls; });
    if (text_calls != 1) {
        return 8;
    }

    HudSpriteAudioPulseState pulse{0, 3};
    int sprite_calls = 0;
    int audio_calls = 0;
    if (!advance_hud_sprite_audio_pulse(
            pulse, [&sprite_calls] { ++sprite_calls; },
            [&audio_calls] { ++audio_calls; }) ||
        pulse.countdown != 1 || pulse.phase != 0 || sprite_calls != 1 ||
        audio_calls != 1) {
        return 9;
    }

    HudOffsetState panel{9};
    events.clear();
    advance_hud_panel_offset(panel, 3,
                             [&events] { events.push_back(10); },
                             [&events] { events.push_back(11); });
    if (panel.offset != 10 || events != std::vector<int>{10, 11}) {
        return 10;
    }
    HudOffsetState footer{0};
    advance_hud_footer_offset(footer, 1, {}, {});
    if (footer.offset != 1) {
        return 11;
    }

    int ready_writes = 0;
    if (!refresh_hud_grid_if_ready([] { return true; },
                                   [&ready_writes] { ++ready_writes; }) ||
        ready_writes != 3 ||
        refresh_hud_grid_if_ready([] { return false; },
                                  [&ready_writes] { ++ready_writes; }) ||
        ready_writes != 3) {
        return 12;
    }

    std::vector<std::uint8_t> metrics(40U, 0U);
    metrics[0x14U] = 3U;
    if (calculate_menu_row_offset(metrics, 1U, -1).value_or(0U) != 0x18U * 0x14U ||
        calculate_menu_row_offset(metrics, 1U, 0).value_or(0U) != 0x17U * 0x14U ||
        calculate_menu_row_offset(metrics, 3U, 0).has_value()) {
        return 13;
    }

    MenuDelayOffsetState delay{};
    calculate_menu_delay_offset(delay, MenuDelayOffsetInput{0U, 2U, 3U});
    if (delay.offset != 0x15) {
        return 14;
    }

    // 0xD81C wraps 240 + 32 to 16 before halving. The menu leaf then adds
    // fixed six and four times the profile component before subtracting 0x1D.
    calculate_menu_delay_offset(delay, MenuDelayOffsetInput{1U, 240U, 32U});
    if (delay.offset != 0x0BU) {
        return 15;
    }

    int wrapper_calls = 0;
    refresh_mode_metadata_wrapper([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid_variant_a([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid_variant_b([&wrapper_calls] { ++wrapper_calls; });
    if (wrapper_calls != 4) {
        return 16;
    }

    HudFadeServiceState fade{};
    int waits = 0;
    int latches = 0;
    int palette_steps = 0;
    advance_hud_fade_service(
        fade, [&waits] { ++waits; }, [&latches] { ++latches; },
        [&palette_steps] { ++palette_steps; });
    if (!fade.dirty || waits != 10 || latches != 10 || palette_steps != 5) {
        return 17;
    }

    MenuHudTickState tick_state{};
    int tick_waits = 0;
    int tick_latches = 0;
    int tick_pulses = 0;
    int tick_random = 0;
    int tick_world = 0;
    int tick_optional = 0;
    int tick_finalize_world = 0;
    int tick_finalize_input = 0;
    int tick_rows = 0;
    run_menu_hud_tick_loop(
        tick_state,
        {[&tick_waits] { ++tick_waits; },
         [&tick_latches] { ++tick_latches; },
         [&tick_pulses] { ++tick_pulses; },
         [&tick_random] { ++tick_random; },
         [&tick_world] { ++tick_world; },
         [&tick_optional] { ++tick_optional; },
         [&tick_finalize_world] { ++tick_finalize_world; },
         [&tick_finalize_input] { ++tick_finalize_input; },
         [&tick_rows] { ++tick_rows; }});
    if (tick_state.frames_completed != 60U || tick_state.selected_row != 0U ||
        tick_state.dirty_flags != 2U || !tick_state.controller_tick_ready ||
        !tick_state.world_tick_ready || tick_waits != 60 ||
        tick_latches != 60 || tick_pulses != 60 || tick_random != 60 ||
        tick_world != 60 || tick_optional != 60 ||
        tick_finalize_world != 1 || tick_finalize_input != 1 ||
        tick_rows != 1) {
        return 17;
    }

    std::vector<int> text_events;
    render_menu_text_progression(
        2, false,
        {[&text_events] { text_events.push_back(1); },
         [&text_events] { text_events.push_back(2); }});
    if (text_events != std::vector<int>{1, 1, 2, 1, 1}) {
        return 18;
    }
    text_events.clear();
    render_menu_text_progression(
        3, false,
        {[&text_events] { text_events.push_back(1); },
         [&text_events] { text_events.push_back(2); }});
    if (text_events != std::vector<int>{1, 1, 1, 2}) {
        return 19;
    }

    MenuOverlayTimerState overlay{11U, 0, 0U, 3};
    int overlay_grids = 0;
    if (!advance_menu_overlay_timer(overlay,
                                    [&overlay_grids] { ++overlay_grids; }) ||
        overlay.overlay_phase != 12U || overlay.countdown != 3 ||
        overlay_grids != 2 ||
        advance_menu_overlay_timer(overlay,
                                   [&overlay_grids] { ++overlay_grids; }) ||
        overlay.service_marker != 0xFFU || overlay_grids != 2) {
        return 20;
    }

    MenuRandomGlitchState glitch{};
    int random_sample = 0;
    std::vector<int> glitch_events;
    advance_menu_random_glitch(
        glitch,
        {[&random_sample] {
             ++random_sample;
             return static_cast<std::int16_t>(random_sample == 1 ? 0 :
                                                   (random_sample == 2 ? 3 : 0));
         },
         [&glitch_events] { glitch_events.push_back(1); },
         [&glitch_events] { glitch_events.push_back(2); },
         [&glitch_events] { glitch_events.push_back(3); }});
    if (glitch.phase != 6 || glitch.random_offset != 62 ||
        glitch_events != std::vector<int>{1, 2}) {
        return 21;
    }
    advance_menu_random_glitch(
        glitch,
        {{}, [&glitch_events] { glitch_events.push_back(1); },
         [&glitch_events] { glitch_events.push_back(2); }, {}});
    if (glitch.phase != 5 ||
        glitch_events != std::vector<int>{1, 2, 1, 2}) {
        return 22;
    }

    FieldHudAssetState field_assets{4U};
    events.clear();
    refresh_field_hud_assets(
        field_assets,
        {[&events] { events.push_back(1); },
         [&events] { events.push_back(2); },
         [&events] { events.push_back(3); },
         [&events] { events.push_back(4); },
         [&events] { events.push_back(5); }});
    if (field_assets.selected_row != 0U ||
        events != std::vector<int>{1, 2, 3, 4, 5, 5}) {
        return 23;
    }

    HudIndicatorGridState indicators{{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1}};
    int indicator_writes = 0;
    render_hud_indicator_grids(indicators,
                               [&indicator_writes] { ++indicator_writes; });
    if (indicator_writes != 7) {
        return 24;
    }
    write_hud_repeated_grid_variant_c(
        [&indicator_writes] { ++indicator_writes; });
    write_two_hud_grids([&indicator_writes] { ++indicator_writes; });
    if (indicator_writes != 10) {
        return 25;
    }

    MenuDirectionalInputState input{4U, 1U, {0, 0, 0, 0, 0},
                                    {0, 6, 0, 0, 0}};
    events.clear();
    handle_menu_directional_input(
        input, [&events] { events.push_back(1); },
        [&events] { events.push_back(2); },
        [&events] { events.push_back(3); });
    if (events != std::vector<int>{1, 2, 3}) {
        return 26;
    }
    input.row_status[1] = 3;
    handle_menu_directional_input(
        input, [&events] { events.push_back(4); },
        [&events] { events.push_back(5); },
        [&events] { events.push_back(6); });
    if (events != std::vector<int>{1, 2, 3}) {
        return 27;
    }

    HudSelectionRowState selection{0};
    events.clear();
    advance_hud_selection_row(selection, -1,
                              [&events] { events.push_back(7); },
                              [&events] { events.push_back(8); });
    if (selection.selected_row != 4 || events != std::vector<int>{7, 8}) {
        return 28;
    }
    write_hud_panel_grid([&events] { events.push_back(9); });
    if (events.back() != 9) {
        return 29;
    }

    HudHistoryState history{0, {1, 2, 3, 4}, false};
    rotate_hud_history(history);
    if (history.countdown != 4 || !history.dirty ||
        history.values != std::array<std::uint16_t, 4>{2, 3, 4, 1}) {
        return 30;
    }

    std::vector<std::uint16_t> grid_commands;
    std::vector<std::uint16_t> grid_words;
    const WordTransferCallbacks grid_callbacks{
        [&grid_commands](const VdpTransferCommand& command) {
            grid_commands.push_back(command.low_word);
        },
        [&grid_words](const std::uint16_t word) { grid_words.push_back(word); },
    };
    write_hud_repeated_grid(0U, grid_callbacks);
    if (grid_commands.size() != 8U ||
        grid_commands[0] != make_vdp_transfer_command(0xC300U).low_word ||
        grid_commands[1] != make_vdp_transfer_command(0xC380U).low_word ||
        grid_words.size() != 24U || grid_words.front() != 0x6070U ||
        grid_words.back() != 0x6070U) {
        return 31;
    }

    std::array<std::uint32_t, 8> footer_bank{};
    std::uint8_t dirty = 0U;
    draw_hud_footer(0x2000U, footer_bank, dirty);
    if (footer_bank != kHudFooterPaletteBanks[0] || dirty == 0U) {
        return 32;
    }
    dirty = 0U;
    draw_hud_footer(0x6000U, footer_bank, dirty);
    if (footer_bank != kHudFooterPaletteBanks[2] || dirty == 0U) {
        return 33;
    }

    return 0;
}
