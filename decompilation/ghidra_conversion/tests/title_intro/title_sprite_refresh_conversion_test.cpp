#include "shadowrun_ghidra/title_intro/title_sprite_refresh_conversion.hpp"

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    int builds = 0;
    const TitleSpriteRefreshCallbacks callbacks{
        [&]() { ++builds; }};
    TitleSpriteRefreshState state{};

    if (expect(refresh_title_sprite_records(state, callbacks) ==
                   TitleSpriteRefreshAction::title_mode_inactive &&
               builds == 0) != 0) {
        return 1;
    }

    state.title_mode_visible = true;
    state.records_pending = true;
    state.refresh_countdown = 2;
    state.render_flags = 2U;
    if (expect(refresh_title_sprite_records(state, callbacks) ==
                   TitleSpriteRefreshAction::throttled &&
               state.refresh_countdown == 1 && builds == 0) != 0) {
        return 2;
    }

    state.render_flags = 0U;
    if (expect(refresh_title_sprite_records(state, callbacks) ==
                   TitleSpriteRefreshAction::records_built &&
               state.refresh_countdown == 0 && builds == 1) != 0) {
        return 3;
    }

    state.refresh_countdown = 0;
    state.records_pending = true;
    if (expect(refresh_title_sprite_records(state, callbacks) ==
                   TitleSpriteRefreshAction::records_built &&
               !state.records_pending && builds == 2) != 0) {
        return 4;
    }
    return 0;
}
