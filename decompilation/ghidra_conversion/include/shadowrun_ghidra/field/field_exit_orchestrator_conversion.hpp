#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldExitOrchestratorState {
    std::uint8_t controller_flags = 0; // FF0110
    bool prompt_display_prepared = false; // FFF10C
    std::int8_t service_marker = 0; // FFF0CC
    // The caller at ROM 0x09B2 tests the D6 word left by this orchestrator.
    // Converted route services publish the behaviorally relevant result here
    // instead of leaking a host register or a source address.
    std::int16_t caller_exit_status = 0; // D6
};

using FieldExitOrchestratorStage =
    std::function<bool(FieldExitOrchestratorState&)>;

struct FieldExitOrchestratorCallbacks {
    std::function<void()> route_to_graphics_service;
    FieldExitOrchestratorStage prepare_world_exit;
    FieldExitOrchestratorStage dispatch_world_effect;
    FieldExitOrchestratorStage run_indexed_title_services;
    FieldExitOrchestratorStage select_field_exit_prompt;
    FieldExitOrchestratorStage prepare_field_mode_exit;
    FieldExitOrchestratorStage select_field_mode_service;
    FieldExitOrchestratorStage finalize_field_mode_exit;
};

enum class FieldExitOrchestratorAction {
    graphics_service_routed,
    aborted_after_world_exit,
    aborted_after_world_effect,
    aborted_after_indexed_services,
    aborted_during_scene_route,
    completed,
};

// ROM 0x1F5CA: gate controller-owned graphics handling, clear the transient
// prompt/service markers, run world/title services, and invoke the four
// scene-specific exit policies in order until one reports the source D6<0
// abort condition.
FieldExitOrchestratorAction run_field_exit_orchestrator(
    FieldExitOrchestratorState& state,
    const FieldExitOrchestratorCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
