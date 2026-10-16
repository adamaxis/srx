#include "shadowrun_ghidra/menus/attribute_skill_upgrade_prompt_conversion.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint16_t kOptionDecrement = 0x0100U;
constexpr std::uint16_t kOptionIncrement = 0x0200U;
constexpr std::uint16_t kConfirm = 0x1000U;
constexpr std::uint16_t kModeAction = 0x2000U;
constexpr std::uint16_t kCancel = 0x4000U;

} // namespace

AttributeSkillUpgradePromptResult run_attribute_skill_upgrade_prompt(
    const AttributeSkillUpgradePromptCallbacks& callbacks) {
    AttributeSkillUpgradePromptResult result{};
    if (callbacks.render_upgrade_cost_prompt) {
        callbacks.render_upgrade_cost_prompt();
    }
    if (callbacks.render_upgrade_choices) {
        callbacks.render_upgrade_choices();
    }
    if (callbacks.publish_selection_cursor) {
        callbacks.publish_selection_cursor(result.selected_option);
    }

    for (;;) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        const auto input = callbacks.sample_controller_input
                               ? callbacks.sample_controller_input()
                               : std::uint16_t{0};

        if ((input & kOptionDecrement) != 0U) {
            result.selected_option = move_wrapped_menu_cursor(
                result.selected_option, -1, 1U);
            if (callbacks.publish_selection_cursor) {
                callbacks.publish_selection_cursor(result.selected_option);
            }
            continue;
        }
        if ((input & kOptionIncrement) != 0U) {
            result.selected_option = move_wrapped_menu_cursor(
                result.selected_option, 1, 1U);
            if (callbacks.publish_selection_cursor) {
                callbacks.publish_selection_cursor(result.selected_option);
            }
            continue;
        }
        if ((input & (kConfirm | kModeAction | kCancel)) != 0U) {
            result.outcome = result.selected_option == 0U
                                 ? AttributeSkillUpgradePromptOutcome::accepted
                                 : AttributeSkillUpgradePromptOutcome::declined;
            return result;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
