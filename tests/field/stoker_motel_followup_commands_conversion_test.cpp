#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    StokerMotelFollowupState state{};
    state.story_flags = 0x80000000U;
    state.referral_flags = 0x80U;

    const auto debt = resolve_stoker_motel_followup_command(
        StokerMotelFollowupCommand::debt_collection, state);
    assert(debt.prompt == StokerMotelFollowupPrompt::michael_debt_question);
    assert(state.story_flags == 0x80000002U);
    assert(state.referral_flags == 0x80U);

    const auto referral = resolve_stoker_motel_followup_command(
        StokerMotelFollowupCommand::jump_house_referral, state);
    assert(referral.prompt ==
           StokerMotelFollowupPrompt::jump_house_nuyen_referral);
    assert(state.story_flags == 0x80000002U);
    assert(state.referral_flags == 0x81U);

    // BSET is idempotent on already-published source flags.
    resolve_stoker_motel_followup_command(
        StokerMotelFollowupCommand::debt_collection, state);
    resolve_stoker_motel_followup_command(
        StokerMotelFollowupCommand::jump_house_referral, state);
    assert(state.story_flags == 0x80000002U);
    assert(state.referral_flags == 0x81U);
    return 0;
}
