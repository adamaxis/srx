#include "shadowrun_ghidra/field/field_interaction_continuation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldInteractionPromptPlan
prepare_field_interaction_prompt(
    FieldInteractionContinuationState& state) {
    // 0x20E48 begins by setting FFF10C to FF.  The common service therefore
    // invokes the callback selected below before presenting the prompt.
    FieldInteractionPromptPlan plan{};
    plan.display_prepared = true;

    if (state.interaction_suppressed) {
        // The source writes FF to FFE16E before selecting the alternate
        // descriptor at 0xF4874 and the callback at 0x21616.
        state.interaction_selection = 0xFFU;
        plan.prompt = FieldInteractionPrompt::electromagnetic_lock_resolution;
        plan.preparation =
            FieldInteractionPreparation::prepare_electromagnetic_lock_resolution;
        return plan;
    }

    // The fall-through branch at 0x20E7A selects the ordinary doorway
    // descriptor at 0x11CCCE and the 0x20E92 choice-grid callback.
    plan.prompt = FieldInteractionPrompt::electromagnetic_lock_door;
    plan.preparation =
        FieldInteractionPreparation::write_electromagnetic_lock_choice_grid;
    return plan;
}

FieldInteractionPromptPlan
prepare_electromagnetic_lock_choice_grid(
    const std::uint8_t interaction_selection) {
    // 0x20E92 uses a signed BMI after subtracting two.  Express the same
    // source boundary without relying on byte underflow in host state.
    if (interaction_selection < 2U) {
        return prepare_building_security_terminal_prompt();
    }

    const auto choice_grid_index =
        static_cast<std::uint8_t>(interaction_selection - 2U);

    FieldInteractionPromptPlan plan{};
    plan.prompt = FieldInteractionPrompt::electromagnetic_lock_door;
    plan.preparation =
        FieldInteractionPreparation::write_electromagnetic_lock_choice_grid;
    plan.display_prepared = true;
    plan.choice_grid_index = choice_grid_index;
    return plan;
}

FieldInteractionPromptPlan prepare_building_security_terminal_prompt() {
    FieldInteractionPromptPlan plan{};
    plan.prompt = FieldInteractionPrompt::building_security_terminal;
    plan.preparation = FieldInteractionPreparation::none;
    plan.display_prepared = false;
    plan.optional_resource_pending = true;
    return plan;
}

} // namespace shadowrun::ghidra_conversion
