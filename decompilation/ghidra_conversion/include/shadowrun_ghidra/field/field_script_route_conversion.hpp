#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

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
    std::uint8_t event_code = 0;
    bool event_table_available = false;
    std::array<std::uint8_t, 32> event_table{};
};

struct FieldScriptRoute {
    FieldScriptRouteKind kind = FieldScriptRouteKind::no_route;
    FieldScriptId script = FieldScriptId::none;
    std::uint8_t event_code = 0;
    // This is the selected byte from the area's event table. It may identify
    // a field map, resource activity, or another source activity; the caller
    // resolves its meaning through the native world-activity policy.
    std::optional<std::uint8_t> area_event_target;
};

// Prioritize the scene-2 story/service properties that own their events.
// Other events flow through the active area's table. This preserves the
// source's distinction between Stoker's C2 dialogue, Ares's CB service, and
// the paired C0 doorway that selects scene 0x14.
FieldScriptRoute resolve_field_script_route(
    const FieldScriptRouteContext& context) noexcept;

} // namespace shadowrun::ghidra_conversion
