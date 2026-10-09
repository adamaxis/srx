#include "shadowrun_ghidra/field/stoker_motel_prompt_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void add_option(StokerMotelPromptView& view,
                const StokerMotelPromptOptionRole role,
                const std::string_view choice_id,
                const StokerMotelPromptCommand command) {
    if (view.option_count >= view.options.size()) {
        return;
    }
    view.options[view.option_count++] = {role, choice_id, command};
}

StokerMotelPromptView make_sentinel(const StokerMotelPromptId id,
                                    const std::string_view event_id) {
    StokerMotelPromptView view;
    view.id = id;
    view.event_id = event_id;
    view.returns_to_field = true;
    return view;
}

} // namespace

StokerMotelPromptView make_stoker_motel_prompt(
    const StokerMotelPromptId id) {
    StokerMotelPromptView view;
    view.id = id;
    switch (id) {
    case StokerMotelPromptId::arrival:
        view.event_id = "arrival";
        view.has_more = true;
        add_option(view, StokerMotelPromptOptionRole::continue_arrival, "more",
                   StokerMotelPromptCommand::resolve_arrival);
        return view;

    case StokerMotelPromptId::manager_greeting:
    case StokerMotelPromptId::manager_greeting_with_package:
        view.source_complete = false;
        view.source_boundary_reason =
            "manager menu is defined by the gameplay script package";
        return view;

    case StokerMotelPromptId::room_offer:
        view.menu_id = "room";
        add_option(view, StokerMotelPromptOptionRole::pay_room, "pay",
                   StokerMotelPromptCommand::pay_room);
        add_option(view, StokerMotelPromptOptionRole::leave_motel, "leave",
                   StokerMotelPromptCommand::open_leave_notice);
        return view;

    case StokerMotelPromptId::debt_question:
        view.menu_id = "debt";
        add_option(view, StokerMotelPromptOptionRole::pay_michael_debt,
                   "debt-payment",
                   StokerMotelPromptCommand::pay_michael_debt);
        add_option(view, StokerMotelPromptOptionRole::ask_jump_house,
                   "jump-house",
                   StokerMotelPromptCommand::open_jump_house_referral);
        return view;

    case StokerMotelPromptId::street_ambush:
        return make_sentinel(id, "street-ambush");
    case StokerMotelPromptId::package_handoff:
        return make_sentinel(id, "package-handoff");
    case StokerMotelPromptId::package_question:
        return make_sentinel(id, "package-question");
    case StokerMotelPromptId::leave_notice:
        return make_sentinel(id, "leave");
    case StokerMotelPromptId::room_paid:
        return make_sentinel(id, "room.paid");
    case StokerMotelPromptId::room_insufficient:
        return make_sentinel(id, "room.insufficient");
    case StokerMotelPromptId::jump_house_referral:
        return make_sentinel(id, "jump-house");
    case StokerMotelPromptId::source_boundary:
        view.source_complete = false;
        view.source_boundary_reason =
            "Stoker prompt command returns to an unconverted source record";
        return view;
    }
    return view;
}

std::optional<StokerMotelPromptId> stoker_motel_prompt_target(
    const StokerMotelPromptId id, const std::size_t option_index) {
    switch (id) {
    case StokerMotelPromptId::room_offer:
        if (option_index == 1U) {
            return StokerMotelPromptId::leave_notice;
        }
        return std::nullopt;
    case StokerMotelPromptId::debt_question:
        if (option_index == 1U) {
            return StokerMotelPromptId::jump_house_referral;
        }
        // The 0x05 debt-payment branch continues through the source package
        // sequence; its complete post-payment record is still a boundary.
        if (option_index == 0U) {
            return StokerMotelPromptId::source_boundary;
        }
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::optional<StokerMotelPromptCommand> stoker_motel_prompt_command(
    const StokerMotelPromptId id, const std::size_t option_index) {
    const auto view = make_stoker_motel_prompt(id);
    if (option_index >= view.option_count) {
        return std::nullopt;
    }
    return view.options[option_index].command;
}

} // namespace shadowrun::ghidra_conversion
