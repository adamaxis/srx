#include "dialogue_panel.hpp"

#include <algorithm>
#include <array>
#include <type_traits>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

using integration::RecruitedRunnerConversationCommand;
using integration::RecruitedRunnerConversationScreen;
using integration::RecruitedRunnerConversationView;

DialoguePanelCommand map_runner_command(
    const RecruitedRunnerConversationCommand command) {
    switch (command) {
    case RecruitedRunnerConversationCommand::ask_for_information:
        return DialoguePanelCommand::runner_ask_for_information;
    case RecruitedRunnerConversationCommand::ask_to_hire_runner:
        return DialoguePanelCommand::runner_ask_to_hire;
    case RecruitedRunnerConversationCommand::leave_conversation:
        return DialoguePanelCommand::runner_leave_conversation;
    case RecruitedRunnerConversationCommand::continue_page:
        return DialoguePanelCommand::runner_continue_page;
    case RecruitedRunnerConversationCommand::back_to_common_prompt:
        return DialoguePanelCommand::runner_back_to_common_prompt;
    case RecruitedRunnerConversationCommand::select_first_subject:
        return DialoguePanelCommand::runner_select_first_subject;
    case RecruitedRunnerConversationCommand::select_second_subject:
        return DialoguePanelCommand::runner_select_second_subject;
    case RecruitedRunnerConversationCommand::select_third_subject:
        return DialoguePanelCommand::runner_select_third_subject;
    case RecruitedRunnerConversationCommand::pay_short_term_run:
        return DialoguePanelCommand::runner_pay_short_term_run;
    case RecruitedRunnerConversationCommand::pay_permanent_hire:
        return DialoguePanelCommand::runner_pay_permanent_hire;
    case RecruitedRunnerConversationCommand::accept_free_permanent_hire:
        return DialoguePanelCommand::runner_accept_free_permanent_hire;
    }
    return DialoguePanelCommand::runner_leave_conversation;
}

DialoguePanelOptionRole map_stoker_role(
    const StokerMotelPromptOptionRole role) {
    switch (role) {
    case StokerMotelPromptOptionRole::continue_arrival:
        return DialoguePanelOptionRole::field_continue;
    case StokerMotelPromptOptionRole::rent_room:
        return DialoguePanelOptionRole::field_rent_room;
    case StokerMotelPromptOptionRole::leave_motel:
        return DialoguePanelOptionRole::field_leave_motel;
    case StokerMotelPromptOptionRole::pay_room:
        return DialoguePanelOptionRole::field_pay_room;
    case StokerMotelPromptOptionRole::collect_package:
        return DialoguePanelOptionRole::field_collect_package;
    case StokerMotelPromptOptionRole::pay_michael_debt:
        return DialoguePanelOptionRole::field_pay_debt;
    case StokerMotelPromptOptionRole::ask_jump_house:
        return DialoguePanelOptionRole::field_jump_house;
    }
    return DialoguePanelOptionRole::field_continue;
}

DialoguePanelCommand map_stoker_command(
    const StokerMotelPromptCommand command) {
    switch (command) {
    case StokerMotelPromptCommand::resolve_arrival:
        return DialoguePanelCommand::stoker_resolve_arrival;
    case StokerMotelPromptCommand::open_room_offer:
        return DialoguePanelCommand::stoker_open_room_offer;
    case StokerMotelPromptCommand::open_leave_notice:
        return DialoguePanelCommand::stoker_open_leave_notice;
    case StokerMotelPromptCommand::pay_room:
        return DialoguePanelCommand::stoker_pay_room;
    case StokerMotelPromptCommand::open_debt_question:
        return DialoguePanelCommand::stoker_open_debt_question;
    case StokerMotelPromptCommand::pay_michael_debt:
        return DialoguePanelCommand::stoker_pay_michael_debt;
    case StokerMotelPromptCommand::open_jump_house_referral:
        return DialoguePanelCommand::stoker_open_jump_house_referral;
    case StokerMotelPromptCommand::source_boundary:
        return DialoguePanelCommand::stoker_source_boundary;
    }
    return DialoguePanelCommand::stoker_source_boundary;
}

void copy_text_stream(DialoguePanelView& view,
                      const RecruitedRunnerInformationTextStream& stream) {
    const auto count = std::min(stream.segment_count,
                                view.text_segments.size());
    std::copy_n(stream.segments.begin(), count,
                view.text_segments.begin());
    view.text_segment_count = count;
}

void add_option(DialoguePanelView& view, const DialoguePanelOptionRole role,
                const std::string_view source_label,
                const DialoguePanelCommand command) {
    if (view.option_count >= view.options.size()) {
        return;
    }
    view.options[view.option_count++] = {role, source_label, command};
}

void mark_missing_text(DialoguePanelView& view, const std::string_view alias) {
    view.source_complete = false;
    view.source_boundary_reason = alias.empty()
                                     ? "dialogue text alias is empty"
                                     : "dialogue text alias is missing";
}

