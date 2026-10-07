#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <optional>

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 32> scene2_events{};
    scene2_events[0] = 0x14U;
    scene2_events[2] = 0x1AU;
    scene2_events[11] = 0x33U;

    const auto stoker = resolve_field_script_route(
        {2U, 2U, 2U, true, scene2_events});
    if (!expect(stoker.kind == FieldScriptRouteKind::dialogue &&
                    stoker.script == FieldScriptId::stoker_motel_interaction &&
                    !stoker.area_event_target.has_value(),
                "scene-2 event 2 must select Stoker before its table row")) {
        return 1;
    }

    const auto motel_door = resolve_field_script_route(
        {2U, 2U, 0U, true, scene2_events});
    if (!expect(motel_door.kind == FieldScriptRouteKind::area_event &&
                    motel_door.script == FieldScriptId::area_event_table &&
                    motel_door.area_event_target ==
                        std::optional<std::uint8_t>{0x14U},
                "scene-2 event 0 must use its scene-20 destination row")) {
        return 1;
    }

    const auto ares = resolve_field_script_route(
        {2U, 2U, 11U, true, scene2_events});
    if (!expect(ares.kind == FieldScriptRouteKind::field_service &&
                    ares.script == FieldScriptId::ares_weapon_emporium &&
                    !ares.area_event_target.has_value(),
                "scene-2 event 11 must select the Ares service before row 11")) {
        return 1;
    }

    std::array<std::uint8_t, 32> scene4_events{};
    scene4_events[2] = 0x0DU;
    const auto other_area = resolve_field_script_route(
        {4U, 4U, 2U, true, scene4_events});
    if (!expect(other_area.kind == FieldScriptRouteKind::area_event &&
                    other_area.area_event_target ==
                        std::optional<std::uint8_t>{0x0DU},
                "the same event byte in another area must use that area's row")) {
        return 1;
    }

    const auto missing_table = resolve_field_script_route(
        {4U, 4U, 2U, false, scene4_events});
    const auto out_of_range = resolve_field_script_route(
        {4U, 4U, 32U, true, scene4_events});
    if (!expect(missing_table.kind == FieldScriptRouteKind::no_route &&
                    out_of_range.kind == FieldScriptRouteKind::no_route,
                "missing or out-of-range event rows must remain unresolved")) {
        return 1;
    }

    return 0;
}
