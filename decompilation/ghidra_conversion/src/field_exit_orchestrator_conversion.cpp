#include "shadowrun_ghidra/field_exit_orchestrator_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool run_stage(const FieldExitOrchestratorStage& stage,
               FieldExitOrchestratorState& state) {
    return stage && stage(state);
}

} // namespace

FieldExitOrchestratorAction run_field_exit_orchestrator(
    FieldExitOrchestratorState& state,
    const FieldExitOrchestratorCallbacks& callbacks) {
    if ((state.controller_flags & 0x40U) != 0U ||
        (state.controller_flags & 0x10U) != 0U) {
        if (callbacks.route_to_graphics_service) {
            callbacks.route_to_graphics_service();
        }
        return FieldExitOrchestratorAction::graphics_service_routed;
    }

    if (run_stage(callbacks.prepare_world_exit, state)) {
        state.service_marker = 0;
        return FieldExitOrchestratorAction::aborted_after_world_exit;
    }
    state.prompt_display_prepared = false;
    state.service_marker = 0;

    if (run_stage(callbacks.dispatch_world_effect, state) ||
        static_cast<std::int8_t>(state.service_marker) < 0) {
        state.service_marker = 0;
        return FieldExitOrchestratorAction::aborted_after_world_effect;
    }

    if (run_stage(callbacks.run_indexed_title_services, state) ||
        static_cast<std::int8_t>(state.service_marker) < 0) {
        state.service_marker = 0;
        return FieldExitOrchestratorAction::aborted_after_indexed_services;
    }

    const FieldExitOrchestratorStage routes[] = {
        callbacks.select_field_exit_prompt,
        callbacks.prepare_field_mode_exit,
        callbacks.select_field_mode_service,
        callbacks.finalize_field_mode_exit,
    };
    for (const auto& route : routes) {
        if (run_stage(route, state)) {
            state.service_marker = 0;
            return FieldExitOrchestratorAction::aborted_during_scene_route;
        }
    }

    state.service_marker = 0;
    return FieldExitOrchestratorAction::completed;
}

} // namespace shadowrun::ghidra_conversion
