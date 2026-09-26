#include "shadowrun_ghidra/action_detail_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    EntityActionDetailInput entity{};
    entity.source_mode = 0U;
    entity.selected_action_code = 5U;
    const auto selected = select_entity_action_detail(entity);
    if (!selected.has_value() || !selected->valid ||
        selected->label_index != 4U || selected->source_code != 5U) {
        return 1;
    }

    entity.selected_action_code = 0U;
    entity.cyberware_base = 2U;
    entity.cyberware_flags = static_cast<std::uint16_t>(1U << 6U);
    const auto cyberware = select_entity_action_detail(entity);
    if (!cyberware.has_value() || cyberware->source_code != 4U ||
        cyberware->label_index != 3U) {
        return 2;
    }

    entity.source_mode = 2U;
    entity.display_state = -1;
    entity.auxiliary_value = 9U;
    const auto auxiliary = select_entity_action_detail(entity);
    if (!auxiliary.has_value() || auxiliary->source_code != 9U ||
        auxiliary->label_index != 8U) {
        return 3;
    }

    ActionDetailResourceTable resources{};
    for (std::size_t index = 0; index < resources.detail_values.size();
         ++index) {
        resources.detail_values[index] = static_cast<std::uint8_t>(index + 1U);
    }
    ResourceVariantState variant{};
    int uploads = 0;
    const auto field = build_field_action_detail_plan(
        FieldActionDetailInput{3U, 0x25U}, resources, variant,
        [&] { ++uploads; });
    if (!field.valid || field.resource_row != 3U || field.detail_value != 4U ||
        field.label != ActionDetailLabelVariant::ordinary ||
        field.phase_overlays != std::array<bool, 6U>{false, true, true, false,
                                                      false, true} ||
        field.alternate_resource_variant || field.resource_upload_requested ||
        uploads != 0) {
        return 4;
    }

    const auto extended = build_field_action_detail_plan(
        FieldActionDetailInput{12U, 0U}, resources, variant,
        [&] { ++uploads; });
    if (!extended.valid ||
        extended.label != ActionDetailLabelVariant::extended ||
        !extended.alternate_resource_variant ||
        !extended.resource_upload_requested || uploads != 1) {
        return 5;
    }

    const auto flagged = build_field_action_detail_plan(
        FieldActionDetailInput{0x13U, 0xA5U}, resources, variant,
        [&] { ++uploads; });
    if (!flagged.valid ||
        flagged.label != ActionDetailLabelVariant::action_dependent ||
        flagged.detail_value != 0xA5U) {
        return 6;
    }

    const auto item = build_inventory_action_detail_plan(
        InventoryActionDetailInput{1U, 0x06U, std::nullopt}, resources, variant,
        [&] { ++uploads; });
    if (!item.has_value() || item->resource_row != 0U ||
        item->detail_value != 1U ||
        item->label != ActionDetailLabelVariant::ordinary ||
        item->phase_overlays != std::array<bool, 6U>{true, true, false, false,
                                                      false, false}) {
        return 7;
    }

    const auto action_item = build_inventory_action_detail_plan(
        InventoryActionDetailInput{0x14U, 0x02U, 0x27U}, resources, variant,
        [&] { ++uploads; });
    if (!action_item.has_value() ||
        action_item->label != ActionDetailLabelVariant::action_dependent ||
        action_item->detail_value != 0x02U ||
        !action_item->action_code.has_value() ||
        *action_item->action_code != 0x27U) {
        return 8;
    }

    if (build_inventory_action_detail_plan(
            InventoryActionDetailInput{0U, 0U, std::nullopt}, resources,
            variant)
            .has_value()) {
        return 9;
    }

    return 0;
}
