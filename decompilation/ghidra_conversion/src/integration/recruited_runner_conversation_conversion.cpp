#include "shadowrun_ghidra/integration/recruited_runner_conversation_conversion.hpp"

#include <optional>
#include <type_traits>

namespace shadowrun::ghidra_conversion::integration {
namespace {

constexpr std::string_view kStarkConversationBoundary =
    "Stark's separate story conversation root is not yet source-converted.";
constexpr std::string_view kSubjectTreeBoundary =
    "This runner's subject tree is not yet source-converted; return to the ROM trace before implementing it.";

RecruitedRunnerConversationView make_view(
    const RecruitedRunnerConversationState& state,
    const RecruitedRunnerConversationTransition transition) {
    RecruitedRunnerConversationView view{};
    view.screen = state.screen;
    view.transition = transition;
    view.runner = state.runner;
    view.page_index = state.page_index;
    view.common_prompt = state.common_prompt;
    view.information_prompt = state.information_prompt;
    view.subject_prompt = state.subject_prompt;
    view.information_response = state.information_response;
    view.service_offer = state.service_offer;
    view.service_result = state.service_result;
    view.source_boundary_reason = state.source_boundary_reason;
    return view;
}

RecruitedRunnerConversationView set_invalid_command(
    const RecruitedRunnerConversationState& state) {
    return make_view(state,
                     RecruitedRunnerConversationTransition::invalid_command);
}

RecruitedRunnerConversationView open_common_prompt(
    RecruitedRunnerConversationState& state,
    const RecruitedRunnerConversationTransition transition) {
    state.common_prompt = select_recruited_runner_conversation_prompt(
        state.service);
    state.runner = state.common_prompt.runner;
    state.page_index = 0U;
    state.source_boundary_reason = {};
    if (state.common_prompt.action ==
        RecruitedRunnerConversationAction::story_runner_special_case) {
        state.screen = RecruitedRunnerConversationScreen::source_boundary;
        state.source_boundary_reason = kStarkConversationBoundary;
        return make_view(
            state, RecruitedRunnerConversationTransition::source_boundary);
    }
    if (state.common_prompt.action !=
        RecruitedRunnerConversationAction::selected) {
        state.screen = RecruitedRunnerConversationScreen::unavailable;
        return make_view(state,
                         RecruitedRunnerConversationTransition::unavailable);
    }
    state.screen = RecruitedRunnerConversationScreen::common_prompt;
    return make_view(state, transition);
}

RecruitedRunnerConversationView open_subject_prompt(
    RecruitedRunnerConversationState& state) {
    state.page_index = 0U;
    state.source_boundary_reason = {};
    switch (state.runner) {
    case RecruitedRunnerConversationRunner::ricky:
        state.subject_prompt = ricky_information_subject_prompt();
        state.screen = RecruitedRunnerConversationScreen::ricky_subject_prompt;
        break;
    case RecruitedRunnerConversationRunner::winston_marrs:
        state.subject_prompt = winston_information_subject_prompt();
        state.screen =
            RecruitedRunnerConversationScreen::winston_subject_prompt;
        break;
    case RecruitedRunnerConversationRunner::trent:
        state.subject_prompt = trent_information_subject_prompt();
        state.screen = RecruitedRunnerConversationScreen::trent_subject_prompt;
        break;
    default:
        state.screen = RecruitedRunnerConversationScreen::source_boundary;
        state.source_boundary_reason = kSubjectTreeBoundary;
        return make_view(
            state, RecruitedRunnerConversationTransition::source_boundary);
    }
    return make_view(
        state, RecruitedRunnerConversationTransition::subject_prompt_opened);
}

std::optional<std::size_t> subject_index(
    const RecruitedRunnerConversationCommand command) {
    switch (command) {
    case RecruitedRunnerConversationCommand::select_first_subject:
        return 0U;
    case RecruitedRunnerConversationCommand::select_second_subject:
        return 1U;
    case RecruitedRunnerConversationCommand::select_third_subject:
        return 2U;
    default:
        return std::nullopt;
    }
}

std::optional<RecruitedRunnerServiceOffer> service_offer(
    const RecruitedRunnerConversationCommand command) {
    switch (command) {
    case RecruitedRunnerConversationCommand::pay_short_term_run:
        return RecruitedRunnerServiceOffer::short_term_run;
    case RecruitedRunnerConversationCommand::pay_permanent_hire:
        return RecruitedRunnerServiceOffer::permanent_hire;
    case RecruitedRunnerConversationCommand::accept_free_permanent_hire:
        return RecruitedRunnerServiceOffer::free_permanent_hire;
    default:
        return std::nullopt;
    }
}

RecruitedRunnerConversationView select_information_subject(
    RecruitedRunnerConversationState& state,
    const std::size_t index) {
    if (index >= 3U) {
        return set_invalid_command(state);
    }
    switch (state.runner) {
    case RecruitedRunnerConversationRunner::ricky:
        state.information_response = select_ricky_information_response(
            static_cast<RickyInformationSubject>(index));
        break;
    case RecruitedRunnerConversationRunner::winston_marrs:
        state.information_response = select_winston_information_response(
            static_cast<WinstonInformationSubject>(index));
        break;
    case RecruitedRunnerConversationRunner::trent:
        state.information_response = select_trent_information_response(
            static_cast<TrentInformationSubject>(index), state.service);
        break;
    default:
        state.screen = RecruitedRunnerConversationScreen::source_boundary;
        state.source_boundary_reason = kSubjectTreeBoundary;
        return make_view(
            state, RecruitedRunnerConversationTransition::source_boundary);
    }
    state.page_index = 0U;
    state.screen = RecruitedRunnerConversationScreen::information_response;
    return make_view(
        state, RecruitedRunnerConversationTransition::information_response_opened);
}

RecruitedRunnerConversationView continue_information_response(
    RecruitedRunnerConversationState& state) {
    std::size_t page_count = 0U;
    std::visit(
        [&page_count](const auto& response) {
            using Response = std::decay_t<decltype(response)>;
            if constexpr (!std::is_same_v<Response, std::monostate>) {
                page_count = response.page_count;
            }
        },
        state.information_response);
    if (state.page_index + 1U < page_count) {
        ++state.page_index;
        return make_view(
            state, RecruitedRunnerConversationTransition::information_response_opened);
    }
    return open_common_prompt(
        state, RecruitedRunnerConversationTransition::returned_to_common_prompt);
}

} // namespace

RecruitedRunnerConversationView begin_recruited_runner_conversation(
    RecruitedRunnerConversationState& state) {
    state.screen = RecruitedRunnerConversationScreen::unavailable;
    state.page_index = 0U;
    state.subject_prompt = std::monostate{};
    state.information_response = std::monostate{};
    state.source_boundary_reason = {};
    return open_common_prompt(
        state, RecruitedRunnerConversationTransition::opened);
}

RecruitedRunnerConversationView dispatch_recruited_runner_conversation(
    RecruitedRunnerConversationState& state,
    const RecruitedRunnerConversationCommand command) {
    switch (state.screen) {
    case RecruitedRunnerConversationScreen::common_prompt:
        switch (command) {
        case RecruitedRunnerConversationCommand::ask_for_information:
            state.information_prompt =
                select_recruited_runner_information_prompt(state.service);
            if (state.information_prompt.action !=
                RecruitedRunnerInformationAction::selected) {
                state.screen = RecruitedRunnerConversationScreen::unavailable;
                return make_view(
                    state, RecruitedRunnerConversationTransition::unavailable);
            }
            state.runner = state.information_prompt.runner;
            state.page_index = 0U;
            state.screen = RecruitedRunnerConversationScreen::information_page;
            return make_view(
                state, RecruitedRunnerConversationTransition::information_opened);
        case RecruitedRunnerConversationCommand::ask_to_hire_runner:
            state.service_offer = prepare_recruited_runner_service_offer(
                state.service, state.cost_modifiers);
            state.screen = RecruitedRunnerConversationScreen::service_offer;
            return make_view(
                state, RecruitedRunnerConversationTransition::service_offer_opened);
        case RecruitedRunnerConversationCommand::leave_conversation:
            state.screen = RecruitedRunnerConversationScreen::left;
            return make_view(
                state, RecruitedRunnerConversationTransition::left_conversation);
        default:
            return set_invalid_command(state);
        }

    case RecruitedRunnerConversationScreen::information_page:
        if (command == RecruitedRunnerConversationCommand::continue_page) {
            if (state.page_index == 0U) {
                state.page_index = 1U;
                return make_view(
                    state, RecruitedRunnerConversationTransition::information_opened);
            }
            return open_subject_prompt(state);
        }
        if (command ==
            RecruitedRunnerConversationCommand::back_to_common_prompt) {
            return open_common_prompt(
                state, RecruitedRunnerConversationTransition::returned_to_common_prompt);
        }
        return set_invalid_command(state);

    case RecruitedRunnerConversationScreen::ricky_subject_prompt:
    case RecruitedRunnerConversationScreen::winston_subject_prompt:
    case RecruitedRunnerConversationScreen::trent_subject_prompt: {
        const auto index = subject_index(command);
        if (!index.has_value()) {
            if (command ==
                RecruitedRunnerConversationCommand::back_to_common_prompt) {
                return open_common_prompt(
                    state, RecruitedRunnerConversationTransition::returned_to_common_prompt);
            }
            return set_invalid_command(state);
        }
        return select_information_subject(state, *index);
    }

    case RecruitedRunnerConversationScreen::information_response:
        if (command == RecruitedRunnerConversationCommand::continue_page) {
            return continue_information_response(state);
        }
        if (command ==
            RecruitedRunnerConversationCommand::back_to_common_prompt) {
            return open_common_prompt(
                state, RecruitedRunnerConversationTransition::returned_to_common_prompt);
        }
        return set_invalid_command(state);

    case RecruitedRunnerConversationScreen::service_offer: {
        const auto offer = service_offer(command);
        if (!offer.has_value()) {
            return set_invalid_command(state);
        }
        state.service_result = resolve_recruited_runner_service_payment(
            state.service, *offer, state.service_callbacks);
        state.screen = RecruitedRunnerConversationScreen::service_result;
        return make_view(
            state, RecruitedRunnerConversationTransition::service_payment_resolved);
    }

    case RecruitedRunnerConversationScreen::service_result:
    case RecruitedRunnerConversationScreen::source_boundary:
    case RecruitedRunnerConversationScreen::left:
    case RecruitedRunnerConversationScreen::unavailable:
        return set_invalid_command(state);
    }
    return set_invalid_command(state);
}

} // namespace shadowrun::ghidra_conversion::integration
