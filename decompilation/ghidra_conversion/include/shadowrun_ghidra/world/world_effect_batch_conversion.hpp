#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MenuOffsetPreparationState {
    bool prepared = false;
    std::int32_t offset = 0;
};

void prepare_menu_offset_once(
    MenuOffsetPreparationState& state,
    const std::function<std::int32_t()>& calculate_offset,
    const std::function<void()>& initialize_followup);

void copy_seven_words(const std::vector<std::uint16_t>& source,
                      std::vector<std::uint16_t>& destination);
void copy_eleven_words(const std::vector<std::uint16_t>& source,
                       std::vector<std::uint16_t>& destination);

struct WorldEffectMotionState {
    std::int16_t horizontal = 0;
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;
    std::uint16_t destination_x = 0;
    std::uint16_t destination_y = 0;
    std::int16_t lifetime = 0;
    std::int16_t palette = 0;
    bool audio_started = false;
};

// ROM 0x1CA00: initialize the world-effect motion record and service audio.
void initialize_world_effect_motion(
    WorldEffectMotionState& state,
    bool alternate_motion,
    const std::function<void()>& service_audio);

void clear_world_effect_work(
    bool source_active,
    std::array<std::uint32_t, 7>& work_values);

std::uint16_t advance_effect_vertical_position(std::uint16_t value);
std::uint16_t retreat_effect_vertical_position(std::uint16_t value);
std::uint16_t advance_effect_horizontal_position(std::uint16_t value);
std::uint16_t retreat_effect_horizontal_position(std::uint16_t value);

void forward_effect_sequence(const std::function<void()>& sequence);

struct WorldActivityState {
    std::uint8_t timeout = 0;
    std::uint16_t source_value = 0;
};

// ROM 0x1E2E0 / 0x1E2EC: start an activity timeout conditionally or
// unconditionally, retaining the source value captured at entry.
bool begin_world_activity_if_idle(WorldActivityState& state,
                                  std::uint16_t source_value);
void begin_world_activity(WorldActivityState& state, std::uint16_t source_value);

struct WorldActivityRequestRecord {
    bool active = false;      // record +0x10 bit 0
    bool high_effect = false; // record +0xB0 bit 7
};

struct WorldActivityRequestState {
    bool request_pending = false; // FFE183
    std::uint8_t reset_a = 0;     // FFE180
    std::uint8_t reset_b = 0;     // FFE182
    std::uint8_t reset_c = 0;     // FFE184
    std::uint8_t reset_d = 0;     // FFE187
    std::int8_t availability = 0; // FFE181
    std::uint32_t activity_handler_token = 0; // FFDD2C
};

enum class WorldActivityRequestAction {
    no_request,
    activity_started,
};

// ROM 0x1E43C: clear the request-side fields, scan the thirteen effect
// records for an active high-effect entry, then begin activity through the
// source handler boundary.
WorldActivityRequestAction handle_world_activity_request(
    WorldActivityRequestState& state,
    const std::vector<WorldActivityRequestRecord>& records,
    const std::function<void()>& begin_activity);

struct WorldEffectCandidateState {
    std::vector<std::pair<std::int16_t, std::int16_t>> candidates;
};

struct WorldEffectCandidateCallbacks {
    std::function<bool(std::int16_t x, std::int16_t y)> is_valid;
};

// ROM 0x1E40C: construct the source's bounded five-step rings around both
// candidate anchors, retaining only table-valid coordinates.
void build_world_effect_candidates(
    WorldEffectCandidateState& state,
    std::int16_t first_x,
    std::int16_t first_y,
    std::int16_t second_x,
    std::int16_t second_y,
    const WorldEffectCandidateCallbacks& callbacks);

// ROM 0x1E41E: resolve a compact tile-table lookup through a checked host
// vector rather than an address calculation.
std::optional<std::uint8_t> lookup_world_effect_tile(
    const std::vector<std::uint8_t>& tile_table,
    std::uint16_t first_coordinate,
    std::uint16_t second_coordinate);

struct EffectSlotState {
    std::uint8_t effect_flags = 0;
    std::uint8_t entity_flags = 0;
    bool global_dirty = false;
};

// ROM 0x1E514: clear the effect's high/active bits and clear the slot when
// the collision flag is no longer present.
bool clear_effect_slot_state(EffectSlotState& state,
                             const std::function<void()>& clear_slot);

struct EffectFadeMarkerState {
    std::int16_t marker = 0;
    std::uint8_t countdown = 0;
};

void initialize_effect_fade_marker(EffectFadeMarkerState& state);

struct WorldEffectGlobals {
    std::int16_t selected_effect = -1;
    std::uint8_t collision_flags = 0;
};

void reset_world_effect_globals(WorldEffectGlobals& state);

struct WorldEffectDispatchState {
    std::uint8_t selected_effect = 0;
    std::uint8_t flags = 0;
    std::uint8_t input_effect = 0xFF;
};

// ROM 0x1EEDC: reset the per-frame mode and dispatch only valid effect ids.
bool dispatch_world_effect(
    WorldEffectDispatchState& state,
    const std::array<std::function<void()>, 6>& handlers);

std::uint8_t advance_eight_step_counter(std::uint8_t value);

void mark_effect_bit(std::uint32_t& mask, std::int32_t source_index);

} // namespace shadowrun::ghidra_conversion
