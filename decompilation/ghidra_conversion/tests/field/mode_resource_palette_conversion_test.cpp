#include "shadowrun_ghidra/field/mode_resource_state_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeSelectionStateTable selection_table{};
    std::array<std::uint32_t, 24> selection_source{};
    for (std::size_t index = 0; index < selection_source.size(); ++index) {
        selection_source[index] = static_cast<std::uint32_t>(0x100U + index);
    }
    copy_mode_selection_state_table(selection_source, selection_table);
    if (selection_table.words != selection_source) {
        return 1;
    }

    ModeResourcePaletteBanks palette{};
    for (auto& bank : palette.banks) {
        bank.fill(0xFFFFFFFFU);
    }
    std::array<std::uint32_t, 8> bank_source{};
    for (std::size_t index = 0; index < bank_source.size(); ++index) {
        bank_source[index] = static_cast<std::uint32_t>(0x200U + index);
    }
    copy_mode_resource_palette_bank(
        bank_source, ModeResourcePaletteBank::bank_zero, palette);
    if (!palette.dirty || palette.banks[0] != bank_source ||
        palette.banks[1][0] != 0xFFFFFFFFU) {
        return 2;
    }

    std::array<std::uint32_t, 24> block_source{};
    for (std::size_t index = 0; index < block_source.size(); ++index) {
        block_source[index] = static_cast<std::uint32_t>(0x300U + index);
    }
    palette.dirty = false;
    copy_mode_palette_block(block_source, palette);
    if (!palette.dirty || palette.banks[0] != bank_source) {
        return 3;
    }
    for (std::size_t bank_index = 1U; bank_index < palette.banks.size();
         ++bank_index) {
        for (std::size_t word = 0; word < palette.banks[bank_index].size();
             ++word) {
            const auto expected = block_source[(bank_index - 1U) * 8U + word];
            if (palette.banks[bank_index][word] != expected) {
                return 4;
            }
        }
    }

    palette.dirty = false;
    copy_mode_resource_palette_bank(
        bank_source, ModeResourcePaletteBank::bank_three, palette);
    if (!palette.dirty || palette.banks[3] != bank_source ||
        palette.banks[1][0] != block_source[0]) {
        return 5;
    }
    return 0;
}
