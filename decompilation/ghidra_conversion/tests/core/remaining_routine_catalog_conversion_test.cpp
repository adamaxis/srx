#include "shadowrun_ghidra/core/remaining_routine_catalog_conversion.hpp"

#include <cstdint>

using namespace shadowrun::ghidra_conversion;

int main() {
    const auto& catalog = remaining_routine_catalog();
    if (catalog.size() < 291U ||
        find_remaining_routine(catalog.front().rom_entry) == nullptr ||
        find_remaining_routine(0xFFFFFFFFU) != nullptr) {
        return 1;
    }
    const auto* field_action = find_remaining_routine(0x56DC4U);
    const auto* resource_selection = find_remaining_routine(0x575F8U);
    const auto* selection_context = find_remaining_routine(0x57610U);
    const auto* hospital_cost = find_remaining_routine(0xCDF6U);
    const auto* hotel_menu = find_remaining_routine(0xCF38U);
    const auto* hotel_display = find_remaining_routine(0xCFFEU);
    const auto* hotel_checkout = find_remaining_routine(0xDA6AU);
    if (field_action == nullptr || resource_selection == nullptr ||
        selection_context == nullptr ||
        field_action->family != RoutineFamily::field ||
        resource_selection->family != RoutineFamily::field ||
        selection_context->family != RoutineFamily::field ||
        hospital_cost == nullptr ||
        hospital_cost->alias != "calculate_hospital_medical_attention_cost" ||
        hospital_cost->family != RoutineFamily::menu ||
        hospital_cost->source_bytes != 134U ||
        hotel_menu == nullptr ||
        hotel_menu->alias != "run_hotel_options_menu" ||
        hotel_menu->family != RoutineFamily::menu ||
        hotel_menu->source_bytes != 198U ||
        hotel_display == nullptr ||
        hotel_display->alias != "prepare_hotel_options_display" ||
        hotel_display->family != RoutineFamily::menu ||
        hotel_display->source_bytes != 30U ||
        hotel_checkout == nullptr ||
        hotel_checkout->alias != "run_hotel_checkout_prompt" ||
        hotel_checkout->family != RoutineFamily::menu ||
        hotel_checkout->source_bytes != 188U) {
        return 2;
    }
    RemainingRoutineCounters counters{};
    std::size_t family_events = 0;
    const RemainingRoutineCallbacks callbacks{
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; },
        [&](const RemainingRoutine&) { ++family_events; }};
    for (const auto& routine : catalog) {
        if (!record_remaining_routine(counters, routine.rom_entry, callbacks)) {
            return 3;
        }
    }
    if (counters.calls != catalog.size() || family_events != catalog.size()) {
        return 4;
    }
    const auto* overview_hud = find_remaining_routine(0x110BCU);
    const auto* glyph_row = find_remaining_routine(0x11136U);
    if (overview_hud == nullptr ||
        overview_hud->alias != "draw_command_overview_hud_panel" ||
        overview_hud->source_bytes != 44U || glyph_row == nullptr ||
        glyph_row->alias != "render_inventory_glyph_row" ||
        glyph_row->source_bytes != 90U) {
        return 5;
    }
    const auto* auxiliary = find_remaining_routine(0x1562EU);
    const auto* palette_fade = find_remaining_routine(0x16396U);
    const auto* effect_slots = find_remaining_routine(0x163DAU);
    if (auxiliary == nullptr ||
        auxiliary->alias != "service_entity_auxiliary_behavior" ||
        auxiliary->source_bytes != 24U || palette_fade == nullptr ||
        palette_fade->alias != "advance_palette_fade" ||
        palette_fade->source_bytes != 68U || effect_slots == nullptr ||
        effect_slots->alias != "service_active_effect_slots" ||
        effect_slots->source_bytes != 64U) {
        return 6;
    }
    const auto* tile_script = find_remaining_routine(0x1641AU);
    const auto* populate = find_remaining_routine(0x164A8U);
    const auto* clear_slots = find_remaining_routine(0x164DCU);
    if (tile_script == nullptr ||
        tile_script->alias != "advance_effect_slot_tile_script" ||
        tile_script->source_bytes != 142U || populate == nullptr ||
        populate->alias != "populate_entity_table" ||
        populate->source_bytes != 52U || clear_slots == nullptr ||
        clear_slots->alias != "clear_scene_record_transition_words" ||
        clear_slots->source_bytes != 22U) {
        return 7;
    }
    const auto* project_x = find_remaining_routine(0x1669CU);
    const auto* project_y = find_remaining_routine(0x166D0U);
    const auto* timed_sprites = find_remaining_routine(0x5A582U);
    const auto* seattle_outline = find_remaining_routine(0x5B378U);
    if (project_x == nullptr ||
        project_x->alias != "project_entity_x_axis" ||
        project_x->source_bytes != 52U || project_y == nullptr ||
        project_y->alias != "project_entity_y_axis" ||
        project_y->source_bytes != 62U || timed_sprites == nullptr ||
        timed_sprites->alias != "begin_title_intro_timed_sprites" ||
        timed_sprites->source_bytes != 26U || seattle_outline == nullptr ||
        seattle_outline->alias != "stamp_seattle_intro_outline" ||
        seattle_outline->source_bytes != 266U) {
        return 8;
    }
    const auto* startup_text = find_remaining_routine(0x436U);
    const auto* repeated_mask = find_remaining_routine(0x26BFEU);
    const auto* masked_column = find_remaining_routine(0x26C26U);
    if (startup_text == nullptr ||
        startup_text->alias != "write_startup_text_to_vdp" ||
        startup_text->source_bytes != 54U || repeated_mask == nullptr ||
        repeated_mask->alias != "write_repeated_masked_word" ||
        repeated_mask->source_bytes != 40U || masked_column == nullptr ||
        masked_column->alias != "write_masked_transfer_with_display_control" ||
        masked_column->source_bytes != 56U) {
        return 9;
    }
    const auto* boot_clear = find_remaining_routine(0xC2CU);
    const auto* hud_panel = find_remaining_routine(0x3648U);
    const auto* menu_panel = find_remaining_routine(0x17D72U);
    if (boot_clear == nullptr ||
        boot_clear->alias != "clear_runtime_memory_and_vdp" ||
        boot_clear->source_bytes != 192U || hud_panel == nullptr ||
        hud_panel->alias != "draw_hud_panel" ||
        hud_panel->source_bytes != 44U || menu_panel == nullptr ||
        menu_panel->alias != "draw_menu_hud_panel" ||
        menu_panel->source_bytes != 30U) {
        return 10;
    }
    const auto* footer = find_remaining_routine(0x17F8EU);
    const auto* repeated_grid = find_remaining_routine(0x17FB4U);
    if (footer == nullptr || footer->alias != "draw_hud_footer" ||
        footer->source_bytes != 38U || repeated_grid == nullptr ||
        repeated_grid->alias != "write_hud_repeated_grid" ||
        repeated_grid->source_bytes != 38U) {
        return 11;
    }
    const auto* variant_a = find_remaining_routine(0x1897CU);
    const auto* two_grids = find_remaining_routine(0x189D8U);
    const auto* panel_grid = find_remaining_routine(0x19504U);
    if (variant_a == nullptr ||
        variant_a->alias != "write_hud_repeated_grid_variant_a" ||
        variant_a->source_bytes != 28U || two_grids == nullptr ||
        two_grids->alias != "write_two_hud_grids" ||
        two_grids->source_bytes != 136U || panel_grid == nullptr ||
        panel_grid->alias != "write_hud_panel_grid" ||
        panel_grid->source_bytes != 40U) {
        return 12;
    }
    const auto* title_font = find_remaining_routine(0xC0C2U);
    const auto* scene_entities = find_remaining_routine(0x5A37CU);
    const auto* status_clear = find_remaining_routine(0x19762U);
    if (title_font == nullptr || title_font->alias != "load_title_font" ||
        title_font->source_bytes != 24U || scene_entities == nullptr ||
        scene_entities->alias != "render_title_intro_scene_entities" ||
        scene_entities->source_bytes != 28U || status_clear == nullptr ||
        status_clear->alias != "clear_menu_status_at" ||
        status_clear->source_bytes != 38U) {
        return 13;
    }
    const auto* effect_reset = find_remaining_routine(0x4B6CU);
    const auto* field_hud = find_remaining_routine(0x19856U);
    if (effect_reset == nullptr ||
        effect_reset->alias != "reset_effect_record" ||
        effect_reset->source_bytes != 68U || field_hud == nullptr ||
        field_hud->alias != "refresh_field_hud" ||
        field_hud->source_bytes != 68U) {
        return 14;
    }
    const auto* projected = find_remaining_routine(0x481EU);
    if (projected == nullptr ||
        projected->alias != "refresh_projected_entity_sprites" ||
        projected->source_bytes != 148U) {
        return 15;
    }
    const auto* effect_timer = find_remaining_routine(0x4ADCU);
    if (effect_timer == nullptr ||
        effect_timer->alias != "advance_entity_effect_timer" ||
        effect_timer->source_bytes != 144U) {
        return 16;
    }
    return 0;
}
