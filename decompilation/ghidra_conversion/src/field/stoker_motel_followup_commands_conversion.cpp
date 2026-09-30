#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"

namespace shadowrun::ghidra_conversion {

StokerMotelFollowupOutcome resolve_stoker_motel_followup_command(
    const StokerMotelFollowupCommand command,
    StokerMotelFollowupState& state) {
    switch (command) {
    case StokerMotelFollowupCommand::debt_collection:
        // 0x213CA: BSET #1 in the long FFFC30 story word, then return the
        // F4E8C descriptor beginning "So, you're Michael's brother, eh?".
        state.story_flags |= (std::uint32_t{1U} << 1U);
        return {StokerMotelFollowupPrompt::michael_debt_question};

    case StokerMotelFollowupCommand::jump_house_referral:
        // 0x213E2: BSET #0 in the byte FFFC27 referral field, then return the
        // F5AB0 descriptor beginning "Hey, if you need nuyen...".
        state.referral_flags = static_cast<std::uint8_t>(
            state.referral_flags | std::uint8_t{1U});
        return {StokerMotelFollowupPrompt::jump_house_nuyen_referral};
    }

    // The enum is closed and both source dispatch entries are represented;
    // retaining a default keeps this host boundary total if it is extended.
    return {StokerMotelFollowupPrompt::michael_debt_question};
}

} // namespace shadowrun::ghidra_conversion
