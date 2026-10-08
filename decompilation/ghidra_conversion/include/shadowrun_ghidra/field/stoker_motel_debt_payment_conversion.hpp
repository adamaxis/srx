#pragma once

#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class StokerMotelDebtPaymentPrompt {
    jump_house_nuyen_referral,
    michael_package_contents,
};

enum class StokerMotelDebtPaymentResult {
    insufficient_funds,
    debt_paid,
};

struct StokerMotelDebtPaymentState {
    // Source FFFBFE: the player's 32-bit nuyen balance.
    std::uint32_t funds = 0;

    // Source FFFC1E: the persistent world/effect flag word updated through
    // 0x2196C with the pending effect marker below.
    std::uint32_t world_flags = 0;

    // Source FFDD64: 0x2196C reloads this marker instead of using the
    // prompt command value. The upstream indexed field-exit service owns its
    // value, so the host boundary keeps it explicit rather than guessing a
    // Stoker-specific bit.
    std::uint32_t effect_marker = 0;

    // Source FF018F: the runner feature/status byte. The paid branch sets bit
    // zero, the cyberdeck-present flag used by the Matrix entry gate.
    std::uint8_t runner_feature_flags = 0;

    // Source FFFC30 and FFFC27. The insufficient-funds branch enters the
    // already converted Jump House referral command at 0x213E2.
    StokerMotelFollowupState followup_state{};

    // Source FF018D, FF028D, and FF038D. 0x59C34 adds one to all three
    // source bytes with byte-sized wraparound after the debt is paid.
    std::array<std::uint8_t, 3> runner_source_bytes{};
};

struct StokerMotelDebtPaymentOutcome {
    StokerMotelDebtPaymentResult result =
        StokerMotelDebtPaymentResult::insufficient_funds;
    StokerMotelDebtPaymentPrompt prompt =
        StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral;
    std::uint32_t remaining_funds = 0;

    // The successful source record at F487C begins with the recovered text
    // “The creds roll off your stick...” and continues into Michael's package
    // contents and letter sequence.
    bool michael_package_sequence_follows = false;
};

// ROM 0x213FA, prompt-dispatch command 0x05 from the Stoker debt question.
// The source requires 0xFA nuyen. Failure enters 0x213E2; success performs
// the source progress mutations and returns the Michael package sequence at
// 0xF487C. No ROM addresses or emulated registers cross this host boundary.
StokerMotelDebtPaymentOutcome resolve_stoker_motel_debt_payment(
    StokerMotelDebtPaymentState& state);

} // namespace shadowrun::ghidra_conversion
