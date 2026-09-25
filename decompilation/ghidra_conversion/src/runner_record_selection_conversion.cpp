#include "shadowrun_ghidra/runner_record_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<RunnerRecordTableSelection> select_primary_runner_record_table(
    const std::vector<RunnerRecordTableEntry>& table,
    const std::uint8_t table_index) {
    if (table_index >= table.size() || table[table_index].primary_alias.empty()) {
        return std::nullopt;
    }
    return RunnerRecordTableSelection{
        table_index, RunnerRecordBank::primary, table[table_index].primary_alias};
}

std::optional<RunnerRecordTableSelection> select_secondary_runner_record_table(
    const std::vector<RunnerRecordTableEntry>& table,
    const std::uint8_t table_index) {
    if (table_index >= table.size() || table[table_index].secondary_alias.empty()) {
        return std::nullopt;
    }
    return RunnerRecordTableSelection{
        table_index, RunnerRecordBank::secondary, table[table_index].secondary_alias};
}

} // namespace shadowrun::ghidra_conversion
