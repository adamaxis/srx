#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"

#include <optional>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(classify_field_combat_action(0, 1U) ==
                    FieldCombatActionKind::weapon_ammo &&
                classify_field_combat_action(7, 12U) ==
                    FieldCombatActionKind::weapon_ammo &&
                classify_field_combat_action(-1, 1U) ==
                    FieldCombatActionKind::magic_spell &&
                classify_field_combat_action(-1, 13U) ==
                    FieldCombatActionKind::magic_spell &&
                classify_field_combat_action(0, 13U) ==
                    FieldCombatActionKind::effect_record &&
                classify_field_combat_action(0, 15U) ==
                    FieldCombatActionKind::effect_record &&
                classify_field_combat_action(-1, 14U) ==
                    FieldCombatActionKind::unsupported &&
                classify_field_combat_action(0, 16U) ==
                    FieldCombatActionKind::unsupported)) {
        return 1;
    }

    EntityEquipmentState empty{0U, 4};
    if (!expect(service_entity_ammo(empty, 7U, {}, {}) ==
                    EntityAmmoServiceAction::no_action &&
                empty.current_ammo == 0U)) {
        return 2;
    }

    EntityEquipmentState delegated{5U, 2};
    int weapon_services = 0;
    std::int8_t weapon_delta = 0;
    if (!expect(service_entity_ammo(delegated, 7U,
                    [](const std::uint8_t) {
                        return std::optional<std::uint8_t>{1U};
                    }, [&weapon_services, &weapon_delta](
                        const std::int8_t delta) {
                        ++weapon_services;
                        weapon_delta = delta;
                    }) == EntityAmmoServiceAction::delegated_to_weapon_service &&
                weapon_services == 1 && weapon_delta == -1 &&
                delegated.current_ammo == 5U)) {
        return 3;
    }

    EntityEquipmentState multi_round_action{5U, 2};
    weapon_delta = 0;
    if (!expect(service_entity_ammo(
                    multi_round_action, 9U,
                    [](const std::uint8_t code) {
                        return std::optional<std::uint8_t>{
                            code >= 8U && code <= 10U ? 3U : 1U};
                    },
                    [&weapon_delta](const std::int8_t delta) {
                        weapon_delta = delta;
                    }) == EntityAmmoServiceAction::delegated_to_weapon_service &&
                weapon_delta == -3)) {
        return 4;
    }

    EntityEquipmentState unresolved{5U, 2};
    if (!expect(service_entity_ammo(
                    unresolved, 7U,
                    [](const std::uint8_t) {
                        return std::optional<std::uint8_t>{1U};
                    }, {}) ==
                    EntityAmmoServiceAction::weapon_service_unresolved &&
                unresolved.current_ammo == 5U)) {
        return 5;
    }

    EntityEquipmentState decremented{5U, 3};
    if (!expect(service_entity_ammo(decremented, 7U, {}, {}) ==
                    EntityAmmoServiceAction::decremented &&
                decremented.current_ammo == 4U)) {
        return 6;
    }

    std::vector<char> order;
    run_entity_animation_triplet(
        [&order] { order.push_back('a'); },
        [&order] { order.push_back('b'); },
        [&order] { order.push_back('c'); });
    if (!expect(order == std::vector<char>{'a', 'b', 'c', 'a', 'b', 'c',
                                            'a', 'b', 'c'})) {
        return 7;
    }

    order.clear();
    refresh_field_animation_assets(
        [&order] { order.push_back('g'); },
        [&order] { order.push_back('h'); },
        [&order] { order.push_back('a'); },
        [&order] { order.push_back('b'); },
        [&order] { order.push_back('c'); });
    if (!expect(order == std::vector<char>{'g', 'h', 'a', 'b', 'c', 'a',
                                            'b', 'c', 'a', 'b', 'c'})) {
        return 8;
    }
    return 0;
}
