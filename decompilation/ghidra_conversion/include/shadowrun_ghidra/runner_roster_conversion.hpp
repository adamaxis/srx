#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kCompactRunnerRecordSize = 0x4AU;
constexpr std::size_t kLiveRunnerRecordSize = 0x100U;

// The roster source records are 0x4A-byte packed records.  Their complete
// meaning is not established yet, so the conversion keeps their bytes intact
// while giving the destination record its own typed container.
struct CompactRunnerRecord {
    std::array<std::uint8_t, kCompactRunnerRecordSize> bytes{};
};

struct LiveRunnerRecord {
    std::array<std::uint8_t, kLiveRunnerRecordSize> bytes{};
};

// ROM 0x598B0: project one packed roster record into the live 0x100-byte
// record.  The explicit byte ranges below preserve the source field mapping;
// opaque fields remain opaque until another consumer proves their meaning.
void project_compact_runner_record(const CompactRunnerRecord& source,
                                   LiveRunnerRecord& destination);

enum class RunnerRecordDestination {
    default_record,
    primary_record,
    secondary_record,
};

enum class RunnerRecordPopulationAction {
    completed,
    no_source_records,
    source_record_unavailable,
};

struct RunnerRecordPopulationResult {
    RunnerRecordPopulationAction action =
        RunnerRecordPopulationAction::no_source_records;
    RunnerRecordDestination last_destination =
        RunnerRecordDestination::default_record;
};

struct RunnerRosterState {
    // The source uses signed bytes with 0xFF meaning that a slot is absent.
    // Keeping the raw marker makes the source sentinel explicit without
    // making the native record model depend on a RAM address.
    std::uint8_t primary_marker = 0xFFU;
    std::uint8_t secondary_marker = 0xFFU;
    LiveRunnerRecord default_record{};
    LiveRunnerRecord primary_record{};
    LiveRunnerRecord secondary_record{};
};

using RunnerRecordProjectionBoundary =
    std::function<void(const CompactRunnerRecord&, LiveRunnerRecord&)>;

// ROM 0x5966E: project the default packed record, then the selected primary
// and secondary packed records in that order.  The returned destination is
// the source A1 target that the caller subsequently modifies.
RunnerRecordPopulationResult refresh_runner_record_population(
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record = {});

struct RunnerRosterTemplate {
    std::array<std::uint8_t, kLiveRunnerRecordSize> record{};
};

enum class RunnerRosterTemplateAction {
    loaded,
    skipped_duplicate_primary,
    invalid_template_index,
};

struct RunnerRosterTemplateResult {
    RunnerRosterTemplateAction action =
        RunnerRosterTemplateAction::invalid_template_index;
    RunnerRecordPopulationResult population{};
};

// ROM 0x42F2: load one recruited-roster template into the primary slot,
// install the source metadata words at +0x3E/+0x20/+0x26, refresh the compact
// record projections, and apply the caller's byte at +0x8F to the final
// populated destination.
RunnerRosterTemplateResult initialize_primary_roster_template(
    std::uint16_t selected_index,
    const std::vector<RunnerRosterTemplate>& templates,
    std::uint8_t population_byte,
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record = {});

// ROM 0x431C: do the same for the secondary slot, except that selecting the
// same low-byte index as the primary slot skips the template copy.  The
// common population path still runs in that branch, matching the source call
// to 0x5966E.
RunnerRosterTemplateResult initialize_secondary_roster_template(
    std::uint16_t selected_index,
    const std::vector<RunnerRosterTemplate>& templates,
    std::uint8_t population_byte,
    RunnerRosterState& state,
    const std::vector<CompactRunnerRecord>& compact_records,
    const RunnerRecordProjectionBoundary& project_record = {});

} // namespace shadowrun::ghidra_conversion
