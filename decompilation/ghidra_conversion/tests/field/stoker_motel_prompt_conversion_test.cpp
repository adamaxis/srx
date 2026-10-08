#include "shadowrun_ghidra/field/stoker_motel_prompt_conversion.hpp"

#include <iostream>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    StokerMotelStoryState story_state{};
    if (!expect(resolve_stoker_motel_story_entry(story_state) ==
                    StokerMotelStoryEntryAction::opening_dialogue &&
                    story_state.opening_dialogue_seen,
                "first Stoker visit did not select its opening dialogue") ||
        !expect(resolve_stoker_motel_story_entry(story_state) ==
                    StokerMotelStoryEntryAction::package_offer,
                "repeat Stoker visit lost the Michael package offer")) {
        return 6;
    }
    story_state.michael_debt_paid = true;
    if (!expect(resolve_stoker_motel_story_entry(story_state) ==
                    StokerMotelStoryEntryAction::package_offer,
                "paid debt skipped the still-uncollected package")) {
        return 7;
    }
    story_state.michael_package_collected = true;
    if (!expect(resolve_stoker_motel_story_entry(story_state) ==
                    StokerMotelStoryEntryAction::ordinary_motel,
                "Stoker did not return to ordinary motel service after collection")) {
        return 8;
    }

    const auto arrival = make_stoker_motel_prompt(
        StokerMotelPromptId::arrival);
    if (!expect(arrival.title_alias == "stoker.motel.arrival.title" &&
                    arrival.has_more && arrival.option_count == 1U &&
                    arrival.options[0].text_alias ==
                        "stoker.motel.arrival.more" &&
                    arrival.options[0].command ==
                        StokerMotelPromptCommand::resolve_arrival,
                "arrival prompt is not source-shaped") ||
        !expect(!stoker_motel_prompt_target(
                     StokerMotelPromptId::arrival, 0U)
                     .has_value(),
                "arrival random command was collapsed into a guessed target")) {
        return 1;
    }

    const auto manager = make_stoker_motel_prompt(
        StokerMotelPromptId::manager_greeting);
    if (!expect(manager.option_count == 2U &&
                    manager.options[0].text_alias ==
                        "stoker.motel.option.crash" &&
                    manager.options[1].text_alias ==
                        "stoker.motel.option.leave" &&
                    stoker_motel_prompt_target(
                        StokerMotelPromptId::manager_greeting, 0U) ==
                        StokerMotelPromptId::room_offer &&
                    stoker_motel_prompt_target(
                        StokerMotelPromptId::manager_greeting, 1U) ==
                        StokerMotelPromptId::leave_notice,
                "default manager prompt graph is wrong")) {
        return 2;
    }

    const auto package_manager = make_stoker_motel_prompt(
        StokerMotelPromptId::manager_greeting_with_package);
    if (!expect(package_manager.option_count == 3U &&
                    package_manager.options[2].role ==
                        StokerMotelPromptOptionRole::collect_package &&
                    stoker_motel_prompt_target(
                        StokerMotelPromptId::manager_greeting_with_package, 2U) ==
                        StokerMotelPromptId::debt_question,
                "package manager prompt did not expose its third option")) {
        return 3;
    }

    const auto debt = make_stoker_motel_prompt(
        StokerMotelPromptId::debt_question);
    if (!expect(debt.option_count == 2U &&
                    stoker_motel_prompt_command(
                        StokerMotelPromptId::debt_question, 0U) ==
                        StokerMotelPromptCommand::pay_michael_debt &&
                    stoker_motel_prompt_target(
                        StokerMotelPromptId::debt_question, 0U) ==
                        StokerMotelPromptId::source_boundary &&
                    stoker_motel_prompt_target(
                        StokerMotelPromptId::debt_question, 1U) ==
                        StokerMotelPromptId::jump_house_referral,
                "debt prompt lost its source command branches")) {
        return 4;
    }

    const auto ambush = make_stoker_motel_prompt(
        StokerMotelPromptId::street_ambush);
    if (!expect(ambush.option_count == 0U && ambush.returns_to_field,
                "sentinel ambush prompt is incorrectly selectable")) {
        return 5;
    }
    return 0;
}
