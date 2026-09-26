#include "shadowrun_ghidra/entities/entity_animation_link_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<EntityAnimationCandidateRecord, 7> candidates{
        EntityAnimationCandidateRecord{100U, 0x01U, 0U, 0U, 1},
        EntityAnimationCandidateRecord{101U, 0x41U, 0U, 0U, 1},
        EntityAnimationCandidateRecord{102U, 0x11U, 0U, 0U, 1},
        EntityAnimationCandidateRecord{103U, 0x01U, 0x80U, 0U, 1},
        EntityAnimationCandidateRecord{104U, 0x01U, 0U, 0x20U, 1},
        EntityAnimationCandidateRecord{105U, 0x01U, 0U, 0x08U, 1},
        EntityAnimationCandidateRecord{106U, 0x01U, 0U, 0U, 0},
    };
    if (!expect(collect_entity_action_primary_candidates(candidates) ==
                std::vector<std::uint32_t>{100U})) {
        return 1;
    }

    EntityAnimationLinkSelectionState primary{};
    primary.use_primary_table = true;
    primary.primary_targets = {11U, 22U, 33U};
    primary.linked_target = 22U;
    bool random_called = false;
    const auto kept = select_entity_animation_link(
        primary,
        [&random_called](const std::size_t) {
            random_called = true;
            return std::optional<std::size_t>{0U};
        },
        {});
    if (!expect(kept.action == EntityAnimationLinkSelectionAction::
                    existing_primary_target_kept &&
                kept.existing_target_found && !random_called &&
                primary.linked_target == std::optional<std::uint32_t>{22U})) {
        return 2;
    }

    primary.linked_target = 99U;
    const auto selected = select_entity_animation_link(
        primary,
        [](const std::size_t count) {
            return count == 3U ? std::optional<std::size_t>{2U}
                               : std::nullopt;
        },
        {});
    if (!expect(selected.action == EntityAnimationLinkSelectionAction::
                    primary_target_selected &&
                primary.linked_target == std::optional<std::uint32_t>{33U})) {
        return 3;
    }

    EntityAnimationLinkSelectionState empty_primary{};
    empty_primary.use_primary_table = true;
    bool reset_called = false;
    const auto cleared = select_entity_animation_link(
        empty_primary, {}, [&reset_called]() { reset_called = true; });
    if (!expect(cleared.action == EntityAnimationLinkSelectionAction::
                    primary_table_cleared &&
                reset_called && !empty_primary.linked_target.has_value())) {
        return 4;
    }

    EntityAnimationLinkSelectionState alternate{};
    alternate.alternate_targets = {44U, 55U};
    alternate.linked_target = 99U;
    const auto alternate_selected = select_entity_animation_link(
        alternate,
        [](const std::size_t count) {
            return count == 2U ? std::optional<std::size_t>{1U}
                               : std::nullopt;
        },
        {});
    if (!expect(alternate_selected.action == EntityAnimationLinkSelectionAction::
                    alternate_target_selected_with_delay &&
                alternate_selected.target_changed &&
                alternate.animation_delay == 6U &&
                alternate.linked_target == std::optional<std::uint32_t>{55U})) {
        return 5;
    }

    EntityAnimationLinkSelectionState empty_alternate{};
    empty_alternate.linked_target = 77U;
    const auto alternate_cleared =
        select_entity_animation_link(empty_alternate, {}, {});
    if (!expect(alternate_cleared.action == EntityAnimationLinkSelectionAction::
                    alternate_table_cleared &&
                !empty_alternate.linked_target.has_value())) {
        return 6;
    }

    return 0;
}
