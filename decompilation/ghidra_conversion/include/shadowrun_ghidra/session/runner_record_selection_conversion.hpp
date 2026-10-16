#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kRunnerRecordTableEntryCount = 16U;

enum class RunnerRecordBank {
    primary,
    secondary,
};

struct RunnerRecordTableEntry {
    std::string primary_alias;
    std::string secondary_alias;
};

using RunnerRecordTable =
    std::array<RunnerRecordTableEntry, kRunnerRecordTableEntryCount>;

struct RunnerRecordTableSelection {
    std::uint8_t table_index = 0;
    RunnerRecordBank bank = RunnerRecordBank::primary;
    std::string alias;
};

// ROM 0x53200 / 0x53216: select the primary or secondary compact
// runner-record bank for the current scene/resource id. The source table at
// 0x15DB48 contains exactly sixteen primary/secondary pointer pairs. The
// host receives a named extracted data file rather than a source pointer.
std::optional<RunnerRecordTableSelection> select_primary_runner_record_table(
    const RunnerRecordTable& table,
    std::uint8_t table_index);
std::optional<RunnerRecordTableSelection> select_secondary_runner_record_table(
    const RunnerRecordTable& table,
    std::uint8_t table_index);

} // namespace shadowrun::ghidra_conversion
