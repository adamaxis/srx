#include "shadowrun_ghidra/entities/entity_action_sequence_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void service_entity_action_menu(const std::function<void()>& read_action,
                                const std::function<void()>& render_menu) {
    if (read_action) {
        read_action();
    }
    if (render_menu) {
        render_menu();
    }
}

EntityActionMenuRenderPlan render_entity_action_menu_plan(
    const std::uint8_t action_row) {
    EntityActionMenuRenderPlan plan;
    const auto& catalog = us_item_catalog();
    const auto detail =
        action_row < catalog.size()
            ? static_cast<std::uint8_t>(catalog[action_row].signed_detail_asset_index)
            : std::uint8_t{0};
    plan.grid_source =
        0x000CF84CU + static_cast<std::uint32_t>(detail) * 0x000EU;
    return plan;
}

void render_entity_action_menu(const std::function<void()>& update_menu,
                               const std::function<void()>& render_text,
                               const std::function<void()>& write_grid) {
    if (update_menu) {
        update_menu();
    }
    if (render_text) {
        render_text();
    }
    if (write_grid) {
        write_grid();
    }
}

EntityActionMenuAdvanceResult advance_entity_action_menu(
    const EntityActionMenuAdvanceInput& input,
    const EntityActionMenuAdvanceCallbacks& callbacks) {
    EntityActionMenuAdvanceResult result{};
    const EntityActionMenuTextPlan text_plan{};
    if (callbacks.render_text) {
        callbacks.render_text(text_plan);
    }
    result.text_rendered = true;

    if (input.active_action_code == 0U || !input.magic_spell.has_value() ||
        input.magic_spell->spell_code !=
            static_cast<std::uint8_t>(input.active_action_code - 1U)) {
        result.magic_spell_missing = true;
        return result;
    }

    const auto action_row_index = static_cast<std::uint8_t>(
        input.active_action_code - 1U);
    result.action_row_index = action_row_index;
    const auto residual = calculate_action_residual(
        action_row_index, input.current_force,
        calculate_action_effect_roll_count(input.entity),
        *input.magic_spell);
    result.residual_calculated = true;
    result.residual_effect = residual.residual_effect;
    if (residual.residual_effect <= 0) {
        return result;
    }

    const EntityActionMenuGridPlan grid_plan{
        static_cast<std::uint16_t>(residual.residual_effect - 1),
        0xC4AAU,
        0U,
        6U,
        0x2000U};
    result.detail_grid = grid_plan;
    if (callbacks.write_grid) {
        callbacks.write_grid(grid_plan);
    }
    result.detail_grid_written = true;
    return result;
}

} // namespace shadowrun::ghidra_conversion
