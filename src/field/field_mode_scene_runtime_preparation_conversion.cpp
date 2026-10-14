#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldSceneRuntimePreparationDecision prepare_field_scene_runtime(
    const std::uint8_t field_action_counter,
    const FieldSceneRuntimePreparationCallbacks& callbacks) {
    if (field_action_counter != 0U) {
        if (callbacks.refresh_runner_state_and_assets) {
            callbacks.refresh_runner_state_and_assets();
            return {FieldSceneRuntimePreparationPath::
                        refresh_runner_state_and_assets,
                    true};
        }
        return {FieldSceneRuntimePreparationPath::
                    refresh_runner_state_and_assets,
                false};
    }

    if (callbacks.finalize_field_mode_exit_transition) {
        callbacks.finalize_field_mode_exit_transition();
        return {FieldSceneRuntimePreparationPath::
                    finalize_field_mode_exit_transition,
                true};
    }
    return {FieldSceneRuntimePreparationPath::
                finalize_field_mode_exit_transition,
            false};
}

} // namespace shadowrun::ghidra_conversion
