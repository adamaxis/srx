#include "dialogue_panel.hpp"

#include <algorithm>
#include <type_traits>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

using integration::RecruitedRunnerConversationCommand;
using integration::RecruitedRunnerConversationScreen;
using integration::RecruitedRunnerConversationView;

void copy_text_stream(DialoguePanelView& view,
                      const RecruitedRunnerInformationTextStream& stream) {
    view.text_segments = stream.segments;
    view.text_segment_count = stream.segment_count;
}

void add_option(DialoguePanelView& view, const DialoguePanelOptionRole role,
                const std::string_view source_label,
                const RecruitedRunnerConversationCommand command) {
    if (view.option_count >= view.options.size()) {
        return;
    }
    view.options[view.option_count++] = {role, source_label, command};
}

void add_common_options(DialoguePanelView& view) {
    // The role is source-derived; visible labels remain blank until the
    // common prompt's text records are traced. This prevents an invented
    // English label from becoming a false visual reference.
    add_option(view, DialoguePanelOptionRole::information, {},
               RecruitedRunnerConversationCommand::ask_for_information);
    add_option(view, DialoguePanelOptionRole::hire_runner, {},
               RecruitedRunnerConversationCommand::ask_to_hire_runner);
    add_option(view, DialoguePanelOptionRole::leave_conversation, {},
               RecruitedRunnerConversationCommand::leave_conversation);
}

template <typename SubjectPrompt>
void add_subject_options(DialoguePanelView& view,
                         const SubjectPrompt& subject_prompt) {
    view.text_segments[0] = subject_prompt.title_text;
    view.text_segment_count = 1U;
    constexpr std::array<DialoguePanelOptionRole, 3> kRoles{
        DialoguePanelOptionRole::information_subject,
        DialoguePanelOptionRole::information_subject,
        DialoguePanelOptionRole::information_subject};
    constexpr std::array<RecruitedRunnerConversationCommand, 3> kCommands{
        RecruitedRunnerConversationCommand::select_first_subject,
        RecruitedRunnerConversationCommand::select_second_subject,
        RecruitedRunnerConversationCommand::select_third_subject};
    for (std::size_t index = 0; index < subject_prompt.options.size();
         ++index) {
        add_option(view, kRoles[index], subject_prompt.options[index],
                   kCommands[index]);
    }
}

template <typename Response>
void add_response_page(DialoguePanelView& view, const Response& response) {
    if (view.page_index >= response.page_count) {
        view.source_complete = false;
        view.source_boundary_reason =
            "converted dialogue page cursor exceeded its source page count";
        return;
    }
    copy_text_stream(view, response.pages[view.page_index]);
    view.has_more = response.more_after_page[view.page_index];
    view.returns_to_conversation = !view.has_more &&
                                   response.returns_to_common_conversation;
    if (view.has_more) {
        view.continuation_label = "< MORE >";
    }
}

} // namespace

std::string_view dialogue_panel_runner_name(
    const RecruitedRunnerConversationRunner runner) {
    switch (runner) {
    case RecruitedRunnerConversationRunner::ricky:
        return "Ricky";
    case RecruitedRunnerConversationRunner::winston_marrs:
        return "Winston Marrs";
    case RecruitedRunnerConversationRunner::trent:
        return "Trent";
    case RecruitedRunnerConversationRunner::petr_uvehr:
        return "Petr Uvehr";
    case RecruitedRunnerConversationRunner::walking_bear:
        return "Walking Bear";
    case RecruitedRunnerConversationRunner::phantom:
        return "Phantom";
    case RecruitedRunnerConversationRunner::two_fists:
        return "Two Fists";
    case RecruitedRunnerConversationRunner::freya:
        return "Freya";
    case RecruitedRunnerConversationRunner::rianna:
        return "Rianna";
    case RecruitedRunnerConversationRunner::stark:
        return "Stark";
    }
    return {};
}

