#include "shadowrun_ghidra/matrix_program_slot_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::int8_t, 5> slots{2, -1, 7, -1, -1};
    const auto loaded = prepare_matrix_program_slot(7U, slots, 4, 2U);
    if (!expect(loaded.preparation ==
                    MatrixProgramSlotPreparation::already_loaded &&
                loaded.slot == 2U && loaded.slot_available)) {
        return 1;
    }
    const auto ready = prepare_matrix_program_slot(5U, slots, 4, 4U);
    if (!expect(ready.preparation ==
                    MatrixProgramSlotPreparation::ready_to_install &&
                ready.slot == 1U)) {
        return 2;
    }
    const auto insufficient = prepare_matrix_program_slot(5U, slots, 3, 4U);
    if (!expect(insufficient.preparation ==
                    MatrixProgramSlotPreparation::insufficient_memory &&
                insufficient.slot == 1U)) {
        return 3;
    }
    const auto no_slot = prepare_matrix_program_slot(
        5U, std::array<std::int8_t, 5>{0, 1, 2, 3, 4}, 100, 1U);
    if (!expect(no_slot.preparation ==
                    MatrixProgramSlotPreparation::no_available_slot &&
                !no_slot.slot_available)) {
        return 4;
    }

    MatrixLoadedProgramPromptState prompt{};
    update_matrix_loaded_program_prompt(prompt, -1);
    if (!expect(prompt.selection == 1U && prompt.redraw_requested)) {
        return 5;
    }
    if (!expect(choose_matrix_loaded_program_prompt(0U, true) ==
                    MatrixLoadedProgramPromptRoute::erase_program &&
                choose_matrix_loaded_program_prompt(1U, true) ==
                    MatrixLoadedProgramPromptRoute::cancel &&
                choose_matrix_loaded_program_prompt(0U, false) ==
                    MatrixLoadedProgramPromptRoute::no_action)) {
        return 6;
    }
    return 0;
}
