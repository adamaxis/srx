#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct ModeResourceStore {
    // The source's metadata region begins at word 0. Its second eight-word
    // region begins 0x40 bytes later, at word 16.
    std::array<std::uint32_t, 24> words{};
    bool dirty = false;
};

// ROM 0x566E2: copy eight longwords into the mode metadata region.
void refresh_mode_resource_metadata(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source);

// ROM 0x566F6: copy eight longwords into the mode state region.
void refresh_mode_resource_state(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source);

// ROM 0x5670E: copy the 24-longword palette/resource block into the shared
// mode resource area.
void refresh_mode_palette_data(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 24>& source);

} // namespace shadowrun::ghidra_conversion
