#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

// US Genesis ROM D4D50, seven records at a 0x30-byte stride.  The remaining
// bytes carry encoded armor labels and other source presentation data; only
// the modifier/selector fields with verified converted consumers are named.
const UsArmorCatalog kUsArmors{{
    {1U, 0x02U, 0x01U},
    {2U, 0x01U, 0x02U},
    {3U, 0x05U, 0x03U},
    {4U, 0x04U, 0x03U},
    {5U, 0x03U, 0x02U},
    {6U, 0x06U, 0x04U},
    {7U, 0x08U, 0x06U},
}};

} // namespace

const UsArmorCatalog& us_armor_catalog() {
    return kUsArmors;
}

const UsArmorDefinition* find_us_armor_definition(
    const std::uint8_t armor_index) {
    if (armor_index == 0U || armor_index > kUsArmorDefinitionCount) {
        return nullptr;
    }
    return &kUsArmors[static_cast<std::size_t>(armor_index - 1U)];
}

const UsArmorDefinition* find_us_armor_definition_for_item_code(
    const std::uint8_t item_code) {
    if (item_code < 0x2CU || item_code > 0x32U) {
        return nullptr;
    }
    return find_us_armor_definition(
        static_cast<std::uint8_t>(item_code - 0x2BU));
}

namespace {

std::uint16_t subtract_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const UsArmorCatalog& catalog,
    const bool primary) {
    if (armor_index == 0U || armor_index > catalog.size()) {
        return value;
    }
    const auto& armor = catalog[static_cast<std::size_t>(armor_index - 1U)];
    const auto modifier =
        primary ? armor.primary_modifier : armor.secondary_modifier;
    const auto low_byte = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(value) - modifier);
    return static_cast<std::uint16_t>((value & 0xFF00U) | low_byte);
}

} // namespace

std::uint16_t subtract_us_armor_primary_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const UsArmorCatalog& catalog) {
    return subtract_modifier(value, armor_index, catalog, true);
}

std::uint16_t subtract_us_armor_secondary_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const UsArmorCatalog& catalog) {
    return subtract_modifier(value, armor_index, catalog, false);
}

} // namespace shadowrun::ghidra_conversion
