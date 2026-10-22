#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/weapon_accessory_conversion.hpp"
#include "shadowrun_ghidra/cyberspace/cyberdeck_record.hpp"
#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace shadowrun::ghidra_conversion {

enum class VidPhoneServiceAction : std::uint8_t {
    purchase_frag_grenades,
    purchase_hk227_s,
    purchase_maglock_passkey_level_3,
    purchase_maglock_passkey_level_5,
    purchase_smart_goggles,
    purchase_merlins_lore_discount,
    purchase_power_focus,
    purchase_riannons_discount,
    purchase_max_hk227_s,
    purchase_ak97,
    purchase_max_maglock_passkey_level_4,
    purchase_max_maglock_passkey_level_5,
    hire_sharkey,
    hire_macklemann,
    hire_gregory_wilns,
    issue_hollywood_badge,
    issue_max_corporate_badge,
};

enum class VidPhonePassageResult : std::uint8_t {
    passage_requested,
    insufficient_funds,
    unavailable,
};

struct VidPhonePassageRequest {
    std::uint16_t destination_scene;
    std::uint32_t price;
};

inline std::optional<VidPhonePassageRequest>
parse_vid_phone_passage_request(const std::string_view service_id) noexcept {
    constexpr std::string_view prefix = "vid_phone.passage_to_scene_";
    constexpr std::string_view fare_separator = ".fare_";
    if (service_id.rfind(prefix, 0U) != 0U) {
        return std::nullopt;
    }
    const auto request = service_id.substr(prefix.size());
    const auto separator = request.find(fare_separator);
    if (separator == std::string_view::npos || separator == 0U ||
        separator + fare_separator.size() >= request.size()) {
        return std::nullopt;
    }

    const auto scene_text = request.substr(0U, separator);
    const auto fare_text = request.substr(separator + fare_separator.size());
    std::uint32_t scene = 0U;
    std::uint32_t price = 0U;
    const auto parsed_scene = std::from_chars(
        scene_text.data(), scene_text.data() + scene_text.size(), scene);
    const auto parsed_price = std::from_chars(
        fare_text.data(), fare_text.data() + fare_text.size(), price);
    if (parsed_scene.ec != std::errc{} ||
        parsed_scene.ptr != scene_text.data() + scene_text.size() ||
        parsed_price.ec != std::errc{} ||
        parsed_price.ptr != fare_text.data() + fare_text.size() ||
        scene > 0xFFU) {
        return std::nullopt;
    }
    return VidPhonePassageRequest{
        static_cast<std::uint16_t>(scene), price};
}

inline VidPhonePassageResult apply_vid_phone_passage_fare(
    const std::uint32_t price,
    std::optional<std::uint32_t>& funds) noexcept {
    if (!funds.has_value()) {
        return VidPhonePassageResult::unavailable;
    }
    if (*funds < price) {
        return VidPhonePassageResult::insufficient_funds;
    }
    *funds -= price;
    return VidPhonePassageResult::passage_requested;
}

inline std::optional<std::string_view> vid_phone_passage_script_result(
    const VidPhonePassageResult result) noexcept {
    switch (result) {
    case VidPhonePassageResult::passage_requested:
        return "passage_requested";
    case VidPhonePassageResult::insufficient_funds:
        return "insufficient";
    case VidPhonePassageResult::unavailable:
        return std::nullopt;
    }
    return std::nullopt;
}

enum class VidPhoneServiceResult : std::uint8_t {
    purchased,
    hired,
    badge_issued,
    insufficient_funds,
    inventory_full,
    ineligible,
    discount_enabled,
    unavailable,
};

enum class VidPhoneContactIndex : std::uint8_t {
    sharkey = 2U,
    macklemann = 5U,
    gregory_wilns = 6U,
};

struct VidPhoneSupplyOffer {
    UsItemCode item_code;
    std::uint32_t price;
    // Source service commands set the inventory phase byte. For weapons it
    // stores attachment flags; this is not the loaded-round counter.
    std::uint8_t item_phase;
};

