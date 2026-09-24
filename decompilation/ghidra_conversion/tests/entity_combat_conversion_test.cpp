#include "shadowrun_ghidra/entity_combat_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityEquipmentState empty{0U, 7U, 4};
    if (!expect(service_entity_ammo(empty, {}) ==
                    EntityAmmoServiceAction::no_action &&
                empty.current_ammo == 0U)) {
        return 1;
    }

    EntityEquipmentState delegated{5U, 7U, 2};
    int weapon_services = 0;
    if (!expect(service_entity_ammo(delegated, [&weapon_services] {
                        ++weapon_services;
                    }) == EntityAmmoServiceAction::delegated_to_weapon_service &&
                weapon_services == 1 && delegated.current_ammo == 5U)) {
        return 2;
    }

    EntityEquipmentState decremented{5U, 7U, 3};
    if (!expect(service_entity_ammo(decremented, {}) ==
                    EntityAmmoServiceAction::decremented &&
                decremented.current_ammo == 4U)) {
        return 3;
    }

    std::vector<char> order;
    run_entity_animation_triplet(
        [&order] { order.push_back('a'); },
        [&order] { order.push_back('b'); },
        [&order] { order.push_back('c'); });
    if (!expect(order == std::vector<char>{'a', 'b', 'c', 'a', 'b', 'c',
                                            'a', 'b', 'c'})) {
        return 4;
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
        return 5;
    }
    return 0;
}
