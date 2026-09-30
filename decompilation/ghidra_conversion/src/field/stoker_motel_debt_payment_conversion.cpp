#include "shadowrun_ghidra/field/stoker_motel_debt_payment_conversion.hpp"

#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"

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

    // Source order is significant: subtract 250, mark the pending effect
    // through 0x2196C, then add 500 back. The net gameplay result is +250
    // nuyen, but retaining the two source operations makes the conversion
    // auditable.
    state.funds -= kDebt;
    // 0x2196C reloads FFDD64 and masks it to a 32-bit bit index; it does not
    // consume the command value 0x05 still present in D0.
    mark_effect_bit(
        state.world_flags, static_cast<std::int32_t>(state.effect_marker));
    state.funds += kStokerPaymentCredit;

    // 0x59C34 is already a typed shared conversion of the source triplet
    // update used by this command.
    adjust_runner_source_byte_triplet(state.runner_source_bytes, 1U);

    // 0x21428 encodes BSET #$FF against a memory byte. The 68000 memory-bit
    // operation uses the low three bits of the immediate, therefore this is
    // FF018F bit 7, not a fabricated 32-bit bit-31 field.
    state.runner_feature_flags = static_cast<std::uint8_t>(
        state.runner_feature_flags | 0x80U);

    // 0x21436..0x2144A sets FFFC30 bits 0 through 4 as one source story
    // progress update. Keep this in the shared typed follow-up state so the
    // earlier command 0x30 observes the same story word.
    state.followup_state.story_flags |= 0x1FU;

    outcome.result = StokerMotelDebtPaymentResult::debt_paid;
    outcome.prompt = StokerMotelDebtPaymentPrompt::michael_package_contents;
    outcome.remaining_funds = state.funds;
    outcome.michael_package_sequence_follows = true;
    return outcome;
}

} // namespace shadowrun::ghidra_conversion
