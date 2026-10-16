#include "shadowrun_ghidra/field/stoker_motel_arrival_command_conversion.hpp"

namespace shadowrun::ghidra_conversion {

StokerMotelArrivalCommandResult resolve_stoker_motel_arrival_command(
    const StokerMotelArrivalCommandCallbacks& callbacks) {
    if (!callbacks.sample_random_bound) {
        return StokerMotelArrivalCommandResult::unresolved_callback;
    }

    const auto random_value = callbacks.sample_random_bound(100U);
    if (!random_value.has_value()) {
        return StokerMotelArrivalCommandResult::unresolved_callback;
    }

    // 0x249C4 compares the sampled value with 0x50. The ordinary 0..79
    // range skips the effect sequence and returns the manager record.
    if (*random_value < 0x50U) {
        return StokerMotelArrivalCommandResult::manager_greeting;
    }

    if (!callbacks.forward_ambush_effect_sequence) {
        return StokerMotelArrivalCommandResult::unresolved_callback;
    }

    return callbacks.forward_ambush_effect_sequence()
               ? StokerMotelArrivalCommandResult::street_ambush
               : StokerMotelArrivalCommandResult::manager_greeting;
}

} // namespace shadowrun::ghidra_conversion
