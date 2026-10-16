#include "shadowrun_ghidra/field/stoker_motel_room_payment_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    {
        StokerMotelRoomPaymentState state{};
        state.funds = 250U;
        state.room_cost = 0x14U;

        const auto outcome = resolve_stoker_motel_room_payment(state);
        assert(outcome.result == StokerMotelRoomPaymentResult::room_paid);
        assert(outcome.prompt ==
               StokerMotelRoomPaymentPrompt::bring_your_own_towels);
        assert(state.funds == 230U);
        assert(outcome.remaining_funds == 230U);
        assert(outcome.hotel_options_menu_follows);
    }

    {
        StokerMotelRoomPaymentState state{};
        state.funds = 19U;
        state.room_cost = 0x14U;

        const auto outcome = resolve_stoker_motel_room_payment(state);
        assert(outcome.result ==
               StokerMotelRoomPaymentResult::insufficient_funds);
        assert(outcome.prompt == StokerMotelRoomPaymentPrompt::get_patched);
        assert(state.funds == 19U);
        assert(outcome.remaining_funds == 19U);
        assert(!outcome.hotel_options_menu_follows);
    }

    {
        // The source accepts an exact balance and treats a zero charge as a
        // successful nonnegative subtraction.
        StokerMotelRoomPaymentState exact{};
        exact.funds = 0x14U;
        assert(resolve_stoker_motel_room_payment(exact).result ==
               StokerMotelRoomPaymentResult::room_paid);

        StokerMotelRoomPaymentState free_room{};
        free_room.funds = 0U;
        free_room.room_cost = 0U;
        assert(resolve_stoker_motel_room_payment(free_room).result ==
               StokerMotelRoomPaymentResult::room_paid);
    }

    return 0;
}
