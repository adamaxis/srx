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

void award_party_karma(std::array<std::uint8_t, 3>& party_karma,
                       const std::uint8_t amount) {
    for (auto& karma : party_karma) {
        karma = static_cast<std::uint8_t>(karma + amount);
    }
}

} // namespace shadowrun::ghidra_conversion
