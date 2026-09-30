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
                    RecruitedRunnerServiceOfferPreparationAction::roster_full &&
                full_offer.prompt == RecruitedRunnerServicePrompt::roster_full)) {
        return 3;
    }

    offer_state.roster.primary_record.bytes[0x10U] = 0U;
    offer_state.roster.secondary_record.bytes[0x10U] = 0U;
    offer_state.selected_runner_template_index = 0U;
    offer_state.service_progress_counters[0U] = 0U;
    const auto neutral_prompt =
        select_recruited_runner_conversation_prompt(offer_state);
    if (!expect(neutral_prompt.action ==
                    RecruitedRunnerConversationAction::selected &&
                neutral_prompt.runner ==
                    RecruitedRunnerConversationRunner::ricky &&
                neutral_prompt.tone ==
                    RecruitedRunnerConversationTone::neutral &&
                neutral_prompt.options[1] ==
                    RecruitedRunnerConversationOption::ask_to_hire_runner)) {
        return 4;
    }
    offer_state.service_progress_counters[0U] = 1U;
    if (!expect(select_recruited_runner_conversation_prompt(offer_state).tone ==
                RecruitedRunnerConversationTone::receptive)) {
        return 5;
    }
    offer_state.service_progress_counters[0U] = 0xFFU;
    if (!expect(select_recruited_runner_conversation_prompt(offer_state).tone ==
                RecruitedRunnerConversationTone::hostile)) {
        return 6;
    }
    offer_state.selected_runner_template_index = 9U;
    const auto stark_prompt =
        select_recruited_runner_conversation_prompt(offer_state);
    if (!expect(stark_prompt.action ==
                    RecruitedRunnerConversationAction::story_runner_special_case &&
                stark_prompt.runner == RecruitedRunnerConversationRunner::stark)) {
        return 7;
    }

    offer_state.selected_runner_template_index = 0U;
    offer_state.runner_information_flags = 0U;
    const auto ricky_information =
        select_recruited_runner_information_prompt(offer_state);
    if (!expect(ricky_information.action ==
                    RecruitedRunnerInformationAction::selected &&
                ricky_information.runner ==
                    RecruitedRunnerConversationRunner::ricky &&
                ricky_information.topic ==
                    RecruitedRunnerInformationTopic::ricky_glow_city_origin &&
                ricky_information.changed_flag == 0x0001U &&
                ricky_information.information_flags == 0x0001U &&
                ricky_information.opening_text ==
                    "I've lived in Glow City all my life." &&
                ricky_information.continuation_text ==
                    "Then I heard my totem calling.  I have embraced that power, the power of Rat!" &&
                ricky_information.more_option_text == "< MORE >")) {
        return 8;
    }
    const auto repeated_ricky_information =
        select_recruited_runner_information_prompt(offer_state);
    if (!expect(repeated_ricky_information.changed_flag == 0U &&
                repeated_ricky_information.information_flags == 0x0001U)) {
        return 9;
    }

    offer_state.selected_runner_template_index = 9U;
    const auto stark_information =
        select_recruited_runner_information_prompt(offer_state);
    if (!expect(stark_information.action ==
                    RecruitedRunnerInformationAction::selected &&
                stark_information.runner ==
                    RecruitedRunnerConversationRunner::stark &&
                stark_information.topic ==
                    RecruitedRunnerInformationTopic::stark_amerindian_heritage &&
                stark_information.changed_flag == 0x0200U &&
                stark_information.information_flags == 0x0201U &&
                stark_information.opening_text.find("AmerIndian") !=
                    std::string_view::npos)) {
        return 10;
    }

    offer_state.runner_information_flags = 0U;
    for (std::size_t runner_index = 0U; runner_index < 10U;
         ++runner_index) {
        offer_state.selected_runner_template_index =
            static_cast<std::uint8_t>(runner_index);
        const auto information =
            select_recruited_runner_information_prompt(offer_state);
        const auto expected_flag = static_cast<std::uint16_t>(
            std::uint16_t{1U} << runner_index);
        if (!expect(information.action ==
                        RecruitedRunnerInformationAction::selected &&
                    information.changed_flag == expected_flag &&
                    information.information_flags ==
                        static_cast<std::uint16_t>(expected_flag * 2U - 1U) &&
                    !information.opening_text.empty() &&
                    !information.continuation_text.empty() &&
                    information.more_option_text == "< MORE >")) {
            return 11;
        }
    }

    offer_state.selected_runner_template_index = 10U;
    if (!expect(select_recruited_runner_information_prompt(offer_state).action ==
                RecruitedRunnerInformationAction::runner_index_unavailable)) {
        return 12;
    }

    const auto ricky_subjects = ricky_information_subject_prompt();
    if (!expect(ricky_subjects.title_text == "Like what?" &&
                ricky_subjects.options[0] ==
                    "Tell me about shaman magic." &&
                ricky_subjects.options[1] ==
                    "Tell me about the local area." &&
                ricky_subjects.options[2] ==
                    "Do you know of any Johnsons in the area?")) {
        return 13;
    }
    const auto shaman_response = select_ricky_information_response(
        RickyInformationSubject::shaman_magic);
    if (!expect(shaman_response.action ==
                    RickyInformationResponseAction::selected &&
                shaman_response.page_count == 3U &&
                shaman_response.more_after_page ==
                    std::array<bool, 3>{{true, true, false}} &&
                shaman_response.page_text[1] ==
                    "<He shows you his Rat totem, a dingy block of wood.>" &&
                shaman_response.returns_to_common_conversation)) {
        return 14;
    }
    const auto local_response = select_ricky_information_response(
        RickyInformationSubject::local_area);
    if (!expect(local_response.page_count == 2U &&
                local_response.more_after_page ==
                    std::array<bool, 3>{{true, false, false}} &&
                local_response.page_text[1] ==
                    "Glow City sprung up when they ran out of room in the Rat's Nest.")) {
        return 15;
    }
    const auto johnson_response = select_ricky_information_response(
        RickyInformationSubject::johnson_gunderson);
    if (!expect(johnson_response.page_count == 1U &&
                !johnson_response.more_after_page[0] &&
                johnson_response.page_text[0] ==
                    "Yeah, sure.  Gunderson's a Johnson I've used when I needed some quick nuyen.")) {
        return 16;
    }
    if (!expect(select_ricky_information_response(
                    static_cast<RickyInformationSubject>(3U))
                    .action == RickyInformationResponseAction::
                                   subject_unavailable)) {
        return 17;
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
        return 18;
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
        return 19;
    }

    state = make_state();
    state.permanent_fee = 500U;
    const auto free_hire = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::free_permanent_hire);
    if (!expect(free_hire.action == RecruitedRunnerServiceAction::completed &&
                free_hire.charged_amount == 0U && state.funds == 1000U &&
                state.permanent_fee == 0U &&
                state.roster.primary_record.bytes[0x8FU] == 0xFFU)) {
        return 20;
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
        return 21;
    }

    state = make_state();
    state.roster.primary_record.bytes[0x10U] = 0x01U;
    state.roster.secondary_record.bytes[0x10U] = 0x01U;
    if (!expect(recruited_runner_service_availability(state.roster) ==
                RecruitedRunnerServiceAvailability::roster_full)) {
        return 22;
    }
    state.roster.secondary_record.bytes[0x10U] = 0U;
    if (!expect(recruited_runner_service_availability(state.roster) ==
                RecruitedRunnerServiceAvailability::available)) {
        return 23;
    }

    state = make_state(1U);
    const auto invalid = resolve_recruited_runner_service_payment(
        state, RecruitedRunnerServiceOffer::short_term_run);
    if (!expect(invalid.action ==
                    RecruitedRunnerServiceAction::invalid_runner_template &&
                state.funds == 750U)) {
        return 24;
    }

    return 0;
}
