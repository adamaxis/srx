#include "shadowrun_ghidra/queued_transfer_flush_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void flush(QueuedTransferState& state,
           const QueuedTransferWriter& write_wrapped_block) {
    if (write_wrapped_block) {
        for (std::size_t index = 0; index < state.queued_count; ++index) {
            write_wrapped_block();
        }
    }
    state.queued_count = 0U;
}

} // namespace

void flush_primary_queued_transfers(
    QueuedTransferState& state,
    const QueuedTransferWriter& write_wrapped_block) {
    flush(state, write_wrapped_block);
}

void flush_secondary_queued_transfers(
    QueuedTransferState& state,
    const QueuedTransferWriter& write_wrapped_block) {
    flush(state, write_wrapped_block);
}

} // namespace shadowrun::ghidra_conversion
