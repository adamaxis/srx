#pragma once

#include "shadowrun_ghidra/entities/action_detail_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct FieldInventoryActionDetailState {
    std::uint8_t primary_cursor = 0; // FFF0D0
    // Active-runner record +0x66..+0x6D and +0x6E..+0x75.
    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};

    // 0x58540 arms FFF0B1=-1 before entering this branch. The source clears
    // it on the valid-item exit, so the typed state retains that transition.
    bool suppress_runner_cost = true;
    FieldActionCostContext cost_context{};
};

using InventoryRunnerCostLookup =
    std::function<std::optional<std::uint16_t>(std::uint8_t code)>;

enum class FieldInventoryActionDetailRenderKind {
    presented,
    unavailable,
    invalid_cursor,
    missing_detail_data,
    missing_cost_data,
};

struct FieldInventoryActionDetailRenderResult {
    FieldInventoryActionDetailRenderKind kind =
        FieldInventoryActionDetailRenderKind::invalid_cursor;
    std::uint8_t item_code = 0;
    // The source 0x12148 result is recorded for auditability, but 0x13228
    // restores the item code before returning and the enclosing renderer
    // continues to key text, cost, and name presentation by item_code.
    std::optional<std::uint8_t> action_lookup_code;
    std::uint32_t runner_cost = 0;
    std::optional<ActionDetailRenderPlan> detail_plan;
    std::optional<FieldActionDetailTextSelection> detail_text;
};

struct FieldInventoryActionDetailRenderCallbacks {
    std::function<void()> show_unavailable;
    std::function<void(const ActionDetailRenderPlan&)> present_detail_plan;
    std::function<void(const FieldActionDetailTextSelection&)>
        render_detail_text;
    std::function<void()> render_runner_action_header;
    std::function<void(std::uint32_t)> render_runner_cost;
    // 0x585E8 renders the selected inventory record's encoded name. The
    // source record index is the item code returned by 0x13228.
    std::function<void(std::uint8_t)> render_item_name;
    std::function<void()> upload_resource;
};

// ROM 0x58540. Validate the selected runner inventory item, use the typed
// 0x13228 plan and 0x5878E detail-text selector, preserve the source's
// runner-cost suppression flag, and finish through the selected item's
// encoded name. The action-code lookup inside 0x13228 is retained as a typed
// source call boundary, but it does not replace the item-code key used by the
// enclosing 0x58540 renderer.
FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const InventoryRunnerCostLookup& runner_cost_lookup,
    const FieldInventoryActionDetailRenderCallbacks& callbacks = {});

// Source-data-bound 0x58540 path. The typed catalog supplies both the
// selected row's +0x24 detail value and its +0x22 cost word; no generic ROM
// table callback is needed.
FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const FieldInventoryActionDetailRenderCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
