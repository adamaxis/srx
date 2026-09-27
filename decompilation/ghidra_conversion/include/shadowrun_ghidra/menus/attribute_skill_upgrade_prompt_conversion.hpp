#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class AttributeSkillUpgradePromptOutcome {
    accepted,
    declined,
};

struct AttributeSkillUpgradePromptResult {
    AttributeSkillUpgradePromptOutcome outcome =
        AttributeSkillUpgradePromptOutcome::declined;
    // ROM FFF0DC: option 0 accepts the pending upgrade; option 1 declines it.
    std::uint8_t selected_option = 0;
};

struct AttributeSkillUpgradePromptCallbacks {
    // ROM 0xD53C renders the already-calculated upgrade/cost heading and then
    // the two-choice selection text before it publishes the initial option.
    std::function<void()> render_upgrade_cost_prompt;
    std::function<void()> render_upgrade_choices;
    std::function<void(std::uint8_t)> publish_selection_cursor;

    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
};

// ROM 0x0D53C: prompt for the pending Attributes/Skills Karma upgrade. The
// source starts on option 0, wraps the two choices through 0xC376 on bits
// 8/9, and closes on confirm/cancel/mode-action bits 12/13/14. Directional
// input has priority when it arrives in the same sample as an exit bit.
AttributeSkillUpgradePromptResult run_attribute_skill_upgrade_prompt(
    const AttributeSkillUpgradePromptCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
