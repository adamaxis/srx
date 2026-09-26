#include "shadowrun_ghidra/field/mode_resource_state_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

void refresh_mode_resource_metadata(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source) {
    std::copy(source.begin(), source.end(), store.words.begin());
    store.dirty = true;
}

void refresh_mode_resource_state(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 8>& source) {
    std::copy(source.begin(), source.end(), store.words.begin() + 16);
    store.dirty = true;
}

void refresh_mode_palette_data(
    ModeResourceStore& store,
    const std::array<std::uint32_t, 24>& source) {
    std::copy(source.begin(), source.end(), store.words.begin());
    store.dirty = true;
}

} // namespace shadowrun::ghidra_conversion
