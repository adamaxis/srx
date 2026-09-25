#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class MatrixProgramSlotPreparation {
    already_loaded,
    no_available_slot,
    insufficient_memory,
    ready_to_install,
};

struct MatrixProgramSlotDecision {
    MatrixProgramSlotPreparation preparation =
        MatrixProgramSlotPreparation::no_available_slot;
    std::uint8_t slot = 0;
    bool slot_available = false;
};

struct SelectedMatrixProgramAvailability {
    bool slot_valid = false;
    std::int8_t program_index = -1;
    bool available = false;
};

// ROM 0x19C54: follow FFF0D4 to its loaded-program slot, then read that
// program's FFBCC availability byte. 0x19C5C is an interior address in this
// same leaf, not a second function.
SelectedMatrixProgramAvailability read_selected_matrix_program_availability(
    std::uint8_t selected_slot,
    const std::array<std::int8_t, 5>& slot_programs,
    const std::array<std::uint8_t, 12>& availability_flags);

// ROM 0x19238: after the selected-program gate, find an existing slot or the
// first free slot, then compare the selected program's footprint with the
// available Matrix memory.
MatrixProgramSlotDecision prepare_matrix_program_slot(
    std::uint8_t program_index,
    const std::array<std::int8_t, 5>& slot_programs,
    std::int32_t available_memory,
    std::uint32_t required_memory);

struct MatrixLoadedProgramPromptState {
    // The source prompt is Yes (0) / No (1).
    std::uint8_t selection = 0;
    bool redraw_requested = false;
};

// ROM 0x19368..0x19428: cycle the loaded-program erase prompt.
void update_matrix_loaded_program_prompt(
    MatrixLoadedProgramPromptState& state, std::int8_t step);

enum class MatrixLoadedProgramPromptRoute {
    erase_program,
    cancel,
    no_action,
};

// ROM 0x193F4: Yes reaches the existing slot-command mutation; No returns
// to the Matrix panel without changing the slot.
MatrixLoadedProgramPromptRoute choose_matrix_loaded_program_prompt(
    std::uint8_t selection, bool action_requested);

} // namespace shadowrun::ghidra_conversion
