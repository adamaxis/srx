#include "shadowrun_ghidra/entity_action_math_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

}  // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionRecord entity{};
    entity.item18_base = 7U;
    entity.item_codes[1] = 0x18U;
    entity.item_phases[1] = 3U;
    entity.item_codes[6] = 0x18U;
    entity.item_phases[6] = 2U;
    if (!expect(calculate_item18_skill_value(entity) == 9U)) {
        return 1;
    }
    entity.item_codes[6] = 0U;
    if (!expect(calculate_item18_skill_value(entity) == 10U)) {
        return 2;
    }

    if (!expect(select_posture_adjusted_value(20U, 2U, true) == 10U &&
                select_posture_adjusted_value(20U, 2U, false) == 10U &&
                select_posture_adjusted_value(9U, 1U, true) == 2U &&
                select_posture_adjusted_value(9U, 1U, false) == 7U)) {
        return 3;
    }

    entity.item_codes[6] = 0x1AU;
    entity.item_phases[6] = 4U;
    entity.item_codes[2] = 0x1AU;
    entity.item_phases[2] = 9U;
    if (!expect(add_first_matching_item_phase(5U, entity, 0x1AU) == 9U &&
                add_first_matching_item_phase(5U, entity, 0x1BU) == 5U)) {
        return 4;
    }

    if (!expect(add_wound_difficulty_penalty(4U, 0U, 4U) == 8U &&
                add_wound_difficulty_penalty(4U, 5U, 7U) == 6U &&
                add_wound_difficulty_penalty(4U, 8U, 10U) == 4U)) {
        return 5;
    }

    EntityActionTarget target{};
    entity.cyberware_flags = 0x0002U;
    target.class_code = 0x16U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 5U)) {
        return 6;
    }
    entity.cyberware_flags = 0U;
    entity.race_or_state = 0U;
    target.class_code = 0U;
    target.flags = 0x80U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 8U)) {
        return 7;
    }
    target.flags = 0U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 4U)) {
        return 8;
    }

    const EntityActionCatalogRecord action{1U, 10};
    const auto residual = calculate_action_residual(6U, 5U, 8U, action);
    if (!expect(residual.valid && residual.effective_difficulty == 4U &&
                residual.adjusted_roll_count == 2U &&
                residual.residual_effect == 8)) {
        return 9;
    }
    const auto hard_residual = calculate_action_residual(6U, 14U, 8U,
                                                          {0U, 10});
    if (!expect(hard_residual.effective_difficulty == 7U &&
                hard_residual.adjusted_roll_count == 0U &&
                hard_residual.residual_effect == 10)) {
        return 10;
    }
    if (!expect(!calculate_action_residual(0U, 4U, 8U, action).valid)) {
        return 11;
    }

    return 0;
}
