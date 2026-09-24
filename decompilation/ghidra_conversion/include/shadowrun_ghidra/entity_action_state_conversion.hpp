#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityActionState {
    std::uint8_t action_code = 0;
    std::uint8_t state_code = 0;
    std::uint8_t active_marker = 0;
    std::uint8_t reset_marker = 0;
};

// ROM 0x13466: publish an action/state pair, clear the source reset markers
// on the zero/nonzero branches, then refresh action assets.
void set_entity_action_state(
    EntityActionState& state,
    std::uint8_t action_code,
    std::uint8_t default_state_code,
    std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets);

} // namespace shadowrun::ghidra_conversion
