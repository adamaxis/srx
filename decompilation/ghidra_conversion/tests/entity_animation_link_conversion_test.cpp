#include "shadowrun_ghidra/entity_animation_link_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

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
        return 1;
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
        return 2;
    }

    EntityAnimationLinkSelectionState empty_primary{};
    empty_primary.use_primary_table = true;
    bool reset_called = false;
    const auto cleared = select_entity_animation_link(
        empty_primary, {}, [&reset_called]() { reset_called = true; });
    if (!expect(cleared.action == EntityAnimationLinkSelectionAction::
                    primary_table_cleared &&
                reset_called && !empty_primary.linked_target.has_value())) {
        return 3;
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
        return 4;
    }

    EntityAnimationLinkSelectionState empty_alternate{};
    empty_alternate.linked_target = 77U;
    const auto alternate_cleared =
        select_entity_animation_link(empty_alternate, {}, {});
    if (!expect(alternate_cleared.action == EntityAnimationLinkSelectionAction::
                    alternate_table_cleared &&
                !empty_alternate.linked_target.has_value())) {
        return 5;
    }

    return 0;
}
