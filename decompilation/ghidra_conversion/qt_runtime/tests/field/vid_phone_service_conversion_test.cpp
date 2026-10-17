#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <optional>

namespace {

using namespace shadowrun::ghidra_conversion;

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

} // namespace

int main() {
    const auto* frag_grenade =
        find_us_item_definition(UsItemCode::frag_grenade);
    const auto* hk227_s = find_us_item_definition(UsItemCode::hk227_s_smg);
    const auto* maglock_passkey =
        find_us_item_definition(UsItemCode::maglock_passkey);
    const auto grenade_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_frag_grenades);
    const auto hk227_s_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_hk227_s);
    const auto level3_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_maglock_passkey_level_3);
    const auto level5_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_maglock_passkey_level_5);
    if (!expect(frag_grenade != nullptr && hk227_s != nullptr &&
                    maglock_passkey != nullptr &&
                    grenade_offer.has_value() && hk227_s_offer.has_value() &&
                    level3_passkey_offer.has_value() &&
                    level5_passkey_offer.has_value() &&
                    grenade_offer->item_code == UsItemCode::frag_grenade &&
                    hk227_s_offer->item_code == UsItemCode::hk227_s_smg &&
                    grenade_offer->price == 400U &&
                    grenade_offer->item_phase == 6U &&
                    hk227_s_offer->price == 1000U &&
                    hk227_s_offer->item_phase ==
                        static_cast<std::uint8_t>(
                            weapon_accessory_flag(
                                UsWeaponAccessory::laser_sight) |
                            weapon_accessory_flag(
                                UsWeaponAccessory::gas_vent_ii)) &&
                    hk227_s->source_combat_factor == 28U &&
                    hk227_s_offer->item_phase !=
                        hk227_s->source_combat_factor &&
                    level3_passkey_offer->item_code ==
                        UsItemCode::maglock_passkey &&
                    level3_passkey_offer->price == 10000U &&
                    level3_passkey_offer->item_phase == 3U &&
                    level5_passkey_offer->item_code ==
                        UsItemCode::maglock_passkey &&
                    level5_passkey_offer->price == 20000U &&
                    level5_passkey_offer->item_phase == 5U &&
                    maglock_passkey->base_cost == 10000U,
                "service offer records did not preserve ROM item identity, "
                "price, attachment phase, and clip capacity separately")) {
        return 10;
    }

    if (!expect(vid_phone_service_action_for_id(
                    "vid_phone.purchase_frag_grenades") ==
                    VidPhoneServiceAction::purchase_frag_grenades &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_hk227_s") ==
                    VidPhoneServiceAction::purchase_hk227_s &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_maglock_passkey_level_3") ==
                    VidPhoneServiceAction::purchase_maglock_passkey_level_3 &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_maglock_passkey_level_5") ==
                    VidPhoneServiceAction::purchase_maglock_passkey_level_5 &&
                    vid_phone_service_action_for_id(
                        "vid_phone.hire_sharkey") ==
                    VidPhoneServiceAction::hire_sharkey &&
                    vid_phone_service_action_for_id(
                        "vid_phone.hire_macklemann") ==
                    VidPhoneServiceAction::hire_macklemann &&
                    !vid_phone_service_action_for_id(
                        "vid_phone.unknown_offer").has_value(),
                "Lua Vid-Phone service ids did not map to typed host actions")) {
        return 8;
    }

    std::array<std::uint8_t, 8> items{};
    std::array<std::uint8_t, 8> phases{};
    std::optional<std::uint32_t> funds = 500U;
    std::optional<std::uint32_t> contacts = 0U;

    auto result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_frag_grenades,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::purchased &&
                    funds == 100U &&
                    items[0] == frag_grenade->item_code &&
                    phases[0] == 6U,
                "Frag Grenade purchase did not charge 400 and grant six uses")) {
        return 1;
    }

    items = {};
    phases = {};
    funds = 399U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_frag_grenades,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::insufficient_funds &&
                    funds == 399U && items[0] == 0U && phases[0] == 0U,
                "failed grenade purchase mutated runner state")) {
        return 2;
    }

    items.fill(1U);
    phases.fill(8U);
    funds = 2000U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_hk227_s,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::inventory_full &&
                    funds == 2000U && items[0] == 1U && phases[0] == 8U,
                "full inventory purchase charged credits or overwrote an item")) {
        return 3;
    }

    items = {};
    phases = {};
    funds = 1000U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_hk227_s,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::purchased &&
                    funds == 0U &&
                    items[0] == hk227_s->item_code &&
                    phases[0] == hk227_s_offer->item_phase,
                "HK227-S purchase did not charge 1000 and install its "
                "source accessory flags")) {
        return 4;
    }

    funds = 2800U;
    contacts = 0U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_sharkey,
        funds, items, phases, contacts);
    const auto sharkey_mask = static_cast<std::uint32_t>(
        1U << static_cast<unsigned>(VidPhoneContactIndex::sharkey));
    if (!expect(result == VidPhoneServiceResult::hired && funds == 0U &&
                    contacts == sharkey_mask,
                "rigger introduction did not charge 2800 and unlock Sharkey")) {
        return 5;
    }

    funds = 2799U;
    contacts = 0U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_sharkey,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::insufficient_funds &&
                    funds == 2799U && contacts == 0U,
                "failed rigger introduction changed funds or contacts")) {
        return 6;
    }

    funds = 2800U;
    contacts.reset();
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_sharkey,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::unavailable && funds == 2800U,
                "unresolved notebook contacts charged the player")) {
        return 7;
    }

    funds = 10000U;
    contacts = 0U;
    items = {};
    phases = {};
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_maglock_passkey_level_3,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::purchased && funds == 0U &&
                    items[0] == maglock_passkey->item_code && phases[0] == 3U,
                "Alesandro's level-3 passkey did not charge 10000 or preserve level 3")) {
        return 10;
    }

    funds = 20000U;
    items = {};
    phases = {};
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_maglock_passkey_level_5,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::purchased && funds == 0U &&
                    items[0] == maglock_passkey->item_code && phases[0] == 5U,
                "Alesandro's level-5 passkey did not charge 20000 or preserve level 5")) {
        return 11;
    }

    funds = 8000U;
    contacts = 0U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_macklemann,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::hired && funds == 0U &&
                    contacts == (1U << 5U),
                "Alesandro's offer did not charge 8000 and unlock contact bit 5")) {
        return 12;
    }

    funds = 7999U;
    contacts = 0U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_macklemann,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::insufficient_funds &&
                    funds == 7999U && contacts == 0U,
                "failed Macklemann introduction changed money or contact flags")) {
        return 13;
    }
    if (!expect(vid_phone_service_script_result(
                    VidPhoneServiceResult::insufficient_funds) ==
                    "insufficient" &&
                    vid_phone_service_script_result(
                        VidPhoneServiceResult::inventory_full) ==
                    "out_of_stock" &&
                    !vid_phone_service_script_result(
                         VidPhoneServiceResult::unavailable).has_value(),
                "typed transaction results did not map back to Lua outcomes")) {
        return 9;
    }

    return 0;
}
