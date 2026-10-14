#pragma once

#include <cstddef>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct QueuedTransferState {
    std::size_t queued_count = 0;
};

using QueuedTransferWriter = std::function<void()>;

// ROM 0x6148 / 0x61F4: emit one wrapped transfer per queued block and clear
// the corresponding source queue count.
void flush_primary_queued_transfers(
    QueuedTransferState& state,
    const QueuedTransferWriter& write_wrapped_block);
void flush_secondary_queued_transfers(
    QueuedTransferState& state,
    const QueuedTransferWriter& write_wrapped_block);

} // namespace shadowrun::ghidra_conversion
