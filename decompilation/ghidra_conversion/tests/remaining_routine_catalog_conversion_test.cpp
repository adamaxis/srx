#include "shadowrun_ghidra/remaining_routine_catalog_conversion.hpp"

#include <cstdint>

using namespace shadowrun::ghidra_conversion;

int main() {
    const auto& catalog = remaining_routine_catalog();
    if (catalog.size() < 271U ||
        find_remaining_routine(catalog.front().rom_entry) == nullptr ||
        find_remaining_routine(0xFFFFFFFFU) != nullptr) {
        return 1;
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
            return 2;
        }
    }
    if (counters.calls != catalog.size() || family_events != catalog.size()) {
        return 3;
    }
    return 0;
}
