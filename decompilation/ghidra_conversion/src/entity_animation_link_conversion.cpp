#include "shadowrun_ghidra/entity_animation_link_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

bool contains_target(const std::vector<std::uint32_t>& targets,
                     const std::optional<std::uint32_t>& target) {
    return target.has_value() &&
           std::find(targets.begin(), targets.end(), *target) != targets.end();
}

std::optional<std::uint32_t> choose_target(
    const std::vector<std::uint32_t>& targets,
    const EntityAnimationRandomIndexSelector& select_random_index) {
    if (targets.empty() || !select_random_index) {
        return std::nullopt;
    }
    const auto index = select_random_index(targets.size());
    if (!index.has_value() || *index >= targets.size()) {
        return std::nullopt;
    }
    return targets[*index];
}

} // namespace

EntityAnimationLinkSelectionResult select_entity_animation_link(
    EntityAnimationLinkSelectionState& state,
    const EntityAnimationRandomIndexSelector& select_random_index,
    const EntityAnimationServiceRecordReset& reset_service_records) {
    EntityAnimationLinkSelectionResult result{};

    const auto& targets = state.use_primary_table ? state.primary_targets
                                                  : state.alternate_targets;
    if (state.use_primary_table) {
        if (targets.empty()) {
            if (!state.service_records_active && reset_service_records) {
                reset_service_records();
            }
            state.linked_target.reset();
            result.action =
                EntityAnimationLinkSelectionAction::primary_table_cleared;
            return result;
        }

        if (contains_target(targets, state.linked_target)) {
            result.existing_target_found = true;
            result.action = EntityAnimationLinkSelectionAction::
                existing_primary_target_kept;
            return result;
        }

        const auto selected = choose_target(targets, select_random_index);
        if (!selected.has_value()) {
            return result;
        }
        state.linked_target = selected;
        result.target_changed = true;
        result.action =
            EntityAnimationLinkSelectionAction::primary_target_selected;
        return result;
    }

    if (targets.empty()) {
        state.linked_target.reset();
        result.action =
            EntityAnimationLinkSelectionAction::alternate_table_cleared;
        return result;
    }

    if (contains_target(targets, state.linked_target)) {
        result.existing_target_found = true;
        result.action = EntityAnimationLinkSelectionAction::
            existing_alternate_target_kept;
        return result;
    }

    const auto selected = choose_target(targets, select_random_index);
    if (!selected.has_value()) {
        return result;
    }
    result.target_changed = state.linked_target != selected;
    state.linked_target = selected;
    if (result.target_changed) {
        state.animation_delay = 6U;
        result.action = EntityAnimationLinkSelectionAction::
            alternate_target_selected_with_delay;
    } else {
        result.action =
            EntityAnimationLinkSelectionAction::alternate_target_selected;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
