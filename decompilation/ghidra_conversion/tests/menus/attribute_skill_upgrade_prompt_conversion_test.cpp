#include "shadowrun_ghidra/menus/attribute_skill_upgrade_prompt_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint16_t> inputs{0x0100U, 0x1000U};
    std::size_t input_index = 0;
    std::vector<std::string> events;
    std::vector<std::uint8_t> published;
    AttributeSkillUpgradePromptCallbacks callbacks{};
    callbacks.render_upgrade_cost_prompt = [&] {
        events.emplace_back("cost");
    };
    callbacks.render_upgrade_choices = [&] {
        events.emplace_back("choices");
    };
    callbacks.publish_selection_cursor = [&](const std::uint8_t option) {
        events.emplace_back("cursor");
        published.push_back(option);
    };
    callbacks.wait_for_vblank = [&] { events.emplace_back("vblank"); };
    callbacks.latch_controller_events = [&] { events.emplace_back("latch"); };
    callbacks.sample_controller_input = [&] { return inputs[input_index++]; };

    const auto wrapped_to_decline = run_attribute_skill_upgrade_prompt(callbacks);
    if (!expect(wrapped_to_decline.outcome ==
                    AttributeSkillUpgradePromptOutcome::declined &&
                wrapped_to_decline.selected_option == 1U &&
                published == std::vector<std::uint8_t>{0U, 1U} &&
                events == std::vector<std::string>{
                              "cost", "choices", "cursor", "vblank", "latch",
                              "cursor", "vblank", "latch"})) {
        return 1;
    }

    // ROM checks bits 8/9 before bits 12/13/14. A simultaneous movement and
    // mode-action sample therefore moves to option 1 and stays in the loop.
    inputs = {0x0100U | 0x2000U, 0x4000U};
    input_index = 0;
    published.clear();
    AttributeSkillUpgradePromptCallbacks priority_callbacks{};
    priority_callbacks.sample_controller_input =
        [&] { return inputs[input_index++]; };
    priority_callbacks.publish_selection_cursor =
        [&](const std::uint8_t option) { published.push_back(option); };
    const auto declined = run_attribute_skill_upgrade_prompt(priority_callbacks);
    if (!expect(declined.outcome ==
                    AttributeSkillUpgradePromptOutcome::declined &&
                declined.selected_option == 1U &&
                published == std::vector<std::uint8_t>{0U, 1U})) {
        return 2;
    }

    inputs = {0x2000U};
    input_index = 0;
    const auto mode_action = run_attribute_skill_upgrade_prompt(
        AttributeSkillUpgradePromptCallbacks{
            {}, {}, {}, {}, {}, [&] { return inputs[input_index++]; }});
    if (!expect(mode_action.outcome ==
                    AttributeSkillUpgradePromptOutcome::accepted &&
                mode_action.selected_option == 0U)) {
        return 3;
    }
    return 0;
}
