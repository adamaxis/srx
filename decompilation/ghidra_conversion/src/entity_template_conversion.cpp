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

void process_pending_entity_template_lists(
    EntityTemplateCoordinatorState& state,
    const EntityTemplateServiceBoundary& service_scene_two,
    const EntityTemplatePreparationBoundary& prepare_lists,
    const EntityTemplateListBoundary& spawn_list,
    const EntityTemplateTimeoutBoundary& schedule_timeout) {
    if (state.scene_mode == 2U && service_scene_two) {
        service_scene_two();
    }
    if (prepare_lists) {
        prepare_lists(state);
    }

    if (state.generated_list.has_value()) {
        if (spawn_list) {
            spawn_list(state.generated_list->descriptors);
        }
        state.generated_list.reset();
    }

    if (!state.selected_list.has_value()) {
        if (spawn_list) {
            spawn_list(state.default_list);
        }
        return;
    }

    const auto selected = *state.selected_list;
    if (selected.timeout_source.has_value()) {
        if (schedule_timeout) {
            schedule_timeout(*selected.timeout_source);
        }
        state.world_state = 4U;
    }
    if (spawn_list) {
        spawn_list(selected.descriptors);
        spawn_list(state.default_list);
    }
    state.selected_list.reset();
}

EntityTemplatePreparationAction prepare_pending_entity_template_list(
    EntityTemplatePreparationState& state,
    const PendingEntityTemplateList& gate_b_fallback,
    const PendingEntityTemplateList& mode_four_fallback,
    const EntityTemplateRandomByteBoundary& next_random_byte,
    const EntityTemplateRandomListBoundary& select_random_list) {
    const auto install = [&state](const PendingEntityTemplateList& list) {
        state.generated_list = list;
        state.generated_header = list.header_value;
    };

    if (!state.gate_a_closed) {
        if (!state.gate_b_closed) {
            if (state.event_phase != 5U || state.world_mode != 2U) {
                return EntityTemplatePreparationAction::no_list;
            }
            if (state.selection_counter == 0U) {
                const auto random_byte =
                    next_random_byte ? next_random_byte() : 0U;
                state.selection_counter = static_cast<std::uint8_t>(
                    random_byte + 1U);
            }
            if (state.expected_context != state.current_context ||
                state.active_entity_present || !select_random_list) {
                return EntityTemplatePreparationAction::no_list;
            }
            const auto selected = select_random_list();
            if (!selected.has_value()) {
                return EntityTemplatePreparationAction::no_list;
            }
            install(*selected);
            return EntityTemplatePreparationAction::random_list;
        }
        install(gate_b_fallback);
        return EntityTemplatePreparationAction::gate_b_fallback;
    }

    if (state.current_context != 4U) {
        return EntityTemplatePreparationAction::no_list;
    }
    install(mode_four_fallback);
    return EntityTemplatePreparationAction::mode_four_fallback;
}

void finalize_pending_entity_template_update(
    EntityTemplateUpdateState& state,
    const std::function<void()>& process_pending_lists) {
    if (process_pending_lists) {
        process_pending_lists();
    }
    if (state.scene_mode == 2U) {
        state.alternate_state_marker = 6U;
    }
    if (state.scene_mode == 1U) {
        state.primary_state_marker = 6U;
    }
}

} // namespace shadowrun::ghidra_conversion
