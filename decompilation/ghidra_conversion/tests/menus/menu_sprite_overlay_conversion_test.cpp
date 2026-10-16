#include "shadowrun_ghidra/menus/menu_sprite_overlay_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<MenuSpriteOverlaySource> sources{
        {"menu.cursor.phase0", {}},
        {"menu.cursor.phase1", {{{10, 20, 2, 0x10U}}}},
        {"menu.cursor.phase2", {}},
        {"menu.cursor.phase3", {}},
    };
    MenuSpriteOverlayState state{};
    state.primary_selector = 3;
    state.secondary_selector = 2;
    std::size_t built = 0;
    std::size_t cleared = 0;
    std::size_t grids = 0;
    MenuSpriteOverlayBuildPlan observed{};
    const auto rebuilt = advance_menu_sprite_overlay(
        state, sources,
        {[&](const MenuSpriteOverlayBuildPlan& plan,
             const std::vector<SpriteSourceRecord>& records) {
             observed = plan;
             built = records.size();
             return records.size();
         },
         [&] { ++cleared; },
         [&] { ++grids; }});
    if (!expect(rebuilt) || !expect(state.refresh_countdown == 6) ||
        !expect(state.source_phase == 1) || !expect(state.sprite_record_count == 2) ||
        !expect(state.sprite_list_ready) ||
        !expect((state.display_dirty_flags & 0x02U) != 0U) ||
        !expect(observed.source_alias == "menu.cursor.phase1") ||
        !expect(observed.primary_position_offset == 0x60) ||
        !expect(observed.secondary_position_offset == 0x30) ||
        !expect(observed.first_sequence == 2U) ||
        !expect(observed.attribute_flags == 0x4000U) ||
        !expect(built == 1U && cleared == 1U && grids == 1U)) {
        return 1;
    }

    state.refresh_countdown = 4;
    if (!expect(!advance_menu_sprite_overlay(state, sources,
                                              {nullptr, nullptr, [&] { ++grids; }})) ||
        !expect(state.refresh_countdown == 3) ||
        !expect(state.source_phase == 1) || !expect(grids == 2U)) {
        return 2;
    }

    state.refresh_countdown = -1;
    state.source_phase = 3;
    state.primary_selector = 1;
    state.secondary_selector = 1;
    if (!expect(advance_menu_sprite_overlay(state, sources,
                                             {nullptr, nullptr, [&] { ++grids; }})) ||
        !expect(state.source_phase == 0) || !expect(state.refresh_countdown == 6) ||
        !expect(grids == 3U)) {
        return 3;
    }

    return 0;
}
