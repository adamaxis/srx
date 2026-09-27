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
    missing_action_code,
    missing_detail_data,
    missing_cost_data,
};

struct FieldInventoryActionDetailRenderResult {
    FieldInventoryActionDetailRenderKind kind =
        FieldInventoryActionDetailRenderKind::invalid_cursor;
    std::uint8_t item_code = 0;
    std::uint8_t resolved_action_code = 0;
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
    std::function<void(std::uint8_t)> render_common_action_name;
    std::function<void()> upload_resource;
};

// ROM 0x58540. Validate the selected runner inventory item, use the typed
// 0x13228 plan and 0x5878E detail-text selector, preserve the source's
// runner-cost suppression flag, and finish through the common action-name
// presentation. The action-code lookup for dependent inventory items remains
// an explicit input boundary rather than a fabricated fallback.
FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& selected_action_code,
    const InventoryRunnerCostLookup& runner_cost_lookup,
    const FieldInventoryActionDetailRenderCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
