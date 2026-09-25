#include "shadowrun_ghidra/runner_record_access_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t read_be16(const LiveRunnerRecord& record,
                        const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(record.bytes[offset]) << 8U) |
        record.bytes[offset + 1U]);
}

std::uint32_t read_be32(const LiveRunnerRecord& record,
                        const std::size_t offset) {
    return (static_cast<std::uint32_t>(read_be16(record, offset)) << 16U) |
           read_be16(record, offset + 2U);
}

void write_be16(LiveRunnerRecord& record, const std::size_t offset,
                const std::uint16_t value) {
    record.bytes[offset] = static_cast<std::uint8_t>(value >> 8U);
    record.bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be32(LiveRunnerRecord& record, const std::size_t offset,
                const std::uint32_t value) {
    write_be16(record, offset, static_cast<std::uint16_t>(value >> 16U));
    write_be16(record, offset + 2U,
               static_cast<std::uint16_t>(value & 0xFFFFU));
}

constexpr std::array<std::size_t, kRunnerRecordAuxiliaryCount>
    kAuxiliaryOffsets{{0x7FU, 0x86U, 0x8DU, 0x8EU}};

} // namespace

RunnerRecordFields read_runner_record_fields(const LiveRunnerRecord& record) {
    RunnerRecordFields fields;
    fields.projection_word_08 = read_be16(record, 0x08U);
    fields.kind = read_be16(record, 0x0EU);
    fields.flags = read_be16(record, 0x10U);
    fields.state_pointer = read_be32(record, 0x12U);
    fields.state_selector = read_be16(record, 0x16U);
    fields.timing_multiplier = read_be16(record, 0x18U);
    fields.timing_accumulator = read_be16(record, 0x1AU);
    fields.timing_threshold = read_be16(record, 0x1CU);
    fields.state_index = read_be16(record, 0x1EU);
    fields.record_type = read_be16(record, 0x3EU);

    fields.race_index = record.bytes[0x00U];
    fields.archetype_index = record.bytes[0x9BU];
    fields.physical_value = record.bytes[0x4CU];
    fields.mental_value = record.bytes[0x9CU];
    fields.equipped_inventory_slot =
        static_cast<std::int8_t>(record.bytes[0x56U]);
    fields.equipped_item_code = record.bytes[0x57U];
    fields.posture = record.bytes[0x4DU];
    fields.ammo_rounds = record.bytes[0x4EU];
    fields.ammo_clips = record.bytes[0x4FU];
    fields.equipped_armor_index = record.bytes[0x76U];
    fields.cyberware_flags = read_be16(record, 0x90U);
    fields.magic_table_index = read_be16(record, 0xA0U);

    for (std::size_t index = 0; index < kRunnerRecordInventorySlots;
         ++index) {
        fields.inventory_item_codes[index] = record.bytes[0x66U + index];
        fields.inventory_phase_bytes[index] = record.bytes[0x6EU + index];
    }
    for (std::size_t index = 0; index < kRunnerRecordAttributeCount;
         ++index) {
        fields.attributes[index] = record.bytes[0x77U + index];
    }
    for (std::size_t index = 0; index < 6U; ++index) {
        fields.skills[index] = record.bytes[0x80U + index];
        fields.skills[6U + index] = record.bytes[0x87U + index];
    }
    for (std::size_t index = 0; index < kRunnerRecordAuxiliaryCount;
         ++index) {
        fields.auxiliary[index] = record.bytes[kAuxiliaryOffsets[index]];
    }
    return fields;
}

void write_runner_record_fields(const RunnerRecordFields& fields,
                                LiveRunnerRecord& destination) {
    write_be16(destination, 0x08U, fields.projection_word_08);
    write_be16(destination, 0x0EU, fields.kind);
    write_be16(destination, 0x10U, fields.flags);
    write_be32(destination, 0x12U, fields.state_pointer);
    write_be16(destination, 0x16U, fields.state_selector);
    write_be16(destination, 0x18U, fields.timing_multiplier);
    write_be16(destination, 0x1AU, fields.timing_accumulator);
    write_be16(destination, 0x1CU, fields.timing_threshold);
    write_be16(destination, 0x1EU, fields.state_index);
    write_be16(destination, 0x3EU, fields.record_type);

    destination.bytes[0x00U] = fields.race_index;
    destination.bytes[0x9BU] = fields.archetype_index;
    destination.bytes[0x4CU] = fields.physical_value;
    destination.bytes[0x9CU] = fields.mental_value;
    destination.bytes[0x56U] =
        static_cast<std::uint8_t>(fields.equipped_inventory_slot);
    destination.bytes[0x57U] = fields.equipped_item_code;
    destination.bytes[0x4DU] = fields.posture;
    destination.bytes[0x4EU] = fields.ammo_rounds;
    destination.bytes[0x4FU] = fields.ammo_clips;
    destination.bytes[0x76U] = fields.equipped_armor_index;
    write_be16(destination, 0x90U, fields.cyberware_flags);
    write_be16(destination, 0xA0U, fields.magic_table_index);

    for (std::size_t index = 0; index < kRunnerRecordInventorySlots;
         ++index) {
        destination.bytes[0x66U + index] = fields.inventory_item_codes[index];
        destination.bytes[0x6EU + index] = fields.inventory_phase_bytes[index];
    }
    for (std::size_t index = 0; index < kRunnerRecordAttributeCount;
         ++index) {
        destination.bytes[0x77U + index] = fields.attributes[index];
    }
    for (std::size_t index = 0; index < 6U; ++index) {
        destination.bytes[0x80U + index] = fields.skills[index];
        destination.bytes[0x87U + index] = fields.skills[6U + index];
    }
    for (std::size_t index = 0; index < kRunnerRecordAuxiliaryCount;
         ++index) {
        destination.bytes[kAuxiliaryOffsets[index]] = fields.auxiliary[index];
    }
}

std::optional<std::int16_t> lookup_runner_animation_delta(
    const RunnerTimingRosterState& roster) {
    static constexpr std::array<std::int16_t, 16> kAnimationDeltas{
        100, 20, 10, 9, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2};
    const auto record_index = static_cast<std::size_t>(roster.current_index);
    if (record_index >= roster.records.size()) {
        return std::nullopt;
    }
    const auto index = static_cast<std::size_t>(
        roster.records[record_index].bytes[0x10U]);
    if (index >= kAnimationDeltas.size()) {
        return std::nullopt;
    }
    return kAnimationDeltas[index];
}

bool runner_record_has_active_effect(const RunnerTimingRosterState& roster) {
    const auto record_index = static_cast<std::size_t>(roster.current_index);
    return record_index < roster.records.size() &&
           roster.records[record_index].bytes[0x0FU] != 0U;
}

} // namespace shadowrun::ghidra_conversion
