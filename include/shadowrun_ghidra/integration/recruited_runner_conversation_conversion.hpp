#pragma once

#include "shadowrun_ghidra/session/recruited_runner_service_conversion.hpp"

#include <cstddef>
#include <string_view>
#include <variant>

namespace shadowrun::ghidra_conversion::integration {

// This is the conversion-side owner of the recruited-runner conversation
// graph. It composes the source-derived session routines; it is not a second
// Qt menu state machine and it does not emulate the Genesis command table.
enum class RecruitedRunnerConversationScreen {
    unavailable,
    common_prompt,
    information_page,
    ricky_subject_prompt,
    winston_subject_prompt,
    trent_subject_prompt,
    information_response,
    service_offer,
    service_result,
    source_boundary,
    left,
};

enum class RecruitedRunnerConversationCommand {
    ask_for_information,
    ask_to_hire_runner,
    leave_conversation,
    continue_page,
    back_to_common_prompt,
    select_first_subject,
    select_second_subject,
    select_third_subject,
    pay_short_term_run,
    pay_permanent_hire,
    accept_free_permanent_hire,
};

enum class RecruitedRunnerConversationTransition {
    opened,
    information_opened,
    subject_prompt_opened,
    information_response_opened,
    service_offer_opened,
    service_payment_resolved,
    returned_to_common_prompt,
    left_conversation,
    source_boundary,
    invalid_command,
    unavailable,
};

// The view is deliberately a typed union of the already converted domain
// results. A Qt presenter can consume this without learning ROM pointers,
// mode-command numbers, or Genesis RAM addresses.
struct RecruitedRunnerConversationView {
    RecruitedRunnerConversationScreen screen =
        RecruitedRunnerConversationScreen::unavailable;
    RecruitedRunnerConversationTransition transition =
        RecruitedRunnerConversationTransition::unavailable;
    RecruitedRunnerConversationRunner runner =
        RecruitedRunnerConversationRunner::ricky;
    std::size_t page_index = 0;

    RecruitedRunnerConversationPromptResult common_prompt{};
    RecruitedRunnerInformationPromptResult information_prompt{};
    std::variant<std::monostate,
                 RickyInformationSubjectPrompt,
                 WinstonInformationSubjectPrompt,
                 TrentInformationSubjectPrompt>
        subject_prompt;
    std::variant<std::monostate,
                 RickyInformationResponse,
                 WinstonInformationResponse,
                 TrentInformationResponse>
        information_response;
    RecruitedRunnerServiceOfferPreparationResult service_offer{};
    RecruitedRunnerServiceResult service_result{};
    std::string_view source_boundary_reason;
};

// This state owns the mutable conversation cursor and the already converted
// service state. Static text/rules remain in the session conversion module;
// no source descriptor pointer is retained here.
struct RecruitedRunnerConversationState {
    RecruitedRunnerServiceState service{};
    RecruitedRunnerServiceCostModifiers cost_modifiers{};
    RecruitedRunnerServiceCallbacks service_callbacks{};

    RecruitedRunnerConversationScreen screen =
        RecruitedRunnerConversationScreen::unavailable;
    RecruitedRunnerConversationRunner runner =
        RecruitedRunnerConversationRunner::ricky;
    std::size_t page_index = 0;
    RecruitedRunnerConversationPromptResult common_prompt{};
    RecruitedRunnerInformationPromptResult information_prompt{};
    std::variant<std::monostate,
                 RickyInformationSubjectPrompt,
                 WinstonInformationSubjectPrompt,
                 TrentInformationSubjectPrompt>
        subject_prompt;
    std::variant<std::monostate,
                 RickyInformationResponse,
                 WinstonInformationResponse,
                 TrentInformationResponse>
        information_response;
    RecruitedRunnerServiceOfferPreparationResult service_offer{};
    RecruitedRunnerServiceResult service_result{};
    std::string_view source_boundary_reason;
};

// Entry point for command 0x04's common runner conversation. The selected
// runner, signed attitude, and common information/hire/leave choices come
// from the converted 0x211E8 branch.
RecruitedRunnerConversationView begin_recruited_runner_conversation(
    RecruitedRunnerConversationState& state);

// Routes one host input through the source-derived conversation graph. The
// graph currently has source-complete Ricky, Winston, and Trent subject
// trees; the other seven information branches return an explicit boundary so
// a missing conversion cannot silently become invented gameplay.
RecruitedRunnerConversationView dispatch_recruited_runner_conversation(
    RecruitedRunnerConversationState& state,
    RecruitedRunnerConversationCommand command);

} // namespace shadowrun::ghidra_conversion::integration
