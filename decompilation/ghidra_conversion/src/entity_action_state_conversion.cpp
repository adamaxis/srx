#include "shadowrun_ghidra/entity_action_state_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void set_entity_action_state(
    EntityActionState& state,
    const std::uint8_t action_code,
    const std::uint8_t default_state_code,
    const std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets) {
    state.action_code = action_code;
    if (action_code == 0U) {
        state.action_code = 0U;
        state.reset_marker = 0U;
        state.state_code = default_state_code;
    } else {
        state.state_code = nonzero_state_code;
        if (state.active_marker != 0U) {
            state.active_marker = 0U;
            state.reset_marker = 0U;
        }
    }
    if (service_action) {
        service_action();
    }
    if (prepare_assets) {
        prepare_assets();
    }
}

} // namespace shadowrun::ghidra_conversion
