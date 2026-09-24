#include "shadowrun_ghidra/runner_roster_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion {
namespace {

void copy_bytes(const CompactRunnerRecord& source,
                const std::size_t source_offset,
                LiveRunnerRecord& destination,
                const std::size_t destination_offset,
                const std::size_t count) {
    for (std::size_t index = 0; index < count; ++index) {
        destination.bytes[destination_offset + index] =
            source.bytes[source_offset + index];
    }
}

void write_big_endian_word(LiveRunnerRecord& record,
                           const std::size_t offset,
                           const std::uint16_t value) {
    record.bytes[offset] = static_cast<std::uint8_t>(value >> 8U);
    record.bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void apply_roster_template_words(LiveRunnerRecord& record,
                                 const std::uint16_t record_type,
                                 const std::uint16_t profile_word,
                                 const std::uint16_t template_metric) {
    write_big_endian_word(record, 0x3EU, record_type);
    write_big_endian_word(record, 0x20U, profile_word);
    write_big_endian_word(record, 0x26U, template_metric);
}

bool marker_is_active(const std::uint8_t marker) {
    return static_cast<std::int8_t>(marker) >= 0;
}

LiveRunnerRecord& destination_record(RunnerRosterState& state,
                                     const RunnerRecordDestination destination) {
    switch (destination) {
    case RunnerRecordDestination::primary_record:
        return state.primary_record;
    case RunnerRecordDestination::secondary_record:
        return state.secondary_record;
    case RunnerRecordDestination::default_record:
    default:
        return state.default_record;
    }
}

void apply_population_byte(RunnerRosterState& state,
                           const RunnerRecordPopulationResult& population,
                           const std::uint8_t value) {
    destination_record(state, population.last_destination).bytes[0x8FU] =
        value;
}

RunnerRosterTemplateResult initialize_roster_template(
    const std::uint16_t selected_index,
    const std::vector<RunnerRosterTemplate>& templates,
    const std::uint8_t population_byte,
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record,
    LiveRunnerRecord& destination,
    const std::uint16_t record_type,
    const std::uint16_t profile_word,
    const std::uint16_t template_metric) {
    if (selected_index >= templates.size() || selected_index > 0x00FEU) {
        return {};
    }

    destination.bytes = templates[selected_index].record;
    apply_roster_template_words(destination, record_type, profile_word,
                                template_metric);

    const auto population = refresh_runner_record_population(
        state, compact_records, project_record);
    apply_population_byte(state, population, population_byte);
    return {RunnerRosterTemplateAction::loaded, population};
}

} // namespace

void project_compact_runner_record(const CompactRunnerRecord& source,
                                   LiveRunnerRecord& destination) {
    // These ranges are the destination order in 0x598B0.  The source bytes
    // are copied directly so 68k word/longword byte order is preserved in the
    // extracted record rather than reinterpreted as a host integer.
    copy_bytes(source, 0x00U, destination, 0x0EU, 2U);
    copy_bytes(source, 0x02U, destination, 0x2CU, 2U);
    copy_bytes(source, 0x04U, destination, 0x4CU, 4U);
    copy_bytes(source, 0x08U, destination, 0x56U, 2U);
    copy_bytes(source, 0x0AU, destination, 0x58U, 0x0EU);
    copy_bytes(source, 0x18U, destination, 0x66U, 0x10U);
    copy_bytes(source, 0x28U, destination, 0x76U, 0x18U);
    copy_bytes(source, 0x40U, destination, 0x8EU, 1U);
    copy_bytes(source, 0x41U, destination, 0x9AU, 3U);
    copy_bytes(source, 0x44U, destination, 0x90U, 2U);
    copy_bytes(source, 0x46U, destination, 0xA0U, 2U);
    copy_bytes(source, 0x48U, destination, 0x8FU, 1U);
    copy_bytes(source, 0x49U, destination, 0x9DU, 1U);
}

void project_live_runner_record_to_compact(
    const LiveRunnerRecord& source, CompactRunnerRecord& destination) {
    // This is the source order in 0x597EE, expressed as the inverse of the
    // 0x598B0 projection. Unmapped live bytes are intentionally ignored.
    for (std::size_t index = 0; index < 2U; ++index) {
        destination.bytes[index] = source.bytes[0x0EU + index];
        destination.bytes[2U + index] = source.bytes[0x2CU + index];
    }
    for (std::size_t index = 0; index < 4U; ++index) {
        destination.bytes[4U + index] = source.bytes[0x4CU + index];
    }
    for (std::size_t index = 0; index < 2U; ++index) {
        destination.bytes[8U + index] = source.bytes[0x56U + index];
    }
    for (std::size_t index = 0; index < 0x0EU; ++index) {
        destination.bytes[0x0AU + index] = source.bytes[0x58U + index];
    }
    for (std::size_t index = 0; index < 0x10U; ++index) {
        destination.bytes[0x18U + index] = source.bytes[0x66U + index];
    }
    for (std::size_t index = 0; index < 0x18U; ++index) {
        destination.bytes[0x28U + index] = source.bytes[0x76U + index];
    }
    destination.bytes[0x40U] = source.bytes[0x8EU];
    for (std::size_t index = 0; index < 3U; ++index) {
        destination.bytes[0x41U + index] = source.bytes[0x9AU + index];
    }
    for (std::size_t index = 0; index < 2U; ++index) {
        destination.bytes[0x44U + index] = source.bytes[0x90U + index];
        destination.bytes[0x46U + index] = source.bytes[0xA0U + index];
    }
    destination.bytes[0x48U] = source.bytes[0x8FU];
    destination.bytes[0x49U] = source.bytes[0x9DU];
}

RunnerRecordPopulationResult refresh_runner_record_population(
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record) {
    if (compact_records.empty()) {
        return {};
    }

    const auto project = project_record
                             ? project_record
                             : RunnerRecordProjectionBoundary{
                                   project_compact_runner_record};
    project(compact_records[0], state.default_record);
    RunnerRecordPopulationResult result{
        RunnerRecordPopulationAction::completed,
        RunnerRecordDestination::default_record};

    const auto project_selected = [&](const std::uint8_t marker,
                                      LiveRunnerRecord& destination,
                                      const RunnerRecordDestination slot) {
        if (!marker_is_active(marker)) {
            return true;
        }
        if (static_cast<std::size_t>(marker) >= compact_records.size()) {
            result.action = RunnerRecordPopulationAction::source_record_unavailable;
            return false;
        }
        project(compact_records[marker], destination);
        result.last_destination = slot;
        return true;
    };

    if (!project_selected(state.primary_marker, state.primary_record,
                          RunnerRecordDestination::primary_record)) {
        return result;
    }
    project_selected(state.secondary_marker, state.secondary_record,
                     RunnerRecordDestination::secondary_record);
    return result;
}

RunnerRecordSaveResult serialize_runner_record_population(
    const RunnerRosterState& state,
    std::vector<CompactRunnerRecord>& compact_records) {
    if (compact_records.empty()) {
        return {};
    }

    project_live_runner_record_to_compact(state.default_record,
                                          compact_records[0]);
    RunnerRecordSaveResult result{
        RunnerRecordSaveAction::completed,
        RunnerRecordDestination::default_record};

    const auto project_selected = [&](const std::uint8_t marker,
                                      const LiveRunnerRecord& source,
                                      const RunnerRecordDestination slot) {
        if (!marker_is_active(marker)) {
            return true;
        }
        if (static_cast<std::size_t>(marker) >= compact_records.size()) {
            result.action = RunnerRecordSaveAction::destination_record_unavailable;
            return false;
        }
        project_live_runner_record_to_compact(source, compact_records[marker]);
        result.last_destination = slot;
        return true;
    };

    if (!project_selected(state.primary_marker, state.primary_record,
                          RunnerRecordDestination::primary_record)) {
        return result;
    }
    project_selected(state.secondary_marker, state.secondary_record,
                     RunnerRecordDestination::secondary_record);
    return result;
}

std::optional<SaveRosterBuffer> initialize_save_roster_buffer(
    const std::array<std::uint8_t, 0x14U>& header,
    const std::array<RunnerRosterTemplate, kSaveRosterTemplateCount>&
        templates) {
    SaveRosterBuffer buffer{};
    buffer.header = header;
    buffer.compact_records.resize(kSaveRosterTemplateCount);
    for (std::size_t index = 0; index < templates.size(); ++index) {
        LiveRunnerRecord source{};
        source.bytes = templates[index].record;
        project_live_runner_record_to_compact(source,
                                              buffer.compact_records[index]);
    }
    return buffer;
}

RunnerRosterTemplateResult initialize_primary_roster_template(
    const std::uint16_t selected_index,
    const std::vector<RunnerRosterTemplate>& templates,
    const std::uint8_t population_byte,
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record) {
    if (selected_index >= templates.size() || selected_index > 0x00FEU) {
        return {};
    }
    state.primary_marker = static_cast<std::uint8_t>(selected_index + 1U);
    return initialize_roster_template(
        selected_index, templates, population_byte, state, compact_records,
        project_record, state.primary_record, 1U, 0x19U, 0x320U);
}

RunnerRosterTemplateResult initialize_secondary_roster_template(
    const std::uint16_t selected_index,
    const std::vector<RunnerRosterTemplate>& templates,
    const std::uint8_t population_byte,
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record) {
    if (selected_index >= templates.size() || selected_index > 0x00FEU) {
        return {};
    }

    const auto selected_byte = static_cast<std::uint8_t>(selected_index);
    const auto primary_index =
        static_cast<std::uint8_t>(state.primary_marker - 1U);
    if (selected_byte == primary_index) {
        const auto population = refresh_runner_record_population(
            state, compact_records, project_record);
        apply_population_byte(state, population, population_byte);
        return {RunnerRosterTemplateAction::skipped_duplicate_primary,
                population};
    }

    state.secondary_marker = static_cast<std::uint8_t>(selected_index + 1U);
    return initialize_roster_template(
        selected_index, templates, population_byte, state, compact_records,
        project_record, state.secondary_record, 2U, 0x31U, 0x620U);
}

} // namespace shadowrun::ghidra_conversion
