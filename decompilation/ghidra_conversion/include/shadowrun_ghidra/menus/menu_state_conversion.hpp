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

struct MenuTileGridPlan {
    std::uint32_t glyph_source = 0;
    std::uint32_t destination = 0;
    std::uint16_t column_max = 0;
    std::uint16_t row_max = 0;
    std::uint16_t palette = 0;
};

constexpr std::uint32_t kMenuTileBlockDestination = 0x0000EB9EU;
constexpr std::uint16_t kMenuTileBlockColumnMax = 0x000EU;
constexpr std::uint16_t kMenuTileBlockRowMax = 0x0001U;
constexpr std::uint32_t kMenuColumnGridDestinationNonneg = 0x0000EBA0U;
constexpr std::uint32_t kMenuColumnGridDestinationNeg = 0x0000EBA2U;

// ROM 0x111D0: dest 0xEB9E, D7=0xE, D1=1, D2=0, then 0x1232. A1 is the
// caller-supplied glyph source.
MenuTileGridPlan plan_menu_tile_block(std::uint32_t glyph_source);

// ROM 0x111D0: upload one menu tile block.
void upload_menu_tile_block(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

struct MenuColumnTileUploadState {
    std::uint8_t column = 0;      // FFF0D0
    std::int16_t layout_word = 0; // FFF116
    std::uint32_t glyph_base = 0; // A1
    MenuTileGridPlan grid{};
};

// ROM 0x11454: A1 += (FFF0D0-2)*8. Dest (FFF0D0-2)*6 plus 0xEBA0 when
// FFF116>=0 or 0xEBA2 when FFF116<0. CLR FFF116. D7=1 D1=1 D2=0, 0x1232.
void reset_menu_upload_cursor_and_upload(MenuColumnTileUploadState& state);

// Host upload-cursor helper used by neighboring menu batch paths.
void reset_menu_upload_cursor_and_upload(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

// ROM 0x17EE4: clear the overlay state and upload two tile blocks.
void reset_menu_overlay_and_upload_two(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block);

} // namespace shadowrun::ghidra_conversion
