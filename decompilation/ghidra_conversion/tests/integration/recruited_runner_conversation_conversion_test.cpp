#include "shadowrun_ghidra/integration/recruited_runner_conversation_conversion.hpp"

#include <cstdint>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;

RecruitedRunnerConversationState make_state(
    const std::uint8_t runner_index) {
    RecruitedRunnerConversationState state{};
    state.service.selected_runner_template_index = runner_index;
    state.service.templates.resize(10U);
    state.service.compact_records.resize(13U);
    state.service.service_progress_counters.resize(10U);
    state.service.funds = 1000U;
    state.service.selected_entity.present = true;
    state.service.selected_entity.service_flags = 0x01U;
    state.service.selected_entity.source_position_y = 0x3456U;
    state.service.short_term_fee = 250U;
    state.service.permanent_fee = 700U;
    return state;
}

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    auto ricky = make_state(0U);
    auto view = begin_recruited_runner_conversation(ricky);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::common_prompt &&
                view.transition ==
                    RecruitedRunnerConversationTransition::opened &&
                view.common_prompt.runner ==
                    RecruitedRunnerConversationRunner::ricky)) {
        return 1;
    }

    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::ask_for_information);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::information_page &&
                view.page_index == 0U &&
                view.information_prompt.opening_text.segment_count == 3U &&
                ricky.service.runner_information_flags == 0x0001U)) {
        return 2;
    }
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::information_page &&
                view.page_index == 1U)) {
        return 3;
    }
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::ricky_subject_prompt &&
                std::get<RickyInformationSubjectPrompt>(view.subject_prompt)
                        .option_keys[0] ==
                    "runner.ricky.subject.option.shaman_magic")) {
        return 4;
    }
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::select_first_subject);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::information_response &&
                view.page_index == 0U &&
                std::get<RickyInformationResponse>(view.information_response)
                        .page_count == 3U)) {
        return 5;
    }
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::continue_page);
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::continue_page);
    view = dispatch_recruited_runner_conversation(
        ricky, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::common_prompt &&
                view.transition == RecruitedRunnerConversationTransition::
                    returned_to_common_prompt)) {
        return 6;
    }

    auto winston = make_state(1U);
    view = begin_recruited_runner_conversation(winston);
    view = dispatch_recruited_runner_conversation(
        winston, RecruitedRunnerConversationCommand::ask_for_information);
    view = dispatch_recruited_runner_conversation(
        winston, RecruitedRunnerConversationCommand::continue_page);
    view = dispatch_recruited_runner_conversation(
        winston, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::winston_subject_prompt &&
                std::get<WinstonInformationSubjectPrompt>(view.subject_prompt)
                        .option_keys[2] ==
                    "runner.winston_marrs.subject.option.troll_origin")) {
        return 7;
    }
    view = dispatch_recruited_runner_conversation(
        winston, RecruitedRunnerConversationCommand::select_third_subject);
    if (!expect(std::get<WinstonInformationResponse>(view.information_response)
                    .page_count == 2U)) {
        return 8;
    }

    auto trent = make_state(2U);
    view = begin_recruited_runner_conversation(trent);
    view = dispatch_recruited_runner_conversation(
        trent, RecruitedRunnerConversationCommand::ask_for_information);
    view = dispatch_recruited_runner_conversation(
        trent, RecruitedRunnerConversationCommand::continue_page);
    view = dispatch_recruited_runner_conversation(
        trent, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::trent_subject_prompt)) {
        return 9;
    }
    view = dispatch_recruited_runner_conversation(
        trent, RecruitedRunnerConversationCommand::select_second_subject);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::information_response &&
                trent.service.story_flags == (std::uint32_t{1U} << 12U) &&
                std::get<TrentInformationResponse>(view.information_response)
                        .pages[0]
                        .text_key ==
                    "runner.trent.subject.contacts.page_0")) {
        return 10;
    }

    auto unresolved = make_state(3U);
    view = begin_recruited_runner_conversation(unresolved);
    view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::ask_for_information);
    view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    view = dispatch_recruited_runner_conversation(
        unresolved, RecruitedRunnerConversationCommand::continue_page);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::source_boundary &&
                view.source_boundary_reason.find("not yet source-converted") !=
                    std::string_view::npos)) {
        return 11;
    }

    auto hire = make_state(0U);
    view = begin_recruited_runner_conversation(hire);
    view = dispatch_recruited_runner_conversation(
        hire, RecruitedRunnerConversationCommand::ask_to_hire_runner);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::service_offer &&
                view.service_offer.action ==
                    RecruitedRunnerServiceOfferPreparationAction::prepared)) {
        return 12;
    }
    view = dispatch_recruited_runner_conversation(
        hire, RecruitedRunnerConversationCommand::pay_short_term_run);
    if (!expect(view.screen ==
                    RecruitedRunnerConversationScreen::service_result &&
                view.service_result.action ==
                    RecruitedRunnerServiceAction::completed)) {
        return 13;
    }

    return 0;
}