inline std::optional<VidPhoneSupplyOffer> vid_phone_supply_offer(
    const VidPhoneServiceAction action) noexcept {
    switch (action) {
    case VidPhoneServiceAction::purchase_frag_grenades:
        return VidPhoneSupplyOffer{UsItemCode::frag_grenade, 400U, 6U};
    case VidPhoneServiceAction::purchase_hk227_s: {
        constexpr std::uint8_t hk227_accessories = static_cast<std::uint8_t>(
            weapon_accessory_flag(UsWeaponAccessory::laser_sight) |
            weapon_accessory_flag(UsWeaponAccessory::gas_vent_ii));
        return VidPhoneSupplyOffer{
            UsItemCode::hk227_s_smg, 1000U, hk227_accessories};
    }
    case VidPhoneServiceAction::purchase_maglock_passkey_level_3:
        return VidPhoneSupplyOffer{UsItemCode::maglock_passkey, 10000U, 3U};
    case VidPhoneServiceAction::purchase_maglock_passkey_level_5:
        return VidPhoneSupplyOffer{UsItemCode::maglock_passkey, 20000U, 5U};
    case VidPhoneServiceAction::purchase_smart_goggles:
        return VidPhoneSupplyOffer{UsItemCode::smart_goggles, 3000U, 0U};
    case VidPhoneServiceAction::purchase_max_hk227_s:
        // Max command 0x86, ROM 0x22E52: H&K with source accessory byte 0x1C.
        return VidPhoneSupplyOffer{UsItemCode::hk227_s_smg, 1500U, 0x1CU};
    case VidPhoneServiceAction::purchase_ak97:
        // Max command 0x87, ROM 0x22E8C: AK-97 with phase byte 0x25.
        return VidPhoneSupplyOffer{UsItemCode::ak97_smg, 2000U, 0x25U};
    case VidPhoneServiceAction::purchase_max_maglock_passkey_level_4:
        return VidPhoneSupplyOffer{UsItemCode::maglock_passkey, 30000U, 4U};
    case VidPhoneServiceAction::purchase_max_maglock_passkey_level_5:
        return VidPhoneSupplyOffer{UsItemCode::maglock_passkey, 42000U, 5U};
    case VidPhoneServiceAction::purchase_power_focus:
        // Gregory Wilns, command 0x78 at ROM 0x22C46. Item 0x18 is the
        // Power Focus; the transaction initializes its phase/charge byte to 2.
        return VidPhoneSupplyOffer{UsItemCode::power_focus, 50000U, 2U};
    case VidPhoneServiceAction::purchase_merlins_lore_discount:
    case VidPhoneServiceAction::purchase_riannons_discount:
    case VidPhoneServiceAction::hire_sharkey:
    case VidPhoneServiceAction::hire_macklemann:
    case VidPhoneServiceAction::hire_gregory_wilns:
    case VidPhoneServiceAction::issue_hollywood_badge:
    case VidPhoneServiceAction::issue_max_corporate_badge:
        return std::nullopt;
    }
    return std::nullopt;
}

inline std::optional<VidPhoneServiceAction> vid_phone_service_action_for_id(
    const std::string_view service_id) noexcept {
    if (service_id == "vid_phone.purchase_frag_grenades") {
        return VidPhoneServiceAction::purchase_frag_grenades;
    }
    if (service_id == "vid_phone.purchase_hk227_s") {
        return VidPhoneServiceAction::purchase_hk227_s;
    }
    if (service_id == "vid_phone.purchase_maglock_passkey_level_3") {
        return VidPhoneServiceAction::purchase_maglock_passkey_level_3;
    }
    if (service_id == "vid_phone.purchase_maglock_passkey_level_5") {
        return VidPhoneServiceAction::purchase_maglock_passkey_level_5;
    }
    if (service_id == "vid_phone.purchase_smart_goggles") {
        return VidPhoneServiceAction::purchase_smart_goggles;
    }
    if (service_id == "vid_phone.purchase_merlins_lore_discount") {
        return VidPhoneServiceAction::purchase_merlins_lore_discount;
    }
    if (service_id == "vid_phone.purchase_power_focus") {
        return VidPhoneServiceAction::purchase_power_focus;
    }
    if (service_id == "vid_phone.purchase_riannons_discount") {
        return VidPhoneServiceAction::purchase_riannons_discount;
    }
    if (service_id == "vid_phone.purchase_max_hk227_s") {
        return VidPhoneServiceAction::purchase_max_hk227_s;
    }
    if (service_id == "vid_phone.purchase_ak97") {
        return VidPhoneServiceAction::purchase_ak97;
    }
    if (service_id == "vid_phone.purchase_max_maglock_passkey_level_4") {
        return VidPhoneServiceAction::purchase_max_maglock_passkey_level_4;
    }
    if (service_id == "vid_phone.purchase_max_maglock_passkey_level_5") {
        return VidPhoneServiceAction::purchase_max_maglock_passkey_level_5;
    }
    if (service_id == "vid_phone.hire_sharkey") {
        return VidPhoneServiceAction::hire_sharkey;
    }
    if (service_id == "vid_phone.hire_macklemann") {
        return VidPhoneServiceAction::hire_macklemann;
    }
    if (service_id == "vid_phone.hire_gregory_wilns") {
        return VidPhoneServiceAction::hire_gregory_wilns;
    }
    if (service_id == "vid_phone.issue_hollywood_badge") {
        return VidPhoneServiceAction::issue_hollywood_badge;
    }
    if (service_id == "vid_phone.issue_max_corporate_badge") {
        return VidPhoneServiceAction::issue_max_corporate_badge;
    }
    return std::nullopt;
}

