#include "shadowrun_ghidra/world_effect_initializer_conversion.hpp"

#include <cstdint>
#include <optional>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldEffectInitializationTable table;
    table.records.resize(8U);
    table.records[3] = {5U, 9U, 100U, 2U};

    WorldEffectInitializationState state{};
    state.context = 0U;
    state.setup_marker = 7U;
    state.random_scale = 3U;
    int resets = 0;
    int dispatched = 0;
    std::uint8_t dispatched_handler = 0U;
    int random_calls = 0;
    const auto action = initialize_world_effect_selection(
        state, table,
        WorldEffectInitializationCallbacks{
            [&](const std::uint16_t bound) -> std::optional<std::uint16_t> {
                ++random_calls;
                if (bound == 8U) {
                    return 3U;
                }
                if (bound == 7U) {
                    return 2U;
                }
                return std::nullopt;
            },
            [&] { ++resets; },
            [&](const std::uint8_t handler) {
                ++dispatched;
                dispatched_handler = handler;
                return true;
            }});
    if (action != WorldEffectInitializationAction::initialized ||
        state.setup_marker != 0U || state.selected_context != 0U ||
        state.handler_index != 5U || state.auxiliary_value != 9U ||
        state.base_value != 100U || state.randomized_value != 110U ||
        resets != 1 || dispatched != 1 || dispatched_handler != 5U ||
        random_calls != 2) {
        return 1;
    }

    state.context = 1U;
    if (initialize_world_effect_selection(
            state, table,
            WorldEffectInitializationCallbacks{
                [](const std::uint16_t) {
                    return std::optional<std::uint16_t>{0U};
                },
                {},
                {}}) != WorldEffectInitializationAction::table_unresolved) {
        return 2;
    }

    state.context = 0U;
    if (initialize_world_effect_selection(
            state, table,
            WorldEffectInitializationCallbacks{
                [](const std::uint16_t) {
                    return std::optional<std::uint16_t>{0U};
                },
                {},
                [](const std::uint8_t) { return false; }}) !=
        WorldEffectInitializationAction::initializer_unresolved) {
        return 3;
    }

    if (initialize_world_effect_selection(state, table, {}) !=
        WorldEffectInitializationAction::random_unresolved) {
        return 4;
    }
    return 0;
}
