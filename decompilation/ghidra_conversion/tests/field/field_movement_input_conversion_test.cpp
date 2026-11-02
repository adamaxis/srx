#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    // ROM 0x19D2 accepts these eight single/adjacent input combinations.
    // Empty, opposing, and three/four-way combinations have no movement.
    constexpr std::array<std::optional<MovementDirection>, 16> expected{{
        std::nullopt,
        MovementDirection::north,
        MovementDirection::south,
        std::nullopt,
        MovementDirection::west,
        // 0x19D2[5]=0xE000 (Up+Left), [6]=0xA000 (Down+Left).
        MovementDirection::north_west,
        MovementDirection::south_west,
        std::nullopt,
        MovementDirection::east,
        MovementDirection::north_east,
        MovementDirection::south_east,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
    }};

    for (std::uint8_t input = 0U; input < expected.size(); ++input) {
        if (field_movement_direction_from_input(input) != expected[input]) {
            return static_cast<int>(input) + 1;
        }
    }
    return 0;
}
