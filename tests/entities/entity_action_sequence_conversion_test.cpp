#include "shadowrun_ghidra/entities/entity_action_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    service_entity_action_menu([&order] { order.push_back(1); },
                               [&order] { order.push_back(2); });
    render_entity_action_menu([&order] { order.push_back(3); },
                              [&order] { order.push_back(4); },
                              [&order] { order.push_back(5); });
    if (order != std::vector<int>{1, 2, 3, 4, 5}) {
        return 1;
    }
    const auto render_plan = render_entity_action_menu_plan(4U);
    const auto render_plan_zero = render_entity_action_menu_plan(0U);
    if (render_plan.text_source != 0x000D19B0U ||
        render_plan.text_destination != 0x0000C51CU ||
        render_plan.grid_destination != 0x0000C52AU ||
        render_plan.grid_height != 6U || render_plan.grid_width != 0U ||
        render_plan.grid_palette != 0x2000U ||
        render_plan_zero.grid_source != 0x000CF84CU ||
        render_plan.grid_source != 0x000CF84CU + 0x0EU) {
        return 4;
    }

    EntityActionMenuAdvanceInput advance_input{};
    advance_input.active_action_code = 3U;
    advance_input.current_force = 2U;
    advance_input.entity.item18_base = 10U;
    advance_input.magic_spell = *find_us_magic_spell(2U);
    std::vector<int> advance_events;
    const auto advance = advance_entity_action_menu(
        advance_input,
        {[&](const EntityActionMenuTextPlan& plan) {
             advance_events.push_back(
                 plan.destination_tile_base == 0xC49CU ? 6 : 99);
         },
         [&](const EntityActionMenuGridPlan& plan) {
             advance_events.push_back(
                 plan.destination_tile_base == 0xC4AAU ? 7 : 99);
         }});
    if (!(advance.text_rendered && advance.residual_calculated &&
          advance.detail_grid_written && advance.residual_effect > 0 &&
          advance.detail_grid->residual_row ==
              static_cast<std::uint16_t>(advance.residual_effect - 1) &&
          advance_events == std::vector<int>{6, 7})) {
        return 2;
    }

    advance_input.magic_spell->base_effect = 1;
    advance_events.clear();
    const auto no_grid = advance_entity_action_menu(
        advance_input,
        {[&](const EntityActionMenuTextPlan&) { advance_events.push_back(8); },
         [&](const EntityActionMenuGridPlan&) { advance_events.push_back(9); }});
    if (!(no_grid.text_rendered && no_grid.residual_calculated &&
          !no_grid.detail_grid_written && advance_events == std::vector<int>{8})) {
        return 3;
    }

    advance_input.active_action_code = 0U;
    const auto missing = advance_entity_action_menu(advance_input);
    if (!(missing.text_rendered && missing.magic_spell_missing &&
          !missing.residual_calculated)) {
        return 4;
    }

    advance_input.active_action_code = 1U;
    advance_input.magic_spell->spell_code = 2U;
    const auto mismatched = advance_entity_action_menu(advance_input);
    if (!(mismatched.text_rendered && mismatched.magic_spell_missing &&
          !mismatched.residual_calculated)) {
        return 5;
    }
    return 0;
}
