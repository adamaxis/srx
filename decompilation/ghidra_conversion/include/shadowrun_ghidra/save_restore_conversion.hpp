#pragma once

#include "shadowrun_ghidra/save_slot_conversion.hpp"

#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class SaveWorkLoadAction {
    loaded_from_primary_backup,
    loaded_from_secondary_backup,
    loaded_from_sram_fallback,
    no_valid_save,
};

struct SaveWorkLoadResult {
    SaveWorkLoadAction action = SaveWorkLoadAction::no_valid_save;
};

using SaveWorkFallbackBoundary =
    std::function<std::optional<std::vector<std::uint8_t>>()>;

// ROM 0x59372: try the two extracted backup slots in source order, repair the
// first slot if only the second validates, then use the explicit SRAM fallback
// boundary when neither backup slot is usable. The selected save is published
// as the host work buffer.
SaveWorkLoadResult load_save_work_buffer(
    SaveSlotPair& backup_slots,
    std::vector<std::uint8_t>& work_buffer,
    const SaveWorkFallbackBoundary& load_sram_fallback,
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
