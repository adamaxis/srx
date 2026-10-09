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
    const auto package_manager = make_stoker_motel_prompt(
        StokerMotelPromptId::manager_greeting_with_package);
    if (!expect(!manager.source_complete && manager.option_count == 0U &&
                    !package_manager.source_complete &&
                    package_manager.option_count == 0U,
                "manager menu still has a duplicate C++ content definition")) {
        return 2;
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
        return 3;
    }

    const auto ambush = make_stoker_motel_prompt(
        StokerMotelPromptId::street_ambush);
    if (!expect(ambush.option_count == 0U && ambush.returns_to_field,
                "sentinel ambush prompt is incorrectly selectable")) {
        return 4;
    }
    return 0;
}