DialoguePanelView make_dialogue_panel_view(
    const RecruitedRunnerConversationView& source_view) {
    DialoguePanelView view{};
    view.actor = source_view.runner;
    view.actor_name = dialogue_panel_runner_name(source_view.runner);
    view.page_index = source_view.page_index;

    switch (source_view.screen) {
    case RecruitedRunnerConversationScreen::common_prompt:
        view.open = true;
        view.kind = DialoguePanelKind::common_conversation_choices;
        add_common_options(view);
        return view;

    case RecruitedRunnerConversationScreen::information_page:
        view.open = true;
        view.kind = DialoguePanelKind::information_page;
        if (source_view.page_index == 0U) {
            copy_text_stream(view, source_view.information_prompt.opening_text);
        } else if (source_view.page_index == 1U) {
            copy_text_stream(view,
                            source_view.information_prompt.continuation_text);
        } else {
            view.source_complete = false;
            view.source_boundary_reason =
                "converted runner information page exceeded its two source streams";
            return view;
        }
        view.has_more = source_view.page_index == 0U;
        view.returns_to_conversation = source_view.page_index == 1U;
        if (view.has_more) {
            view.continuation_label =
                source_view.information_prompt.more_option_text;
        }
        return view;

    case RecruitedRunnerConversationScreen::ricky_subject_prompt:
    case RecruitedRunnerConversationScreen::winston_subject_prompt:
    case RecruitedRunnerConversationScreen::trent_subject_prompt:
        view.open = true;
        view.kind = DialoguePanelKind::subject_choices;
        std::visit(
            [&view](const auto& subject_prompt) {
                using Prompt = std::decay_t<decltype(subject_prompt)>;
                if constexpr (!std::is_same_v<Prompt, std::monostate>) {
                    add_subject_options(view, subject_prompt);
                }
            },
            source_view.subject_prompt);
        return view;

    case RecruitedRunnerConversationScreen::information_response:
        view.open = true;
        view.kind = DialoguePanelKind::information_response_page;
        std::visit(
            [&view](const auto& response) {
                using Response = std::decay_t<decltype(response)>;
                if constexpr (!std::is_same_v<Response, std::monostate>) {
                    add_response_page(view, response);
                }
            },
            source_view.information_response);
        return view;

    case RecruitedRunnerConversationScreen::service_offer:
        view.open = true;
        view.kind = DialoguePanelKind::service_offer;
        view.short_term_fee = source_view.service_offer.short_term_fee;
        view.permanent_fee = source_view.service_offer.permanent_fee;
        if (source_view.service_offer.action !=
            RecruitedRunnerServiceOfferPreparationAction::prepared) {
            // The source roster-full descriptor is title-only. Do not expose
            // payment choices when the converted preparation did not publish
            // an offer.
            return view;
        }
        add_option(view, DialoguePanelOptionRole::short_term_run, {},
                   RecruitedRunnerConversationCommand::pay_short_term_run);
        add_option(view, DialoguePanelOptionRole::permanent_hire, {},
                   RecruitedRunnerConversationCommand::pay_permanent_hire);
        add_option(view, DialoguePanelOptionRole::free_permanent_hire, {},
                   RecruitedRunnerConversationCommand::accept_free_permanent_hire);
        return view;

    case RecruitedRunnerConversationScreen::service_result:
        view.open = true;
        view.kind = DialoguePanelKind::service_result;
        view.charged_amount = source_view.service_result.charged_amount;
        view.service_prompt = source_view.service_result.prompt;
        return view;

    case RecruitedRunnerConversationScreen::source_boundary:
        view.open = true;
        view.kind = DialoguePanelKind::source_boundary;
        view.source_complete = false;
        view.source_boundary_reason = source_view.source_boundary_reason;
        return view;

    case RecruitedRunnerConversationScreen::unavailable:
    case RecruitedRunnerConversationScreen::left:
        return view;
    }
    return view;
}

std::optional<integration::RecruitedRunnerConversationCommand>
dialogue_panel_option_command(const DialoguePanelView& view,
                              const std::size_t option_index) {
    if (!view.open || option_index >= view.option_count) {
        return std::nullopt;
    }
    return view.options[option_index].command;
}

std::string_view dialogue_panel_portrait_alias(
    const FieldExitResourceProfile profile) {
    if (profile == FieldExitResourceProfile::stoker_motel) {
        return "dialogue.portraits.stoker.motel";
    }
    return {};
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
