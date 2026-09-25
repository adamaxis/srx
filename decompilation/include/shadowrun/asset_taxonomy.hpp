#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace shadowrun::resources {

// The source item dispatch gives us a stable semantic split for item art.
// Keep this mapping in one place so extraction and the native asset loader
// cannot silently disagree about where an item belongs.
enum class ItemAssetGroup {
    Weapons,
    FirstAid,
    Miscellaneous,
    Magic,
    Armor,
};

constexpr ItemAssetGroup item_asset_group_for_code(std::size_t item_code) {
    if (item_code <= 0x0FU) {
        return ItemAssetGroup::Weapons;
    }
    if (item_code <= 0x12U) {
        return ItemAssetGroup::FirstAid;
    }
    if (item_code <= 0x16U) {
        return ItemAssetGroup::Miscellaneous;
    }
    if (item_code <= 0x2BU) {
        return ItemAssetGroup::Magic;
    }
    return ItemAssetGroup::Armor;
}

constexpr std::string_view item_asset_group_name(ItemAssetGroup group) {
    switch (group) {
    case ItemAssetGroup::Weapons:
        return "weapons";
    case ItemAssetGroup::FirstAid:
        return "first-aid";
    case ItemAssetGroup::Miscellaneous:
        return "miscellaneous";
    case ItemAssetGroup::Magic:
        return "magic";
    case ItemAssetGroup::Armor:
        return "armor";
    }
    return "miscellaneous";
}

inline std::string item_code_name(std::size_t item_code) {
    auto result = std::to_string(item_code);
    if (result.size() < 2U) {
        result.insert(result.begin(), '0');
    }
    return result;
}

inline std::string item_detail_asset_path(std::size_t item_code) {
    return "items/" +
           std::string(item_asset_group_name(
               item_asset_group_for_code(item_code))) +
           "/detail/item-" + item_code_name(item_code) + ".bmp";
}

inline std::string item_icon_asset_path(std::size_t item_code,
                                        std::size_t variant) {
    return "items/" +
           std::string(item_asset_group_name(
               item_asset_group_for_code(item_code))) +
           "/icons/item-" + item_code_name(item_code) + "-" +
           std::to_string(variant) + ".bmp";
}

} // namespace shadowrun::resources
