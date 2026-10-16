#include "shadowrun_ghidra/field/mode_resource_state_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

void copy_mode_selection_state_table(
    const std::array<std::uint32_t, 24>& source,
    ModeSelectionStateTable& destination) {
    destination.words = source;
}

void copy_mode_resource_palette_bank(
    const std::array<std::uint32_t, 8>& source,
    const ModeResourcePaletteBank bank,
    ModeResourcePaletteBanks& destination) {
    const auto bank_index = static_cast<std::size_t>(bank);
    if (bank_index >= destination.banks.size()) {
        return;
    }
    destination.banks[bank_index] = source;
    destination.dirty = true;
}

void copy_mode_palette_block(
    const std::array<std::uint32_t, 24>& source,
    ModeResourcePaletteBanks& destination) {
    // 0x5670E starts at FFD89E, so the full block fills banks one, two, and
    // three. FFD87E/bank zero is the separate 0x566D8 entry-point target.
    for (std::size_t bank_index = 1U; bank_index < destination.banks.size();
         ++bank_index) {
        const auto source_offset = (bank_index - 1U) * 8U;
        std::copy_n(source.begin() + static_cast<std::ptrdiff_t>(source_offset),
                    8U, destination.banks[bank_index].begin());
    }
    destination.dirty = true;
}

void refresh_mode_resource_metadata(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source) {
    // 0x566E2 enters the shared loop with A1=FFD89E: bank one, word 8.
    std::copy(source.begin(), source.end(), store.words.begin() + 8);
    store.dirty = true;
}

void refresh_mode_resource_state(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source) {
    // 0x566F6 enters the shared loop with A1=FFD8DE: bank three, word 24.
    std::copy(source.begin(), source.end(), store.words.begin() + 24);
    store.dirty = true;
}

void refresh_mode_palette_data(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 24>& source) {
    // 0x5670E enters the same copy loop with A1=FFD89E, so its 24 words fill
    // banks one, two, and three while bank zero remains untouched.
    std::copy(source.begin(), source.end(), store.words.begin() + 8);
    store.dirty = true;
}

} // namespace shadowrun::ghidra_conversion
