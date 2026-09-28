#include "shadowrun_ghidra/menus/menu_sprite_overlay_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool advance_menu_sprite_overlay(
    MenuSpriteOverlayState& state,
    const std::vector<MenuSpriteOverlaySource>& sources,
    const MenuSpriteOverlayCallbacks& callbacks) {
    state.refresh_countdown = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(state.refresh_countdown) - 1U);

    bool rebuilt = false;
    if (state.refresh_countdown < 0) {
        state.refresh_countdown = 6;
        state.source_phase = static_cast<std::uint8_t>(
            (state.source_phase + 1U) % 4U);

        if (state.source_phase < sources.size()) {
            const auto& source = sources[state.source_phase];
            const MenuSpriteOverlayBuildPlan plan{
                source.alias,
                state.source_phase,
                static_cast<std::int16_t>(
                    static_cast<std::uint16_t>(state.primary_selector) * 0x20U),
                static_cast<std::int16_t>(
                    static_cast<std::uint16_t>(state.secondary_selector) *
                    0x18U),
                2U,
                0x4000U,
            };

            state.sprite_record_count = 1U;
            if (callbacks.build_sprite_records) {
                const auto built_count =
                    callbacks.build_sprite_records(plan, source.records);
                state.sprite_record_count = static_cast<std::uint16_t>(
                    1U + static_cast<std::uint16_t>(built_count));
            }
            if (callbacks.clear_last_sprite_sequence) {
                callbacks.clear_last_sprite_sequence();
            }
            state.sprite_list_ready = true;
            state.display_dirty_flags = static_cast<std::uint8_t>(
                state.display_dirty_flags | 0x02U);
            rebuilt = true;
        }
    }

    if (callbacks.write_menu_grid_quadruple) {
        callbacks.write_menu_grid_quadruple();
    }
    return rebuilt;
}

} // namespace shadowrun::ghidra_conversion
