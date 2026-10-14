#include "shadowrun_ghidra/session/runner_status_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_runner_status_triplet(
    std::array<RunnerStatusRecord, 3>& records,
    const RunnerStatusPreparationBoundary& prepare_record,
    const RunnerStatusFinalizeBoundary& finalize_record) {
    for (std::size_t index = 0; index < records.size(); ++index) {
        auto& record = records[index];
        record.flags_at_0x10 = static_cast<std::uint8_t>(
            record.flags_at_0x10 & 0xBFU & 0xEFU);

        if (prepare_record) {
            prepare_record(index, record, 0x0AU);
        }
        if (finalize_record) {
            finalize_record(index, record);
        }
    }

    for (auto& record : records) {
        record.timer_at_0x4c = 0x0AU;
        record.timer_at_0x9c = 0x0AU;
    }
}

void adjust_runner_source_byte_triplet(
    std::array<std::uint8_t, 3>& source_bytes, const std::uint8_t delta) {
    for (auto& source_byte : source_bytes) {
        source_byte = static_cast<std::uint8_t>(source_byte + delta);
    }
}

} // namespace shadowrun::ghidra_conversion
