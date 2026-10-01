#pragma once

#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/integration/recruited_runner_conversation_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
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
};

struct DialoguePanelOption {
    DialoguePanelOptionRole role{};
    // This is populated only when the converted source has the actual visible
    // label. Common runner actions currently expose semantic roles, because
    // their menu text records have not yet been traced from the ROM.
    std::string_view source_label;
    integration::RecruitedRunnerConversationCommand command =
        integration::RecruitedRunnerConversationCommand::leave_conversation;
};

struct DialoguePanelView {
    bool open = false;
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

std::optional<integration::RecruitedRunnerConversationCommand>
dialogue_panel_option_command(const DialoguePanelView& view,
                              std::size_t option_index);

std::string_view dialogue_panel_runner_name(
    RecruitedRunnerConversationRunner runner);

// ROM 0x1F65A profile 0 is the Stoker's Coffin Motel portrait. The other
// field-exit profiles are deliberately unbound until their portrait streams
// are traced, so a shared panel cannot accidentally borrow Stoker's sprite.
std::string_view dialogue_panel_portrait_alias(
    FieldExitResourceProfile profile);

} // namespace shadowrun::ghidra_conversion::qt_runtime
