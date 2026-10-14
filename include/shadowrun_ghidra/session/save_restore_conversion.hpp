#pragma once

#include "shadowrun_ghidra/session/save_slot_conversion.hpp"

#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class SaveWorkLoadAction {
    loaded_from_primary_backup,
    loaded_from_primary_backup_repaired_secondary,
    loaded_from_secondary_backup,
    loaded_from_secondary_backup_repaired_primary,
    no_valid_save,
};

struct SaveWorkLoadResult {
    SaveWorkLoadAction action = SaveWorkLoadAction::no_valid_save;
};

// ROM 0x59372: try the two extracted backup slots in source order, repair the
// first slot if only the second validates. The selected save is published as
// the host work buffer; when neither source slot validates, the ROM returns
// without publishing a save. No host-side fallback is invented here.
SaveWorkLoadResult load_save_work_buffer(
    SaveSlotPair& backup_slots,
    std::vector<std::uint8_t>& work_buffer,
    const SaveSlotValidator& validate = validate_backup_game_save);

enum class SaveLoadCoordinatorAction {
    loaded,
    sram_slots_unavailable,
};

struct SaveLoadCoordinatorResult {
    SaveLoadCoordinatorAction action =
        SaveLoadCoordinatorAction::sram_slots_unavailable;
    SaveWorkLoadResult load{};
};

using SaveSlotPreparationBoundary =
    std::function<SaveSlotNormalizationResult()>;
using SaveWorkLoadBoundary = std::function<SaveWorkLoadResult()>;

// ROM 0x59360: run SRAM slot preparation and only enter the save-work-buffer
// loader when that preparation succeeds.
SaveLoadCoordinatorResult coordinate_save_load(
    const SaveSlotPreparationBoundary& prepare_sram_slots,
    const SaveWorkLoadBoundary& load_work_buffer);

} // namespace shadowrun::ghidra_conversion
