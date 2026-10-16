#include "shadowrun_ghidra/session/save_restore_conversion.hpp"

namespace shadowrun::ghidra_conversion {

SaveWorkLoadResult load_save_work_buffer(
    SaveSlotPair& backup_slots,
    std::vector<std::uint8_t>& work_buffer,
    const SaveSlotValidator& validate) {
    const auto validator = validate ? validate : SaveSlotValidator{
                                                     validate_backup_game_save};
    if (validator(backup_slots.first)) {
        work_buffer = backup_slots.first;
        if (!validator(backup_slots.second)) {
            backup_slots.second = backup_slots.first;
            return {SaveWorkLoadAction::
                        loaded_from_primary_backup_repaired_secondary};
        }
        return {SaveWorkLoadAction::loaded_from_primary_backup};
    }
    if (validator(backup_slots.second)) {
        work_buffer = backup_slots.second;
        backup_slots.first = backup_slots.second;
        return {SaveWorkLoadAction::
                    loaded_from_secondary_backup_repaired_primary};
    }
    return {};
}

SaveLoadCoordinatorResult coordinate_save_load(
    const SaveSlotPreparationBoundary& prepare_sram_slots,
    const SaveWorkLoadBoundary& load_work_buffer) {
    if (!prepare_sram_slots || !load_work_buffer) {
        return {};
    }
    const auto preparation = prepare_sram_slots();
    if (preparation.action == SaveSlotNormalizationAction::no_valid_slot) {
        return {};
    }
    const auto load = load_work_buffer();
    return {SaveLoadCoordinatorAction::loaded, load};
}

} // namespace shadowrun::ghidra_conversion
