#include "shadowrun_ghidra/animation_table_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto caller = EntityAnimationTable::CallerSelected;
    if (!expect(
            select_entity_animation_table(0x00FFU, false, caller) ==
                EntityAnimationTable::Below100 &&
            select_entity_animation_table(0x0100U, false, caller) ==
                EntityAnimationTable::From100 &&
            select_entity_animation_table(0x01FFU, false, caller) ==
                EntityAnimationTable::From100 &&
            select_entity_animation_table(0x0200U, false, caller) ==
                EntityAnimationTable::From200 &&
            select_entity_animation_table(0x035FU, false, caller) ==
                EntityAnimationTable::From200 &&
            select_entity_animation_table(0x0360U, false, caller) ==
                EntityAnimationTable::From360 &&
            select_entity_animation_table(0x045FU, false, caller) ==
                EntityAnimationTable::From360)) {
        return 1;
    }

    if (!expect(
            select_entity_animation_table(0x0460U, false,
                                          EntityAnimationTable::From100) ==
                EntityAnimationTable::From100 &&
            select_entity_animation_table(0xFFFFU, true, caller) ==
                EntityAnimationTable::SpecialHighMetric)) {
        return 2;
    }
    return 0;
}
