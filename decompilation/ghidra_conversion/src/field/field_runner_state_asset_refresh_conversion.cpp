#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"

#include "shadowrun_ghidra/core/counter_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::int8_t kEffectCounterDelta = -2;

bool update_effect_counter(const std::uint8_t marker,
                           std::vector<std::uint8_t>& counters) {
    if (marker == 0U || static_cast<std::size_t>(marker) > counters.size()) {
        return false;
    }

    const auto counter_index = static_cast<std::size_t>(marker - 1U);
    counters[counter_index] = apply_byte_counter_delta_with_negative_floor(
        counters[counter_index], kEffectCounterDelta);
    return true;
}

} // namespace

FieldRunnerRefreshResult refresh_field_runner_state_and_assets(
    FieldRunnerRefreshState& state,
    const EntityIdentity active_runner_identity,
    const FieldRunnerRefreshCallbacks& callbacks) {
    FieldRunnerRefreshResult result{};

    if (callbacks.serialize_runner_record_population) {
        callbacks.serialize_runner_record_population();
    }

    for (std::size_t index = 0U; index < state.runners.size(); ++index) {
        const auto& runner = state.runners[index];
        if (!runner.active || runner.effect_gate != 0U ||
            runner.animation_gate != 0U) {
            continue;
        }

        result.processed[index] = true;
        const auto slot = index == 0U ? FieldRunnerSlot::primary
                                     : FieldRunnerSlot::secondary;
        const auto marker = state.compact_record_markers[index];
        state.compact_record_markers[index] = 0xFFU;
        result.counter_updates[index] =
            update_effect_counter(marker, state.effect_counters)
                ? FieldRunnerCounterUpdate::updated
                : FieldRunnerCounterUpdate::marker_unavailable;

        if (callbacks.clear_effect_slot_state) {
            callbacks.clear_effect_slot_state(slot);
        }
        if (runner.identity == active_runner_identity &&
            callbacks.handle_mode_action_event) {
            callbacks.handle_mode_action_event(slot);
        }
    }

    if (callbacks.upload_field_asset_setup) {
        callbacks.upload_field_asset_setup();
    }
    if (callbacks.run_entity_animation_triplet) {
        callbacks.run_entity_animation_triplet();
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