inline std::optional<std::string_view> vid_phone_service_script_result(
    const VidPhoneServiceResult result) noexcept {
    switch (result) {
    case VidPhoneServiceResult::purchased: return "purchased";
    case VidPhoneServiceResult::hired: return "hired";
    case VidPhoneServiceResult::badge_issued: return "badge_issued";
    case VidPhoneServiceResult::insufficient_funds: return "insufficient";
    case VidPhoneServiceResult::inventory_full: return "out_of_stock";
    case VidPhoneServiceResult::ineligible: return "ineligible";
    case VidPhoneServiceResult::discount_enabled: return "discount_enabled";
    case VidPhoneServiceResult::unavailable: return std::nullopt;
    }
    return std::nullopt;
}

// The contact-services commands use a fixed one-time price and write into the
// active runner's eight ordinary item slots. These inputs are views of the
// current typed game state, not ROM-addressed records.
inline VidPhoneServiceResult apply_vid_phone_service(
    const VidPhoneServiceAction action,
    std::optional<std::uint32_t>& funds,
    std::array<std::uint8_t, 8>& inventory_item_codes,
    std::array<std::uint8_t, 8>& inventory_phase_bytes,
    std::optional<std::uint32_t>& notebook_contacts_flags,
    std::optional<std::uint8_t>* runner_feature_flags = nullptr,
    std::optional<std::int8_t> runner_archetype_status = std::nullopt,
    std::optional<std::uint32_t>* group_item_flags = nullptr) noexcept {
    // Max's command 0x83 charges 6,000 and sets FFFC22 bit 10. This is a
    // typed host transaction because the item/flag word belongs to C++ state.
    if (action == VidPhoneServiceAction::issue_max_corporate_badge) {
        constexpr std::uint32_t kBadgePrice = 6000U;
        constexpr std::uint32_t kCorporateBadgeBit = 1U << 10U;
        if (!funds.has_value() || group_item_flags == nullptr ||
            !group_item_flags->has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*funds < kBadgePrice) {
            return VidPhoneServiceResult::insufficient_funds;
        }
        *funds -= kBadgePrice;
        **group_item_flags |= kCorporateBadgeBit;
        return VidPhoneServiceResult::badge_issued;
    }

    // 0x22BF6 charges 5,000 and sets FFFC2C bit 28. The 0x03FFFFFF
    // contact/call-history mask would drop that bit. Command 0x64
    // clears it when the Hollywood gate plays.
    if (action == VidPhoneServiceAction::issue_hollywood_badge) {
        constexpr std::uint32_t kBadgePrice = 0x1388U;
        constexpr std::uint32_t kHollywoodGateBit = 1U << 28U;
        if (!funds.has_value() || !notebook_contacts_flags.has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*funds < kBadgePrice) {
            return VidPhoneServiceResult::insufficient_funds;
        }
        *funds -= kBadgePrice;
        *notebook_contacts_flags |= kHollywoodGateBit;
        return VidPhoneServiceResult::hired;
    }

    // Gregory's command 0x77 (ROM 0x22C22) charges 4,000 and sets
    // FF018F bit 7, the Merlin's Lore corporate discount flag.
    if (action == VidPhoneServiceAction::purchase_merlins_lore_discount ||
        action == VidPhoneServiceAction::purchase_riannons_discount) {
        const bool riannons =
            action == VidPhoneServiceAction::purchase_riannons_discount;
        const std::uint32_t discount_price = riannons ? 1500U : 0x0FA0U;
        const std::uint8_t discount_bit = riannons ? 6U : 7U;
        if (runner_feature_flags == nullptr ||
            !runner_feature_flags->has_value() || !funds.has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*funds < discount_price) {
            return VidPhoneServiceResult::insufficient_funds;
        }
        *funds -= discount_price;
        **runner_feature_flags |=
            static_cast<std::uint8_t>(1U << discount_bit);
        return VidPhoneServiceResult::discount_enabled;
    }

    // Command 0x78 (ROM 0x22C46) permits only +0x9B values below 2,
    // then checks for a free ordinary inventory record before charging.
    if (action == VidPhoneServiceAction::purchase_power_focus) {
        if (!runner_archetype_status.has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*runner_archetype_status >= 2) {
            return VidPhoneServiceResult::ineligible;
        }
    }

    const auto contact_offer = [&action]()
        -> std::optional<std::pair<std::uint32_t, VidPhoneContactIndex>> {
        switch (action) {
        case VidPhoneServiceAction::hire_sharkey:
            return std::pair{2800U, VidPhoneContactIndex::sharkey};
        case VidPhoneServiceAction::hire_macklemann:
            return std::pair{8000U, VidPhoneContactIndex::macklemann};
        case VidPhoneServiceAction::hire_gregory_wilns:
            return std::pair{2000U, VidPhoneContactIndex::gregory_wilns};
        default:
            return std::nullopt;
        }
    }();
    if (contact_offer.has_value()) {
        if (!funds.has_value() || !notebook_contacts_flags.has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*funds < contact_offer->first) {
            return VidPhoneServiceResult::insufficient_funds;
        }
        *funds -= contact_offer->first;
        const auto contact_mask = static_cast<std::uint32_t>(
            1U << static_cast<unsigned>(contact_offer->second));
        // The ROM sets only the selected low contact bit and writes back the
        // full FFFC2C long. Preserve its unrelated high scene/access flags.
        *notebook_contacts_flags |= contact_mask;
        return VidPhoneServiceResult::hired;
    }

    if (!funds.has_value()) {
        return VidPhoneServiceResult::unavailable;
    }

    const auto free_slot = [&inventory_item_codes]() -> std::size_t {
        for (std::size_t slot = 0U; slot < inventory_item_codes.size(); ++slot) {
            if (inventory_item_codes[slot] == 0U) {
                return slot;
            }
        }
        return inventory_item_codes.size();
    }();
    if (free_slot == inventory_item_codes.size()) {
        return VidPhoneServiceResult::inventory_full;
    }

    const auto offer = vid_phone_supply_offer(action);
    if (!offer.has_value()) {
        return VidPhoneServiceResult::unavailable;
    }
    const auto* item = find_us_item_definition(offer->item_code);
    if (item == nullptr) {
        return VidPhoneServiceResult::unavailable;
    }
    if (*funds < offer->price) {
        return VidPhoneServiceResult::insufficient_funds;
    }

    *funds -= offer->price;
    inventory_item_codes[free_slot] = item->item_code;
    inventory_phase_bytes[free_slot] = offer->item_phase;
    return VidPhoneServiceResult::purchased;
}

