#include "shadowrun_ghidra/core/remaining_routine_catalog_conversion.hpp"

#include <cstdint>

using namespace shadowrun::ghidra_conversion;

int main() {
    const auto& catalog = remaining_routine_catalog();
    if (catalog.size() < 271U ||
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
    return 0;
}
