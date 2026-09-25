#include "shadowrun_ghidra/matrix_program_slot_conversion.hpp"

#include "shadowrun_ghidra/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

MatrixProgramSlotDecision prepare_matrix_program_slot(
    const std::uint8_t program_index,
    const std::array<std::int8_t, 5>& slot_programs,
    const std::int32_t available_memory,
    const std::uint32_t required_memory) {
    for (std::uint8_t slot = 0; slot < slot_programs.size(); ++slot) {
        if (slot_programs[slot] ==
            static_cast<std::int8_t>(program_index)) {
            return {MatrixProgramSlotPreparation::already_loaded, slot, true};
        }
    }

    for (std::uint8_t slot = 0; slot < slot_programs.size(); ++slot) {
        if (slot_programs[slot] >= 0) {
            continue;
        }
        if (available_memory < static_cast<std::int32_t>(required_memory)) {
            return {MatrixProgramSlotPreparation::insufficient_memory, slot,
                    true};
        }
        return {MatrixProgramSlotPreparation::ready_to_install, slot, true};
    }
    return {MatrixProgramSlotPreparation::no_available_slot, 0, false};
}

void update_matrix_loaded_program_prompt(
    MatrixLoadedProgramPromptState& state, const std::int8_t step) {
    const auto next = move_wrapped_menu_cursor(state.selection, step, 1U);
    if (next == state.selection) {
        return;
    }
    state.selection = next;
    state.redraw_requested = true;
}

MatrixLoadedProgramPromptRoute choose_matrix_loaded_program_prompt(
    const std::uint8_t selection, const bool action_requested) {
    if (!action_requested) {
        return MatrixLoadedProgramPromptRoute::no_action;
    }
    return selection == 0U ? MatrixLoadedProgramPromptRoute::erase_program
                           : MatrixLoadedProgramPromptRoute::cancel;
}

} // namespace shadowrun::ghidra_conversion