// Kipp David, contact record 0x10AD8A. Commands 0x71-0x73 are handlers
// 0x22A54, 0x22A9E, and 0x22AF4. 0x71 does not set FF018F.
enum class VidPhoneDeckAction : std::uint8_t {
    purchase_excalibur,
    purchase_rebound,
    purchase_degrade,
    purchase_wilma_rebound,
    purchase_wilma_degrade,
};

enum class VidPhoneDeckResult : std::uint8_t {
    deck_installed,
    program_installed,
    insufficient_funds,
    insufficient_storage,
    unavailable,
};

inline std::optional<VidPhoneDeckAction> vid_phone_deck_action_for_id(
    const std::string_view service_id) noexcept {
    if (service_id == "vid_phone.purchase_excalibur") {
        return VidPhoneDeckAction::purchase_excalibur;
    }
    if (service_id == "vid_phone.purchase_rebound") {
        return VidPhoneDeckAction::purchase_rebound;
    }
    if (service_id == "vid_phone.purchase_degrade") {
        return VidPhoneDeckAction::purchase_degrade;
    }
    if (service_id == "vid_phone.purchase_wilma_rebound") {
        return VidPhoneDeckAction::purchase_wilma_rebound;
    }
    if (service_id == "vid_phone.purchase_wilma_degrade") {
        return VidPhoneDeckAction::purchase_wilma_degrade;
    }
    return std::nullopt;
}

