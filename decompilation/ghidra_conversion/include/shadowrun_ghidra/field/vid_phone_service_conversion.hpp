#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/weapon_accessory_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion {

enum class VidPhoneServiceAction : std::uint8_t {
    purchase_frag_grenades,
    purchase_hk227_s,
    purchase_maglock_passkey_level_3,
    purchase_maglock_passkey_level_5,
    purchase_smart_goggles,
    hire_sharkey,
    hire_macklemann,
};

enum class VidPhonePassageAction : std::uint8_t {
    salish_shidhe,
    sinsearach,
};

enum class VidPhonePassageResult : std::uint8_t {
    passage_requested,
    insufficient_funds,
    unavailable,
};

struct VidPhonePassageOffer {
    std::uint16_t destination_scene;
    std::uint32_t price;
};

inline VidPhonePassageOffer vid_phone_passage_offer(
    const VidPhonePassageAction action) noexcept {
    // ROM commands 0x6E/0x6F charge 3,000/5,000, then enter world modes 6/9.
    // The profile crosswalk names those destination maps Wilderness 1/4.
    switch (action) {
    case VidPhonePassageAction::salish_shidhe:
        return {6U, 3000U};
    case VidPhonePassageAction::sinsearach:
        return {9U, 5000U};
    }
    return {0U, 0U};
}

inline std::optional<VidPhonePassageAction>
vid_phone_passage_action_for_id(const std::string_view service_id) noexcept {
    if (service_id == "vid_phone.pass_salish_shidhe") {
        return VidPhonePassageAction::salish_shidhe;
    }
    if (service_id == "vid_phone.pass_sinsearach") {
        return VidPhonePassageAction::sinsearach;
    }
    return std::nullopt;
}

inline VidPhonePassageResult apply_vid_phone_passage(
    const VidPhonePassageAction action,
    std::optional<std::uint32_t>& funds) noexcept {
    if (!funds.has_value()) {
        return VidPhonePassageResult::unavailable;
    }
    const auto offer = vid_phone_passage_offer(action);
    if (*funds < offer.price) {
        return VidPhonePassageResult::insufficient_funds;
    }
    *funds -= offer.price;
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
    insufficient_funds,
    inventory_full,
    unavailable,
};

enum class VidPhoneContactIndex : std::uint8_t {
    sharkey = 2U,
    macklemann = 5U,
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
    case VidPhoneServiceAction::hire_sharkey:
    case VidPhoneServiceAction::hire_macklemann:
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
    if (service_id == "vid_phone.hire_sharkey") {
        return VidPhoneServiceAction::hire_sharkey;
    }
    if (service_id == "vid_phone.hire_macklemann") {
        return VidPhoneServiceAction::hire_macklemann;
    }
    return std::nullopt;
}

inline std::optional<std::string_view> vid_phone_service_script_result(
    const VidPhoneServiceResult result) noexcept {
    switch (result) {
    case VidPhoneServiceResult::purchased: return "purchased";
    case VidPhoneServiceResult::hired: return "hired";
    case VidPhoneServiceResult::insufficient_funds: return "insufficient";
    case VidPhoneServiceResult::inventory_full: return "out_of_stock";
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
    std::optional<std::uint32_t>& notebook_contacts_flags) noexcept {
    const auto contact_offer = [&action]()
        -> std::optional<std::pair<std::uint32_t, VidPhoneContactIndex>> {
        switch (action) {
        case VidPhoneServiceAction::hire_sharkey:
            return std::pair{2800U, VidPhoneContactIndex::sharkey};
        case VidPhoneServiceAction::hire_macklemann:
            return std::pair{8000U, VidPhoneContactIndex::macklemann};
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
        *notebook_contacts_flags =
            (*notebook_contacts_flags | contact_mask) & 0x03FFFFFFU;
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

} // namespace shadowrun::ghidra_conversion
