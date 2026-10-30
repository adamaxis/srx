#include "shadowrun_ghidra/field/stoker_motel_debt_payment_conversion.hpp"

#include "shadowrun_ghidra/session/runner_status_conversion.hpp"

namespace shadowrun::ghidra_conversion {

StokerMotelDebtPaymentOutcome resolve_stoker_motel_debt_payment(
    StokerMotelDebtPaymentState& state) {
    constexpr std::uint32_t kDebt = 0xFAU;
    constexpr std::uint32_t kStokerPaymentCredit = 0x1F4U;

    StokerMotelDebtPaymentOutcome outcome{};
    outcome.remaining_funds = state.funds;

    // 0x213FA compares the balance with 250 and branches directly to 0x213E2
    // when the signed result is negative. Gameplay balances are nonnegative
    // host values, so this is the same boundary without a source register.
    if (state.funds < kDebt) {
        const auto referral = resolve_stoker_motel_followup_command(
            StokerMotelFollowupCommand::jump_house_referral,
            state.followup_state);
        outcome.result = StokerMotelDebtPaymentResult::insufficient_funds;
        outcome.prompt = StokerMotelDebtPaymentPrompt::jump_house_nuyen_referral;
        outcome.remaining_funds = state.funds;
        (void)referral;
        return outcome;
    }

    // Paying the 250 nuyen debt grants 500 nuyen credit, for a net gain of
    // 250. The dialogue graph records the named motel_bill_paid story fact
    // after this service returns successfully.
    state.funds -= kDebt;
    state.funds += kStokerPaymentCredit;

    // 0x59C34 awards one Karma point to each of the three runners.
    award_party_karma(state.party_karma, 1U);

    // 0x21428 sets bit zero of the runner feature/status byte at FF018F.
    state.runner_feature_flags = static_cast<std::uint8_t>(
        state.runner_feature_flags | 0x01U);

    // 0x21436..0x2144A clears FFFC30 bits 0 and 1, then sets bits 2, 3,
    // and 4. Keep the mask exact so command 0x30 and Notebook Tips observe
    // the source's post-payment story state.
    state.followup_state.story_flags =
        (state.followup_state.story_flags & ~std::uint32_t{0x03U}) |
        std::uint32_t{0x1CU};

    outcome.result = StokerMotelDebtPaymentResult::debt_paid;
    outcome.prompt = StokerMotelDebtPaymentPrompt::michael_package_contents;
    outcome.remaining_funds = state.funds;
    outcome.michael_package_sequence_follows = true;
    return outcome;
}

} // namespace shadowrun::ghidra_conversion
