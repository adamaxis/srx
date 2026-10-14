#include "shadowrun_ghidra/core/queued_transfer_flush_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    QueuedTransferState primary{3U};
    std::vector<int> writes;
    flush_primary_queued_transfers(primary, [&writes] { writes.push_back(1); });
    if (primary.queued_count != 0U || writes != std::vector<int>{1, 1, 1}) {
        return 1;
    }

    QueuedTransferState secondary{2U};
    flush_secondary_queued_transfers(secondary, [&writes] { writes.push_back(2); });
    if (secondary.queued_count != 0U ||
        writes != std::vector<int>{1, 1, 1, 2, 2}) {
        return 2;
    }

    QueuedTransferState without_writer{4U};
    flush_primary_queued_transfers(without_writer, {});
    if (without_writer.queued_count != 0U) {
        return 3;
    }

    return 0;
}
