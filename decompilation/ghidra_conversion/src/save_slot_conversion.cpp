#include "shadowrun_ghidra/save_slot_conversion.hpp"

namespace shadowrun::ghidra_conversion {

SaveSlotNormalizationResult normalize_sram_save_slots(
    SaveSlotPair& slots,
    const SaveSlotValidator& validate) {
    const auto validator = validate ? validate : SaveSlotValidator{
                                                     validate_sram_game_save};
    const bool first_valid = validator(slots.first);
    const bool second_valid = validator(slots.second);

    if (first_valid && second_valid) {
        return {SaveSlotNormalizationAction::both_valid};
    }
    if (first_valid) {
        slots.second = slots.first;
        return {SaveSlotNormalizationAction::repaired_second_from_first};
    }
    if (second_valid) {
        slots.first = slots.second;
        return {SaveSlotNormalizationAction::repaired_first_from_second};
    }
    return {SaveSlotNormalizationAction::no_valid_slot};
}

} // namespace shadowrun::ghidra_conversion
