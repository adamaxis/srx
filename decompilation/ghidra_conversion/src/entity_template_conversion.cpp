#include "shadowrun_ghidra/entity_template_conversion.hpp"

namespace shadowrun::ghidra_conversion {

EntityRecord initialize_entity_record_from_template(
    const EntityRecordInitialization& initialization,
    const EntityTemplateData& template_data,
    const EntityRecordDefaultFactory& make_default_record,
    const EntityRecordRandomBoundary& next_random,
    const EntityRecordOverrideBoundary& apply_override) {
    EntityRecord record = make_default_record ? make_default_record()
                                              : EntityRecord{};

    record.entity_slot = initialization.entity_slot;
    record.behavior_flags = 0x05U;
    record.state_word = initialization.state_word;
    record.auxiliary_word = initialization.auxiliary_word;
    record.vertical_origin = initialization.vertical_origin;
    record.source_position = initialization.source_position;
    record.world_position = {
        initialization.source_position.x,
        static_cast<std::uint16_t>(initialization.source_position.y +
                                   initialization.vertical_origin)};
    record.base_position = record.world_position;
    record.template_data = template_data;

    const auto random_value = next_random ? next_random() : 0U;
    record.random_variant = initialization.scene_id == 0x0037U
                                ? 0x0028U
                                : static_cast<std::uint16_t>(random_value &
                                                              0x000FU);
    if (initialization.special_state_active) {
        record.effect_flags = static_cast<std::uint8_t>(
            record.effect_flags | 0x40U);
    }

    if (initialization.override_mode == 2U) {
        for (const auto& override_value : initialization.overrides) {
            if (apply_override) {
                apply_override(record, override_value);
            }
        }
    }
    return record;
}

EntityTemplateSpawnResult spawn_entity_template_list(
    const std::vector<EntityTemplateSpawnDescriptor>& descriptors,
    EntityTemplateSpawnState& state,
    const EntityRecordSlotAllocator& allocate_entity_slot,
    const EntityRecordSlotAllocator& allocate_effect_slot,
    const EntityRecordDefaultFactory& make_default_record,
    const EntityTemplateRecordInitializer& initialize_record) {
    EntityTemplateSpawnResult result{};

    for (const auto& descriptor : descriptors) {
        if (descriptor.sentinel) {
            break;
        }

        if (!allocate_entity_slot) {
            result.action = EntityTemplateSpawnAction::entity_table_full;
            break;
        }
        const auto entity_slot = allocate_entity_slot();
        if (!entity_slot.has_value()) {
            result.action = EntityTemplateSpawnAction::entity_table_full;
            break;
        }
        if (!allocate_effect_slot) {
            result.action = EntityTemplateSpawnAction::effect_table_full;
            break;
        }
        const auto effect_slot = allocate_effect_slot();
        if (!effect_slot.has_value()) {
            result.action = EntityTemplateSpawnAction::effect_table_full;
            break;
        }
        if (descriptor.root_record) {
            state.vertical_origin = 0U;
        } else {
            state.vertical_origin = static_cast<std::uint16_t>(
                state.vertical_origin + 0x00C0U);
        }

        const EntityRecordInitialization initialization{
            *entity_slot,
            state.state_word,
            state.auxiliary_word,
            state.vertical_origin,
            descriptor.source_position,
            state.scene_id,
            state.special_state_active,
            0U,
            {}};
        EntityRecord record = initialize_record
                                  ? initialize_record(initialization,
                                                      descriptor.template_data)
                                  : (make_default_record ? make_default_record()
                                                          : EntityRecord{});
        record.entity_slot = *entity_slot;
        record.effect_slot = *effect_slot;
        record.vertical_origin = state.vertical_origin;
        if (descriptor.root_record) {
            record.behavior_flags = static_cast<std::uint8_t>(
                record.behavior_flags | 0x20U);
            state.root_entity_slot = *entity_slot;
        } else {
            record.linked_root_entity_slot = state.root_entity_slot;
        }
        result.records.push_back(record);
    }

    result.final_vertical_origin = state.vertical_origin;
    return result;
}

} // namespace shadowrun::ghidra_conversion
