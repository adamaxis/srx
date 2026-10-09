#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldCombatActionKind classify_field_combat_action(
    const std::int16_t equipped_inventory_slot,
    const std::uint8_t action_code) {
    constexpr std::uint8_t kFirstActionCode = 1U;
    constexpr std::uint8_t kLastWeaponActionCode = 12U;
    constexpr std::uint8_t kLastSpellActionCode = 13U;
    constexpr std::uint8_t kFirstEffectRecordCode = 13U;
    constexpr std::uint8_t kLastEffectRecordCode = 15U;

    if (equipped_inventory_slot < 0) {
        return action_code >= kFirstActionCode &&
                       action_code <= kLastSpellActionCode
                   ? FieldCombatActionKind::magic_spell
                   : FieldCombatActionKind::unsupported;
    }
    if (action_code >= kFirstActionCode &&
        action_code <= kLastWeaponActionCode) {
        return FieldCombatActionKind::weapon_ammo;
    }
    if (action_code >= kFirstEffectRecordCode &&
        action_code <= kLastEffectRecordCode) {
        return FieldCombatActionKind::effect_record;
    }
    return FieldCombatActionKind::unsupported;
}

EntityAmmoServiceAction service_entity_ammo(
    EntityEquipmentState& state,
    const std::uint8_t source_action_code,
    const std::function<void(std::int8_t)>& weapon_service) {
    if (state.current_ammo == 0U || source_action_code == 0U) {
        return EntityAmmoServiceAction::no_action;
    }

    if (state.record_type < 3) {
        if (!weapon_service) {
            return EntityAmmoServiceAction::weapon_service_unresolved;
        }
        const auto meter_delta =
            source_action_code >= 8U && source_action_code <= 10U
                ? static_cast<std::int8_t>(-3)
                : static_cast<std::int8_t>(-1);
        weapon_service(meter_delta);
        return EntityAmmoServiceAction::delegated_to_weapon_service;
    }

    state.current_ammo = static_cast<std::uint8_t>(state.current_ammo - 1U);
    return EntityAmmoServiceAction::decremented;
}

void run_entity_animation_triplet(
    const EntityAnimationPassBoundary& advance_primary,
    const EntityAnimationPassBoundary& advance_secondary,
    const EntityAnimationPassBoundary& service_weapon) {
    for (int pass = 0; pass < 3; ++pass) {
        if (advance_primary) {
            advance_primary();
        }
        if (advance_secondary) {
            advance_secondary();
        }
        if (service_weapon) {
            service_weapon();
        }
    }
}

void refresh_field_animation_assets(
    const EntityAnimationPassBoundary& refresh_grid,
    const EntityAnimationPassBoundary& refresh_hud,
    const EntityAnimationPassBoundary& advance_primary,
    const EntityAnimationPassBoundary& advance_secondary,
    const EntityAnimationPassBoundary& service_weapon) {
    if (refresh_grid) {
        refresh_grid();
    }
    if (refresh_hud) {
        refresh_hud();
    }
    run_entity_animation_triplet(advance_primary, advance_secondary,
                                 service_weapon);
}

} // namespace shadowrun::ghidra_conversion
