#include "shadowrun_ghidra/runner_record_selection_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::vector<RunnerRecordTableEntry> table{
        {"records/scene-0-primary.bin", "records/scene-0-secondary.bin"},
        {"records/scene-1-primary.bin", ""},
    };
    const auto primary = select_primary_runner_record_table(table, 1);
    const auto secondary = select_secondary_runner_record_table(table, 0);
    if (!expect(primary && primary->table_index == 1U &&
                primary->bank == RunnerRecordBank::primary &&
                primary->alias == "records/scene-1-primary.bin" && secondary &&
                secondary->bank == RunnerRecordBank::secondary &&
                secondary->alias == "records/scene-0-secondary.bin" &&
                !select_secondary_runner_record_table(table, 1) &&
                !select_primary_runner_record_table(table, 3))) {
        return 1;
    }
    return 0;
}