inline std::optional<std::string_view> vid_phone_deck_script_result(
    const VidPhoneDeckResult result) noexcept {
    switch (result) {
    case VidPhoneDeckResult::deck_installed: return "deck_installed";
    case VidPhoneDeckResult::program_installed: return "program_installed";
    case VidPhoneDeckResult::insufficient_funds: return "insufficient";
    case VidPhoneDeckResult::insufficient_storage: return "storage_full";
    case VidPhoneDeckResult::unavailable: return std::nullopt;
    }
    return std::nullopt;
}

inline VidPhoneDeckResult apply_vid_phone_deck_service(
    const VidPhoneDeckAction action,
    std::optional<std::uint32_t>& funds,
    std::optional<CyberdeckRecord>& deck) noexcept {
    if (!funds.has_value()) {
        return VidPhoneDeckResult::unavailable;
    }
    if (action == VidPhoneDeckAction::purchase_excalibur) {
        constexpr std::uint32_t kExcaliburPrice = 0x2D2A8U;
        if (*funds < kExcaliburPrice) {
            return VidPhoneDeckResult::insufficient_funds;
        }
        *funds -= kExcaliburPrice;
        auto record = deck.value_or(CyberdeckRecord{});
        record.mpcp = 0x0CU;
        record.hardening = 5U;
        record.memory_megapulses = 0x1F4U;
        record.storage_megapulses = 0x3E8U;
        record.load_io_speed = 0x64U;
        record.response = 3U;
        record.model_index = 4;
        deck = std::move(record);
        return VidPhoneDeckResult::deck_installed;
    }

    const bool wilma_program =
        action == VidPhoneDeckAction::purchase_wilma_rebound ||
        action == VidPhoneDeckAction::purchase_wilma_degrade;
    // Kipp 0x22A9E/0x22AF4: 30,000, size as rating 4, store 6.
    // Wilma 0x22B4A/0x22BA0: 5,000, size as rating 3, store 3.
    const auto price = wilma_program ? 0x1388U : 0x7530U;
    const auto size_rating = static_cast<std::uint8_t>(wilma_program ? 3U : 4U);
    const auto stored_rating =
        static_cast<std::uint8_t>(wilma_program ? 3U : 6U);
    if (*funds < price) {
        return VidPhoneDeckResult::insufficient_funds;
    }
    const auto program = static_cast<std::uint8_t>(
        action == VidPhoneDeckAction::purchase_rebound ||
                action == VidPhoneDeckAction::purchase_wilma_rebound
            ? 3U
            : 2U);
    auto record = deck.value_or(CyberdeckRecord{});
    MatrixStorageUsage usage{};
    usage.total_memory = record.storage_megapulses.value_or(0U);
    usage.availability_flags =
        record.program_ratings.value_or(std::array<std::uint8_t, 12>{});
    if (record.data_files.has_value()) {
        for (std::size_t index = 0; index < usage.loaded_slot_usage.size();
             ++index) {
            usage.loaded_slot_usage[index] = static_cast<std::uint8_t>(
                (*record.data_files)[index].size);
        }
    }
    // 0x19558's free-storage word is compared with signed BPL.
    const auto available = static_cast<std::int16_t>(
        calculate_matrix_storage_capacity(usage).available_memory);
    const auto required = static_cast<std::int16_t>(
        matrix_program_storage_weight(program, size_rating));
    if (available < required) {
        return VidPhoneDeckResult::insufficient_storage;
    }
    *funds -= price;
    auto ratings =
        record.program_ratings.value_or(std::array<std::uint8_t, 12>{});
    ratings[program] = stored_rating;
    record.program_ratings = ratings;
    deck = std::move(record);
    return VidPhoneDeckResult::program_installed;
}

} // namespace shadowrun::ghidra_conversion
