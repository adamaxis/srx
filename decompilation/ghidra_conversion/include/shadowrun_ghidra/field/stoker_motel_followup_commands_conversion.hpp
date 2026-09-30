#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class StokerMotelFollowupCommand {
    debt_collection,
    jump_house_referral,
};

enum class StokerMotelFollowupPrompt {
    michael_debt_question,
    jump_house_nuyen_referral,
};

struct StokerMotelFollowupState {
    // Source FFFC30: the long story-flag word updated by command 0x30.
    std::uint32_t story_flags = 0;

    // Source FFFC27: the byte flag field updated by command 0x31.
    std::uint8_t referral_flags = 0;
};

struct StokerMotelFollowupOutcome {
    StokerMotelFollowupPrompt prompt =
        StokerMotelFollowupPrompt::michael_debt_question;
};

// ROM 0x213CA and 0x213E2, reached through prompt commands 0x30 and 0x31.
// These are the two named Stoker follow-up branches: each sets its source
// progress bit and returns the next prompt record instead of exposing an A0
// text-record pointer to the host runtime.
StokerMotelFollowupOutcome resolve_stoker_motel_followup_command(
    StokerMotelFollowupCommand command,
    StokerMotelFollowupState& state);

} // namespace shadowrun::ghidra_conversion
