#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kUsItemDefinitionCount = 50U;

// The confirmed D4540 fields used by converted field/menu routines.  The
// source table stores fifty 0x30-byte records.  item_code is the one-based
// code used by ordinary item records; field-action byte 0 indexes the array
// zero-based before reaching the same record.
struct UsItemDefinition {
    std::uint8_t item_code = 0;
    // D4540 +0x1E is a signed resource pointer. The native conversion keeps
    // the source meaning (present versus -1) rather than carrying a ROM
    // address into runtime code.
    bool has_detail_art = false;
    std::uint16_t base_cost = 0; // D4540 +0x22
    std::uint8_t source_combat_factor = 0; // D4540 +0x24
    std::int8_t signed_detail_asset_index = 0; // D4540 +0x25
    // D4540 +0x26.  0x130C0 treats this as a detail selector; 0x55948
    // consumes the same byte as the initial equipment-action difficulty.
    std::uint8_t detail_selector = 0;

    // Name the 0x55948 interpretation without duplicating the source byte.
    std::uint8_t source_action_difficulty() const {
        return detail_selector;
    }
};

using UsItemCatalog = std::array<UsItemDefinition, kUsItemDefinitionCount>;

// Typed US-ROM data extracted from D4540 with its 0x30-byte record stride.
const UsItemCatalog& us_item_catalog();

// Ordinary item references are one-based.  This helper intentionally does
// not model 0x57A44's zero-based field-action table index; that caller uses
// the catalog array directly after validating its source record byte.
const UsItemDefinition* find_us_item_definition(std::uint8_t item_code);

} // namespace shadowrun::ghidra_conversion
