#include "shadowrun_ghidra/menu_hud_service_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

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
    calculate_menu_delay_offset(delay, [] {
        return std::array<std::int8_t, 2>{2, 3};
    });
    if (delay.offset != 0x18) {
        return 14;
    }

    int wrapper_calls = 0;
    refresh_mode_metadata_wrapper([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid_variant_a([&wrapper_calls] { ++wrapper_calls; });
    write_hud_repeated_grid_variant_b([&wrapper_calls] { ++wrapper_calls; });
    if (wrapper_calls != 4) {
        return 15;
    }

    HudFadeServiceState fade{};
    int waits = 0;
    int latches = 0;
    int palette_steps = 0;
    advance_hud_fade_service(
        fade, [&waits] { ++waits; }, [&latches] { ++latches; },
        [&palette_steps] { ++palette_steps; });
    if (!fade.dirty || waits != 10 || latches != 10 || palette_steps != 5) {
        return 16;
    }

    return 0;
}
