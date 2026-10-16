#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class EntityAnimationTable {
    CallerSelected,
    Below100,
    From100,
    From200,
    From360,
    SpecialHighMetric,
};

// ROM 0xB6E4: select the animation table bank for an entity. The source's
// caller-selected table is preserved at metrics >= 0x460 unless bit 3 of the
// entity flags requests the special high-metric table.
EntityAnimationTable select_entity_animation_table(
    std::uint16_t animation_metric,
    bool special_high_metric_flag,
    EntityAnimationTable caller_selected);

} // namespace shadowrun::ghidra_conversion
