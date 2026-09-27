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
    if (field_action == nullptr || resource_selection == nullptr ||
        selection_context == nullptr ||
        field_action->family != RoutineFamily::field ||
        resource_selection->family != RoutineFamily::field ||
        selection_context->family != RoutineFamily::field) {
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
    return 0;
}
