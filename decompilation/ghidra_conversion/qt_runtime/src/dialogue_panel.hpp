#pragma once

#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_prompt_conversion.hpp"
#include "shadowrun_ghidra/integration/recruited_runner_conversation_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

// All NPC conversations use the same host presentation contract. The source
// conversion owns the branch/state machine; this adapter only projects its
// typed result into the panel's common portrait/text/choice/page shape.
enum class DialoguePanelKind {
    closed,
    common_conversation_choices,
    information_page,
    subject_choices,
    information_response_page,
    service_offer,
    service_result,
    field_npc_prompt,
    source_boundary,
};

enum class DialoguePanelOptionRole {
    information,
    hire_runner,
    leave_conversation,
    information_subject,
    short_term_run,
    permanent_hire,
    free_permanent_hire,
    field_continue,
    field_rent_room,
    field_leave_motel,
    field_pay_room,
    field_collect_package,
    field_pay_debt,
    field_jump_house,
};

enum class DialoguePanelActorKind {
    recruited_runner,
    field_npc,
};

// The presenter dispatches this semantic command back to the owning
// conversion controller. It is deliberately broader than either source
// command table, so the common panel does not carry Genesis command numbers.
enum class DialoguePanelCommand {
    runner_ask_for_information,
    runner_ask_to_hire,
    runner_leave_conversation,
    runner_continue_page,
    runner_back_to_common_prompt,
    runner_select_first_subject,
    runner_select_second_subject,
    runner_select_third_subject,
    runner_pay_short_term_run,
    runner_pay_permanent_hire,
    runner_accept_free_permanent_hire,
    stoker_resolve_arrival,
    stoker_open_room_offer,
    stoker_open_leave_notice,
    stoker_pay_room,
    stoker_open_debt_question,
    stoker_pay_michael_debt,
    stoker_open_jump_house_referral,
    stoker_source_boundary,
};

struct DialoguePanelOption {
    DialoguePanelOptionRole role{};
    // This is populated only when the converted source has the actual visible
    // label. Common runner actions currently expose semantic roles, because
    // their menu text records have not yet been traced from the ROM.
    std::string_view source_label;
    DialoguePanelCommand command =
        DialoguePanelCommand::runner_leave_conversation;
};

struct DialoguePanelView {
    bool open = false;
    DialoguePanelActorKind actor_kind =
        DialoguePanelActorKind::recruited_runner;
    RecruitedRunnerConversationRunner actor =
        RecruitedRunnerConversationRunner::ricky;
    std::string_view actor_name;
    // Empty means the actor's portrait record has not been source-bound yet;
    // the presenter must not substitute another NPC's image.
    std::string_view portrait_alias;
    DialoguePanelKind kind = DialoguePanelKind::closed;
    std::size_t page_index = 0;

    std::array<std::string_view, 4> text_segments{};
    std::size_t text_segment_count = 0;
    std::array<DialoguePanelOption, 3> options{};
    std::size_t option_count = 0;
    std::size_t selected_option = 0;

    bool has_more = false;
    bool returns_to_conversation = false;
    bool returns_to_field = false;
    std::string_view continuation_label;

    std::uint32_t short_term_fee = 0;
    std::uint32_t permanent_fee = 0;
    std::uint32_t charged_amount = 0;
    RecruitedRunnerServicePrompt service_prompt =
        RecruitedRunnerServicePrompt::none;

    bool source_complete = true;
    std::string_view source_boundary_reason;
};

// One projection function covers the common runner/NPC conversation shell.
// It intentionally does not own mutable cursor state or dispatch commands;
// input returns to dispatch_recruited_runner_conversation().
DialoguePanelView make_dialogue_panel_view(
    const integration::RecruitedRunnerConversationView& source_view);

std::optional<DialoguePanelCommand>
dialogue_panel_option_command(const DialoguePanelView& view,
                              std::size_t option_index);

// Common command projection used by both runner and field-NPC panels.
std::optional<DialoguePanelCommand> dialogue_panel_command(
    const DialoguePanelView& view, std::size_t option_index);

std::string_view dialogue_panel_runner_name(
    RecruitedRunnerConversationRunner runner);

// ROM 0x1F65A profile 0 is the Stoker's Coffin Motel portrait. The other
// field-exit profiles are deliberately unbound until their portrait streams
// are traced, so a shared panel cannot accidentally borrow Stoker's sprite.
std::string_view dialogue_panel_portrait_alias(
    FieldExitResourceProfile profile);

using DialogueTextResolver =
    std::function<std::string_view(std::string_view alias)>;

// Project the source-backed Stoker prompt records into the same panel shape
// as recruited-runner conversations. Text is resolved through the direct
// asset catalog; a missing alias remains an explicit source boundary.
DialoguePanelView make_dialogue_panel_view(
    const StokerMotelPromptView& source_view,
    const DialogueTextResolver& resolve_text);

} // namespace shadowrun::ghidra_conversion::qt_runtime
