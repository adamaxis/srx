#include "shadowrun_ghidra/field_mode_exit_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeExitState first_route{};
    first_route.world_flags = (1UL << 23U) | (1UL << 24U);
    std::uint32_t marked_effect = 0;
    int service_calls = 0;
    const auto first_action = prepare_field_mode_exit(
        first_route,
        { [&](const std::uint32_t marker) { marked_effect = marker; },
          [&](FieldModeExitState& state) {
              ++service_calls;
              state.alternate_result = -1;
          },
          {} });
    if (expect(first_action ==
                   FieldModeExitAction::services_completed_and_display_refreshed &&
               first_route.menu_mode == 0x0BU &&
               first_route.effect_flags == (1UL << 19U) &&
               first_route.effect_marker == 0x19U && marked_effect == 0x19U &&
               first_route.alternate_result == 0 && service_calls == 1) != 0) {
        return 1;
    }

    FieldModeExitState second_route{};
    second_route.world_flags = (1UL << 23U) | (1UL << 24U) | (1UL << 25U);
    second_route.service_flags = 0x01U;
    second_route.service_options = 0;
    second_route.effect_flags = 1UL << 20U;
    second_route.render_flags = (1UL << 2U) | (1UL << 3U) | (1UL << 11U);
    second_route.scene_id = 0U;
    second_route.submode = 0U;
    if (expect(prepare_field_mode_exit(second_route, {}) ==
                   FieldModeExitAction::services_completed &&
               second_route.service_flags == 0U &&
               second_route.service_options == 0x0200U &&
               second_route.effect_flags == 0U && second_route.render_flags == 0U &&
               (second_route.world_flags & (1UL << 29U)) != 0U) != 0) {
        return 2;
    }

    FieldModeExitState mode_routes{};
    mode_routes.scene_id = 2U;
    mode_routes.submode = 1U;
    if (expect(prepare_field_mode_exit(mode_routes, {}) ==
                   FieldModeExitAction::services_completed &&
               mode_routes.menu_mode == 8U) != 0) {
        return 3;
    }
    mode_routes.scene_id = 1U;
    mode_routes.submode = 6U;
    if (expect(prepare_field_mode_exit(mode_routes, {}) ==
                   FieldModeExitAction::services_completed &&
               mode_routes.menu_mode == 9U) != 0) {
        return 4;
    }
    mode_routes.scene_id = 4U;
    mode_routes.submode = 4U;
    if (expect(prepare_field_mode_exit(mode_routes, {}) ==
                   FieldModeExitAction::services_completed &&
               mode_routes.menu_mode == 10U) != 0) {
        return 5;
    }

    mode_routes.scene_id = 8U;
    if (expect(prepare_field_mode_exit(mode_routes, {}) ==
                   FieldModeExitAction::no_matching_route &&
               mode_routes.menu_mode == 10U) != 0) {
        return 6;
    }

    return 0;
}
