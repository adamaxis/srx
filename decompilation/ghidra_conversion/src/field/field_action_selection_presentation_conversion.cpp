#include "shadowrun_ghidra/field/field_action_selection_presentation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool source_quantity_is_below_nine(const std::uint8_t quantity) {
    // 0x58136 uses CMPI.B #9 followed by BMI.  The branch observes the
    // wrapped subtraction's N flag, so this is not a host signed comparison
    // for quantities in the high half of the byte domain.
    return (static_cast<std::uint8_t>(quantity - 9U) & 0x80U) != 0U;
}

} // namespace

FieldActionDescriptionRenderResult render_field_action_description(
    const FieldActionDescriptionResourceTable& resources,
    const FieldActionDescriptionInput& input,
    const FieldActionDescriptionRenderCallbacks& callbacks) {
    FieldActionDescriptionRenderResult result{};
    result.description_index = input.description_index;
    result.quantity = input.quantity;
    if (input.description_index >= resources.aliases.size()) {
        return result;
    }

    result.description_alias = resources.aliases[input.description_index];
    const auto quantity_is_small = source_quantity_is_below_nine(input.quantity);

    if (callbacks.render_description_text) {
        callbacks.render_description_text(result.description_alias);
    }

    if (!quantity_is_small) {
        result.kind = FieldActionDescriptionRenderKind::description_only;
        if (callbacks.render_highest_version_notice) {
            callbacks.render_highest_version_notice();
        }
        if (callbacks.write_description_grid) {
            callbacks.write_description_grid(input.description_index);
        }
        return result;
    }

    if (callbacks.render_quantity_detail_text) {
        callbacks.render_quantity_detail_text(FieldActionQuantityText::level_heading);
        callbacks.render_quantity_detail_text(
            FieldActionQuantityText::size_deck_storage_heading);
        callbacks.render_quantity_detail_text(FieldActionQuantityText::price_label);
    }
    if (callbacks.render_quantity) {
        callbacks.render_quantity(input.quantity);
    }

    if (!input.resource_selection.has_value()) {
        result.kind = FieldActionDescriptionRenderKind::missing_resource_selection;
        if (callbacks.write_description_grid) {
            callbacks.write_description_grid(input.description_index);
        }
        return result;
    }

    const auto& selection = *input.resource_selection;
    result.selected_storage_weight = selection.selection.selected_storage_weight;
    result.displayed_storage_total = static_cast<std::uint16_t>(
        selection.adjusted_available_memory +
        static_cast<std::uint16_t>(selection.selection.selected_storage_weight));
    if (callbacks.render_selected_storage_weight) {
        callbacks.render_selected_storage_weight(
            result.selected_storage_weight);
    }
    if (callbacks.render_storage_total) {
        callbacks.render_storage_total(result.displayed_storage_total);
    }

    if (!input.displayed_cost.has_value()) {
        result.kind = FieldActionDescriptionRenderKind::missing_cost;
        if (callbacks.write_description_grid) {
            callbacks.write_description_grid(input.description_index);
        }
        return result;
    }

    result.kind = FieldActionDescriptionRenderKind::quantity_summary;
    result.displayed_cost = *input.displayed_cost;
    if (callbacks.render_cost) {
        callbacks.render_cost(result.displayed_cost);
    }
    if (callbacks.write_description_grid) {
        callbacks.write_description_grid(input.description_index);
    }
    return result;
}

FieldActionCostSummaryRenderResult render_field_action_cost_summary(
    const FieldActionCostSummaryRecord& record,
    const std::int8_t current_cursor,
    const FieldActionCostContext& cost_context,
    const FieldActionCostSummaryRenderCallbacks& callbacks) {
    FieldActionCostSummaryRenderResult result{};
    result.record = record;
    if (callbacks.render_title) {
        callbacks.render_title();
    }

    if (callbacks.render_numeric_field) {
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::byte_at_6, record.byte_at_6);
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::byte_at_7, record.byte_at_7);
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::byte_at_c, record.byte_at_c);
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::byte_at_d, record.byte_at_d);
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::word_at_8, record.word_at_8);
        callbacks.render_numeric_field(
            FieldActionCostSummaryNumericField::word_at_a, record.word_at_a);
    }

    if (current_cursor < record.availability_marker) {
        result.kind = FieldActionCostSummaryRenderKind::blocked;
        if (callbacks.write_blocked_grid) {
            callbacks.write_blocked_grid();
        }
        return result;
    }

    if (record.base_cost >= 0x10000U) {
        result.kind = FieldActionCostSummaryRenderKind::large_discounted;
        result.displayed_cost = apply_large_field_action_cost_discount(
            record.base_cost, cost_context);
    } else {
        result.kind = FieldActionCostSummaryRenderKind::discounted;
        result.displayed_cost = apply_field_action_cost_discounts(
            record.base_cost, cost_context);
    }
    if (callbacks.render_discounted_cost) {
        callbacks.render_discounted_cost(result.displayed_cost);
    }
    return result;
}

FieldActionFeedbackRenderResult render_field_action_feedback(
    const std::uint8_t mode,
    const FieldActionFeedbackRenderCallbacks& callbacks) {
    FieldActionFeedbackRenderResult result{};
    result.mode = mode;
    if (mode >= 4U) {
        result.kind = FieldActionFeedbackRenderKind::unavailable;
        if (callbacks.render_unavailable) {
            callbacks.render_unavailable();
        }
        if (callbacks.render_tail_text) {
            callbacks.render_tail_text();
        }
        return result;
    }

    result.kind = FieldActionFeedbackRenderKind::mode_feedback;
    result.quantity_text_offset = static_cast<std::uint8_t>(mode * 5U);
    result.grid_group = static_cast<std::uint8_t>(mode / 20U);
    if (callbacks.render_mode_text) {
        callbacks.render_mode_text(mode);
    }
    if (callbacks.render_quantity_text) {
        callbacks.render_quantity_text(result.quantity_text_offset);
    }
    if (callbacks.write_feedback_grid) {
        callbacks.write_feedback_grid(result.grid_group);
    }
    if (callbacks.render_tail_text) {
        callbacks.render_tail_text();
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
