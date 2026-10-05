#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kUsMagicSpellCount = 13U;
constexpr std::size_t kUsMagicSpellTableCount = 5U;
constexpr std::size_t kMagicSpellTableEntryCapacity = 14U;
constexpr std::size_t kUsActionEffectCount = 16U;

// Typed rows decoded from the US Genesis D4EA0 spell table. The source row's
// +0x24 byte indexes 166AD4; `base_effect` is the resulting negated effect
// value used by 0x10660, so callers do not perform ROM-table arithmetic.
struct MagicSpellDefinition {
    std::uint8_t spell_code = 0;
    std::string_view display_name;
    std::int8_t availability_byte = -1; // D4EA0 +0x25
    std::uint8_t difficulty_modifier = 0; // D4EA0 +0x26
    std::uint8_t effect_table_index = 0; // D4EA0 +0x24
    std::int16_t base_effect = 0; // decoded and negated 166AD4 value
    std::int8_t attribute_selector = 0; // D4EA0 +0x27
};

using UsMagicSpellCatalog =
    std::array<MagicSpellDefinition, kUsMagicSpellCount>;

// The shared 0x166AD4 byte table is consumed directly by the equipment
// action resolver at 0x55948 as well as indirectly by D4EA0 spell rows.  The
// source performs NEG.B after loading a byte, so keep both the captured byte
// and the zero-extended wrapped result.  The latter is intentionally not a
// signed host value: a source byte such as 0x03 becomes 0xFD in the data
// register after NEG.B.
struct UsActionEffectDefinition {
    std::uint8_t source_byte = 0;
    std::uint8_t negated_source_byte = 0;
};

using UsActionEffectCatalog =
    std::array<UsActionEffectDefinition, kUsActionEffectCount>;

// Each runner's +0xA0-selected row at ROM 0x166AD8 stores a DBF last index,
// followed by that many entries plus one. `entry_count` is the native total;
// each entry contains a spell code followed by the force/value limit.
struct MagicSpellTableEntry {
    std::uint8_t spell_code = 0;
    std::uint8_t force_limit = 0;
};

struct MagicSpellTable {
    std::uint8_t entry_count = 0;
    std::array<MagicSpellTableEntry, kMagicSpellTableEntryCapacity> entries{};
};

using UsMagicSpellTableCatalog =
    std::array<MagicSpellTable, kUsMagicSpellTableCount>;

// The catalog is source data, not a runtime ROM lookup. It covers all 13 US
// spell rows in D4EA0, including unavailable spells such as Invisibility,
// Stink, Super Barrier, Rockskin, Heal Wounds, and Confusion.
const UsMagicSpellCatalog& us_magic_spell_catalog();

// Typed extraction of the sixteen bytes at ROM 0x166AD4.  This is source
// data for action resolution, not a runtime ROM lookup.
const UsActionEffectCatalog& us_action_effect_catalog();

const UsActionEffectDefinition* find_us_action_effect(
    std::uint8_t effect_table_index);

const MagicSpellDefinition* find_us_magic_spell(std::uint8_t spell_code);

const UsMagicSpellTableCatalog& us_magic_spell_table_catalog();

const MagicSpellTable* find_us_magic_spell_table(std::uint16_t table_index);

} // namespace shadowrun::ghidra_conversion
