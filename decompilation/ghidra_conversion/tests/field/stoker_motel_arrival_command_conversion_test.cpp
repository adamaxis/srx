#include "shadowrun_ghidra/field/stoker_motel_arrival_command_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::uint8_t observed_bound = 0;
    bool effect_called = false;
    StokerMotelArrivalCommandCallbacks callbacks{
        [&](const std::uint8_t bound) {
            observed_bound = bound;
            return std::optional<std::uint8_t>{79U};
        },
        [&] {
            effect_called = true;
            return true;
        }};

    assert(resolve_stoker_motel_arrival_command(callbacks) ==
           StokerMotelArrivalCommandResult::manager_greeting);
    assert(observed_bound == 100U);
    assert(!effect_called);

    callbacks.sample_random_bound = [](const std::uint8_t) {
        return std::optional<std::uint8_t>{80U};
    };
    assert(resolve_stoker_motel_arrival_command(callbacks) ==
           StokerMotelArrivalCommandResult::street_ambush);
    assert(effect_called);

    callbacks.forward_ambush_effect_sequence = [] { return false; };
    assert(resolve_stoker_motel_arrival_command(callbacks) ==
           StokerMotelArrivalCommandResult::manager_greeting);

    callbacks.sample_random_bound = {};
    assert(resolve_stoker_motel_arrival_command(callbacks) ==
           StokerMotelArrivalCommandResult::unresolved_callback);
    return 0;
}
