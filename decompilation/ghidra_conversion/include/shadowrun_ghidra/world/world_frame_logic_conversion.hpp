#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct WorldFrameLogicState {
    std::uint8_t scene_state = 0;          // FFE186
    std::int16_t transition_marker = 0;    // FFF13E +2
    std::int8_t region_or_mode = 0;        // FF044C
    std::uint16_t activity_countdown = 0;  // FFE166
    std::uint16_t resource_countdown = 0;  // FFE11C
    std::uint16_t world_active = 0;        // FFDF84
    std::uint8_t event_lock = 0;           // FFE16F
    std::uint16_t world_mode = 0;          // FFDDC4 scene ID [C].
    std::uint16_t field_mode = 0;          // FFDF40 field mode [C].
    std::int16_t cleanup_countdown = 0;    // FFDF86
    bool current_target_present = false;
    bool current_target_active = false;
    bool current_entity_special = false;   // FF??56 sign bit
    std::uint8_t current_entity_mode = 0;  // FF??57
    bool current_target_effect_locked = false;
    std::uint32_t activity_handler_token = 0;
    bool activity_started = false;
    bool target_selection_requested = false;
    bool service_records_reset = false;
};

struct WorldFrameLogicCallbacks {
    std::function<void()> prepare_mode_entry;
    std::function<void()> begin_world_activity;
    std::function<void()> select_entity_service_target;
    std::function<void()> reset_entity_service_records;
    std::function<void()> clear_inactive_entity_effects;
    std::function<void()> process_terrain_work_batch;
    std::function<void()> advance_world_effect_trigger;
    std::function<void()> trigger_world_effect_sequence;
    std::function<void()> resolve_nearby_effects;
};

// ROM 0x1DD94: execute one world-logic frame in source order. The native
// conversion owns only typed state and calls out to the already-separated
// service/system boundaries.
void advance_world_frame_logic(WorldFrameLogicState& state,
                               const WorldFrameLogicCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
