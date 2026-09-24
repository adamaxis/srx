#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct CollisionEvaluation {
    std::int16_t result = 0;
    std::uint32_t packed_state = 0;
};

struct EntityCollisionRecord {
    std::uint32_t packed_state = 0;
};

using CollisionEvaluator = std::function<CollisionEvaluation(
    std::uint16_t candidate_x, std::uint16_t candidate_y)>;

// ROM 0xB48C: pass the candidate coordinates to the field-collision boundary,
// copy its four-byte collision state into the entity record, and return its
// collision code. The boundary remains an explicit native service interface;
// this conversion does not recreate the ROM collision table or hardware.
std::int16_t evaluate_entity_collision(
    EntityCollisionRecord& entity,
    std::uint16_t candidate_x,
    std::uint16_t candidate_y,
    const CollisionEvaluator& collision_evaluator);

struct CollisionCandidate {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t flags = 0;
    bool collision_enabled = false;
    std::uint8_t interaction_flags = 0;
    std::uint8_t behavior_flags = 0;
};

struct EntityCollisionSearchContext {
    std::uint16_t probe_x = 0;
    std::uint16_t probe_y = 0;
    std::uint8_t source_flags = 0;
    bool source_collision_override = false;
    bool collision_gate_active = false;
    std::optional<std::size_t> excluded_candidate;
};

struct EntityCollisionSearchResult {
    std::optional<std::size_t> candidate_index;
    bool source_position_updated = false;
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
};

// ROM 0x15ABA: scan the ten source entity records for an active, collision-
// enabled candidate in the 0x90 half-open X/Y window. Collision-mode and
// source-override exits are represented as ordinary typed policy fields.
EntityCollisionSearchResult find_colliding_entity(
    const EntityCollisionSearchContext& context,
    const std::vector<CollisionCandidate>& candidates);

} // namespace shadowrun::ghidra_conversion
