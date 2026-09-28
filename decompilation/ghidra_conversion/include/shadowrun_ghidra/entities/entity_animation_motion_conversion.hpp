#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationPoint {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct EntityAnimationHeadingDistance {
    std::uint16_t direction = 0;
    std::uint16_t distance = 0;
};

struct EntityAnimationVectorDelta {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

enum class EntityAnimationDistanceTable {
    by_entity_type,
    by_animation_phase,
};

struct EntityAnimationDistanceBounds {
    std::uint16_t lower = 0;
    std::uint16_t upper = 0;
};

struct EntityAnimationPhaseDistanceSelection {
    std::uint8_t distance_index = 0;
    std::uint16_t source_row = 0;
};

struct EntityAnimationMotionState {
    bool global_phase_active = false; // FFE186
    std::uint8_t entity_flags = 0;     // record +0x10
    std::uint8_t entity_type = 0;      // record +0x4E
    std::uint16_t animation_mode = 0; // F FDFE0
    std::optional<EntityAnimationPoint> linked_position; // record +0xA2
    EntityAnimationPoint current_position{};              // record +0/+4
    EntityAnimationPoint target_position{};               // record +0x96/+98
    std::uint8_t route_flags = 0;       // record +0xB0
    std::uint8_t distance_type = 0;     // record +0x4D
    std::uint8_t delay = 0;              // record +0xB5
    std::int8_t animation_frame = 0;     // record +0x56
    std::uint8_t animation_phase = 0;    // record +0x57
    std::uint16_t terrain_gate = 0;      // FF D FDE
    std::uint16_t frame_counter = 0;     // record +0x16
};

using EntityAnimationHeadingDistanceCalculator =
    std::function<std::optional<EntityAnimationHeadingDistance>(
        const EntityAnimationPoint& current,
        const EntityAnimationPoint& linked)>;
using EntityAnimationDistanceBoundsProvider =
    std::function<std::optional<EntityAnimationDistanceBounds>(
        EntityAnimationDistanceTable table,
        std::uint8_t index)>;
using EntityAnimationPhaseDistanceSelector =
    std::function<std::optional<EntityAnimationPhaseDistanceSelection>(
        bool frame_is_negative,
        std::uint8_t phase)>;
using EntityAnimationVectorTransformer =
    std::function<std::optional<EntityAnimationVectorDelta>(
        std::uint16_t direction,
        std::uint16_t magnitude)>;
using EntityAnimationPositionGate = std::function<std::optional<bool>()>;
using EntityAnimationContinuation = std::function<void()>;
using EntityAnimationFinish = std::function<bool()>;

struct EntityAnimationMotionCallbacks {
    EntityAnimationHeadingDistanceCalculator measure_heading_distance;
    EntityAnimationDistanceBoundsProvider distance_bounds;
    EntityAnimationPhaseDistanceSelector select_phase_distance;
    // Source-bound callers provide both decoded action tables. The motion
    // path then calls 0x14D6A and reads the selected row's +0x27 profile
    // directly; the selector callback remains an explicit audit adapter when
    // source records have not yet been supplied.
    const UsItemCatalog* item_catalog = nullptr;
    const UsMagicSpellCatalog* magic_spell_catalog = nullptr;
    EntityAnimationVectorTransformer transform_vector;
    // The callback returns the source 0x144E8 carry. A true result enters the
    // 0x13E40 frame cutoff; false continues through 0x145A2.
    EntityAnimationPositionGate evaluate_position_gate;
    EntityAnimationContinuation continue_to_target_update;
    // The callback represents 0x13E40 and returns whether +0x3A was installed.
    EntityAnimationFinish finish_animation;
};

struct EntityAnimationMotionResult {
    bool entered = true;
    bool routed_to_finish = false;
    bool blocked_by_animation_mode = false;
    bool linked_record_present = false;
    bool linked_position_copied = false;
    bool distance_table_used = false;
    bool distance_table_unresolved = false;
    bool source_data_missing = false;
    bool target_position_copied = false;
    bool target_position_built = false;
    bool position_gate_called = false;
    bool position_gate_unresolved = false;
    bool position_gate_carry = false;
    bool routed_to_target_update = false;
    bool handler_installed = false;
    std::uint16_t direction = 0;
    std::uint16_t distance = 0;
    std::uint8_t distance_table_index = 0;
    std::uint16_t phase_source_row = 0;
};

// ROM 0x13D0C: coordinate linked-record animation movement. The source
// addresses are represented by typed positions and table callbacks; the
// host model retains the route/delay/target fields but never exposes a bus.
EntityAnimationMotionResult run_entity_animation_motion(
    EntityAnimationMotionState& state,
    const EntityAnimationMotionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
