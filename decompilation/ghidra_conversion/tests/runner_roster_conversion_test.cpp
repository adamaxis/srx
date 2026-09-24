#include "shadowrun_ghidra/runner_roster_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

std::uint16_t read_big_endian_word(
    const shadowrun::ghidra_conversion::LiveRunnerRecord& record,
    const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(record.bytes[offset]) << 8U) |
        record.bytes[offset + 1U]);
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    CompactRunnerRecord default_source{};
    CompactRunnerRecord primary_source{};
    CompactRunnerRecord secondary_source{};
    CompactRunnerRecord tertiary_source{};
    for (std::size_t index = 0; index < default_source.bytes.size(); ++index) {
        default_source.bytes[index] = static_cast<std::uint8_t>(index);
        primary_source.bytes[index] = static_cast<std::uint8_t>(0x40U + index);
        secondary_source.bytes[index] =
            static_cast<std::uint8_t>(0x80U + index);
        tertiary_source.bytes[index] =
            static_cast<std::uint8_t>(0xC0U + index);
    }
    const std::vector<CompactRunnerRecord> compact_records{
        default_source, primary_source, secondary_source, tertiary_source};

    LiveRunnerRecord projected{};
    projected.bytes.fill(0xEEU);
    project_compact_runner_record(primary_source, projected);
    if (!expect(projected.bytes[0x0EU] == 0x40U &&
                projected.bytes[0x0FU] == 0x41U &&
                projected.bytes[0x2CU] == 0x42U &&
                projected.bytes[0x4CU] == 0x44U &&
                projected.bytes[0x4FU] == 0x47U &&
                projected.bytes[0x56U] == 0x48U &&
                projected.bytes[0x57U] == 0x49U &&
                projected.bytes[0x66U] == 0x58U &&
                projected.bytes[0x75U] == 0x67U &&
                projected.bytes[0x76U] == 0x68U &&
                projected.bytes[0x8DU] == 0x7FU &&
                projected.bytes[0x8EU] == 0x80U &&
                projected.bytes[0x9AU] == 0x81U &&
                projected.bytes[0x9CU] == 0x83U &&
                projected.bytes[0x90U] == 0x84U &&
                projected.bytes[0x91U] == 0x85U &&
                projected.bytes[0xA0U] == 0x86U &&
                projected.bytes[0xA1U] == 0x87U &&
                projected.bytes[0x8FU] == 0x88U &&
                projected.bytes[0x9DU] == 0x89U &&
                projected.bytes[0x10U] == 0xEEU)) {
        return 1;
    }

    RunnerRosterState state{};
    state.primary_marker = 1U;
    state.secondary_marker = 2U;
    const auto population =
        refresh_runner_record_population(state, compact_records);
    if (!expect(population.action == RunnerRecordPopulationAction::completed &&
                population.last_destination ==
                    RunnerRecordDestination::secondary_record &&
                state.default_record.bytes[0x00U] == 0x00U &&
                state.primary_record.bytes[0x66U] == 0x58U &&
                state.secondary_record.bytes[0x66U] == 0x98U)) {
        return 2;
    }

    std::vector<RunnerRosterTemplate> templates(3U);
    templates[0].record.fill(0x10U);
    templates[1].record.fill(0x20U);
    templates[2].record.fill(0x30U);
    state = RunnerRosterState{};
    const auto primary = initialize_primary_roster_template(
        1U, templates, 0xA1U, state, compact_records);
    if (!expect(primary.action == RunnerRosterTemplateAction::loaded &&
                state.primary_marker == 2U &&
                state.primary_record.bytes[0x00U] == 0x20U &&
                state.primary_record.bytes[0x10U] == 0x20U &&
                read_big_endian_word(state.primary_record, 0x3EU) == 1U &&
                read_big_endian_word(state.primary_record, 0x20U) == 0x19U &&
                read_big_endian_word(state.primary_record, 0x26U) == 0x320U &&
                state.primary_record.bytes[0x8FU] == 0xA1U &&
                state.default_record.bytes[0x66U] == 0x18U)) {
        return 3;
    }

    const auto duplicate = initialize_secondary_roster_template(
        1U, templates, 0xB2U, state, compact_records);
    if (!expect(duplicate.action ==
                    RunnerRosterTemplateAction::skipped_duplicate_primary &&
                state.secondary_marker == 0xFFU &&
                state.primary_record.bytes[0x8FU] == 0xB2U &&
                state.secondary_record.bytes[0x00U] == 0x00U)) {
        return 4;
    }

    const auto secondary = initialize_secondary_roster_template(
        2U, templates, 0xC3U, state, compact_records);
    if (!expect(secondary.action == RunnerRosterTemplateAction::loaded &&
                state.secondary_marker == 3U &&
                state.secondary_record.bytes[0x10U] == 0x30U &&
                read_big_endian_word(state.secondary_record, 0x3EU) == 2U &&
                read_big_endian_word(state.secondary_record, 0x20U) == 0x31U &&
                read_big_endian_word(state.secondary_record, 0x26U) == 0x620U &&
                state.secondary_record.bytes[0x8FU] == 0xC3U)) {
        return 5;
    }

    if (!expect(initialize_primary_roster_template(
                    3U, templates, 0U, state, compact_records)
                    .action ==
                RunnerRosterTemplateAction::invalid_template_index &&
                initialize_secondary_roster_template(
                    0x00FFU, templates, 0U, state, compact_records)
                    .action ==
                    RunnerRosterTemplateAction::invalid_template_index)) {
        return 6;
    }

    return 0;
}
