#include "shadowrun_ghidra/field/stoker_motel_prompt_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::string_view kArrivalTitle = "stoker.motel.arrival.title";
constexpr std::string_view kMore = "stoker.motel.arrival.more";
constexpr std::string_view kManagerTitle = "stoker.motel.manager.title";
constexpr std::string_view kCrash = "stoker.motel.option.crash";
constexpr std::string_view kLeave = "stoker.motel.option.leave";
constexpr std::string_view kAmbushTitle =
    "stoker.motel.street-ambush.title";
constexpr std::string_view kRoomTitle = "stoker.motel.room.title";
constexpr std::string_view kPay = "stoker.motel.option.pay";
constexpr std::string_view kHandoffTitle =
    "stoker.motel.package-handoff.title";
constexpr std::string_view kQuestionTitle =
    "stoker.motel.package-question.title";
constexpr std::string_view kPackage = "stoker.motel.option.package";
constexpr std::string_view kLeaveTitle = "stoker.motel.leave.title";
constexpr std::string_view kDebtTitle = "stoker.motel.debt.title";
constexpr std::string_view kPaidTitle = "stoker.motel.room.paid.title";
constexpr std::string_view kInsufficientTitle =
    "stoker.motel.room.insufficient.title";
constexpr std::string_view kJumpHouseTitle =
    "stoker.motel.jump-house.title";
constexpr std::string_view kDebtPayment = "stoker.motel.option.debt-payment";
constexpr std::string_view kJumpHouse = "stoker.motel.option.jump-house";

void add_option(StokerMotelPromptView& view,
                const StokerMotelPromptOptionRole role,
                const std::string_view text_alias,
                const StokerMotelPromptCommand command) {
    if (view.option_count >= view.options.size()) {
        return;
    }
    view.options[view.option_count++] = {role, text_alias, command};
}

StokerMotelPromptView make_sentinel(const StokerMotelPromptId id,
                                    const std::string_view title_alias) {
    StokerMotelPromptView view;
    view.id = id;
    view.title_alias = title_alias;
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
        view.title_alias = kArrivalTitle;
        view.has_more = true;
        add_option(view, StokerMotelPromptOptionRole::continue_arrival, kMore,
                   StokerMotelPromptCommand::resolve_arrival);
        return view;

    case StokerMotelPromptId::manager_greeting:
        view.title_alias = kManagerTitle;
        add_option(view, StokerMotelPromptOptionRole::rent_room, kCrash,
                   StokerMotelPromptCommand::open_room_offer);
        add_option(view, StokerMotelPromptOptionRole::leave_motel, kLeave,
                   StokerMotelPromptCommand::open_leave_notice);
        return view;

    case StokerMotelPromptId::manager_greeting_with_package:
        view.title_alias = kManagerTitle;
        add_option(view, StokerMotelPromptOptionRole::rent_room, kCrash,
                   StokerMotelPromptCommand::open_room_offer);
        add_option(view, StokerMotelPromptOptionRole::leave_motel, kLeave,
                   StokerMotelPromptCommand::open_leave_notice);
        add_option(view, StokerMotelPromptOptionRole::collect_package, kPackage,
                   StokerMotelPromptCommand::open_debt_question);
        return view;

    case StokerMotelPromptId::room_offer:
        view.title_alias = kRoomTitle;
        add_option(view, StokerMotelPromptOptionRole::pay_room, kPay,
                   StokerMotelPromptCommand::pay_room);
        add_option(view, StokerMotelPromptOptionRole::leave_motel, kLeave,
                   StokerMotelPromptCommand::open_leave_notice);
        return view;

    case StokerMotelPromptId::debt_question:
        view.title_alias = kDebtTitle;
        add_option(view, StokerMotelPromptOptionRole::pay_michael_debt,
                   kDebtPayment, StokerMotelPromptCommand::pay_michael_debt);
        add_option(view, StokerMotelPromptOptionRole::ask_jump_house,
                   kJumpHouse, StokerMotelPromptCommand::open_jump_house_referral);
        return view;

    case StokerMotelPromptId::street_ambush:
        return make_sentinel(id, kAmbushTitle);
    case StokerMotelPromptId::package_handoff:
        return make_sentinel(id, kHandoffTitle);
    case StokerMotelPromptId::package_question:
        return make_sentinel(id, kQuestionTitle);
    case StokerMotelPromptId::leave_notice:
        return make_sentinel(id, kLeaveTitle);
    case StokerMotelPromptId::room_paid:
        return make_sentinel(id, kPaidTitle);
    case StokerMotelPromptId::room_insufficient:
        return make_sentinel(id, kInsufficientTitle);
    case StokerMotelPromptId::jump_house_referral:
        return make_sentinel(id, kJumpHouseTitle);
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
    case StokerMotelPromptId::manager_greeting:
    case StokerMotelPromptId::manager_greeting_with_package:
        if (option_index == 0U) {
            return StokerMotelPromptId::room_offer;
        }
        if (option_index == 1U) {
            return StokerMotelPromptId::leave_notice;
        }
        if (id == StokerMotelPromptId::manager_greeting_with_package &&
            option_index == 2U) {
            return StokerMotelPromptId::debt_question;
        }
        return std::nullopt;
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
