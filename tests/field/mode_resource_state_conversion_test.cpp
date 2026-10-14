#include "shadowrun_ghidra/field/mode_resource_state_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeResourceStore store{};
    store.words.fill(0xFFFFFFFFU);
    const std::array<std::uint32_t, 8> metadata{
        0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U};
    const std::array<std::uint32_t, 8> state{
        0x20U, 0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U};

    refresh_mode_resource_metadata(store, metadata);
    if (!store.dirty || !std::equal(metadata.begin(), metadata.end(),
                                     store.words.begin() + 8) ||
        store.words[0] != 0xFFFFFFFFU || store.words[16] != 0xFFFFFFFFU ||
        store.words[24] != 0xFFFFFFFFU) {
        return 1;
    }

    store.dirty = false;
    refresh_mode_resource_state(store, state);
    if (!store.dirty || !std::equal(state.begin(), state.end(),
                                    store.words.begin() + 24) ||
        store.words[0] != 0xFFFFFFFFU || store.words[8] != metadata[0] ||
        store.words[16] != 0xFFFFFFFFU) {
        return 2;
    }

    std::array<std::uint32_t, 24> palette{};
    for (std::size_t index = 0; index < palette.size(); ++index) {
        palette[index] = static_cast<std::uint32_t>(0x300U + index);
    }
    store.dirty = false;
    refresh_mode_palette_data(store, palette);
    if (!store.dirty ||
        !std::equal(palette.begin(), palette.end(), store.words.begin() + 8) ||
        store.words[0] != 0xFFFFFFFFU) {
        return 3;
    }

    return 0;
}
