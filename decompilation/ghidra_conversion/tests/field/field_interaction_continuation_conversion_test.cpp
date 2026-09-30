#include "shadowrun_ghidra/field/field_interaction_continuation_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    {
        FieldInteractionContinuationState state{};
        state.interaction_selection = 2U;
        const auto plan = prepare_field_interaction_prompt(state);
        assert(plan.prompt == FieldInteractionPrompt::electromagnetic_lock_door);
        assert(plan.preparation ==
               FieldInteractionPreparation::write_electromagnetic_lock_choice_grid);
        assert(plan.display_prepared);
        assert(!plan.optional_resource_pending);

        const auto grid = prepare_electromagnetic_lock_choice_grid(
            state.interaction_selection);
        assert(grid.prompt == FieldInteractionPrompt::electromagnetic_lock_door);
        assert(grid.choice_grid_index == 0U);
        assert(grid.display_prepared);
    }

    {
        const auto grid = prepare_electromagnetic_lock_choice_grid(5U);
        assert(grid.prompt == FieldInteractionPrompt::electromagnetic_lock_door);
        assert(grid.choice_grid_index == 3U);
    }

    {
        FieldInteractionContinuationState state{};
        state.interaction_suppressed = true;
        state.interaction_selection = 4U;
        const auto plan = prepare_field_interaction_prompt(state);
        assert(plan.prompt ==
               FieldInteractionPrompt::electromagnetic_lock_resolution);
        assert(plan.preparation ==
               FieldInteractionPreparation::prepare_electromagnetic_lock_resolution);
        assert(plan.display_prepared);
        assert(state.interaction_selection == 0xFFU);
    }

    {
        const auto fallback = prepare_electromagnetic_lock_choice_grid(1U);
        assert(fallback.prompt ==
               FieldInteractionPrompt::building_security_terminal);
        assert(fallback.preparation == FieldInteractionPreparation::none);
        assert(!fallback.display_prepared);
        assert(fallback.optional_resource_pending);
    }

    return 0;
}
