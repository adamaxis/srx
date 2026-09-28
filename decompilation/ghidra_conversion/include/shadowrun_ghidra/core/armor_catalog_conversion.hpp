#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kUsArmorDefinitionCount = 7U;

// Confirmed fields from the seven 0x30-byte records at ROM D4D50.  The same
// bytes are consumed as the primary/secondary armor modifiers by 0x55E92 and
// 0x55EB4, and as the two armor bar selectors by the command overview.
struct UsArmorDefinition {
    std::uint8_t armor_index = 0; // one-based active-record +0x76 value
    std::uint8_t primary_modifier = 0; // D4D50 +0x25
    std::uint8_t secondary_modifier = 0; // D4D50 +0x26
};

using UsArmorCatalog =
    std::array<UsArmorDefinition, kUsArmorDefinitionCount>;

const UsArmorCatalog& us_armor_catalog();

// Armor records are selected by the one-based active-runner +0x76 value.
const UsArmorDefinition* find_us_armor_definition(
    std::uint8_t armor_index);

} // namespace shadowrun::ghidra_conversion
