#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"

namespace shadowrun::ghidra_conversion {

EntityAmmoServiceAction service_entity_ammo(
    EntityEquipmentState& state,
    const std::function<void()>& weapon_service) {
    if (state.current_ammo == 0U || state.equipped_item_code == 0U) {
        return EntityAmmoServiceAction::no_action;
    }

    if (state.record_type < 3) {
        if (weapon_service) {
            weapon_service();
        }
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
