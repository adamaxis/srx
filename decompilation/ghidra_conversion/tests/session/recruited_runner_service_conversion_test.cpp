#include "shadowrun_ghidra/session/recruited_runner_service_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

std::uint16_t read_big_endian_word(
    const shadowrun::ghidra_conversion::LiveRunnerRecord& record,
    const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(record.bytes[offset]) << 8U) |
        record.bytes[offset + 1U]);
}

shadowrun::ghidra_conversion::RecruitedRunnerServiceState make_state(
    const std::size_t template_count = 10U) {
    using namespace shadowrun::ghidra_conversion;

    RecruitedRunnerServiceState state{};
    state.funds = 1000U;
    state.short_term_fee = 250U;
    state.permanent_fee = 700U;
    state.selected_runner_template_index = 1U;
    state.templates.resize(template_count);
    for (std::size_t index = 0; index < state.templates.size(); ++index) {
        state.templates[index].record.fill(
            static_cast<std::uint8_t>(0x20U + index));
    }
    state.compact_records.resize(13U);
    state.service_progress_counters.resize(10U);
    state.selected_entity.present = true;
    state.selected_entity.service_flags = 0x01U;
    state.selected_entity.source_position_y = 0x3456U;
    return state;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    auto offer_state = make_state();
    const auto prepared = prepare_recruited_runner_service_offer(
        offer_state,
        RecruitedRunnerServiceCostModifiers{10U, 20U, 30U});
    if (!expect(prepared.action ==
                    RecruitedRunnerServiceOfferPreparationAction::prepared &&
                prepared.progress_multiplier == 0x14U &&
                prepared.short_term_fee == 60U &&
                prepared.permanent_fee == 600U &&
                offer_state.short_term_fee == 60U &&
                offer_state.permanent_fee == 600U)) {
        return 1;
    }

    offer_state.selected_runner_template_index = 9U;
    const auto stark_offer = prepare_recruited_runner_service_offer(
        offer_state, RecruitedRunnerServiceCostModifiers{0U, 0U, 0U});
    if (!expect(stark_offer.action ==
                    RecruitedRunnerServiceOfferPreparationAction::prepared &&
                stark_offer.progress_multiplier == 0x14U &&
                stark_offer.short_term_fee == 0U &&
                stark_offer.permanent_fee == 0U)) {
        return 2;
    }

    offer_state.roster.primary_record.bytes[0x10U] = 0x01U;
    offer_state.roster.secondary_record.bytes[0x10U] = 0x01U;
    const auto full_offer = prepare_recruited_runner_service_offer(
        offer_state, RecruitedRunnerServiceCostModifiers{});
    if (!expect(full_offer.action ==
                RecruitedRunnerServiceOfferPreparationAction::roster_full)) {
        return 3;
    }

    auto state = make_state();
    std::vector<int> callback_order;
    const RecruitedRunnerServiceCallbacks callbacks{
        [&] { callback_order.push_back(0); },
        [&] { callback_order.push_back(1); },
    };
    const auto short_term = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::short_term_run, callbacks);
    if (!expect(short_term.action == RecruitedRunnerServiceAction::completed &&
                short_term.prompt ==
                    RecruitedRunnerServicePrompt::generic_runner_followup &&
                short_term.charged_amount == 250U && state.funds == 750U &&
                state.roster.primary_marker == 2U &&
                state.timers.primary_delay == 10U &&
                state.timers.primary_behavior == 10U &&
                state.selected_entity.service_flags == 0U &&
                read_big_endian_word(state.roster.primary_record, 0x04U) ==
                    0x3456U &&
                state.roster.primary_record.bytes[0x8FU] == 0U &&
                callback_order == std::vector<int>{0, 1})) {
        return 4;
    }

    state = make_state();
    state.selected_runner_template_index = 9U;
    state.roster.primary_record.bytes[0x10U] = 0x01U;
    state.selected_entity.source_position_y = 0x789AU;
    callback_order.clear();
    const auto permanent = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::permanent_hire, callbacks);
    if (!expect(permanent.action == RecruitedRunnerServiceAction::completed &&
                permanent.prompt == RecruitedRunnerServicePrompt::
                    stark_avenging_michael_followup &&
                permanent.charged_amount == 700U && state.funds == 300U &&
                state.roster.secondary_marker == 10U &&
                state.timers.secondary_delay == 10U &&
                state.timers.secondary_behavior == 10U &&
                state.roster.secondary_record.bytes[0x8FU] == 0xFFU &&
                read_big_endian_word(state.roster.secondary_record, 0x04U) ==
                    0x789AU && callback_order == std::vector<int>{0, 1})) {
        return 5;
    }

    state = make_state();
    state.permanent_fee = 500U;
    const auto free_hire = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::free_permanent_hire);
    if (!expect(free_hire.action == RecruitedRunnerServiceAction::completed &&
                free_hire.charged_amount == 0U && state.funds == 1000U &&
                state.permanent_fee == 0U &&
                state.roster.primary_record.bytes[0x8FU] == 0xFFU)) {
        return 6;
    }

    state = make_state();
    state.funds = 249U;
    const auto insufficient = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::short_term_run);
    if (!expect(insufficient.action ==
                    RecruitedRunnerServiceAction::insufficient_funds &&
                insufficient.prompt ==
                    RecruitedRunnerServicePrompt::insufficient_funds &&
                state.funds == 249U && state.roster.primary_marker == 0xFFU &&
                state.selected_entity.service_flags == 0x01U)) {
        return 7;
    }

    state = make_state();
    state.roster.primary_record.bytes[0x10U] = 0x01U;
    state.roster.secondary_record.bytes[0x10U] = 0x01U;
    if (!expect(recruited_runner_service_availability(state.roster) ==
                RecruitedRunnerServiceAvailability::roster_full)) {
        return 8;
    }
    state.roster.secondary_record.bytes[0x10U] = 0U;
    if (!expect(recruited_runner_service_availability(state.roster) ==
                RecruitedRunnerServiceAvailability::available)) {
        return 9;
    }

    state = make_state(1U);
    const auto invalid = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::short_term_run);
    if (!expect(invalid.action ==
                    RecruitedRunnerServiceAction::invalid_runner_template &&
                state.funds == 750U)) {
        return 10;
    }

    return 0;
}
