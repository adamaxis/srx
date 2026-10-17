#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kUsItemDefinitionCount = 50U;

// Semantic item codes used by native host services. The numeric values are
// the one-based inventory codes in the US ROM's D4540 item catalog.
enum class UsItemCode : std::uint8_t {
    hk227_s_smg = 9U,
    frag_grenade = 13U,
};

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
    std::uint8_t source_action_parameter_tail = 0; // D4540 +0x27

    // Name the 0x55948 interpretation without duplicating the source byte.
    std::uint8_t source_action_difficulty() const {
        return detail_selector;
    }

    // ROM 0x104F6 reads one of these three aligned source words when a
    // slot-7 special item is present: +0x22, +0x24, or +0x26.
    std::uint16_t source_action_modifier_word(
        const std::int8_t source_field_selector) const {
        switch (source_field_selector) {
        case 0:
            return base_cost;
        case 1:
            return static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(source_combat_factor) << 8U) |
                static_cast<std::uint8_t>(signed_detail_asset_index));
        case 2:
            return static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(detail_selector) << 8U) |
                source_action_parameter_tail);
        default:
            return 0U;
        }
    }
};

using UsItemCatalog = std::array<UsItemDefinition, kUsItemDefinitionCount>;

constexpr std::size_t kUsSpecialItemSelectorCount = 16U;

// ROM C8AE0. The source indexes this signed byte table with the selected
// action index. Nonnegative values select D4540 words +0x22/+0x24/+0x26;
// negative values skip the slot-7 special-item adjustment. Values outside
// that three-word range are retained as source evidence and reported as an
// unresolved source-data boundary by the typed consumer.
struct UsSpecialItemModifierSelectorTable {
    std::array<std::int8_t, kUsSpecialItemSelectorCount> field_selectors{};
};

// Typed US-ROM data extracted from D4540 with its 0x30-byte record stride.
const UsItemCatalog& us_item_catalog();

const UsSpecialItemModifierSelectorTable&
us_special_item_modifier_selector_table();

// Ordinary item references are one-based.  This helper intentionally does
// not model 0x57A44's zero-based field-action table index; that caller uses
// the catalog array directly after validating its source record byte.
const UsItemDefinition* find_us_item_definition(std::uint8_t item_code);
const UsItemDefinition* find_us_item_definition(UsItemCode item_code);

} // namespace shadowrun::ghidra_conversion
