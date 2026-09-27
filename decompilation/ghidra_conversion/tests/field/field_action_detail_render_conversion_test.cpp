#include "shadowrun_ghidra/field/field_action_detail_render_conversion.hpp"

#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    ActionDetailResourceTable resources{};
    resources.detail_values[3] = 0x42U;
    ResourceVariantState variant{};
    FieldActionDetailRenderState state{};
    state.primary_cursor = 0;

    int plan_presentations = 0;
    int detail_text_renders = 0;
    int normal_headers = 0;
    int decimal_builds = 0;
    int action_names = 0;
    const FieldActionDetailRenderCallbacks callbacks{
        {}, {}, {}, {}, {},
        {},
        [&](const ActionDetailRenderPlan& plan) {
            if (!plan.valid || plan.source_code != 3U ||
                plan.detail_value != 0x42U) {
                return;
            }
            ++plan_presentations;
        },
        [&](const FieldActionDetailTextSelection& selection) {
            if (selection.table_index == 3U) {
                ++detail_text_renders;
            }
        },
        [&] { ++normal_headers; },
        [&] { ++action_names; },
        {},
    };

    const std::vector<FieldActionCostRecord> ordinary{{3U, 0x01U, 2U}};
    const auto ordinary_result = render_field_action_detail(
        ordinary, resources, variant, state,
        [](const std::uint8_t) { return 500U; }, callbacks);
    if (ordinary_result.kind != FieldActionDetailRenderKind::ordinary_action ||
        ordinary_result.cost != 100U || !ordinary_result.detail_plan ||
        !ordinary_result.detail_text || plan_presentations != 1 ||
        detail_text_renders != 1 || normal_headers != 1 ||
        decimal_builds != 0 || action_names != 1) {
        return 1;
    }

    state.primary_cursor = 0;
    int special_grid = 0;
    int special_header = 0;
    int special_label = 0;
    int tail_renders = 0;
    const FieldActionDetailRenderCallbacks special_callbacks{
        {}, [&] { ++tail_renders; },
        [&] { ++special_grid; }, [&] { ++special_header; },
        [&] { ++special_label; }, [&] { ++decimal_builds; },
        {}, {}, {}, {}, {},
    };
    const auto special_result = render_field_action_detail(
        {{static_cast<std::uint8_t>('E'), 0U, 25U}}, resources, variant, state,
        {}, special_callbacks);
    if (special_result.kind != FieldActionDetailRenderKind::special_action ||
        special_result.cost != 25U || special_grid != 1 ||
        special_header != 1 || special_label != 1 || tail_renders != 1 ||
        decimal_builds != 1) {
        return 2;
    }

    int unavailable_messages = 0;
    FieldActionDetailRenderCallbacks unavailable_callbacks{
        [&] { ++unavailable_messages; }, [&] { ++tail_renders; },
    };
    const auto unavailable_result = render_field_action_detail(
        {{0xFFU, 0U, 0U}}, resources, variant, state, {},
        unavailable_callbacks);
    if (unavailable_result.kind != FieldActionDetailRenderKind::unavailable ||
        unavailable_messages != 1 || tail_renders != 2) {
        return 3;
    }

    state.primary_cursor = 0;
    const auto no_text_result = render_field_action_detail(
        {{0x28U, 0U, 1U}}, resources, variant, state,
        [](const std::uint8_t) { return 100U; }, {});
    if (no_text_result.kind != FieldActionDetailRenderKind::ordinary_action ||
        no_text_result.detail_text || no_text_result.cost != 10U) {
        return 4;
    }

    const auto missing_cost_result = render_field_action_detail(
        ordinary, resources, variant, state, {}, {});
    if (missing_cost_result.kind !=
            FieldActionDetailRenderKind::missing_cost_data ||
        missing_cost_result.cost != 0U) {
        return 5;
    }

    state.primary_cursor = 1;
    const auto invalid_result = render_field_action_detail(
        ordinary, resources, variant, state,
        [](const std::uint8_t) { return 1U; }, {});
    if (invalid_result.kind != FieldActionDetailRenderKind::invalid_selection) {
        return 6;
    }
    return 0;
}
