#include "shadowrun/field/native_port.hpp"

#include <array>
#include <utility>

namespace shadowrun::native {

InputState decode_genesis_controller_word(std::uint16_t sampled_word) {
    constexpr std::array<std::pair<std::uint16_t, Button>, 8> kButtons{{
        {kGenesisInputUp, Button::Up},
        {kGenesisInputDown, Button::Down},
        {kGenesisInputLeft, Button::Left},
        {kGenesisInputRight, Button::Right},
        {kGenesisInputB, Button::B},
        {kGenesisInputC, Button::C},
        {kGenesisInputA, Button::A},
        {kGenesisInputStart, Button::Start},
    }};

    InputState result{};
    for (const auto [genesis_mask, button] : kButtons) {
        if ((sampled_word & genesis_mask) != 0) {
            result.held = static_cast<std::uint16_t>(
                result.held | button_mask(button));
        }
        if ((sampled_word & static_cast<std::uint16_t>(genesis_mask << 8U)) !=
            0) {
            result.pressed = static_cast<std::uint16_t>(
                result.pressed | button_mask(button));
        }
    }
    return result;
}

} // namespace shadowrun::native
