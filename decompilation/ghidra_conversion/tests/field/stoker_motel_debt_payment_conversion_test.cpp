#include "shadowrun_ghidra/field/stoker_motel_debt_payment_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    StokerMotelDebtPaymentState state{};
    state.funds = 250U;
    state.world_flags = 0x80000000U;
    state.effect_marker = 22U;
    state.runner_feature_flags = 0x01U;
    state.followup_state.story_flags = 0x80000000U;
    state.followup_state.referral_flags = 0x80U;
    state.runner_source_bytes = {0xFFU, 0x00U, 0x7FU};

    const auto paid = resolve_stoker_motel_debt_payment(state);
    assert(paid.result == StokerMotelDebtPaymentResult::debt_paid);
    assert(paid.prompt ==
           StokerMotelDebtPaymentPrompt::michael_package_contents);
    assert(paid.remaining_funds == 500U);
    assert(state.funds == 500U);
    assert(state.world_flags == 0x80400000U);
    assert(state.runner_feature_flags == 0x81U);
    assert(state.followup_state.story_flags == 0x8000001FU);
    assert(state.followup_state.referral_flags == 0x80U);
    assert(state.runner_source_bytes[0] == 0x00U);
    assert(state.runner_source_bytes[1] == 0x01U);
    assert(state.runner_source_bytes[2] == 0x80U);
    assert(paid.michael_package_sequence_follows);

    StokerMotelDebtPaymentState unpaid{};
    unpaid.funds = 249U;
    unpaid.effect_marker = 22U;
    unpaid.followup_state.referral_flags = 0x80U;
    const auto referral = resolve_stoker_motel_debt_payment(unpaid);
    assert(referral.result ==
           StokerMotelDebtPaymentResult::insufficient_funds);
    assert(referral.prompt ==
           StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral);
    assert(unpaid.funds == 249U);
    assert(unpaid.followup_state.referral_flags == 0x81U);
    assert(unpaid.world_flags == 0U);
    assert(unpaid.followup_state.story_flags == 0U);
    assert(!referral.michael_package_sequence_follows);

    // The source byte additions wrap independently, and paying twice is
    // idempotent for all bit flags while still advancing the source bytes.
    resolve_stoker_motel_debt_payment(state);
    assert(state.runner_feature_flags == 0x81U);
    assert(state.world_flags == 0x80400000U);
    assert(state.followup_state.story_flags == 0x8000001FU);
    assert(state.runner_source_bytes[0] == 0x01U);
    assert(state.runner_source_bytes[1] == 0x02U);
    assert(state.runner_source_bytes[2] == 0x81U);
    return 0;
}
