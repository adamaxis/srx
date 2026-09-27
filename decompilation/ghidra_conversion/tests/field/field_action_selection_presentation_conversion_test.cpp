#include "shadowrun_ghidra/field/field_action_selection_presentation_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    FieldActionDescriptionResourceTable descriptions{{"resource.firewall",
                                                      "resource.decker"}};
    FieldActionResourceSelectionResult selection{};
    selection.selection.selected_storage_weight = 7U;
    selection.adjusted_available_memory = 0xFFFDU;

    std::vector<std::string> events;
    const FieldActionDescriptionRenderCallbacks description_callbacks{
        [&](const std::string& alias) { events.push_back("description:" + alias); },
        [&](const FieldActionQuantityText text) {
            switch (text) {
            case FieldActionQuantityText::level_heading:
                events.push_back("level-heading");
                break;
            case FieldActionQuantityText::size_deck_storage_heading:
                events.push_back("size-deck-storage-heading");
                break;
            case FieldActionQuantityText::price_label:
                events.push_back("price-label");
                break;
            }
        },
        [&](const std::uint8_t quantity) {
            events.push_back("quantity:" + std::to_string(quantity));
        },
        [&](const std::uint32_t weight) {
            events.push_back("weight:" + std::to_string(weight));
        },
        [&](const std::uint16_t total) {
            events.push_back("storage:" + std::to_string(total));
        },
        [&](const std::uint32_t cost) {
            events.push_back("cost:" + std::to_string(cost));
        },
        [&] { events.push_back("highest-version-notice"); },
        [&](const std::uint8_t index) {
            events.push_back("grid:" + std::to_string(index));
        },
    };
    const auto detail_result = render_field_action_description(
        descriptions,
        FieldActionDescriptionInput{1U, 3U, 120U, selection},
        description_callbacks);
    if (detail_result.kind != FieldActionDescriptionRenderKind::quantity_summary ||
        detail_result.description_alias != "resource.decker" ||
        detail_result.selected_storage_weight != 7U ||
        detail_result.displayed_storage_total != 4U ||
        detail_result.displayed_cost != 120U || events.size() != 9U ||
        events[0] != "description:resource.decker" ||
        events[1] != "level-heading" ||
        events[2] != "size-deck-storage-heading" ||
        events[3] != "price-label" || events[4] != "quantity:3" ||
        events[5] != "weight:7" || events[6] != "storage:4" ||
        events[7] != "cost:120" ||
        events[8] != "grid:1") {
        return 1;
    }

    events.clear();
    const auto description_only = render_field_action_description(
        descriptions, FieldActionDescriptionInput{0U, 9U, {}, {}},
        description_callbacks);
    if (description_only.kind !=
            FieldActionDescriptionRenderKind::description_only ||
        description_only.description_alias != "resource.firewall" ||
        events.size() != 3U || events[0] != "description:resource.firewall" ||
        events[1] != "highest-version-notice" || events[2] != "grid:0") {
        return 2;
    }
    if (render_field_action_description(
            descriptions, FieldActionDescriptionInput{9U, 1U, 1U, selection},
            {}).kind != FieldActionDescriptionRenderKind::invalid_description) {
        return 3;
    }
    events.clear();
    if (render_field_action_description(
            descriptions, FieldActionDescriptionInput{0U, 1U, 1U, {}},
            description_callbacks).kind !=
            FieldActionDescriptionRenderKind::missing_resource_selection ||
        events.size() != 6U || events[0] != "description:resource.firewall" ||
        events[4] != "quantity:1" || events[5] != "grid:0") {
        return 4;
    }
    events.clear();
    selection.adjusted_available_memory = 10U;
    if (render_field_action_description(
            descriptions, FieldActionDescriptionInput{0U, 1U, {}, selection},
            description_callbacks).kind != FieldActionDescriptionRenderKind::missing_cost ||
        events.size() != 8U || events[6] != "storage:17" ||
        events[7] != "grid:0") {
        return 11;
    }
    if (render_field_action_description(
            descriptions, FieldActionDescriptionInput{0U, 0xFFU, 1U, selection},
            {}).kind != FieldActionDescriptionRenderKind::quantity_summary) {
        return 5;
    }

    // 0x80 - 9 wraps to 0x77, so the source BMI takes the highest-version
    // notice path. A host int8_t comparison would incorrectly treat 0x80 as
    // a small quantity.
    if (render_field_action_description(
            descriptions, FieldActionDescriptionInput{0U, 0x80U, {}, {}},
            {}).kind != FieldActionDescriptionRenderKind::description_only) {
        return 6;
    }

    FieldActionCostSummaryRecord summary{};
    summary.availability_marker = 3;
    summary.base_cost = 1000U;
    summary.byte_at_6 = 6U;
    summary.byte_at_7 = 7U;
    summary.word_at_8 = 8U;
    summary.word_at_a = 10U;
    summary.byte_at_c = 12U;
    summary.byte_at_d = 13U;
    int title_count = 0;
    int numeric_count = 0;
    int blocked_grid_count = 0;
    std::uint32_t displayed_summary_cost = 0;
    const FieldActionCostSummaryRenderCallbacks summary_callbacks{
        [&] { ++title_count; },
        [&](const FieldActionCostSummaryNumericField field,
            const std::uint32_t value) {
            ++numeric_count;
            if (field == FieldActionCostSummaryNumericField::word_at_a &&
                value != 10U) {
                ++numeric_count;
            }
        },
        [&] { ++blocked_grid_count; },
        [&](const std::uint32_t value) { displayed_summary_cost = value; },
    };
    FieldActionCostContext cost_context{};
    const auto blocked = render_field_action_cost_summary(
        summary, 2, cost_context, summary_callbacks);
    if (blocked.kind != FieldActionCostSummaryRenderKind::blocked ||
        title_count != 1 || numeric_count != 6 || blocked_grid_count != 1 ||
        displayed_summary_cost != 0U) {
        return 6;
    }

    const auto discounted = render_field_action_cost_summary(
        summary, 3, cost_context, summary_callbacks);
    if (discounted.kind != FieldActionCostSummaryRenderKind::discounted ||
        discounted.displayed_cost != 1000U || blocked_grid_count != 1) {
        return 7;
    }
    summary.availability_marker = 0x7F;
    const auto wrapped_cursor = render_field_action_cost_summary(
        summary, static_cast<std::int8_t>(0x80U), cost_context,
        summary_callbacks);
    if (wrapped_cursor.kind != FieldActionCostSummaryRenderKind::discounted ||
        wrapped_cursor.displayed_cost != 1000U) {
        return 24;
    }
    summary.availability_marker = 3;
    summary.base_cost = 0x10000U;
    const auto large = render_field_action_cost_summary(
        summary, 3, cost_context, summary_callbacks);
    if (large.kind != FieldActionCostSummaryRenderKind::large_discounted ||
        large.displayed_cost != 0x10000U) {
        return 8;
    }
    summary.base_cost = 0xFFFFFFFFU;
    const auto wrapped_large_cost = render_field_action_cost_summary(
        summary, 3, cost_context, summary_callbacks);
    if (wrapped_large_cost.kind !=
            FieldActionCostSummaryRenderKind::discounted ||
        wrapped_large_cost.displayed_cost != 0xFFFFFFFFU) {
        return 25;
    }

    int unavailable_count = 0;
    int mode_count = 0;
    int quantity_feedback_count = 0;
    int feedback_grid_count = 0;
    int tail_count = 0;
    const FieldActionFeedbackRenderCallbacks feedback_callbacks{
        [&] { ++unavailable_count; },
        [&](const std::uint8_t mode) { mode_count += mode == 2U ? 1 : 10; },
        [&](const std::uint8_t offset) {
            quantity_feedback_count += offset == 10U ? 1 : 10;
        },
        [&](const std::uint8_t group) { feedback_grid_count += group == 0U; },
        [&] { ++tail_count; },
    };
    const auto feedback = render_field_action_feedback(2U, feedback_callbacks);
    if (feedback.kind != FieldActionFeedbackRenderKind::mode_feedback ||
        feedback.quantity_text_offset != 10U || feedback.grid_group != 0U ||
        mode_count != 1 || quantity_feedback_count != 1 ||
        feedback_grid_count != 1 || tail_count != 1) {
        return 9;
    }
    const auto unavailable = render_field_action_feedback(4U, feedback_callbacks);
    if (unavailable.kind != FieldActionFeedbackRenderKind::unavailable ||
        unavailable_count != 1 || tail_count != 2 || mode_count != 1) {
        return 10;
    }

    return 0;
}
