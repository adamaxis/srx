#include "shadowrun_ghidra/title_sprite_refresh_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleSpriteRefreshAction refresh_title_sprite_records(
    TitleSpriteRefreshState& state,
    const TitleSpriteRefreshCallbacks& callbacks) {
    if (!state.title_mode_visible) {
        return TitleSpriteRefreshAction::title_mode_inactive;
    }

    if (state.records_pending) {
        --state.refresh_countdown;
        if (state.refresh_countdown >= 0 &&
            (state.render_flags & 0x3U) > 1U) {
            return TitleSpriteRefreshAction::throttled;
        }
        if (state.refresh_countdown < 0) {
            state.records_pending = false;
        }
    }

    if (callbacks.build_title_sprite_records) {
        callbacks.build_title_sprite_records();
    }
    return TitleSpriteRefreshAction::records_built;
}

} // namespace shadowrun::ghidra_conversion
