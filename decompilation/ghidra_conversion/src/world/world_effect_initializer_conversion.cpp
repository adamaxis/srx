#include "shadowrun_ghidra/world/world_effect_initializer_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {

WorldEffectInitializationAction initialize_world_effect_selection(
    WorldEffectInitializationState& state,
    const WorldEffectInitializationTable& table,
    const WorldEffectInitializationCallbacks& callbacks) {
    state.setup_marker = 0U;
    if (callbacks.reset_effect_globals) {
        callbacks.reset_effect_globals();
    }

    state.selected_context = state.context;
    if (!callbacks.next_random) {
        return WorldEffectInitializationAction::random_unresolved;
    }

    const auto first_roll = callbacks.next_random(
        kWorldEffectInitializerFirstRollBound);
    if (!first_roll.has_value() ||
        *first_roll >= kWorldEffectInitializerFirstRollBound) {
        return WorldEffectInitializationAction::random_unresolved;
    }

    constexpr std::size_t records_per_context = 8U;
    const auto record_index = static_cast<std::size_t>(state.context) *
                                  records_per_context + *first_roll;
    if (record_index >= table.records.size()) {
        return WorldEffectInitializationAction::table_unresolved;
    }

    const auto& record = table.records[record_index];
    state.handler_index = record.handler_index;
    state.auxiliary_value = record.auxiliary_value;
    state.base_value = record.base_value;

    const auto second_bound = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(record.random_weight) *
            state.random_scale +
        1U);
    const auto second_roll = callbacks.next_random(second_bound);
    if (!second_roll.has_value() || *second_roll >= second_bound) {
        return WorldEffectInitializationAction::random_unresolved;
    }
    state.randomized_value = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(record.base_value) +
        static_cast<std::uint32_t>(*second_roll) * 5U);

    if (!callbacks.dispatch_initializer ||
        !callbacks.dispatch_initializer(state.handler_index)) {
        return WorldEffectInitializationAction::initializer_unresolved;
    }
    return WorldEffectInitializationAction::initialized;
}

} // namespace shadowrun::ghidra_conversion
