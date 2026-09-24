#pragma once

#include "shadowrun_ghidra/save_validation_conversion.hpp"

#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct SaveSlotPair {
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> second;
};

enum class SaveSlotNormalizationAction {
    both_valid,
    repaired_second_from_first,
    repaired_first_from_second,
    no_valid_slot,
};

struct SaveSlotNormalizationResult {
    SaveSlotNormalizationAction action =
        SaveSlotNormalizationAction::no_valid_slot;
};

using SaveSlotValidator = std::function<bool(const std::vector<std::uint8_t>&)>;

// ROM 0x59456: validate the two SRAM save slots and mirror the valid slot into
// the invalid slot. The source performs this with the even-byte copy helper;
// host vectors already contain the extracted bytes, so assignment is the
// equivalent native operation.
SaveSlotNormalizationResult normalize_sram_save_slots(
    SaveSlotPair& slots,
    const SaveSlotValidator& validate = validate_sram_game_save);

} // namespace shadowrun::ghidra_conversion
