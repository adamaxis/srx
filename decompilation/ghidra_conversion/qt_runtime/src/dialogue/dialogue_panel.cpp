#include "dialogue_panel.hpp"

#include <algorithm>
#include <array>
#include <string>
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

std::string_view resolve_continuation_label(
    const DialogueTextResolver& resolve_text, const std::string_view alias) {
    auto label = resolve_text ? resolve_text(alias) : std::string_view{};
    // The ROM stores the centered continuation caption after a leading 0x80
    // line delimiter. That delimiter remains in the shared Lua stream for
    // paragraph-aware dialogue, but is not part of the label itself.
    while (!label.empty() && label.front() == '|') {
        label.remove_prefix(1U);
    }
    return label;
}

void set_script_speaker(DialoguePanelView& view,
                        const DialogueSpeakerPresentation& speaker) {
    view.actor_kind = DialoguePanelActorKind::field_npc;
    view.actor_name = speaker.name;
    view.portrait_alias = speaker.portrait_alias;
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

void copy_text_stream(DialoguePanelView& view,
                      const RecruitedRunnerInformationTextStream& stream,
                      const std::string_view speaker_id,
                      const DialogueContentKeyResolver& resolve_content_key,
                      const DialogueTextResolver& resolve_text) {
    const auto text_key = resolve_content_key
                             ? resolve_content_key(speaker_id,
                                                  stream.content_id)
                             : std::string_view{};
    if (text_key.empty()) {
        mark_missing_text(view, stream.content_id);
        return;
    }
    const auto initial_segment_count = view.text_segment_count;
    copy_text_alias_stream(view, text_key, resolve_text);
    if (view.source_complete &&
        view.text_segment_count - initial_segment_count !=
            stream.segment_count) {
        view.source_complete = false;
        view.source_boundary_reason =
            "Lua runner dialogue segment count does not match the source descriptor";
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
                         const SubjectPrompt& subject_prompt,
                         const std::string_view speaker_id,
                         const DialogueContentKeyResolver& resolve_content_key,
                         const DialogueTextResolver& resolve_text) {
    const auto title_key = resolve_content_key
                               ? resolve_content_key(
                                     speaker_id,
                                     subject_prompt.title_content_id)
                               : std::string_view{};
    copy_text_alias_stream(view, title_key, resolve_text);
    if (view.text_segment_count != 1U) {
        view.source_complete = false;
        view.source_boundary_reason =
            "runner subject menu must resolve to one title segment";
    }
    constexpr std::array<DialoguePanelOptionRole, 3> kRoles{
        DialoguePanelOptionRole::information_subject,
        DialoguePanelOptionRole::information_subject,
        DialoguePanelOptionRole::information_subject};
    constexpr std::array<DialoguePanelCommand, 3> kCommands{
        DialoguePanelCommand::runner_select_first_subject,
        DialoguePanelCommand::runner_select_second_subject,
        DialoguePanelCommand::runner_select_third_subject};
    for (std::size_t index = 0;
         index < subject_prompt.option_content_ids.size(); ++index) {
        const auto content_id = subject_prompt.option_content_ids[index];
        const auto label_key = resolve_content_key
                                   ? resolve_content_key(speaker_id, content_id)
                                   : std::string_view{};
        const auto label = resolve_text ? resolve_text(label_key)
                                        : std::string_view{};
        if (label.empty()) {
            mark_missing_text(view, content_id);
        }
        add_option(view, kRoles[index], label,
                   kCommands[index]);
    }
}

template <typename Response>
void add_response_page(DialoguePanelView& view, const Response& response,
                       const std::string_view speaker_id,
                       const DialogueContentKeyResolver& resolve_content_key,
                       const DialogueTextResolver& resolve_text) {
    if (view.page_index >= response.page_count) {
        view.source_complete = false;
        view.source_boundary_reason =
            "converted dialogue page cursor exceeded its source page count";
        return;
    }
    copy_text_stream(view, response.pages[view.page_index], speaker_id,
                     resolve_content_key, resolve_text);
    view.has_more = response.more_after_page[view.page_index];
    view.returns_to_conversation = !view.has_more &&
                                   response.returns_to_common_conversation;
    if (view.has_more) {
        const auto more_key = resolve_content_key
                                  ? resolve_content_key({}, "shared.more")
                                  : std::string_view{};
        view.continuation_label = resolve_continuation_label(resolve_text,
                                                              more_key);
        if (view.continuation_label.empty()) {
            mark_missing_text(view, "shared.more");
        }
    }
}

} // namespace

std::string_view dialogue_panel_runner_id(
    const RecruitedRunnerConversationRunner runner) {
    switch (runner) {
    case RecruitedRunnerConversationRunner::ricky:
        return "ricky";
    case RecruitedRunnerConversationRunner::winston_marrs:
        return "winston_marrs";
    case RecruitedRunnerConversationRunner::trent:
        return "trent";
    case RecruitedRunnerConversationRunner::petr_uvehr:
        return "petr_uvehr";
    case RecruitedRunnerConversationRunner::walking_bear:
        return "walking_bear";
    case RecruitedRunnerConversationRunner::phantom:
        return "phantom";
    case RecruitedRunnerConversationRunner::two_fists:
        return "two_fists";
    case RecruitedRunnerConversationRunner::freya:
        return "freya";
    case RecruitedRunnerConversationRunner::rianna:
        return "rianna";
    case RecruitedRunnerConversationRunner::stark:
        return "stark";
    }
    return {};
}

DialoguePanelView make_dialogue_panel_view(
    const RecruitedRunnerConversationView& source_view,
    const DialogueTextResolver& resolve_text,
    const DialogueContentKeyResolver& resolve_content_key) {
    DialoguePanelView view{};
    view.actor = source_view.runner;
    const auto runner_id = dialogue_panel_runner_id(source_view.runner);
    const auto actor_name_key = resolve_content_key
                                    ? resolve_content_key(runner_id, "name")
                                    : std::string_view{};
    view.actor_name = resolve_text ? resolve_text(actor_name_key)
                                   : std::string_view{};
    if (view.actor_name.empty()) {
        mark_missing_text(view, actor_name_key);
    }
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
            copy_text_stream(view, source_view.information_prompt.opening_text,
                             runner_id, resolve_content_key, resolve_text);
        } else if (source_view.page_index == 1U) {
            copy_text_stream(view,
                             source_view.information_prompt.continuation_text,
                             runner_id, resolve_content_key, resolve_text);
        } else {
            view.source_complete = false;
            view.source_boundary_reason =
                "converted runner information page exceeded its two source streams";
            return view;
        }
        view.has_more = source_view.page_index == 0U;
        view.returns_to_conversation = source_view.page_index == 1U;
        if (view.has_more) {
            const auto more_key = resolve_content_key
                                      ? resolve_content_key(
                                            {}, source_view.information_prompt
                                                   .more_content_id)
                                      : std::string_view{};
            view.continuation_label = resolve_continuation_label(resolve_text,
                                                                  more_key);
            if (view.continuation_label.empty()) {
                mark_missing_text(view,
                                  source_view.information_prompt.more_content_id);
            }
        }
        return view;

    case RecruitedRunnerConversationScreen::ricky_subject_prompt:
    case RecruitedRunnerConversationScreen::winston_subject_prompt:
    case RecruitedRunnerConversationScreen::trent_subject_prompt:
        view.open = true;
        view.kind = DialoguePanelKind::subject_choices;
        std::visit(
            [&view, &resolve_text, &resolve_content_key, runner_id](
                const auto& subject_prompt) {
                using Prompt = std::decay_t<decltype(subject_prompt)>;
                if constexpr (!std::is_same_v<Prompt, std::monostate>) {
                    add_subject_options(view, subject_prompt, runner_id,
                                        resolve_content_key, resolve_text);
                }
            },
            source_view.subject_prompt);
        return view;

    case RecruitedRunnerConversationScreen::information_response:
        view.open = true;
        view.kind = DialoguePanelKind::information_response_page;
        std::visit(
            [&view, &resolve_text, &resolve_content_key, runner_id](
                const auto& response) {
                using Response = std::decay_t<decltype(response)>;
                if constexpr (!std::is_same_v<Response, std::monostate>) {
                    add_response_page(view, response, runner_id,
                                      resolve_content_key, resolve_text);
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
    const DialogueSpeakerPresentation& speaker,
    const std::string_view title_key,
    const std::array<std::string_view, 3>& option_label_keys,
    const DialogueTextResolver& resolve_text) {
    DialoguePanelView view{};
    view.open = true;
    view.actor_kind = DialoguePanelActorKind::field_npc;
    view.actor_name = speaker.name;
    view.portrait_alias = speaker.portrait_alias;
    view.kind = source_view.source_complete
                    ? DialoguePanelKind::field_npc_prompt
                    : DialoguePanelKind::source_boundary;
    if (!title_key.empty()) {
        copy_text_alias_stream(view, title_key, resolve_text);
    } else if (source_view.source_complete) {
        mark_missing_text(view, source_view.event_id.empty()
                                   ? source_view.menu_id
                                   : source_view.event_id);
    }
    if (!view.source_complete) {
        view.kind = DialoguePanelKind::source_boundary;
        view.source_boundary_reason = source_view.source_boundary_reason.empty()
                                          ? view.source_boundary_reason
                                          : source_view.source_boundary_reason;
    }

    for (std::size_t index = 0U; index < source_view.option_count; ++index) {
        const auto& option = source_view.options[index];
        const auto label_key = option_label_keys[index];
        auto label = resolve_text ? resolve_text(label_key)
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
            mark_missing_text(view, option.choice_id);
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

std::optional<DialoguePanelView> make_script_dialogue_panel_view(
    const DialogueSpeakerPresentation& speaker,
    const std::string_view title_key,
    const std::string_view continuation_key,
    const DialogueTextResolver& resolve_text) {
    if (speaker.id.empty() || speaker.name.empty() ||
        speaker.text_profile.empty() || speaker.portrait_alias.empty() ||
        title_key.empty()) {
        return std::nullopt;
    }

    DialoguePanelView view{};
    view.open = true;
    view.actor_kind = DialoguePanelActorKind::field_npc;
    set_script_speaker(view, speaker);
    view.kind = DialoguePanelKind::field_npc_prompt;

    copy_text_alias_stream(view, title_key, resolve_text);

    const auto more = resolve_continuation_label(resolve_text,
                                                continuation_key);
    if (!more.empty()) {
        view.has_more = true;
        view.continuation_label = more;
        view.options[0] = {DialoguePanelOptionRole::field_continue, more,
                           DialoguePanelCommand::field_continue};
        view.option_count = 1U;
    }
    if (!view.source_complete) {
        view.kind = DialoguePanelKind::source_boundary;
    }
    return view;
}

DialoguePanelView make_script_menu_panel_view(
    const std::optional<DialogueSpeakerPresentation>& speaker,
    const std::string_view title_key,
    const std::vector<std::pair<std::string, std::string>>& choices,
    const DialogueTextResolver& resolve_text) {
    DialoguePanelView view{};
    view.open = true;
    view.actor_kind = DialoguePanelActorKind::none;
    view.kind = DialoguePanelKind::field_npc_prompt;
    if (speaker.has_value()) {
        if (speaker->id.empty() || speaker->name.empty() ||
            speaker->text_profile.empty() ||
            speaker->portrait_alias.empty()) {
            mark_missing_text(view, speaker->id);
        } else {
            set_script_speaker(view, *speaker);
        }
    }

    if (title_key.empty()) {
        mark_missing_text(view, title_key);
        view.kind = DialoguePanelKind::source_boundary;
        return view;
    }
    copy_text_alias_stream(view, title_key, resolve_text);

    if (choices.size() > view.options.size()) {
        view.source_complete = false;
        view.source_boundary_reason = "script menu exceeds the panel choice limit";
    }
    for (const auto& [choice_id, label_key] : choices) {
        if (view.option_count >= view.options.size()) {
            break;
        }
        const auto label = resolve_text ? resolve_text(label_key)
                                        : std::string_view{};
        if (choice_id.empty() || label.empty()) {
            mark_missing_text(view, label_key);
            continue;
        }
        add_option(view, DialoguePanelOptionRole::script_menu_choice, label,
                   DialoguePanelCommand::script_menu_choice);
    }
    if (!view.source_complete) {
        view.kind = DialoguePanelKind::source_boundary;
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

} // namespace shadowrun::ghidra_conversion::qt_runtime
