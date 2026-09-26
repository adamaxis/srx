#include "shadowrun_ghidra/entities/entity_template_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<JoshuaArchetypeTemplate> archetype_templates(3U);
    archetype_templates[0].record.fill(0x10U);
    archetype_templates[1].record.fill(0x20U);
    archetype_templates[2].record.fill(0x30U);
    const auto selected_archetype =
        initialize_joshua_archetype_selection(2U, archetype_templates);
    if (!expect(selected_archetype.has_value() &&
                selected_archetype->live_joshua_record[0] == 0x30U &&
                selected_archetype->live_joshua_record[0xFFU] == 0x30U &&
                selected_archetype->record_selection_markers ==
                    std::array<std::uint16_t, 3>{0U, 1U, 2U} &&
                !initialize_joshua_archetype_selection(3U,
                                                        archetype_templates)
                     .has_value())) {
        return 1;
    }

    EntityTemplateData template_data{};
    template_data.word_00 = 0x1111U;
    template_data.word_02 = 0x2222U;
    template_data.bytes_04_to_09 = {1U, 2U, 3U, 4U, 5U, 6U};
    template_data.longwords_0a_to_15 = {0x01020304U, 0x05060708U,
                                         0x090A0B0CU};
    template_data.word_16 = 0x3333U;
    template_data.longwords_18_to_27 = {0x10111213U, 0x14151617U,
                                         0x18191A1BU, 0x1C1D1E1FU};
    template_data.bytes_28_to_3f.fill(0x5AU);
    template_data.byte_41 = 0x41U;
    template_data.byte_42 = 0x42U;
    template_data.byte_43 = 0x43U;
    template_data.word_44 = 0x4444U;
    template_data.word_46 = 0x4646U;
    template_data.byte_48 = 0x48U;
    template_data.byte_49 = 0x49U;

    EntityRecord default_record{};
    default_record.behavior_flags = 0xFFU;
    default_record.random_variant = 0xFFFFU;
    const auto record = initialize_entity_record_from_template(
        EntityRecordInitialization{
            7U, 0x1234U, 0x5678U, 0x00C0U, {0x4000U, 0x0200U},
            0x0012U, true, 2U, {{0x009EU, 0x03U}, {0x004DU, 0x04U}}},
        template_data,
        [&default_record]() { return default_record; },
        []() { return static_cast<std::uint16_t>(0x00ABU); },
        [](EntityRecord& target, const EntityRecordOverride& value) {
            if (value.source_offset == 0x009EU) {
                target.effect_flags = value.value;
            }
        });

    if (!expect(record.entity_slot == 7U && record.behavior_flags == 5U &&
                record.state_word == 0x1234U &&
                record.auxiliary_word == 0x5678U &&
                record.world_position.x == 0x4000U &&
                record.world_position.y == 0x02C0U &&
                record.base_position.x == 0x4000U &&
                record.base_position.y == 0x02C0U &&
                record.template_data.word_00 == 0x1111U &&
                record.template_data.longwords_18_to_27[3] == 0x1C1D1E1FU &&
                record.random_variant == 0x000BU &&
                record.effect_flags == 0x03U)) {
        return 2;
    }

    const auto forced_variant = initialize_entity_record_from_template(
        EntityRecordInitialization{0U, 0U, 0U, 0U, {}, 0x0037U, false, 0U,
                                   {}},
        {},
        {},
        []() { return static_cast<std::uint16_t>(0x000FU); },
        {});
    if (!expect(forced_variant.random_variant == 0x0028U &&
                forced_variant.effect_flags == 0U)) {
        return 3;
    }

    EntityTemplateSpawnState list_state{0xAAAAU, 0xBBBBU, 0x0001U, false,
                                        0x0040U, std::nullopt};
    std::vector<std::uint16_t> initialized_slots;
    std::uint16_t next_entity_slot = 3U;
    std::uint16_t next_effect_slot = 7U;
    const auto list_result = spawn_entity_template_list(
        {{{}, {0x0100U, 0x0200U}, true, false},
         {{}, {0x0300U, 0x0400U}, false, false},
        {{}, {0x0500U, 0x0600U}, true, false},
         {{}, {}, false, true}},
        list_state,
        [&next_entity_slot]() {
            return std::optional<std::uint16_t>{next_entity_slot++};
        },
        [&next_effect_slot]() {
            return std::optional<std::uint16_t>{next_effect_slot++};
        },
        {},
        [&initialized_slots](const EntityRecordInitialization& initialization,
                             const EntityTemplateData&) {
            initialized_slots.push_back(initialization.entity_slot);
            EntityRecord record{};
            record.entity_slot = initialization.entity_slot;
            record.behavior_flags = 0x05U;
            return record;
        });
    if (!expect(list_result.action == EntityTemplateSpawnAction::completed &&
                initialized_slots == std::vector<std::uint16_t>{3U, 4U, 5U} &&
                list_result.records.size() == 3U &&
                list_result.records[0].vertical_origin == 0U &&
                list_result.records[0].behavior_flags == 0x25U &&
                !list_result.records[0].linked_root_entity_slot.has_value() &&
                list_result.records[1].vertical_origin == 0x00C0U &&
                list_result.records[1].linked_root_entity_slot == 3U &&
                list_result.records[2].vertical_origin == 0U &&
                list_result.records[2].behavior_flags == 0x25U &&
                list_result.final_vertical_origin == 0U)) {
        return 4;
    }

    EntityTemplateCoordinatorState coordinator{};
    coordinator.scene_mode = 2U;
    coordinator.default_list = {{{}, {0x7000U, 0x7100U}, true, false}};
    std::vector<char> coordinator_events;
    process_pending_entity_template_lists(
        coordinator,
        [&coordinator_events]() { coordinator_events.push_back('s'); },
        [&coordinator_events](EntityTemplateCoordinatorState& state) {
            coordinator_events.push_back('p');
            state.generated_list =
                PendingEntityTemplateList{{{{}, {0x1000U, 0x1100U}, true,
                                             false}},
                                           std::nullopt};
        },
        [&coordinator_events](
            const std::vector<EntityTemplateSpawnDescriptor>& descriptors) {
            coordinator_events.push_back(
                descriptors.size() == 1U &&
                        descriptors.front().source_position.x == 0x1000U
                    ? 'g'
                    : 'd');
        },
        [&coordinator_events](const EntityTemplateTimeoutSource source) {
            coordinator_events.push_back(
                source == EntityTemplateTimeoutSource::first_event ? 't'
                                                                    : 'x');
        });
    if (!expect(coordinator_events == std::vector<char>{'s', 'p', 'g', 'd'} &&
                coordinator.generated_list == std::nullopt &&
                coordinator.selected_list == std::nullopt)) {
        return 5;
    }

    coordinator_events.clear();
    coordinator.selected_list = PendingEntityTemplateList{
        {{{}, {0x2000U, 0x2100U}, true, false}},
        EntityTemplateTimeoutSource::second_event};
    process_pending_entity_template_lists(
        coordinator, {}, {},
        [&coordinator_events](
            const std::vector<EntityTemplateSpawnDescriptor>& descriptors) {
            coordinator_events.push_back(
                descriptors.front().source_position.x == 0x2000U ? 'q'
                                                                  : 'd');
        },
        [&coordinator_events](const EntityTemplateTimeoutSource source) {
            coordinator_events.push_back(
                source == EntityTemplateTimeoutSource::second_event ? 't'
                                                                     : 'x');
        });
    if (!expect(coordinator_events == std::vector<char>{'t', 'q', 'd'} &&
                coordinator.world_state == 4U &&
                coordinator.selected_list == std::nullopt)) {
        return 6;
    }

    EntityTemplateUpdateState update_state{2U, 0U, 0U};
    bool coordinator_called = false;
    finalize_pending_entity_template_update(
        update_state, [&coordinator_called]() { coordinator_called = true; });
    if (!expect(coordinator_called && update_state.alternate_state_marker == 6U &&
                update_state.primary_state_marker == 0U)) {
        return 7;
    }

    update_state = EntityTemplateUpdateState{1U, 0U, 0U};
    finalize_pending_entity_template_update(update_state, {});
    if (!expect(update_state.primary_state_marker == 6U &&
                update_state.alternate_state_marker == 0U)) {
        return 8;
    }

    const PendingEntityTemplateList random_list{
        {{{}, {0x8000U, 0x8100U}, true, false}}, std::nullopt, 0x12345678U};
    const PendingEntityTemplateList gate_b_list{
        {{{}, {0x8200U, 0x8300U}, true, false}}, std::nullopt, 0x22000000U};
    const PendingEntityTemplateList mode_four_list{
        {{{}, {0x8400U, 0x8500U}, true, false}}, std::nullopt, 0x44000000U};
    EntityTemplatePreparationState preparation{
        false, false, 5U, 2U, 0U, 9U, 9U, false, std::nullopt,
        std::nullopt};
    const auto random_action = prepare_pending_entity_template_list(
        preparation, gate_b_list, mode_four_list,
        []() { return static_cast<std::uint8_t>(0x10U); },
        [&random_list]() { return std::optional<PendingEntityTemplateList>{
                                  random_list}; });
    if (!expect(random_action == EntityTemplatePreparationAction::random_list &&
                preparation.selection_counter == 0x11U &&
                preparation.generated_header == 0x12345678U &&
                preparation.generated_list.has_value())) {
        return 9;
    }

    preparation = EntityTemplatePreparationState{
        false, true, 0U, 0U, 4U, 0U, 7U, true, std::nullopt, std::nullopt};
    if (!expect(prepare_pending_entity_template_list(
                    preparation, gate_b_list, mode_four_list, {}, {}) ==
                    EntityTemplatePreparationAction::gate_b_fallback &&
                preparation.generated_header == 0x22000000U)) {
        return 10;
    }

    preparation = EntityTemplatePreparationState{
        true, false, 0U, 0U, 4U, 0U, 4U, true, std::nullopt, std::nullopt};
    if (!expect(prepare_pending_entity_template_list(
                    preparation, gate_b_list, mode_four_list, {}, {}) ==
                    EntityTemplatePreparationAction::mode_four_fallback &&
                preparation.generated_header == 0x44000000U)) {
        return 11;
    }

    const PendingEntityTemplateList first_selection{
        {{{}, {0x9000U, 0x9100U}, true, false}}, std::nullopt,
        0x90000000U};
    const PendingEntityTemplateList second_selection{
        {{{}, {0x9200U, 0x9300U}, true, false}}, std::nullopt,
        0x92000000U};
    const EntityTemplateSelectionTable selection_table{
        0x0015U, {{first_selection}, {second_selection}}};
    const auto selected = select_pending_entity_template_list(
        0x0016U, 1U, selection_table);
    if (!expect(selected.has_value() && selected->header_value == 0x92000000U &&
                selected->descriptors.front().source_position.x == 0x9200U &&
                !select_pending_entity_template_list(0x0014U, 1U,
                                                     selection_table)
                     .has_value() &&
                !select_pending_entity_template_list(0x0015U, 3U,
                                                     selection_table)
                     .has_value())) {
        return 12;
    }

    return 0;
}
