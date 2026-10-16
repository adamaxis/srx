#include "shadowrun_ghidra/field/field_inventory_action_detail_render_conversion.hpp"

#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    const auto& item_catalog = us_item_catalog();
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
        suppressed, item_catalog, resource_variant, std::nullopt, callbacks);
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
    const auto priced_result = render_field_inventory_action_detail(
        priced, item_catalog, resource_variant, std::nullopt, callbacks);
    if (priced_result.kind != FieldInventoryActionDetailRenderKind::presented ||
        priced_result.runner_cost != 50U ||
        events != std::vector<std::string>{"plan", "detail:1", "header",
                                           "cost:50", "item-name:1"}) {
        return 2;
    }

    events.clear();
    FieldInventoryActionDetailState action_item{};
    action_item.item_codes[0] = 0x14U;
    action_item.item_phases[0] = 0x02U;
    const auto action_result = render_field_inventory_action_detail(
        action_item, item_catalog, resource_variant, 0x27U, callbacks);
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
        missing_action, item_catalog, resource_variant, std::nullopt, callbacks);
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
            unavailable, item_catalog, resource_variant, std::nullopt, callbacks)
            .kind != FieldInventoryActionDetailRenderKind::unavailable ||
        events != std::vector<std::string>{"unavailable"}) {
        return 5;
    }

    FieldInventoryActionDetailState out_of_range{};
    out_of_range.primary_cursor = 8U;
    if (render_field_inventory_action_detail(
            out_of_range, item_catalog, resource_variant, std::nullopt, callbacks)
            .kind != FieldInventoryActionDetailRenderKind::invalid_cursor) {
        return 6;
    }

    events.clear();
    ResourceVariantState typed_variant{};
    FieldInventoryActionDetailState typed_state{};
    typed_state.item_codes[0] = 5U;
    typed_state.item_phases[0] = 0x06U;
    typed_state.suppress_runner_cost = false;
    const auto typed_result = render_field_inventory_action_detail(
        typed_state, item_catalog, typed_variant, std::nullopt, callbacks);
    if (typed_result.kind != FieldInventoryActionDetailRenderKind::presented ||
        typed_result.item_code != 5U || typed_result.runner_cost != 250U ||
        !typed_result.detail_plan ||
        typed_result.detail_plan->detail_value != 0x06U ||
        events != std::vector<std::string>{"plan", "detail:5", "header",
                                           "cost:250", "item-name:5"}) {
        return 7;
    }

    return 0;
}
