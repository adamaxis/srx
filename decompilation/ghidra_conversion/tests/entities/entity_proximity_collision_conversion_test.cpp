#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<EntityProximityRecord> records(3U);
    records[0].x = 1000U;
    records[0].y = 1000U;
    records[0].behavior_value = 0x1234U;
    records[1].x = 1050U;
    records[1].y = 1030U;
    records[1].entity_flags = 0x01U;
    records[1].behavior_value = 0x4321U;
    records[2].x = 1300U;
    records[2].y = 1000U;
    records[2].entity_flags = 0x01U;

    int pair_calls = 0;
    const auto proximity = resolve_entity_proximity_interactions(
        records, 0U,
        {[&](EntityProximityRecord&, EntityProximityRecord&) { ++pair_calls; },
         {}});
    if (proximity.pairs_examined != 2U || proximity.pairs_resolved != 1U ||
        pair_calls != 1 || (records[0].collision_flags & 0x02U) == 0U ||
        (records[1].collision_flags & 0x02U) == 0U ||
        records[1].interaction_value != 0x1234U ||
        records[0].interaction_value != 0x4321U) {
        return 1;
    }

    records[0].primary_link = 1U;
    records[1].x = 1200U;
    records[1].y = 1000U;
    records[1].collision_flags = 0U;
    records[0].collision_flags = 0U;
    const auto linked = resolve_entity_proximity_interactions(records, 0U);
    if (linked.pairs_resolved != 1U) {
        return 2;
    }

    records[0].entity_flags = 0x40U;
    if (!resolve_entity_proximity_interactions(records, 0U)
             .skipped_current_record) {
        return 3;
    }

    records[0].entity_flags = 0U;
    records[0].primary_link.reset();
    records[1].x = 1050U;
    records[1].y = 1030U;
    auto stable = records;
    stable[0].collision_flags = 0U;
    stable[1].collision_flags = 0U;
    resolve_entity_proximity_interactions(
        stable, 0U,
        {{}, [] { return static_cast<std::uint16_t>(0x7000U); }});
    records[0].collision_flags = 0U;
    records[1].collision_flags = 0U;
    const auto rotated = resolve_entity_proximity_interactions(
        records, 0U,
        {{}, [] { return static_cast<std::uint16_t>(0); }});
    if (rotated.pairs_resolved != 1U ||
        records[1].interaction_value != stable[1].interaction_value ||
        records[0].interaction_value != stable[0].interaction_value ||
        records[1].interaction_heading !=
            static_cast<std::uint16_t>(stable[1].interaction_heading + 0x4000U) ||
        records[0].interaction_heading !=
            static_cast<std::uint16_t>(stable[0].interaction_heading + 0xC000U)) {
        return 6;
    }

    EntityPairCollisionEntity current{};
    current.entity_flags = 0x02U;
    EntityPairCollisionEntity candidate{};
    candidate.special_flags = 0x10U;
    EntityPairCollisionWorldState special_world{};
    special_world.scene_activity_id = 10U;
    special_world.field_mode = 6U;
    int special_calls = 0;
    EntityPairActor selected_actor = EntityPairActor::current;
    const auto special = resolve_entity_pair_collision_event(
        current, candidate, special_world,
        {[&](const EntityPairActor actor) {
             ++special_calls;
             selected_actor = actor;
         },
         {}});
    if (!special.special_activity_requested || special_calls != 1 ||
        selected_actor != EntityPairActor::candidate ||
        special.handler != EntityPairActivityHandler::special_mode_six_collision ||
        special.activity_variant != 2U || !special_world.world_activity_started) {
        return 4;
    }

    current = {};
    current.entity_flags = 0x02U;
    candidate = {};
    candidate.special_flags = 0x08U;
    EntityPairCollisionWorldState normal_world{};
    normal_world.source_transfer_value = 0x12345678U;
    normal_world.world_effect_active = true;
    int normal_calls = 0;
    const auto normal = resolve_entity_pair_collision_event(
        current, candidate, normal_world,
        {{}, [&] { ++normal_calls; }});
    if (!normal.normal_activity_requested || normal.flagged_entity !=
            std::optional<EntityPairActor>(EntityPairActor::candidate) ||
        (candidate.activity_flags & 0x80U) == 0U || normal_calls != 1 ||
        normal.handler != EntityPairActivityHandler::world_effect_collision ||
        normal_world.world_effect_active ||
        normal_world.published_transfer_value != 0x12345678U) {
        return 5;
    }

    return 0;
}
