#pragma once

#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityEffectSpawnParent {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t animation_counter = 0;
    std::uint8_t field_0x56 = 0;
    std::uint8_t field_0x57 = 0;
    std::uint8_t field_0x87 = 0;
    std::uint8_t field_0x9D = 0;
};

struct EntityEffectSpawnTarget {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
};

struct EntityEffectSpawnTargetResolution {
    EntityEffectSpawnTarget target{};
    std::uint8_t source_marker = 0;
};

struct EntityEffectSpawnRecord {
    EntityBehaviorRecordState behavior_defaults{};
    std::uint16_t entity_slot = 0; // source record +0x3E
    std::uint16_t world_x = 0; // source record +0x00
    std::uint16_t world_y = 0; // source record +0x04
    std::uint16_t heading = 0; // source record +0x2E
    std::uint16_t movement_scale = 0; // source record +0x2C
    std::uint16_t animation_resource = 0; // source record +0x16
    std::uint16_t effect_slot = 0; // source record +0x30
    std::uint16_t effect_record_parameter = 0; // source record +0x20
    std::uint16_t effect_auxiliary_value = 0; // source record +0x26
    std::uint16_t effect_distance_steps = 0; // source record +0x40
    std::int16_t movement_offset = 0; // source record +0x36
    std::uint16_t vertical_velocity = 0; // source record +0x34
    std::uint16_t lifetime = 0; // source record +0x46
    std::uint16_t directional_probe_x = 0; // source scratch FFDD60
    std::uint16_t directional_probe_y = 0; // source scratch FFDD62
    std::int16_t heading_offset_x = 0; // source record +0x4C
    std::int16_t heading_offset_y = 0; // source record +0x4E
    std::uint8_t behavior_type = 0; // source record +0x0E
    std::uint8_t field_0x56 = 0;
    std::uint8_t field_0x57 = 0;
    std::uint8_t field_0x9D = 0;
    std::uint8_t source_marker = 0; // source record +0x9F
    bool active = false; // source record +0x10 bit 0
    std::uint32_t continuation_address = 0; // source record +0x3A
};

using EntityEffectSpawnEntityAllocator =
    std::function<std::optional<std::uint16_t>()>;
using EntityEffectSpawnMeterAdvance = std::function<void(std::int16_t)>;
using EntityEffectSpawnTimeout = std::function<void()>;
using EntityEffectSpawnTargetTransform = std::function<
    std::optional<EntityEffectSpawnTargetResolution>(
        const EntityEffectSpawnTarget&, std::uint16_t heading)>;

struct EntityEffectSpawnCallbacks {
    EntityEffectSpawnEntityAllocator allocate_entity_slot;
    EntityEffectSpawnMeterAdvance advance_asset_meter;
    EntityEffectSpawnTimeout schedule_world_timeout;
    EntityEffectSpawnTargetTransform transform_target;
};

enum class EntityEffectSpawnAction {
    animation_window_expired,
    no_entity_slot,
    effect_table_full,
    target_transform_unresolved,
    spawned_without_target,
    spawned_toward_target,
};

struct EntityEffectSpawnResult {
    EntityEffectSpawnAction action =
        EntityEffectSpawnAction::animation_window_expired;
    bool source_carry = false;
    std::optional<EntityEffectSpawnRecord> record;
};

// ROM 0x15646: create the source's short-lived entity/effect record while the
// parent animation counter is within its 0x3C-frame window. The host exposes
// entity allocation, timeout scheduling, and the linked-target transform as
// typed boundaries; it does not recreate source pointers or effect memory.
EntityEffectSpawnResult spawn_entity_effect_record(
    const EntityEffectSpawnParent& parent,
    std::optional<EntityEffectSpawnTarget> linked_target,
    std::array<std::uint8_t, 18>& effect_slots,
    std::uint16_t effect_auxiliary_value,
    const EntityEffectSpawnCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
