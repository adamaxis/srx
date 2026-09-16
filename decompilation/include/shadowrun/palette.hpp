#pragma once

#include "shadowrun/genesis_bus.hpp"

#include <cstddef>
#include <cstdint>

namespace shadowrun {

namespace palette_ram {
constexpr GenesisAddress kCurrent = 0xFFD87E;
constexpr GenesisAddress kTarget = 0xFFD8FE;
constexpr std::size_t kEntryCount = 64;
}

// The Genesis color word uses three four-bit intensity fields at bits 1..3,
// 5..7, and 9..11. These routines are the exact 64-word loops at ROM 0x1088
// and 0x10D6; the color values are kept as words until a renderer chooses how
// to present them.
class PaletteAnimator {
public:
    explicit PaletteAnimator(GenesisBus& bus) : bus_(bus) {}

    void step_toward_black() const;
    void step_toward_target() const;

    // ROM 0x1368: copy 32 longwords from a caller-provided Genesis address
    // into the 64-word target palette at FFD8FE.
    void copy_target_from(GenesisAddress source) const;

    static std::uint16_t toward_black(std::uint16_t color);
    static std::uint16_t toward_target(std::uint16_t color,
                                       std::uint16_t target);

private:
    GenesisBus& bus_;
};

} // namespace shadowrun
