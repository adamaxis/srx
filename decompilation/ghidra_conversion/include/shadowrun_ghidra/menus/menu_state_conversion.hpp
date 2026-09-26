#pragma once

#include <array>
#include <cstddef>
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

struct MenuTileUploadState {
    std::uint8_t upload_cursor = 0;
    std::uint8_t overlay_state = 0;
    std::size_t uploaded_blocks = 0;
};

using MenuTileUploadBoundary = std::function<void()>;

// ROM 0x111D0: upload one menu tile block.
void upload_menu_tile_block(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

// ROM 0x11454: reset the menu upload cursor and upload one tile block.
void reset_menu_upload_cursor_and_upload(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

// ROM 0x17EE4: clear the overlay state and upload two tile blocks.
void reset_menu_overlay_and_upload_two(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

} // namespace shadowrun::ghidra_conversion
