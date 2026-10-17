#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/weapon_accessory_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion {

enum class VidPhoneServiceAction : std::uint8_t {
    purchase_frag_grenades,
    purchase_hk227_s,
    hire_sharkey,
};

enum class VidPhoneServiceResult : std::uint8_t {
    purchased,
    hired,
    insufficient_funds,
    inventory_full,
    unavailable,
};

enum class VidPhoneContactIndex : std::uint8_t {
    sharkey = 2U,
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
    case VidPhoneServiceAction::hire_sharkey:
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
    if (service_id == "vid_phone.hire_sharkey") {
        return VidPhoneServiceAction::hire_sharkey;
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
    constexpr std::uint32_t kSharkeyIntroductionPrice = 2800U;

    if (action == VidPhoneServiceAction::hire_sharkey) {
        if (!funds.has_value() || !notebook_contacts_flags.has_value()) {
            return VidPhoneServiceResult::unavailable;
        }
        if (*funds < kSharkeyIntroductionPrice) {
            return VidPhoneServiceResult::insufficient_funds;
        }
        *funds -= kSharkeyIntroductionPrice;
        const auto sharkey_mask = static_cast<std::uint32_t>(
            1U << static_cast<unsigned>(VidPhoneContactIndex::sharkey));
        *notebook_contacts_flags =
            (*notebook_contacts_flags | sharkey_mask) & 0x03FFFFFFU;
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
