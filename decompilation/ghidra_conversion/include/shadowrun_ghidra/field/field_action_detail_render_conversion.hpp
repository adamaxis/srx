#pragma once

#include "shadowrun_ghidra/entities/action_detail_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct FieldActionDetailRenderState {
    std::int8_t primary_cursor = 0; // FFF0D0
    FieldActionCostContext cost_context{};
};

enum class FieldActionDetailRenderKind {
    invalid_selection,
    unavailable,
    special_action,
    ordinary_action,
    missing_cost_data,
};

struct FieldActionDetailRenderResult {
    FieldActionDetailRenderKind kind =
        FieldActionDetailRenderKind::invalid_selection;
    FieldActionCostRecord record{};
    std::uint32_t cost = 0;
    std::optional<ActionDetailRenderPlan> detail_plan;
    std::optional<FieldActionDetailTextSelection> detail_text;
};

struct FieldActionDetailRenderCallbacks {
    // The source's 0x1AA5B2 message and tail render are separate presentation
    // events even though both ultimately use encoded text.
    std::function<void()> show_unavailable;
    std::function<void()> render_tail_text;

    // 0x586F6..0x5874A special-'E' layout and cost presentation.
    std::function<void()> write_special_grid;
    std::function<void()> render_special_header;
    std::function<void()> render_special_label;
    std::function<void()> build_grouped_decimal;

    // Ordinary records first build the typed detail plan, conditionally select
    // detail text through 0x5878E, then render the normal header/cost/name.
    std::function<void(const ActionDetailRenderPlan&)> present_detail_plan;
    std::function<void(const FieldActionDetailTextSelection&)>
        render_detail_text;
    std::function<void()> render_normal_header;
    std::function<void()> render_action_name;
    std::function<void()> upload_resource;
};

// ROM 0x586CA. Select the three-byte record at primary_cursor, preserve the
// unavailable and special-'E' branches, and build the ordinary detail plan
// and cost using typed source data. The table is supplied as a flat host
// sequence of records because the source's A0 + cursor*3 arithmetic is a
// presentation-table detail, not runtime state.
FieldActionDetailRenderResult render_field_action_detail(
    const std::vector<FieldActionCostRecord>& records,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    FieldActionDetailRenderState& state,
    const FieldActionBaseCostLookup& base_cost_lookup,
    const FieldActionDetailRenderCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
