#include "shadowrun_ghidra/cyberspace/matrix_program_slot_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::int8_t, 5> slots{2, -1, 7, -1, -1};
    const std::array<std::uint8_t, 12> availability{0, 0, 1, 0, 0, 0,
                                                     0, 1, 0, 0, 0, 0};
    const auto slot_entry = read_matrix_program_availability_for_slot(
        2U, slots, availability);
    const auto selected = read_selected_matrix_program_availability(
        2U, slots, availability);
    const auto empty = read_selected_matrix_program_availability(
        1U, slots, availability);
    const auto invalid = read_selected_matrix_program_availability(
        5U, slots, availability);
    if (!expect(slot_entry.slot_valid && slot_entry.program_index == 7 &&
                slot_entry.available && selected.slot_valid &&
                selected.program_index == 7 && selected.available &&
                empty.slot_valid &&
                empty.program_index == -1 && !empty.available &&
                !invalid.slot_valid)) {
        return 1;
    }
    const auto loaded = prepare_matrix_program_slot(7U, slots, 4, 2U);
    if (!expect(loaded.preparation ==
                    MatrixProgramSlotPreparation::already_loaded &&
                loaded.slot == 2U && loaded.slot_available)) {
        return 2;
    }
    const auto ready = prepare_matrix_program_slot(5U, slots, 4, 4U);
    if (!expect(ready.preparation ==
                    MatrixProgramSlotPreparation::ready_to_install &&
                ready.slot == 1U)) {
        return 3;
    }
    const auto insufficient = prepare_matrix_program_slot(5U, slots, 3, 4U);
    if (!expect(insufficient.preparation ==
                    MatrixProgramSlotPreparation::insufficient_memory &&
                insufficient.slot == 1U)) {
        return 4;
    }
    const auto no_slot = prepare_matrix_program_slot(
        5U, std::array<std::int8_t, 5>{0, 1, 2, 3, 4}, 100, 1U);
    if (!expect(no_slot.preparation ==
                    MatrixProgramSlotPreparation::no_available_slot &&
                !no_slot.slot_available)) {
        return 5;
    }

    MatrixLoadedProgramPromptState prompt{};
    update_matrix_loaded_program_prompt(prompt, -1);
    if (!expect(prompt.selection == 1U && prompt.redraw_requested)) {
        return 6;
    }
    if (!expect(choose_matrix_loaded_program_prompt(0U, true) ==
                    MatrixLoadedProgramPromptRoute::erase_program &&
                choose_matrix_loaded_program_prompt(1U, true) ==
                    MatrixLoadedProgramPromptRoute::cancel &&
                choose_matrix_loaded_program_prompt(0U, false) ==
                    MatrixLoadedProgramPromptRoute::no_action)) {
        return 7;
    }
    return 0;
}
