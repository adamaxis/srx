#pragma once

#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x12E9E: read the current entity action before entering its menu/render
// service.
void service_entity_action_menu(const std::function<void()>& read_action,
                                const std::function<void()>& render_menu);

struct EntityActionMenuRenderPlan {
    std::uint32_t text_source = 0x000D19B0U;
    std::uint32_t text_destination = 0x0000C51CU;
    std::uint16_t text_palette = 0x6000;
    std::uint32_t grid_source = 0x000CF84CU;
    std::uint32_t grid_destination = 0x0000C52AU;
    std::uint16_t grid_palette = 0x2000;
    std::uint16_t grid_height = 6; // D7
    std::uint16_t grid_width = 0;  // D1
};

// ROM 0x12F50: 0x13000, 0xC0DA dest C51C source D19B0, then 0x1232 dest C52A
// from 0xCF84C + unsigned(+0x25)*0xE, D2=0x2000 D7=6 D1=0.
EntityActionMenuRenderPlan render_entity_action_menu_plan(
    std::uint8_t action_row);
void render_entity_action_menu(const std::function<void()>& update_menu,
                               const std::function<void()>& render_text,
                               const std::function<void()>& write_grid);

struct EntityActionMenuTextPlan {
    // ROM 0x12FA8 source 0xD19B7 -> tile base 0xC49C.
    std::uint16_t destination_tile_base = 0xC49CU;
    std::uint16_t palette_flags = 0x6000U;
};

struct EntityActionMenuGridPlan {
    // ROM 0x12FD6 source 0xCF8F0 + (residual - 1) * 0x0E -> 0xC4AA.
    std::uint16_t residual_row = 0;
    std::uint16_t destination_tile_base = 0xC4AAU;
    std::uint16_t outer_loop_counter = 0;
    std::uint16_t inner_loop_counter = 6U;
    std::uint16_t palette_flags = 0x2000U;
};

struct EntityActionMenuAdvanceInput {
    EntityActionRecord entity{};
    // Active record +0x57. The source subtracts one before entering
    // 0x10660, so action code one selects the valid zero-based row zero.
    std::uint8_t active_action_code = 0;
    // Active record +0x9D, copied into D4 as the current action force.
    std::uint8_t current_force = 0;
    // Decoded row from the source D4EA0 magic-spell table. The source uses
    // active +0x57 - 1 as the row index; callers must provide that exact
    // typed spell rather than a generic available-count/action record.
    std::optional<MagicSpellDefinition> magic_spell;
};

struct EntityActionMenuAdvanceCallbacks {
    std::function<void(const EntityActionMenuTextPlan&)> render_text;
    std::function<void(const EntityActionMenuGridPlan&)> write_grid;
};

struct EntityActionMenuAdvanceResult {
    std::optional<std::uint8_t> action_row_index;
    bool text_rendered = false;
    bool residual_calculated = false;
    bool detail_grid_written = false;
    bool magic_spell_missing = false;
    std::int16_t residual_effect = 0;
    std::optional<EntityActionMenuGridPlan> detail_grid;
};

// ROM 0x12FA4: render the action-menu heading, calculate the selected
// action's residual through 0x10660, and write the residual-minus-one detail
// grid only when that signed result is positive.
EntityActionMenuAdvanceResult advance_entity_action_menu(
    const EntityActionMenuAdvanceInput& input,
    const EntityActionMenuAdvanceCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
