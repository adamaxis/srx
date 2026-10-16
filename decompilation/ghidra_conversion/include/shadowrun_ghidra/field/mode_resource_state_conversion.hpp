#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class ModeResourcePaletteBank : std::uint8_t {
    bank_zero = 0,
    bank_one = 1,
    bank_two = 2,
    bank_three = 3,
};

// The four eight-longword destinations entered through 0x566D8, 0x566E2,
// 0x566EC, and 0x566F6. The source stores them at FFD87E, FFD89E,
// FFD8BE, and FFD8DE respectively; the host keeps them as typed banks.
struct ModeResourcePaletteBanks {
    std::array<std::array<std::uint32_t, 8>, 4> banks{};
    bool dirty = false;
};

// ROM 0x566C2: copy the fixed 24-longword mode-selection table from the
// extracted source table into the working state table at FFD91E.
struct ModeSelectionStateTable {
    std::array<std::uint32_t, 24> words{};
};

void copy_mode_selection_state_table(
    const std::array<std::uint32_t, 24>& source,
    ModeSelectionStateTable& destination);

// ROM 0x566D8/0x566E2/0x566EC/0x566F6 and their shared 0x566FC loop: copy
// eight longwords into the selected palette/resource bank and set the source
// palette-dirty bit. The bank enum names the actual entry-point destination,
// not an inferred gameplay meaning.
void copy_mode_resource_palette_bank(
    const std::array<std::uint32_t, 8>& source,
    ModeResourcePaletteBank bank,
    ModeResourcePaletteBanks& destination);

// ROM 0x5670E: copy the 24-longword source palette block into destinations
// FFD89E, FFD8BE, and FFD8DE (banks one through three), then set the same
// palette-dirty bit.
void copy_mode_palette_block(
    const std::array<std::uint32_t, 24>& source,
    ModeResourcePaletteBanks& destination);

struct ModeResourceStore {
    // The four source destinations are contiguous eight-longword banks:
    // FFD87E, FFD89E, FFD8BE, and FFD8DE. The shared store therefore has 32
    // words. The bulk refresh at 0x5670E starts at bank one, not bank zero.
    std::array<std::uint32_t, 32> words{};
    bool dirty = false;
};

// ROM 0x566E2: copy eight longwords into bank one at FFD89E.
void refresh_mode_resource_metadata(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source);

// ROM 0x566F6: copy eight longwords into bank three at FFD8DE.
void refresh_mode_resource_state(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source);

// ROM 0x5670E: copy the 24-longword palette/resource block into banks one
// through three, beginning at FFD89E.
void refresh_mode_palette_data(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 24>& source);

} // namespace shadowrun::ghidra_conversion
