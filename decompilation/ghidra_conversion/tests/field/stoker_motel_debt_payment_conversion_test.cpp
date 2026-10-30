#include "shadowrun_ghidra/field/stoker_motel_debt_payment_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    StokerMotelDebtPaymentState state{};
    state.funds = 250U;
    state.runner_feature_flags = 0x02U;
    state.followup_state.story_flags = 0x80000000U;
    state.followup_state.referral_flags = 0x80U;
    state.party_karma = {0xFFU, 0x00U, 0x7FU};

    const auto paid = resolve_stoker_motel_debt_payment(state);
    assert(paid.result == StokerMotelDebtPaymentResult::debt_paid);
    assert(paid.prompt ==
           StokerMotelDebtPaymentPrompt::michael_package_contents);
    assert(paid.remaining_funds == 500U);
    assert(state.funds == 500U);
    assert(state.runner_feature_flags == 0x03U);
    assert(state.followup_state.story_flags == 0x8000001CU);
    assert(state.followup_state.referral_flags == 0x80U);
    assert(state.party_karma[0] == 0x00U);
    assert(state.party_karma[1] == 0x01U);
    assert(state.party_karma[2] == 0x80U);
    assert(paid.michael_package_sequence_follows);

    StokerMotelDebtPaymentState unpaid{};
    unpaid.funds = 249U;
    unpaid.followup_state.referral_flags = 0x80U;
    const auto referral = resolve_stoker_motel_debt_payment(unpaid);
    assert(referral.result ==
           StokerMotelDebtPaymentResult::insufficient_funds);
    assert(referral.prompt ==
           StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral);
    assert(unpaid.funds == 249U);
    assert(unpaid.followup_state.referral_flags == 0x81U);
    assert(unpaid.followup_state.story_flags == 0U);
    assert(!referral.michael_package_sequence_follows);

    // Payment continues to award the same gameplay resources on repeat calls.
    resolve_stoker_motel_debt_payment(state);
    assert(state.runner_feature_flags == 0x03U);
    assert(state.followup_state.story_flags == 0x8000001CU);
    assert(state.party_karma[0] == 0x01U);
    assert(state.party_karma[1] == 0x02U);
    assert(state.party_karma[2] == 0x81U);
    return 0;
}
