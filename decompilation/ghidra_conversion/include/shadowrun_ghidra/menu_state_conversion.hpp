#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MenuSlotState {
    std::uint8_t status = 0;
    std::uint8_t timer = 0;
};

// ROM 0x177A4: initialize the five source menu slots for the next update.
void initialize_menu_slot_states(std::array<MenuSlotState, 5>& slots);

struct MenuFooterRenderState {
    bool source_block_dirty = false;
};

struct MenuFooterRenderCallbacks {
    std::function<void()> copy_source_word_block;
    std::function<void()> refresh_menu_metadata;
    std::function<void()> draw_footer;
};

// ROM 0x176EA: refresh footer data in source order, preserving the dirty
// marker as typed state and leaving the actual presentation to callbacks.
void render_menu_footer(
    MenuFooterRenderState& state,
    const MenuFooterRenderCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
