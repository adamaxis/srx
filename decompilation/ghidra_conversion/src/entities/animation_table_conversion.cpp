#include "shadowrun_ghidra/entities/animation_table_conversion.hpp"

namespace shadowrun::ghidra_conversion {

EntityAnimationTable select_entity_animation_table(
    const std::uint16_t animation_metric,
    const bool special_high_metric_flag,
    const EntityAnimationTable caller_selected) {
    if (animation_metric >= 0x460U) {
        return special_high_metric_flag
                   ? EntityAnimationTable::SpecialHighMetric
                   : caller_selected;
    }
    if (animation_metric >= 0x360U) {
        return EntityAnimationTable::From360;
    }
    if (animation_metric >= 0x200U) {
        return EntityAnimationTable::From200;
    }
    if (animation_metric >= 0x100U) {
        return EntityAnimationTable::From100;
    }
    return EntityAnimationTable::Below100;
}

} // namespace shadowrun::ghidra_conversion
