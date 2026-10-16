#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationCandidateRecord {
    std::uint32_t identity = 0;
    std::uint8_t behavior_flags = 0; // record +0x10
    std::uint8_t route_flags = 0; // record +0xB0
    std::uint8_t status_flags = 0; // record +0x9E
    std::int16_t collision_state = 0; // record +0x0C
};

// ROM 0x1438C: scan the seven primary field records and retain identities
// eligible for the action/animation target table at FFDF96.
std::vector<std::uint32_t> collect_entity_action_primary_candidates(
    const std::array<EntityAnimationCandidateRecord, 7>& entities);

// The ROM stores these as pointers in FFDF96/FFDFCA.  The conversion keeps
// the identity of the selected record without making the host runtime depend
// on a ROM address or a bus-backed pointer.
struct EntityAnimationLinkSelectionState {
    bool use_primary_table = false; // record +0x10 bit 3
    bool service_records_active = false; // FFE162
    std::optional<std::uint32_t> linked_target;
    std::vector<std::uint32_t> primary_targets; // FFDF96
    std::vector<std::uint32_t> alternate_targets; // FFDFCA
    std::uint8_t animation_delay = 0; // record +0xB5
};

enum class EntityAnimationLinkSelectionAction {
    primary_table_cleared,
    existing_primary_target_kept,
    primary_target_selected,
    alternate_table_cleared,
    existing_alternate_target_kept,
    alternate_target_selected,
    alternate_target_selected_with_delay,
    random_selection_unresolved,
};

using EntityAnimationRandomIndexSelector =
    std::function<std::optional<std::size_t>(std::size_t count)>;
using EntityAnimationServiceRecordReset = std::function<void()>;

struct EntityAnimationLinkSelectionResult {
    EntityAnimationLinkSelectionAction action =
        EntityAnimationLinkSelectionAction::random_selection_unresolved;
    bool existing_target_found = false;
    bool target_changed = false;
};

// ROM 0x14492: refresh the linked record for an entity.  The two source
// pointer tables are represented as identity lists; the random picker and
// service-record reset remain explicit host boundaries.
EntityAnimationLinkSelectionResult select_entity_animation_link(
    EntityAnimationLinkSelectionState& state,
    const EntityAnimationRandomIndexSelector& select_random_index,
    const EntityAnimationServiceRecordReset& reset_service_records);

} // namespace shadowrun::ghidra_conversion
