#include "shadowrun_ghidra/save_restore_conversion.hpp"

namespace shadowrun::ghidra_conversion {

SaveWorkLoadResult load_save_work_buffer(
    SaveSlotPair& backup_slots,
    std::vector<std::uint8_t>& work_buffer,
    const SaveWorkFallbackBoundary& load_sram_fallback,
    const SaveSlotValidator& validate) {
    const auto validator = validate ? validate : SaveSlotValidator{
                                                     validate_backup_game_save};
    if (validator(backup_slots.first)) {
        work_buffer = backup_slots.first;
        return {SaveWorkLoadAction::loaded_from_primary_backup};
    }
    if (validator(backup_slots.second)) {
        work_buffer = backup_slots.second;
        backup_slots.first = backup_slots.second;
        return {SaveWorkLoadAction::loaded_from_secondary_backup};
    }
    if (load_sram_fallback) {
        const auto fallback = load_sram_fallback();
        if (fallback.has_value()) {
            work_buffer = *fallback;
            return {SaveWorkLoadAction::loaded_from_sram_fallback};
        }
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
