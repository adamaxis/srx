#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion {

// These are host identifiers for the variable-length prompt records selected
// by 0x1F65A and the commands in the 0x12CCB4 dispatch table. They are not ROM
// pointers. The corresponding source text is emitted once into the direct
// Lua dialogue text asset.
enum class StokerMotelPromptId {
    arrival,
    manager_greeting,
    street_ambush,
    room_offer,
    package_handoff,
    package_question,
    manager_greeting_with_package,
    leave_notice,
    debt_question,
    room_paid,
    room_insufficient,
    jump_house_referral,
    source_boundary,
};

struct StokerMotelStoryState {
    bool opening_dialogue_seen = false;
    bool michael_debt_paid = false;
    bool michael_package_collected = false;
};

enum class StokerMotelPromptOptionRole {
    continue_arrival,
    rent_room,
    leave_motel,
    pay_room,
    collect_package,
    pay_michael_debt,
    ask_jump_house,
};

enum class StokerMotelPromptCommand {
    resolve_arrival,
    open_room_offer,
    open_leave_notice,
    pay_room,
    open_debt_question,
    pay_michael_debt,
    open_jump_house_referral,
    source_boundary,
};

struct StokerMotelPromptOption {
    StokerMotelPromptOptionRole role{};
    // Key in dialogue/script-text.lua; no Genesis text-table address is
    // carried into the native conversion or Qt runtime.
    std::string_view text_alias;
    StokerMotelPromptCommand command = StokerMotelPromptCommand::source_boundary;
};

struct StokerMotelPromptView {
    StokerMotelPromptId id = StokerMotelPromptId::source_boundary;
    std::string_view title_alias;
    std::array<StokerMotelPromptOption, 3> options{};
    std::size_t option_count = 0;
    bool has_more = false;
    bool waits_for_button = true;
    bool returns_to_field = false;
    bool source_complete = true;
    std::string_view source_boundary_reason;
};

// Source-derived presentation records for the Stoker's Coffin Motel route.
// Sentinel records such as the ambush/package pages intentionally have zero
// selectable options and return to the field after the source wait path.
StokerMotelPromptView make_stoker_motel_prompt(StokerMotelPromptId id);

// Resolve only transitions proven by the prompt record and its dispatch
// command. A missing branch is explicit instead of becoming an invented NPC
// conversation.
std::optional<StokerMotelPromptId> stoker_motel_prompt_target(
    StokerMotelPromptId id, std::size_t option_index);

std::optional<StokerMotelPromptCommand> stoker_motel_prompt_command(
    StokerMotelPromptId id, std::size_t option_index);

} // namespace shadowrun::ghidra_conversion
