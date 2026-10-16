#pragma once

#include "shadowrun_ghidra/world/world_activity_mode.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

enum class FieldSceneId : std::uint8_t {
    stoker_motel_street = 2,
    scene_20 = 20,
};

constexpr std::uint8_t field_scene_id_value(
    const FieldSceneId scene_id) noexcept {
    return static_cast<std::uint8_t>(scene_id);
}

enum class FieldExitMode : std::uint8_t {
    // 0x167AE's resolved event-table path does not select a direct building
    // interaction override.
    world_activity_table = 0,
    building_interaction = 2,
};

constexpr std::uint8_t field_exit_mode_value(
    const FieldExitMode mode) noexcept {
    return static_cast<std::uint8_t>(mode);
}

// The projected collision's lower five bits identify the map event that Lua
// evaluates. The enum is shared by the event policy and building router.
enum class FieldCollisionEventCode : std::uint8_t {
    event_0 = 0,
    event_1 = 1,
    event_2 = 2,
    event_3 = 3,
    event_4 = 4,
    event_5 = 5,
    event_6 = 6,
    event_7 = 7,
    event_8 = 8,
    event_9 = 9,
    event_10 = 10,
    event_11 = 11,
    event_12 = 12,
    event_13 = 13,
    event_14 = 14,
    event_15 = 15,
    event_16 = 16,
    event_17 = 17,
    event_18 = 18,
    event_19 = 19,
    event_20 = 20,
    event_21 = 21,
    event_22 = 22,
    event_23 = 23,
    event_24 = 24,
    event_25 = 25,
    event_26 = 26,
    event_27 = 27,
    event_28 = 28,
    event_29 = 29,
    event_30 = 30,
    event_31 = 31,
    no_event = 255,
};

constexpr FieldCollisionEventCode field_collision_event_code(
    const std::uint16_t projected_collision_code) noexcept {
    return static_cast<FieldCollisionEventCode>(
        projected_collision_code & 31U);
}

constexpr FieldCollisionEventCode field_collision_event_code_from_id(
    const std::uint16_t event_id) noexcept {
    return event_id <= 31U
               ? static_cast<FieldCollisionEventCode>(event_id)
               : FieldCollisionEventCode::no_event;
}

constexpr std::uint8_t field_collision_event_value(
    const FieldCollisionEventCode event_code) noexcept {
    return static_cast<std::uint8_t>(event_code);
}

// A field collision publishes an event identity. The route planner turns
// that identity and the active area's authored event table into a native
// script choice; callers then bind that choice to dialogue, a service, or a
// scene/activity transition. This is not a ROM command table interpreter.
enum class FieldScriptId {
    none,
    stoker_motel_interaction,
    ares_weapon_emporium,
    area_event_table,
};

enum class FieldScriptRouteKind {
    no_route,
    dialogue,
    field_service,
    area_event,
};

struct FieldScriptRouteContext {
    std::uint8_t scene_id = 0;
    std::uint8_t field_mode = 0;
    FieldCollisionEventCode event_code = FieldCollisionEventCode::no_event;
    bool event_table_available = false;
    std::array<std::uint8_t, 32> event_table{};
};

struct FieldScriptRoute {
    FieldScriptRouteKind kind = FieldScriptRouteKind::no_route;
    FieldScriptId script = FieldScriptId::none;
    std::uint8_t event_code = 0;
    // This is the selected byte from the area's event table. It may identify
    // a field map, resource activity, or another source activity. Lua also
    // returns the typed branch selected from the ROM's destination ranges.
    std::optional<std::uint8_t> area_event_target;
    std::optional<WorldActivityMode> area_event_destination_mode;
};

} // namespace shadowrun::ghidra_conversion
