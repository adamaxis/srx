#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <optional>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::scripting;

    // ROM +0x90 bits 1..4 select Cybereyes, Hand Razors, Spurs, and Smartlink
    // at 0x55718/0x55740/0x55DA2..0x55DBC/0x556C6. Bits 5..8 and 9..11
    // select first-match Muscle Replacement and Dermal Plating bonuses at
    // 0x55D2C..0x55D86. Verify each source tier reaches Lua as its exact value.
    const auto basic_cyberware = script_combat_cyberware_from_source_flags(
        0x001EU);
    if (!expect(basic_cyberware.cybereyes && basic_cyberware.hand_razors &&
                basic_cyberware.spurs && basic_cyberware.smartlink &&
                basic_cyberware.melee_strength_bonus == 0U &&
                basic_cyberware.melee_defense_bonus == 0U)) {
        return 14;
    }
    if (!expect(!script_combat_cyberware_from_source_flags(0x0001U).cybereyes &&
                script_combat_cyberware_from_source_flags(0x0001U)
                        .melee_strength_bonus == 0U &&
                script_combat_cyberware_from_source_flags(0x0001U)
                        .melee_defense_bonus == 0U)) {
        return 15;
    }
    for (std::uint8_t tier = 1U; tier <= 4U; ++tier) {
        const auto cyberware = script_combat_cyberware_from_source_flags(
            static_cast<std::uint16_t>(1U << (4U + tier)));
        if (!expect(cyberware.melee_strength_bonus == tier &&
                    cyberware.melee_defense_bonus == 0U)) {
            return 16;
        }
    }
    for (std::uint8_t tier = 1U; tier <= 3U; ++tier) {
        const auto cyberware = script_combat_cyberware_from_source_flags(
            static_cast<std::uint16_t>(1U << (8U + tier)));
        if (!expect(cyberware.melee_defense_bonus == tier &&
                    cyberware.melee_strength_bonus == 0U)) {
            return 17;
        }
    }

    // ROM 0x14D6A selects spell codes 1..14 for a negative equipped slot;
    // nonnegative item action codes 1..12 use weapons, while 13..15 take the
    // separate 0x15646 effect-record path.
    for (std::uint8_t action_code = 1U; action_code <= 12U; ++action_code) {
        if (!expect(classify_field_combat_action(0, action_code) ==
                    FieldCombatActionKind::weapon_ammo)) {
            return 9;
        }
    }
    for (std::uint8_t spell_code = 1U; spell_code <= 14U; ++spell_code) {
        if (!expect(classify_field_combat_action(-1, spell_code) ==
                    FieldCombatActionKind::magic_spell)) {
            return 10;
        }
    }
    for (std::uint8_t item_code = 13U; item_code <= 15U; ++item_code) {
        if (!expect(classify_field_combat_action(0, item_code) ==
                    FieldCombatActionKind::effect_record)) {
            return 11;
        }
    }
    if (!expect(classify_field_combat_action(-1, 15U) ==
                    FieldCombatActionKind::unsupported &&
                classify_field_combat_action(0, 0U) ==
                    FieldCombatActionKind::unsupported &&
                classify_field_combat_action(0, 16U) ==
                    FieldCombatActionKind::unsupported)) {
        return 12;
    }

    if (!expect(classify_field_combat_action(0, 1U) ==
                    FieldCombatActionKind::weapon_ammo &&
                classify_field_combat_action(7, 12U) ==
                    FieldCombatActionKind::weapon_ammo &&
                classify_field_combat_action(-1, 1U) ==
                    FieldCombatActionKind::magic_spell &&
                classify_field_combat_action(-1, 13U) ==
                    FieldCombatActionKind::magic_spell &&
                classify_field_combat_action(-1, 14U) ==
                    FieldCombatActionKind::magic_spell &&
                classify_field_combat_action(0, 13U) ==
                    FieldCombatActionKind::effect_record &&
                classify_field_combat_action(0, 15U) ==
                    FieldCombatActionKind::effect_record &&
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

    // ROM 0x38AC charges exactly three rounds for action codes 8..10 and one
    // for the other conventional weapon codes 1..7 and 11..12.
    for (std::uint8_t action_code = 8U; action_code <= 10U; ++action_code) {
        EntityEquipmentState burst{5U, 2};
        std::int8_t rounds_delta = 0;
        if (!expect(service_entity_ammo(
                        burst, action_code,
                        [](const std::uint8_t code) {
                            return std::optional<std::uint8_t>{
                                code >= 8U && code <= 10U ? 3U : 1U};
                        },
                        [&rounds_delta](const std::int8_t delta) {
                            rounds_delta = delta;
                        }) ==
                        EntityAmmoServiceAction::delegated_to_weapon_service &&
                    rounds_delta == -3 && burst.current_ammo == 5U)) {
            return 13;
        }
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
