#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

enum class StokerMotelArrivalCommandResult {
    unresolved_callback,
    manager_greeting,
    street_ambush,
};

struct StokerMotelArrivalCommandCallbacks {
    // ROM 0x0D3A is the source-bounded random sampler. The callback receives
    // the exact bound used by 0x249C4 (100) and returns a host random result
    // in [0, bound), or no value when the source boundary is unavailable.
    std::function<std::optional<std::uint8_t>(std::uint8_t bound)>
        sample_random_bound;

    // ROM 0x1E1FC forwards the rare ambush effect sequence selected through
    // A6=0x1D6E2A. True means the source returned a nonzero effect result and
    // therefore selects the ambush text; false returns to the greeting.
    std::function<bool()> forward_ambush_effect_sequence;
};

// ROM 0x249C4, dispatched by source prompt command 0xE9. Values below 80
// select the manager greeting record; values 80..99 attempt the ambush effect
// and select its text only when that effect reports success. The host exposes
// the resulting prompt identity instead of returning a source text pointer.
StokerMotelArrivalCommandResult resolve_stoker_motel_arrival_command(
    const StokerMotelArrivalCommandCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
