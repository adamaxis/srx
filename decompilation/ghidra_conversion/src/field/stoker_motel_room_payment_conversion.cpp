#include "shadowrun_ghidra/field/stoker_motel_room_payment_conversion.hpp"

namespace shadowrun::ghidra_conversion {

StokerMotelRoomPaymentOutcome resolve_stoker_motel_room_payment(
    StokerMotelRoomPaymentState& state) {
    StokerMotelRoomPaymentOutcome outcome{};
    outcome.remaining_funds = state.funds;

    // 0x21458 loads the byte marker into a cleared D1 before subtracting it
    // from D0.  Since the source charge is an unsigned byte and funds is a
    // nonnegative 32-bit balance, the source BPL boundary is exactly this
    // host comparison; no wrapped host arithmetic is needed.
    if (state.funds < state.room_cost) {
        outcome.result = StokerMotelRoomPaymentResult::insufficient_funds;
        outcome.prompt = StokerMotelRoomPaymentPrompt::get_patched;
        return outcome;
    }

    state.funds -= state.room_cost;
    outcome.result = StokerMotelRoomPaymentResult::room_paid;
    outcome.prompt = StokerMotelRoomPaymentPrompt::bring_your_own_towels;
    outcome.remaining_funds = state.funds;
    outcome.hotel_options_menu_follows = true;
    return outcome;
}

} // namespace shadowrun::ghidra_conversion
