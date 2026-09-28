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

} // namespace shadowrun::ghidra_conversion
