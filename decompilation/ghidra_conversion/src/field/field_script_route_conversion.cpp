#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldScriptRoute resolve_field_script_route(
    const FieldScriptRouteContext& context) noexcept {
    if (context.scene_id == 2U && context.field_mode == 2U &&
        context.event_code == 2U) {
        return {FieldScriptRouteKind::dialogue,
                FieldScriptId::stoker_motel_interaction,
                context.event_code,
                std::nullopt};
    }

    if (context.scene_id == 2U && context.field_mode == 2U &&
        context.event_code == 11U) {
        return {FieldScriptRouteKind::field_service,
                FieldScriptId::ares_weapon_emporium,
                context.event_code,
                std::nullopt};
    }

    if (!context.event_table_available ||
        context.event_code >= context.event_table.size()) {
        return {FieldScriptRouteKind::no_route,
                FieldScriptId::none,
                context.event_code,
                std::nullopt};
    }

    return {FieldScriptRouteKind::area_event,
            FieldScriptId::area_event_table,
            context.event_code,
            context.event_table[context.event_code]};
}

} // namespace shadowrun::ghidra_conversion
