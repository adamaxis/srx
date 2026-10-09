#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityEquipmentState {
    std::uint8_t current_ammo = 0;
    std::int16_t record_type = 0;
};

enum class EntityAmmoServiceAction {
    no_action,
    delegated_to_weapon_service,
    weapon_service_unresolved,
    decremented,
};

enum class FieldCombatActionKind {
    unsupported,
    weapon_ammo,
    magic_spell,
    effect_record,
};

// ROM 0x14D6A selects the D4EA0 spell table when the equipped-slot field is
// negative, even though its one-based spell code overlaps ordinary item
// action codes. Nonnegative slots use the D4540 item/effect action families.
FieldCombatActionKind classify_field_combat_action(
    std::int16_t equipped_inventory_slot,
    std::uint8_t action_code);

// ROM 0x38AC: service one entity's current ammo value. The source delegates
// records below type 3 to the weapon-service routine with a -1 or -3 meter
// delta; that routine remains an explicit boundary because its inventory and
// presentation work is a separate hill.
EntityAmmoServiceAction service_entity_ammo(
    EntityEquipmentState& state,
    std::uint8_t source_action_code,
    const std::function<void(std::int8_t)>& weapon_service);

using EntityAnimationPassBoundary = std::function<void()>;

// ROM 0x38FA: run the three fixed animation passes in source order.
void run_entity_animation_triplet(
    const EntityAnimationPassBoundary& advance_primary,
    const EntityAnimationPassBoundary& advance_secondary,
    const EntityAnimationPassBoundary& service_weapon);

// ROM 0x39F2: refresh the field animation assets, then run the three-pass
// entity animation sequence. Each graphics operation remains a host boundary.
void refresh_field_animation_assets(
    const EntityAnimationPassBoundary& refresh_grid,
    const EntityAnimationPassBoundary& refresh_hud,
    const EntityAnimationPassBoundary& advance_primary,
    const EntityAnimationPassBoundary& advance_secondary,
    const EntityAnimationPassBoundary& service_weapon);

} // namespace shadowrun::ghidra_conversion
