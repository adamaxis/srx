#include "shadowrun_ghidra/field/field_inventory_action_detail_render_conversion.hpp"

#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    ActionDetailResourceTable resources{};
    resources.detail_values[0] = 0x11U;
    resources.detail_values[0x13U] = 0x22U;
    ResourceVariantState resource_variant{};
    std::vector<std::string> events;
    const FieldInventoryActionDetailRenderCallbacks callbacks{
        [&] { events.emplace_back("unavailable"); },
        [&](const ActionDetailRenderPlan&) { events.emplace_back("plan"); },
        [&](const FieldActionDetailTextSelection& selection) {
            events.emplace_back("detail:" + std::to_string(selection.table_index));
        },
        [&] { events.emplace_back("header"); },
        [&](const std::uint32_t cost) {
            events.emplace_back("cost:" + std::to_string(cost));
        },
        [&](const std::uint8_t code) {
            events.emplace_back("item-name:" + std::to_string(code));
        },
        [&] { events.emplace_back("upload"); },
    };

    FieldInventoryActionDetailState suppressed{};
    suppressed.item_codes[0] = 1U;
    suppressed.item_phases[0] = 0x06U;
    const auto suppressed_result = render_field_inventory_action_detail(
        suppressed, resources, resource_variant, std::nullopt, {}, callbacks);
    if (suppressed_result.kind !=
            FieldInventoryActionDetailRenderKind::presented ||
        suppressed_result.item_code != 1U ||
        events != std::vector<std::string>{"plan", "detail:1",
                                           "item-name:1"}) {
        return 1;
    }

    events.clear();
    FieldInventoryActionDetailState priced{};
    priced.item_codes[0] = 1U;
    priced.suppress_runner_cost = false;
    std::uint8_t priced_lookup_key = 0;
    const auto priced_result = render_field_inventory_action_detail(
        priced, resources, resource_variant, std::nullopt,
        [&](const std::uint8_t code) {
            priced_lookup_key = code;
            return std::optional<std::uint16_t>{100U};
        },
        callbacks);
    if (priced_result.kind != FieldInventoryActionDetailRenderKind::presented ||
        priced_result.runner_cost != 50U ||
        priced_lookup_key != 1U ||
        events != std::vector<std::string>{"plan", "detail:1", "header",
                                           "cost:50", "item-name:1"}) {
        return 2;
    }

    events.clear();
    FieldInventoryActionDetailState action_item{};
    action_item.item_codes[0] = 0x14U;
    action_item.item_phases[0] = 0x02U;
    const auto action_result = render_field_inventory_action_detail(
        action_item, resources, resource_variant, 0x27U, {}, callbacks);
    if (action_result.kind != FieldInventoryActionDetailRenderKind::presented ||
        action_result.action_lookup_code != 0x27U ||
        !action_result.detail_text ||
        action_result.detail_text->table_index != 0x14U ||
        events != std::vector<std::string>{"upload", "plan", "detail:20",
                                           "item-name:20"}) {
        return 3;
    }

    events.clear();
    FieldInventoryActionDetailState missing_action{};
    missing_action.item_codes[0] = 0x14U;
    const auto missing_action_result = render_field_inventory_action_detail(
        missing_action, resources, resource_variant, std::nullopt, {}, callbacks);
    if (missing_action_result.kind !=
            FieldInventoryActionDetailRenderKind::presented ||
        events != std::vector<std::string>{"plan", "detail:20",
                                           "item-name:20"}) {
        return 4;
    }

    events.clear();
    FieldInventoryActionDetailState unavailable{};
    unavailable.item_codes[0] = 0U;
    if (render_field_inventory_action_detail(
            unavailable, resources, resource_variant, std::nullopt, {}, callbacks)
            .kind != FieldInventoryActionDetailRenderKind::unavailable ||
        events != std::vector<std::string>{"unavailable"}) {
        return 5;
    }

    FieldInventoryActionDetailState out_of_range{};
    out_of_range.primary_cursor = 8U;
    if (render_field_inventory_action_detail(
            out_of_range, resources, resource_variant, std::nullopt, {}, callbacks)
            .kind != FieldInventoryActionDetailRenderKind::invalid_cursor) {
        return 6;
    }

    events.clear();
    FieldInventoryActionDetailState missing_cost{};
    missing_cost.item_codes[0] = 1U;
    missing_cost.suppress_runner_cost = false;
    const auto missing_cost_result = render_field_inventory_action_detail(
        missing_cost, resources, resource_variant, std::nullopt, {}, callbacks);
    if (missing_cost_result.kind !=
            FieldInventoryActionDetailRenderKind::missing_cost_data ||
        events != std::vector<std::string>{"upload", "plan", "detail:1",
                                           "header", "item-name:1"}) {
        return 7;
    }

    events.clear();
    ResourceVariantState typed_variant{};
    FieldInventoryActionDetailState typed_state{};
    typed_state.item_codes[0] = 5U;
    typed_state.item_phases[0] = 0x06U;
    typed_state.suppress_runner_cost = false;
    const auto typed_result = render_field_inventory_action_detail(
        typed_state, us_item_catalog(), typed_variant, std::nullopt, callbacks);
    if (typed_result.kind != FieldInventoryActionDetailRenderKind::presented ||
        typed_result.item_code != 5U || typed_result.runner_cost != 0x7AU ||
        !typed_result.detail_plan ||
        typed_result.detail_plan->detail_value != 0x06U ||
        events != std::vector<std::string>{"plan", "detail:5", "header",
                                           "cost:122", "item-name:5"}) {
        return 8;
    }

    return 0;
}
