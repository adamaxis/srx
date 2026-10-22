#include "shadowrun_ghidra/cyberspace/cyberdeck_record.hpp"
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
    const auto* smart_goggles =
        find_us_item_definition(UsItemCode::smart_goggles);
    const auto* power_focus = find_us_item_definition(UsItemCode::power_focus);
    const auto grenade_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_frag_grenades);
    const auto hk227_s_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_hk227_s);
    const auto level3_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_maglock_passkey_level_3);
    const auto level5_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_maglock_passkey_level_5);
    const auto max_hk227_s_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_max_hk227_s);
    const auto ak97_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_ak97);
    const auto max_level4_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_max_maglock_passkey_level_4);
    const auto max_level5_passkey_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_max_maglock_passkey_level_5);
    const auto smart_goggles_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_smart_goggles);
    const auto power_focus_offer = vid_phone_supply_offer(
        VidPhoneServiceAction::purchase_power_focus);
    if (!expect(frag_grenade != nullptr && hk227_s != nullptr &&
                    maglock_passkey != nullptr &&
                    smart_goggles != nullptr &&
                    power_focus != nullptr &&
                    grenade_offer.has_value() && hk227_s_offer.has_value() &&
                    level3_passkey_offer.has_value() &&
                    level5_passkey_offer.has_value() &&
                    max_hk227_s_offer.has_value() &&
                    ak97_offer.has_value() &&
                    max_level4_passkey_offer.has_value() &&
                    max_level5_passkey_offer.has_value() &&
                    smart_goggles_offer.has_value() &&
                    power_focus_offer.has_value() &&
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
                    max_hk227_s_offer->item_code == UsItemCode::hk227_s_smg &&
                    max_hk227_s_offer->price == 1500U &&
                    max_hk227_s_offer->item_phase == 0x1CU &&
                    ak97_offer->item_code == UsItemCode::ak97_smg &&
                    ak97_offer->price == 2000U &&
                    ak97_offer->item_phase == 0x25U &&
                    max_level4_passkey_offer->item_code ==
                        UsItemCode::maglock_passkey &&
                    max_level4_passkey_offer->price == 30000U &&
                    max_level4_passkey_offer->item_phase == 4U &&
                    max_level5_passkey_offer->item_code ==
                        UsItemCode::maglock_passkey &&
                    max_level5_passkey_offer->price == 42000U &&
                    max_level5_passkey_offer->item_phase == 5U &&
                    maglock_passkey->base_cost == 10000U &&
                    smart_goggles_offer->item_code ==
                        UsItemCode::smart_goggles &&
                    smart_goggles_offer->price == 3000U &&
                    smart_goggles_offer->item_phase == 0U &&
                    smart_goggles->item_code == 22U &&
                    power_focus_offer->item_code == UsItemCode::power_focus &&
                    power_focus_offer->price == 50000U &&
                    power_focus_offer->item_phase == 2U &&
                    power_focus->item_code == 24U,
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
                        "vid_phone.purchase_smart_goggles") ==
                    VidPhoneServiceAction::purchase_smart_goggles &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_merlins_lore_discount") ==
                    VidPhoneServiceAction::purchase_merlins_lore_discount &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_power_focus") ==
                    VidPhoneServiceAction::purchase_power_focus &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_riannons_discount") ==
                    VidPhoneServiceAction::purchase_riannons_discount &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_max_hk227_s") ==
                    VidPhoneServiceAction::purchase_max_hk227_s &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_ak97") ==
                    VidPhoneServiceAction::purchase_ak97 &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_max_maglock_passkey_level_4") ==
                    VidPhoneServiceAction::purchase_max_maglock_passkey_level_4 &&
                    vid_phone_service_action_for_id(
                        "vid_phone.purchase_max_maglock_passkey_level_5") ==
                    VidPhoneServiceAction::purchase_max_maglock_passkey_level_5 &&
                    vid_phone_service_action_for_id(
                        "vid_phone.hire_sharkey") ==
                    VidPhoneServiceAction::hire_sharkey &&
                    vid_phone_service_action_for_id(
                        "vid_phone.hire_macklemann") ==
                    VidPhoneServiceAction::hire_macklemann &&
                    vid_phone_service_action_for_id(
                        "vid_phone.hire_gregory_wilns") ==
                    VidPhoneServiceAction::hire_gregory_wilns &&
                    vid_phone_service_action_for_id(
                        "vid_phone.issue_max_corporate_badge") ==
                    VidPhoneServiceAction::issue_max_corporate_badge &&
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

    constexpr std::uint32_t kSceneTwentyEntryFlag = 1U << 26U;
    constexpr std::uint32_t kHollywoodAccessFlag = 1U << 28U;
    funds = 2800U;
    contacts = kSceneTwentyEntryFlag | kHollywoodAccessFlag;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::hire_sharkey,
        funds, items, phases, contacts);
    const auto sharkey_mask = static_cast<std::uint32_t>(
        1U << static_cast<unsigned>(VidPhoneContactIndex::sharkey));
    if (!expect(result == VidPhoneServiceResult::hired && funds == 0U &&
                    contacts == (kSceneTwentyEntryFlag |
                                 kHollywoodAccessFlag | sharkey_mask),
                "rigger introduction did not charge 2800, unlock Sharkey, "
                "and preserve FFFC2C scene/access flags")) {
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

    items = {};
    phases = {};
    funds = 3000U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_smart_goggles,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::purchased && funds == 0U &&
                    items[0] == smart_goggles->item_code && phases[0] == 0U,
                "Smart Goggles purchase did not charge 3,000 or grant item 22")) {
        return 14;
    }

    items.fill(1U);
    phases.fill(7U);
    funds = 4000U;
    result = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_smart_goggles,
        funds, items, phases, contacts);
    if (!expect(result == VidPhoneServiceResult::inventory_full &&
                    funds == 4000U && items[0] == 1U && phases[0] == 7U,
                "full inventory did not reject goggles before charging")) {
        return 15;
    }

    const auto salish_request = parse_vid_phone_passage_request(
        "vid_phone.passage_to_scene_6.fare_3000");
    const auto sinsearach_request = parse_vid_phone_passage_request(
        "vid_phone.passage_to_scene_9.fare_5000");
    if (!expect(salish_request.has_value() &&
                    salish_request->destination_scene == 6U &&
                    salish_request->price == 3000U &&
                    sinsearach_request.has_value() &&
                    sinsearach_request->destination_scene == 9U &&
                    sinsearach_request->price == 5000U &&
                    !parse_vid_phone_passage_request(
                         "vid_phone.unknown_passage").has_value() &&
                    !parse_vid_phone_passage_request(
                         "vid_phone.passage_to_scene_256.fare_1").has_value(),
                "Lua passage requests did not parse as bounded scene/fare data")) {
        return 16;
    }

    funds = 3000U;
    auto passage_result = apply_vid_phone_passage_fare(
        salish_request->price, funds);
    if (!expect(passage_result == VidPhonePassageResult::passage_requested &&
                    funds == 0U &&
                    vid_phone_passage_script_result(passage_result) ==
                        "passage_requested",
                "Salish passage did not charge exactly 3,000 nuyen")) {
        return 17;
    }
    funds = 2999U;
    passage_result = apply_vid_phone_passage_fare(
        salish_request->price, funds);
    if (!expect(passage_result ==
                        VidPhonePassageResult::insufficient_funds &&
                    funds == 2999U &&
                    vid_phone_passage_script_result(passage_result) ==
                        "insufficient",
                "short Salish funds were charged or incorrectly accepted")) {
        return 18;
    }
    funds = 5000U;
    passage_result = apply_vid_phone_passage_fare(
        sinsearach_request->price, funds);
    if (!expect(passage_result == VidPhonePassageResult::passage_requested &&
                    funds == 0U,
                "Sinsearach passage did not charge exactly 5,000 nuyen")) {
        return 19;
    }
    funds.reset();
    passage_result = apply_vid_phone_passage_fare(
        sinsearach_request->price, funds);
    if (!expect(passage_result == VidPhonePassageResult::unavailable &&
                    !vid_phone_passage_script_result(passage_result).has_value(),
                "unresolved wallet did not block passage without a script result")) {
        return 20;
    }

    if (!expect(vid_phone_deck_action_for_id(
                    "vid_phone.purchase_excalibur") ==
                    VidPhoneDeckAction::purchase_excalibur &&
                    vid_phone_deck_action_for_id(
                        "vid_phone.purchase_rebound") ==
                    VidPhoneDeckAction::purchase_rebound &&
                    vid_phone_deck_action_for_id(
                        "vid_phone.purchase_degrade") ==
                    VidPhoneDeckAction::purchase_degrade &&
                    !vid_phone_deck_action_for_id(
                        "vid_phone.unknown_deck").has_value(),
                "Kipp deck service ids did not map to commands 0x71-0x73")) {
        return 21;
    }

    std::optional<CyberdeckRecord> deck;
    funds = 184999U;
    auto deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_excalibur, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_funds &&
                    funds == 184999U && !deck.has_value() &&
                    vid_phone_deck_script_result(deck_result) == "insufficient",
                "short Excalibur funds were charged or installed a deck")) {
        return 22;
    }
    deck = CyberdeckRecord{};
    deck->program_ratings = std::array<std::uint8_t, 12>{};
    (*deck->program_ratings)[0] = 1U;
    funds = 185000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_excalibur, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::deck_installed &&
                    funds == 0U && deck.has_value() &&
                    deck->mpcp == 12U && deck->hardening == 5U &&
                    deck->memory_megapulses == 500U &&
                    deck->storage_megapulses == 1000U &&
                    deck->load_io_speed == 100U && deck->response == 3U &&
                    deck->model_index == 4 &&
                    deck->program_ratings.has_value() &&
                    (*deck->program_ratings)[0] == 1U &&
                    vid_phone_deck_script_result(deck_result) ==
                        "deck_installed",
                "Excalibur purchase did not write ROM 0x22A74's deck bytes")) {
        return 23;
    }

    funds = 29999U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_funds &&
                    funds == 29999U &&
                    (*deck->program_ratings)[3] == 0U,
                "short Rebound funds changed the wallet or program rating")) {
        return 24;
    }
    deck->storage_megapulses = 47U;
    funds = 30000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_storage &&
                    funds == 30000U &&
                    (*deck->program_ratings)[3] == 0U &&
                    vid_phone_deck_script_result(deck_result) == "storage_full",
                "47 Mp rejected Rebound after charging, or stored rating 6")) {
        return 25;
    }
    (*deck->program_ratings)[0] = 0U;
    deck->storage_megapulses = 48U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::program_installed &&
                    funds == 0U && (*deck->program_ratings)[3] == 6U &&
                    (*deck->program_ratings)[0] == 0U,
                "48 Mp did not accept the rating-4 size check and store 6")) {
        return 26;
    }
    funds = 30000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_degrade, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_storage &&
                    funds == 30000U &&
                    (*deck->program_ratings)[2] == 0U,
                "stored Rebound rating 6 still left room for Degrade")) {
        return 27;
    }
    deck->storage_megapulses = 1000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_degrade, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::program_installed &&
                    funds == 0U && (*deck->program_ratings)[2] == 6U &&
                    (*deck->program_ratings)[3] == 6U,
                "Degrade did not store rating 6 at program index 2")) {
        return 28;
    }
    funds.reset();
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_excalibur, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::unavailable &&
                    !vid_phone_deck_script_result(deck_result).has_value(),
                "missing wallet still produced a Kipp script result")) {
        return 29;
    }

    if (!expect(vid_phone_deck_action_for_id(
                    "vid_phone.purchase_wilma_rebound") ==
                    VidPhoneDeckAction::purchase_wilma_rebound &&
                    vid_phone_deck_action_for_id(
                        "vid_phone.purchase_wilma_degrade") ==
                    VidPhoneDeckAction::purchase_wilma_degrade,
                "Wilma program ids did not map to commands 0x74 and 0x75")) {
        return 30;
    }
    deck = CyberdeckRecord{};
    deck->storage_megapulses = 26U;
    deck->program_ratings = std::array<std::uint8_t, 12>{};
    funds = 4999U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_funds &&
                    funds == 4999U && (*deck->program_ratings)[3] == 0U,
                "short Wilma Rebound funds changed the wallet or rating")) {
        return 31;
    }
    funds = 5000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_storage &&
                    funds == 5000U && (*deck->program_ratings)[3] == 0U,
                "26 Mp accepted Wilma's rating-3 Rebound size check")) {
        return 32;
    }
    deck->storage_megapulses = 27U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::program_installed &&
                    funds == 0U && (*deck->program_ratings)[3] == 3U,
                "27 Mp did not store Wilma Rebound at rating 3")) {
        return 33;
    }
    funds = 5000U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_degrade, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::insufficient_storage &&
                    funds == 5000U && (*deck->program_ratings)[2] == 0U,
                "stored rating-3 Rebound still left room for Degrade")) {
        return 34;
    }
    deck->program_ratings = std::array<std::uint8_t, 12>{};
    deck->storage_megapulses = 36U;
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_degrade, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::program_installed &&
                    funds == 0U && (*deck->program_ratings)[2] == 3U,
                "36 Mp did not store Wilma Degrade at rating 3")) {
        return 35;
    }
    funds.reset();
    deck_result = apply_vid_phone_deck_service(
        VidPhoneDeckAction::purchase_wilma_rebound, funds, deck);
    if (!expect(deck_result == VidPhoneDeckResult::unavailable &&
                    !vid_phone_deck_script_result(deck_result).has_value(),
                "missing wallet still produced a Wilma script result")) {
        return 36;
    }

    std::array<std::uint8_t, 8> item_codes{};
    std::array<std::uint8_t, 8> item_phases{};
    std::optional<std::uint32_t> badge_flags =
        (1U << 5U) | kSceneTwentyEntryFlag;
    funds = 4999U;
    auto badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_hollywood_badge, funds, item_codes,
        item_phases, badge_flags);
    if (!expect(badge == VidPhoneServiceResult::insufficient_funds &&
                    funds == 4999U &&
                    badge_flags == ((1U << 5U) | kSceneTwentyEntryFlag) &&
                    vid_phone_service_script_result(badge) == "insufficient",
                "short Hollywood badge funds changed the wallet or flags")) {
        return 37;
    }
    funds = 5000U;
    badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_hollywood_badge, funds, item_codes,
        item_phases, badge_flags);
    if (!expect(badge == VidPhoneServiceResult::hired && funds == 0U &&
                    badge_flags == ((1U << 5U) | kSceneTwentyEntryFlag |
                                    kHollywoodAccessFlag) &&
                    vid_phone_service_script_result(badge) == "hired",
                "Hollywood badge did not charge 5,000 and set FFFC2C bit 28")) {
        return 38;
    }
    badge_flags.reset();
    funds = 5000U;
    badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_hollywood_badge, funds, item_codes,
        item_phases, badge_flags);
    if (!expect(badge == VidPhoneServiceResult::unavailable &&
                    funds == 5000U &&
                    !vid_phone_service_script_result(badge).has_value(),
                "missing contact flags still issued the Hollywood badge")) {
        return 39;
    }

    std::optional<std::uint8_t> feature_flags = 0x05U;
    funds = 3999U;
    auto gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_merlins_lore_discount, funds,
        item_codes, item_phases, badge_flags, &feature_flags);
    if (!expect(gregory == VidPhoneServiceResult::insufficient_funds &&
                    funds == 3999U && feature_flags == 0x05U &&
                    vid_phone_service_script_result(gregory) ==
                        "insufficient",
                "short Merlin's Lore funds changed the wallet or feature flags")) {
        return 40;
    }
    funds = 4000U;
    gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_merlins_lore_discount, funds,
        item_codes, item_phases, badge_flags, &feature_flags);
    if (!expect(gregory == VidPhoneServiceResult::discount_enabled &&
                    funds == 0U && feature_flags == 0x85U &&
                    vid_phone_service_script_result(gregory) ==
                        "discount_enabled",
                "4,000-nuyen discount did not set FF018F bit 7")) {
        return 41;
    }
    feature_flags.reset();
    funds = 4000U;
    gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_merlins_lore_discount, funds,
        item_codes, item_phases, badge_flags, &feature_flags);
    if (!expect(gregory == VidPhoneServiceResult::unavailable &&
                    funds == 4000U,
                "unknown runner feature flags still granted the discount")) {
        return 42;
    }

    item_codes = {};
    item_phases = {};
    funds = 50000U;
    gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_power_focus, funds, item_codes,
        item_phases, badge_flags, nullptr, std::int8_t{2});
    if (!expect(gregory == VidPhoneServiceResult::ineligible &&
                    funds == 50000U && item_codes[0] == 0U &&
                    vid_phone_service_script_result(gregory) == "ineligible",
                "non-mage archetype was charged or received a Power Focus")) {
        return 43;
    }
    gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_power_focus, funds, item_codes,
        item_phases, badge_flags, nullptr, std::int8_t{1});
    if (!expect(gregory == VidPhoneServiceResult::purchased && funds == 0U &&
                    item_codes[0] == 24U && item_phases[0] == 2U,
                "eligible Power Focus purchase did not write item 24/phase 2")) {
        return 44;
    }
    item_codes.fill(1U);
    item_phases.fill(7U);
    funds = 50000U;
    gregory = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_power_focus, funds, item_codes,
        item_phases, badge_flags, nullptr, std::int8_t{0});
    if (!expect(gregory == VidPhoneServiceResult::inventory_full &&
                    funds == 50000U && item_codes[0] == 1U &&
                    item_phases[0] == 7U &&
                    vid_phone_service_script_result(gregory) == "out_of_stock",
                "full inventory changed funds/items during Power Focus purchase")) {
        return 45;
    }

    struct MaxItemOfferCase {
        VidPhoneServiceAction action;
        UsItemCode item_code;
        std::uint32_t price;
        std::uint8_t phase;
    };
    constexpr std::array<MaxItemOfferCase, 4> kMaxOffers{{
        {VidPhoneServiceAction::purchase_max_hk227_s,
         UsItemCode::hk227_s_smg, 1500U, 0x1CU},
        {VidPhoneServiceAction::purchase_ak97,
         UsItemCode::ak97_smg, 2000U, 0x25U},
        {VidPhoneServiceAction::purchase_max_maglock_passkey_level_4,
         UsItemCode::maglock_passkey, 30000U, 4U},
        {VidPhoneServiceAction::purchase_max_maglock_passkey_level_5,
         UsItemCode::maglock_passkey, 42000U, 5U},
    }};
    for (const auto& offer : kMaxOffers) {
        item_codes = {};
        item_phases = {};
        funds = offer.price;
        auto max_purchase = apply_vid_phone_service(
            offer.action, funds, item_codes, item_phases, badge_flags);
        const auto* definition = find_us_item_definition(offer.item_code);
        if (!expect(definition != nullptr &&
                        max_purchase == VidPhoneServiceResult::purchased &&
                        funds == 0U && item_codes[0] == definition->item_code &&
                        item_phases[0] == offer.phase,
                    "Max's ROM item offer did not charge and install its exact item/phase")) {
            return 46;
        }
        funds = offer.price - 1U;
        item_codes = {};
        item_phases = {};
        max_purchase = apply_vid_phone_service(
            offer.action, funds, item_codes, item_phases, badge_flags);
        if (!expect(max_purchase == VidPhoneServiceResult::insufficient_funds &&
                        funds == offer.price - 1U && item_codes[0] == 0U &&
                        item_phases[0] == 0U,
                    "Max's short-balance branch mutated nuyen or inventory")) {
            return 47;
        }
        funds = offer.price;
        item_codes.fill(1U);
        item_phases.fill(7U);
        max_purchase = apply_vid_phone_service(
            offer.action, funds, item_codes, item_phases, badge_flags);
        if (!expect(max_purchase == VidPhoneServiceResult::inventory_full &&
                        funds == offer.price && item_codes[0] == 1U &&
                        item_phases[0] == 7U,
                    "Max's full-inventory branch charged or overwrote an item")) {
            return 48;
        }
    }

    constexpr std::uint32_t kMaxCorporateBadgeBit = 1U << 10U;
    std::optional<std::uint32_t> group_flags = (1U << 4U);
    funds = 5999U;
    auto max_badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_max_corporate_badge, funds, item_codes,
        item_phases, badge_flags, nullptr, std::nullopt, &group_flags);
    if (!expect(max_badge == VidPhoneServiceResult::insufficient_funds &&
                    funds == 5999U && group_flags == (1U << 4U),
                "short Max badge funds changed nuyen or group-item flags")) {
        return 49;
    }
    funds = 6000U;
    max_badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_max_corporate_badge, funds, item_codes,
        item_phases, badge_flags, nullptr, std::nullopt, &group_flags);
    if (!expect(max_badge == VidPhoneServiceResult::badge_issued &&
                    funds == 0U &&
                    group_flags == ((1U << 4U) | kMaxCorporateBadgeBit) &&
                    vid_phone_service_script_result(max_badge) == "badge_issued",
                "Max's $6,000 badge did not set FFFC22 bit 10")) {
        return 50;
    }
    group_flags.reset();
    funds = 6000U;
    max_badge = apply_vid_phone_service(
        VidPhoneServiceAction::issue_max_corporate_badge, funds, item_codes,
        item_phases, badge_flags, nullptr, std::nullopt, &group_flags);
    if (!expect(max_badge == VidPhoneServiceResult::unavailable &&
                    funds == 6000U,
                "unresolved group-item flags still issued Max's badge")) {
        return 51;
    }

    feature_flags = 0x80U;
    funds = 1499U;
    auto riannons = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_riannons_discount, funds, item_codes,
        item_phases, badge_flags, &feature_flags);
    if (!expect(riannons == VidPhoneServiceResult::insufficient_funds &&
                    funds == 1499U && feature_flags == 0x80U,
                "short Riannon's fee changed funds or runner features")) {
        return 52;
    }
    funds = 1500U;
    riannons = apply_vid_phone_service(
        VidPhoneServiceAction::purchase_riannons_discount, funds, item_codes,
        item_phases, badge_flags, &feature_flags);
    if (!expect(riannons == VidPhoneServiceResult::discount_enabled &&
                    funds == 0U && feature_flags == 0xC0U,
                "Professor's $1,500 fee did not set FF018F bit 6")) {
        return 53;
    }
    badge_flags = 1U << 5U;
    funds = 2000U;
    auto professor_contact = apply_vid_phone_service(
        VidPhoneServiceAction::hire_gregory_wilns, funds, item_codes,
        item_phases, badge_flags);
    if (!expect(professor_contact == VidPhoneServiceResult::hired &&
                    funds == 0U && badge_flags == ((1U << 5U) | (1U << 6U)),
                "Professor's $2,000 offer did not add Gregory contact bit 6")) {
        return 54;
    }
    funds = 1999U;
    badge_flags = 1U << 5U;
    professor_contact = apply_vid_phone_service(
        VidPhoneServiceAction::hire_gregory_wilns, funds, item_codes,
        item_phases, badge_flags);
    if (!expect(professor_contact == VidPhoneServiceResult::insufficient_funds &&
                    funds == 1999U && badge_flags == (1U << 5U),
                "Professor's insufficient offer changed funds or contact flags")) {
        return 55;
    }

    return 0;
}
