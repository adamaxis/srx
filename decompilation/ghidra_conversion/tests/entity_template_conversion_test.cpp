#include "shadowrun_ghidra/entity_template_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

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
        return 1;
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
        return 2;
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
        return 3;
    }

    return 0;
}
