#include "shadowrun_ghidra/entities/action_detail_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

ActionDetailLabelVariant label_for_field_action(const std::uint8_t code) {
    if (code < 0x0CU) {
        return ActionDetailLabelVariant::ordinary;
    }
    if (code < 0x13U) {
        return ActionDetailLabelVariant::extended;
    }
    if (code == 0x13U || (code >= 0x17U && code < 0x28U)) {
        return ActionDetailLabelVariant::action_dependent;
    }
    return ActionDetailLabelVariant::none;
}

ActionDetailLabelVariant label_for_inventory_item(const std::uint8_t code) {
    if (code < 0x0DU) {
        return ActionDetailLabelVariant::ordinary;
    }
    if (code < 0x14U) {
        return ActionDetailLabelVariant::extended;
    }
    if (code == 0x14U || (code >= 0x18U && code < 0x29U)) {
        return ActionDetailLabelVariant::action_dependent;
    }
    return ActionDetailLabelVariant::none;
}

std::array<bool, 6U> build_phase_overlays(const std::uint8_t phase) {
    // The source tests bits in this order: 1, 2, 0, 3, 4, 5.  This is the
    // visible overlay order, not a bitfield reinterpretation.
    constexpr std::array<std::uint8_t, 6U> kSourceBitOrder{1U, 2U, 0U,
                                                            3U, 4U, 5U};
    std::array<bool, 6U> overlays{};
    for (std::size_t index = 0; index < kSourceBitOrder.size(); ++index) {
        overlays[index] =
            (phase & (std::uint8_t{1U} << kSourceBitOrder[index])) != 0U;
    }
    return overlays;
}

std::uint8_t clamp_entity_label_index(const std::int32_t value) {
    if (value <= 0) {
        return 0U;
    }
    if (value >= static_cast<std::int32_t>(kActionDetailLabelCount - 1U)) {
        return static_cast<std::uint8_t>(kActionDetailLabelCount - 1U);
    }
    return static_cast<std::uint8_t>(value);
}

} // namespace

std::optional<ActionDetailRenderPlan> select_entity_action_detail(
    const EntityActionDetailInput& input) {
    std::uint8_t detail_selector = 0;
    std::optional<std::uint8_t> selected_action = input.selected_action_code;
    bool action_uses_detail_table = false;

    if (input.source_mode < 2U) {
        if (!selected_action.has_value()) {
            return std::nullopt;
        }
        detail_selector = *selected_action;
        if (detail_selector == 0U) {
            detail_selector = input.cyberware_base;
            for (std::uint8_t bit = 5U; bit <= 8U; ++bit) {
                if ((input.cyberware_flags & (std::uint16_t{1U} << bit)) ==
                    0U) {
                    continue;
                }
                detail_selector = static_cast<std::uint8_t>(
                    detail_selector + static_cast<std::uint8_t>(bit - 4U));
                break;
            }
        }
        else {
            action_uses_detail_table = true;
        }
    } else if (input.display_state < 0) {
        detail_selector = input.auxiliary_value;
    } else {
        detail_selector = input.active_action_code;
        if (detail_selector != 0U) {
            action_uses_detail_table = true;
        } else {
            detail_selector = input.cyberware_base;
            for (std::uint8_t bit = 5U; bit <= 8U; ++bit) {
                if ((input.cyberware_flags & (std::uint16_t{1U} << bit)) ==
                    0U) {
                    continue;
                }
                detail_selector = static_cast<std::uint8_t>(
                    detail_selector + static_cast<std::uint8_t>(bit - 4U));
                break;
            }
        }
    }

    if (action_uses_detail_table) {
        // ROM 0x13084/0x1308C reads record +0x26 before the one-based
        // subtraction and thirteen-entry clamp. A missing row is an
        // unresolved source-data boundary, not permission to reuse the
        // action code as a fabricated label selector.
        if (!input.action_detail_code.has_value()) {
            return std::nullopt;
        }
        detail_selector = *input.action_detail_code;
    }

    ActionDetailRenderPlan plan{};
    plan.valid = true;
    plan.source_code = detail_selector;
    plan.label = ActionDetailLabelVariant::ordinary;
    plan.label_index = clamp_entity_label_index(
        static_cast<std::int32_t>(detail_selector) - 1);
    plan.action_code = selected_action;
    return plan;
}

ActionDetailRenderPlan build_field_action_detail_plan(
    const FieldActionDetailInput& input,
    const ActionDetailResourceTable& resources,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource) {
    ActionDetailRenderPlan plan{};
    plan.source_code = input.action_code;
    plan.resource_row = input.action_code;

    if (plan.resource_row >= resources.detail_values.size()) {
        return plan;
    }

    const auto previous_variant = resource_variant.alternate;
    update_resource_variant(resource_variant,
                            static_cast<std::int16_t>(input.action_code) + 1,
                            upload_resource);
    plan.valid = true;
    plan.detail_value = resources.detail_values[plan.resource_row];
    plan.phase_overlays = build_phase_overlays(input.flags);
    plan.label = label_for_field_action(input.action_code);
    plan.detail_value = plan.label == ActionDetailLabelVariant::action_dependent
                            ? input.flags
                            : plan.detail_value;
    plan.alternate_resource_variant = resource_variant.alternate;
    plan.resource_upload_requested =
        previous_variant != resource_variant.alternate;
    return plan;
}

std::optional<ActionDetailRenderPlan> build_inventory_action_detail_plan(
    const InventoryActionDetailInput& input,
    const ActionDetailResourceTable& resources,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource) {
    if (input.item_code == 0U) {
        return std::nullopt;
    }

    const auto row = static_cast<std::size_t>(input.item_code - 1U);
    if (row >= resources.detail_values.size()) {
        return std::nullopt;
    }

    const auto previous_variant = resource_variant.alternate;
    update_resource_variant(resource_variant,
                            static_cast<std::int16_t>(input.item_code),
                            upload_resource);

    ActionDetailRenderPlan plan{};
    plan.valid = true;
    plan.source_code = input.item_code;
    plan.resource_row = row;
    plan.detail_value = resources.detail_values[row];
    plan.phase_overlays = build_phase_overlays(input.phase);
    plan.label = label_for_inventory_item(input.item_code);
    if (plan.label == ActionDetailLabelVariant::action_dependent) {
        plan.detail_value = input.phase;
        plan.action_code = input.selected_action_code;
    }
    plan.alternate_resource_variant = resource_variant.alternate;
    plan.resource_upload_requested =
        previous_variant != resource_variant.alternate;
    return plan;
}

} // namespace shadowrun::ghidra_conversion
