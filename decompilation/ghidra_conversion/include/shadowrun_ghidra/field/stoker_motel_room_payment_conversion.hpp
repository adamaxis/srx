#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class StokerMotelRoomPaymentPrompt {
    get_patched,
    bring_your_own_towels,
};

enum class StokerMotelRoomPaymentResult {
    insufficient_funds,
    room_paid,
};

struct StokerMotelRoomPaymentState {
    // Source FFFBFE: the player's nuyen balance.
    std::uint32_t funds = 0;

    // Source FFF10D: the scene-specific room charge installed by 0x1F65A.
    // Stoker's Coffin Motel supplies 0x14 (20 nuyen) before this command.
    std::uint8_t room_cost = 0x14U;
};

struct StokerMotelRoomPaymentOutcome {
    StokerMotelRoomPaymentResult result =
        StokerMotelRoomPaymentResult::insufficient_funds;
    StokerMotelRoomPaymentPrompt prompt =
        StokerMotelRoomPaymentPrompt::get_patched;
    std::uint32_t remaining_funds = 0;

    // 0x21458 writes FF to FFF10D after a successful payment.  The caller at
    // 0x1F65A tests that published marker after the shared prompt service and
    // enters the source Hotel Options menu when it is negative.
    bool hotel_options_menu_follows = false;
};

// ROM 0x21458, dispatch-table command 0x06 from the Stoker motel room offer.
// The source subtracts the byte-sized charge from the 32-bit funds balance;
// a negative result returns the repair prompt without changing funds, while a
// nonnegative result commits the balance and publishes the marker that causes
// the field-exit caller to enter Hotel Options after the prompt.
StokerMotelRoomPaymentOutcome resolve_stoker_motel_room_payment(
    StokerMotelRoomPaymentState& state);

} // namespace shadowrun::ghidra_conversion
