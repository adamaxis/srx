#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <cstdint>

namespace shadowrun {

// The small state-stream helpers at ROM 0x3674 and 0x37EC both finish by
// calling the renderer-neutral 0x1232 VDP grid streamer. Keeping the selected
// source and DBF values in the result makes the table choice observable in
// tests without coupling the game logic to a graphics backend.
struct EntityStateStreamResult {
    bool streamed;
    bool nested_0x3674_call;
    GenesisAddress source;
    std::uint32_t command;
    std::uint16_t inner_dbf_counter;
    std::uint16_t outer_dbf_counter;
    std::uint8_t phase;
};

struct EntityAuxiliaryStateResult {
    bool source_delta_was_nonzero;
    EntityStateStreamResult position_stream;
    EntityStateStreamResult animation_stream;
    std::uint8_t flags_after;
    std::uint32_t field_0x3A_after;
};

struct EntityRateResult {
    std::uint16_t quotient;
    std::uint16_t residual;
    std::uint16_t selected_result;
};

struct EntityReactionResult {
    bool first_threshold_passed;
    bool second_threshold_passed;
    bool third_threshold_passed;
    bool position_update_called;
    bool animation_update_called;
    bool frame_tick_advanced;
    std::uint8_t random_bucket;
    std::uint8_t final_position_phase;
    std::uint8_t final_animation_phase;
};

struct EntityScalingResult {
    std::uint16_t half_base;
    std::uint16_t adjusted_base;
    std::uint16_t selected_result;
};

struct EntityAnimationStartResult {
    bool started;
    bool skipped_by_flag;
    bool unresolved_0xB330_path;
    std::uint16_t selected_frame;
    std::uint8_t timer_after;
    std::uint8_t flags_after;
    std::uint32_t routine_pointer_after;
};

// Direct translations of the compact state-grid emitters reached by ROM
// 0x55C2C. delta is the low byte placed in D1 by the caller.
EntityStateStreamResult update_entity_state_0x3674(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t delta);
EntityStateStreamResult update_entity_state_0x37EC(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint8_t delta);

// Direct translation of ROM 0x55C2C. source_record_address is A1 at entry;
// entity_record_address is A0. The caller-side 0x55C76 transition is exposed
// separately below.
EntityAuxiliaryStateResult update_entity_auxiliary_state_0x55C2C(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress source_record_address);

// Register-explicit leaf translations used by the larger 0x55C76 state
// machine. They do not hide the original D5/D6/D1 data flow behind globals.
std::uint16_t adjust_entity_cost_0x55E62(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5);
std::uint16_t subtract_entity_modifier_0x55E92(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5);
std::uint16_t subtract_entity_modifier_0x55EB4(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d5);
std::uint16_t calculate_entity_base_0x55ED6(
    GenesisBus& bus,
    GenesisAddress entity_record_address);
EntityRateResult calculate_entity_rate_0x55F20(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d1,
    bool d7_negative);

std::uint8_t roll_entity_threshold_0x0DBA(
    GenesisBus& bus,
    std::uint16_t d4,
    std::uint16_t d6);
std::uint16_t adjust_entity_timing_0x55740(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress target_record_address,
    std::uint16_t d4);
GenesisAddress select_entity_parameter_block_0x55828(
    GenesisBus& bus,
    GenesisAddress entity_record_address);
std::uint8_t calculate_entity_phase_value_0x55F02(
    GenesisBus& bus,
    GenesisAddress entity_record_address);
std::uint16_t calculate_global_phase_bonus_0x1047E(GenesisBus& bus);
std::uint16_t adjust_entity_timing_0x55786(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    std::uint16_t d4,
    std::uint16_t d6);

// Direct translation of the complete 0x55C76 transition routine. A0 and A1
// are explicit because the ROM compares and updates two entity records.
EntityReactionResult run_entity_reaction_0x55C76(
    GenesisBus& bus,
    GenesisAddress entity_record_address,
    GenesisAddress target_record_address);

// Direct translations of ROM 0xD81C and 0x55F44. d1 is the caller's signed
// word; selected_result is the value left in D6 by 0x55F44.
std::uint16_t calculate_entity_half_base_0xD81C(
    GenesisBus& bus,
    GenesisAddress entity_record_address);
EntityScalingResult scale_entity_value_0x55F44(
    GenesisBus& bus,
    std::uint16_t d1);

// Direct translation of ROM 0xB330, including its 0xABB6 wake-timer checks,
// command-table lookup, alternate routine, and frame table.
EntityAnimationStartResult initialize_entity_animation_0xB330(
    GenesisBus& bus, GenesisAddress entity_record_address);

// Direct translation of ROM 0xAFF2. Both the ordinary +0x4C path and its
// 0xB330 fallback are bus-backed; the hook users may provide remains outside
// this leaf API.
EntityAnimationStartResult initialize_entity_animation_0xAFF2(
    GenesisBus& bus,
    GenesisAddress entity_record_address);

} // namespace shadowrun
