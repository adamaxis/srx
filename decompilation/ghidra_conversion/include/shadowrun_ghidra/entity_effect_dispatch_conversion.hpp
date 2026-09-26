#pragma once

#include "shadowrun_ghidra/entity_state_primitives_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The source record at +0xA2 is a linked entity pointer. The host keeps its
// stable entity index instead of carrying a Genesis address through the
// effect path.
struct EntityEffectDispatchSource {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t behavior_type = 0; // source record +0x57
    std::uint8_t source_marker = 0; // source record +0x9D
    std::uint16_t source_entity_id = 0;
};

struct EntityEffectDispatchTarget {
    bool active = false; // source target +0x10 bit 0
    bool excluded = false; // source target +0x10 bit 6
    std::uint16_t world_x = 0; // source target +0x00
    std::uint16_t world_y = 0; // source target +0x04
    std::uint16_t field_0x0A = 0;
    std::uint16_t field_0x0C = 0;
    std::uint8_t scan_flags = 0; // source target +0xDF
    std::uint8_t scan_type = 0; // source target +0xDE
    std::uint8_t effect_type = 0; // source target +0xB2
    std::optional<std::uint16_t> parent_source; // source target +0xD8
    std::uint16_t effect_slot = 0; // source target +0x32
    std::uint16_t effect_record_parameter = 0; // source target +0x2A
    std::uint8_t effect_phase = 0; // source target +0x02
    std::uint8_t effect_subphase = 0; // source target +0x03
};

struct EntityEffectTargetScanResult {
    bool recognized_behavior = false;
    std::optional<std::size_t> selected_nearby_target;
    std::uint16_t linked_field_0x0A = 0;
    std::uint16_t linked_field_0x0C = 0;
};

enum class EntityEffectDispatchAction {
    direct_spawn_boundary,
    no_linked_target,
    effect_table_full,
    nearby_target_not_found,
    applied_to_linked_target,
    applied_to_nearby_target,
};

struct EntityEffectDispatchCallbacks {
    std::function<void()> schedule_world_timeout;
    std::function<void(std::uint8_t,
                       std::uint16_t,
                       std::size_t)> service_audio;
    std::function<void(std::uint8_t, std::size_t)> dispatch_effect_behavior;
    std::function<void(const EntityEffectDispatchSource&)> spawn_direct_effect;
};

struct EntityEffectDispatchResult {
    EntityEffectDispatchAction action =
        EntityEffectDispatchAction::no_linked_target;
    std::optional<std::size_t> target_index;
    EffectSlotAllocation allocation{};
    bool allocation_performed = false;
    bool recognized_behavior = false;
    std::uint16_t linked_field_0x0A = 0;
    std::uint16_t linked_field_0x0C = 0;
};

// ROM 0x1597E: scan the ten entity records around the linked target. Types
// 0x06 and 0x0D select the first compatible nearby record and set its source
// flag/link; types 0x03, 0x05, and 0x0A mark every compatible record but keep
// the original linked target as the effect destination.
EntityEffectTargetScanResult scan_entity_effect_targets(
    const EntityEffectDispatchSource& source,
    std::size_t linked_target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities);

// ROM 0x15830: allocate the source effect slot, select the linked/nearby
// destination, initialize its effect phase fields, and cross the source's
// audio and indirect effect-handler boundaries.
EntityEffectDispatchResult dispatch_entity_effect(
    const EntityEffectDispatchSource& source,
    std::optional<std::size_t> linked_target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectDispatchCallbacks& callbacks);

enum class EntitySecondaryEffectAction {
    invalid_target,
    effect_table_full,
    applied,
};

struct EntitySecondaryEffectResult {
    EntitySecondaryEffectAction action =
        EntitySecondaryEffectAction::invalid_target;
    EffectSlotAllocation allocation{};
    bool allocation_performed = false;
};

// ROM 0x15930: initialize the generic type-2 effect on the caller-selected
// target, then use the same explicit audio/handler boundaries as 0x15830.
EntitySecondaryEffectResult apply_entity_secondary_effect(
    std::size_t target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectDispatchCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
