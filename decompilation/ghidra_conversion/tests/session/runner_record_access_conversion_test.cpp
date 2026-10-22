#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    LiveRunnerRecord source{};
    for (std::size_t index = 0; index < source.bytes.size(); ++index) {
        source.bytes[index] = static_cast<std::uint8_t>(index);
    }

    const auto fields = read_runner_record_fields(source);
    if (!expect(fields.projection_word_08 == 0x0809U &&
                fields.kind == 0x0E0FU && fields.flags == 0x1011U &&
                fields.state_pointer == 0x12131415U &&
                fields.state_selector == 0x1617U &&
                fields.timing_multiplier == 0x1819U &&
                fields.timing_accumulator == 0x1A1BU &&
                fields.timing_threshold == 0x1C1DU &&
                fields.state_index == 0x1E1FU &&
                fields.first_aid_continuation == 0x3A3B3C3DU &&
                fields.command_overview_panel_parameter == 0x2C2DU &&
                fields.record_type == 0x3E3FU && fields.race_index == 0x00U &&
                fields.species_index == 0x9AU &&
                fields.archetype_index == 0x9BU &&
                fields.delay_meter == 0x4CU &&
                fields.behavior_meter == 0x9CU &&
                fields.magic_force == 0x9DU &&
                static_cast<std::uint8_t>(fields.action_result) == 0x9FU &&
                fields.animation_action_gate ==
                    static_cast<std::int16_t>(0xDCDDU) &&
                fields.equipped_inventory_slot == 0x56 &&
                fields.equipped_item_code == 0x57U &&
                fields.posture == 0x4DU && fields.ammo_rounds == 0x4EU &&
                fields.ammo_clips == 0x4FU &&
                fields.equipped_armor_index == 0x76U &&
                fields.cyberware_flags == 0x9091U &&
                fields.magic_table_index == 0xA0A1U &&
                fields.inventory_item_codes[0] == 0x66U &&
                fields.inventory_item_codes[7] == 0x6DU &&
                fields.inventory_phase_bytes[0] == 0x6EU &&
                fields.inventory_phase_bytes[7] == 0x75U &&
                fields.attributes[0] == 0x77U && fields.attributes[7] == 0x7EU &&
                fields.skills[0] == 0x80U && fields.skills[5] == 0x85U &&
                fields.skills[6] == 0x87U && fields.skills[11] == 0x8CU &&
                fields.auxiliary[0] == 0x7FU && fields.auxiliary[1] == 0x86U &&
                fields.auxiliary[2] == 0x8DU &&
                fields.auxiliary[3] == 0x8EU &&
                fields.cyberware_essence_tens() == 0x7EU &&
                fields.cyberware_essence_ones() == 0x8EU)) {
        return 1;
    }

    auto changed = fields;
    changed.projection_word_08 = 0xA1B2U;
    changed.state_pointer = 0xC3D4E5F6U;
    changed.command_overview_panel_parameter = 0x7788U;
    changed.first_aid_continuation = 0xABCDEF01U;
    changed.equipped_inventory_slot = -1;
    changed.posture = 4U;
    changed.inventory_item_codes[3] = 0x2AU;
    changed.inventory_phase_bytes[3] = 0x09U;
    changed.attributes[2] = 0x55U;
    changed.skills[8] = 0x66U;
    changed.cyberware_flags = 0x0123U;
    changed.magic_table_index = 0x4567U;
    changed.species_index = 0x12U;
    changed.action_result = 7;
    changed.animation_action_gate = 0x1234;

    LiveRunnerRecord destination{};
    destination.bytes.fill(0xEEU);
    write_runner_record_fields(changed, destination);
    if (!expect(destination.bytes[0x08U] == 0xA1U &&
                destination.bytes[0x09U] == 0xB2U &&
                destination.bytes[0x12U] == 0xC3U &&
                destination.bytes[0x15U] == 0xF6U &&
                destination.bytes[0x2CU] == 0x77U &&
                destination.bytes[0x2DU] == 0x88U &&
                destination.bytes[0x3AU] == 0xABU &&
                destination.bytes[0x3DU] == 0x01U &&
                destination.bytes[0x56U] == 0xFFU &&
                destination.bytes[0x4DU] == 4U &&
                destination.bytes[0x69U] == 0x2AU &&
                destination.bytes[0x71U] == 0x09U &&
                destination.bytes[0x79U] == 0x55U &&
                destination.bytes[0x89U] == 0x66U &&
                destination.bytes[0x90U] == 0x01U &&
                destination.bytes[0x91U] == 0x23U &&
                destination.bytes[0x9FU] == 7U &&
                destination.bytes[0xDCU] == 0x12U &&
                destination.bytes[0xDDU] == 0x34U &&
                destination.bytes[0x9DU] == 0x9DU &&
                destination.bytes[0x9AU] == 0x12U &&
                destination.bytes[0xA0U] == 0x45U &&
                destination.bytes[0xA1U] == 0x67U &&
                destination.bytes[0x05U] == 0xEEU)) {
        return 2;
    }

    RunnerTimingRosterState timing_roster{};
    timing_roster.records.resize(2U);
    timing_roster.current_record_index = 1U;
    timing_roster.records[1U].animation_index = 0U;
    if (!lookup_runner_animation_delta(timing_roster).has_value() ||
        *lookup_runner_animation_delta(timing_roster) != 100) {
        return 3;
    }
    timing_roster.records[1U].animation_index = 14U;
    if (!lookup_runner_animation_delta(timing_roster).has_value() ||
        *lookup_runner_animation_delta(timing_roster) != -1) {
        return 4;
    }
    timing_roster.records[1U].animation_index = 20U;
    if (!lookup_runner_animation_delta(timing_roster).has_value() ||
        *lookup_runner_animation_delta(timing_roster) != -8) {
        return 7;
    }
    timing_roster.records[1U].active_effect = true;
    if (!runner_record_has_active_effect(timing_roster)) {
        return 5;
    }
    timing_roster.current_record_index = 2U;
    if (lookup_runner_animation_delta(timing_roster).has_value() ||
        runner_record_has_active_effect(timing_roster)) {
        return 6;
    }

    return 0;
}