void copy_text_alias_stream(DialoguePanelView& view,
                            const std::string_view text,
                            const DialogueTextResolver& resolve_text) {
    const auto resolved = resolve_text ? resolve_text(text) : std::string_view{};
    if (resolved.empty()) {
        mark_missing_text(view, text);
        return;
    }
    std::size_t start = 0U;
    while (start <= resolved.size() &&
           view.text_segment_count < view.text_segments.size()) {
        const auto separator = resolved.find('|', start);
        const auto end = separator == std::string_view::npos
                             ? resolved.size()
                             : separator;
        view.text_segments[view.text_segment_count++] =
            resolved.substr(start, end - start);
        if (separator == std::string_view::npos) {
            break;
        }
        start = separator + 1U;
    }
    if (start < resolved.size() &&
        view.text_segment_count == view.text_segments.size()) {
        view.source_complete = false;
        view.source_boundary_reason =
            "dialogue text stream exceeds the shared panel capacity";
    }
}

void add_common_options(DialoguePanelView& view) {
    // The role is source-derived; visible labels remain blank until the
    // common prompt's text records are traced. This prevents an invented
    // English label from becoming a false visual reference.
    add_option(view, DialoguePanelOptionRole::information, {},
               DialoguePanelCommand::runner_ask_for_information);
    add_option(view, DialoguePanelOptionRole::hire_runner, {},
               DialoguePanelCommand::runner_ask_to_hire);
    add_option(view, DialoguePanelOptionRole::leave_conversation, {},
               DialoguePanelCommand::runner_leave_conversation);
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
    constexpr std::array<DialoguePanelCommand, 3> kCommands{
        DialoguePanelCommand::runner_select_first_subject,
        DialoguePanelCommand::runner_select_second_subject,
        DialoguePanelCommand::runner_select_third_subject};
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
                   map_runner_command(
                       RecruitedRunnerConversationCommand::pay_short_term_run));
        add_option(view, DialoguePanelOptionRole::permanent_hire, {},
                   map_runner_command(
                       RecruitedRunnerConversationCommand::pay_permanent_hire));
        add_option(view, DialoguePanelOptionRole::free_permanent_hire, {},
                   map_runner_command(RecruitedRunnerConversationCommand::
                                          accept_free_permanent_hire));
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

DialoguePanelView make_dialogue_panel_view(
    const StokerMotelPromptView& source_view,
    const DialogueTextResolver& resolve_text) {
    DialoguePanelView view{};
    view.open = true;
    view.actor_kind = DialoguePanelActorKind::field_npc;
    view.actor_name = "Stoker";
    view.portrait_alias = "dialogue.portraits.stoker.motel";
    view.kind = source_view.source_complete
                    ? DialoguePanelKind::field_npc_prompt
                    : DialoguePanelKind::source_boundary;
    copy_text_alias_stream(view, source_view.title_alias, resolve_text);
    if (!view.source_complete) {
        view.kind = DialoguePanelKind::source_boundary;
        view.source_boundary_reason = source_view.source_boundary_reason.empty()
                                          ? view.source_boundary_reason
                                          : source_view.source_boundary_reason;
    }

    for (std::size_t index = 0U; index < source_view.option_count; ++index) {
        const auto& option = source_view.options[index];
        auto label = resolve_text ? resolve_text(option.text_alias)
                                  : std::string_view{};
        if (source_view.has_more) {
            // A leading empty source segment is a text-stream delimiter, not
            // part of the centered continuation label. Preserve the authored
            // spaces after it; they carry the source's horizontal placement.
            while (!label.empty() && label.front() == '|') {
                label.remove_prefix(1U);
            }
        }
        if (label.empty()) {
            mark_missing_text(view, option.text_alias);
        }
        add_option(view, map_stoker_role(option.role), label,
                   map_stoker_command(option.command));
    }
    view.has_more = source_view.has_more;
    view.returns_to_field = source_view.returns_to_field;
    if (source_view.has_more && view.option_count != 0U) {
        view.continuation_label = view.options[0].source_label;
    }
    if (!source_view.source_complete) {
        view.source_complete = false;
        if (view.source_boundary_reason.empty()) {
            view.source_boundary_reason = source_view.source_boundary_reason;
        }
    }
    return view;
}

std::optional<DialoguePanelCommand>
dialogue_panel_option_command(const DialoguePanelView& view,
                              const std::size_t option_index) {
    if (!view.open || option_index >= view.option_count) {
        return std::nullopt;
    }
    return view.options[option_index].command;
}

std::optional<DialoguePanelCommand> dialogue_panel_command(
    const DialoguePanelView& view, const std::size_t option_index) {
    return dialogue_panel_option_command(view, option_index);
}

std::string_view dialogue_panel_portrait_alias(
    const FieldExitResourceProfile profile) {
    if (profile == FieldExitResourceProfile::stoker_motel) {
        return "dialogue.portraits.stoker.motel";
    }
    return {};
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
