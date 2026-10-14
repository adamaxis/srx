#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

#include "shadowrun_ghidra/rendering/sprite_record_builder_conversion.hpp"

namespace shadowrun::ghidra_conversion {

struct MenuSpriteOverlaySource {
    std::string_view alias;
    std::vector<SpriteSourceRecord> records;
};

struct MenuSpriteOverlayState {
    std::int8_t refresh_countdown = 0; // FFF0DA
    std::uint8_t source_phase = 0;     // FFF0DB, four source lists
    std::uint8_t primary_selector = 0; // FFF0D0, source +0x20 offset
    std::uint8_t secondary_selector = 0; // FFF0D1, source +0x18 offset
    std::uint16_t sprite_record_count = 1; // FFDD02
    bool sprite_list_ready = false;          // FFD980
    std::uint8_t display_dirty_flags = 0;   // FFEF14
};

struct MenuSpriteOverlayBuildPlan {
    std::string_view source_alias;
    std::uint8_t source_phase = 0;
    std::int16_t primary_position_offset = 0;
    std::int16_t secondary_position_offset = 0;
    std::uint8_t first_sequence = 2;
    std::uint16_t attribute_flags = 0x4000U;
};

struct MenuSpriteOverlayCallbacks {
    // The C8B3E pointer table is supplied as decoded host records. The
    // callback returns the number of records appended after the source's
    // FFDD02=1 initialization.
    std::function<std::size_t(const MenuSpriteOverlayBuildPlan&,
                              const std::vector<SpriteSourceRecord>&)>
        build_sprite_records;
    // 0x1378 leaves A3 at the next record; FDFE backs up one record and clears
    // its sequence byte to terminate the runtime list.
    std::function<void()> clear_last_sprite_sequence;
    // 0xFDAE: four source grid writes happen on every overlay tick.
    std::function<void()> write_menu_grid_quadruple;
};

// ROM 0xFDFE: advance the command/magic/equipment menu cursor-sprite overlay.
// The timer and four-phase source selection are converted here; decoded sprite
// records and grid upload remain explicit host callbacks.
bool advance_menu_sprite_overlay(
    MenuSpriteOverlayState& state,
    const std::vector<MenuSpriteOverlaySource>& sources,
    const MenuSpriteOverlayCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
