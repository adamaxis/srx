#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

const UsMagicSpellCatalog kUsMagicSpells{
    MagicSpellDefinition{0U, "Flame Dart", 1, 1U, 0U, 1, 0},
    MagicSpellDefinition{1U, "Flame Bolt", 2, 1U, 1U, 3, 0},
    MagicSpellDefinition{2U, "Hellblast", 3, 3U, 3U, 10, 0},
    MagicSpellDefinition{3U, "Invisibility", -1, 0U, 1U, 3, 0},
    MagicSpellDefinition{4U, "Sleep", 1, 0U, 2U, 6, -1},
    MagicSpellDefinition{5U, "Stink", -1, 0U, 2U, 6, -1},
    MagicSpellDefinition{6U, "Super Barrier", -1, 2U, 2U, 6, 0},
    MagicSpellDefinition{7U, "Mana Zap", 0, 0U, 0U, 1, -1},
    MagicSpellDefinition{8U, "Mana Blast", 1, 0U, 1U, 3, -1},
    MagicSpellDefinition{9U, "Mana Storm", 2, 1U, 2U, 6, -1},
    MagicSpellDefinition{10U, "Rockskin", -1, 2U, 1U, 3, 0},
    MagicSpellDefinition{11U, "Heal Wounds", -1, 0U, 1U, 3, 0},
    MagicSpellDefinition{12U, "Confusion", -1, 0U, 2U, 6, -1},
};

const UsActionEffectCatalog kUsActionEffects = [] {
    constexpr std::array<std::uint8_t, kUsActionEffectCount> kSourceBytes{
        0xFFU, 0xFDU, 0xFAU, 0xF6U, 0x00U, 0x03U, 0x05U, 0x03U,
        0x01U, 0x01U, 0x03U, 0x01U, 0x0AU, 0x02U, 0xFFU, 0x00U};

    UsActionEffectCatalog effects{};
    for (std::size_t index = 0; index < effects.size(); ++index) {
        effects[index] = {kSourceBytes[index],
                          static_cast<std::uint8_t>(0U - kSourceBytes[index])};
    }
    return effects;
}();

const UsMagicSpellTableCatalog kUsMagicSpellTables = [] {
    UsMagicSpellTableCatalog tables{};

    // ROM 0x166AD8, row 0: count 3, codes 5/1/3.
    tables[0].entry_count = 3U;
    tables[0].entries[0] = {5U, 3U};
    tables[0].entries[1] = {1U, 1U};
    tables[0].entries[2] = {3U, 1U};

    // ROM 0x166AFA, row 1: count 2, codes 0/11.
    tables[1].entry_count = 2U;
    tables[1].entries[0] = {0U, 2U};
    tables[1].entries[1] = {11U, 2U};

    // ROM 0x166B1C, row 2: count 4, codes 7/9/4/11.
    tables[2].entry_count = 4U;
    tables[2].entries[0] = {7U, 1U};
    tables[2].entries[1] = {9U, 4U};
    tables[2].entries[2] = {4U, 3U};
    tables[2].entries[3] = {11U, 2U};

    // ROM 0x166B3E, row 3: count 5, codes 8/2/6/3/4.
    tables[3].entry_count = 5U;
    tables[3].entries[0] = {8U, 3U};
    tables[3].entries[1] = {2U, 3U};
    tables[3].entries[2] = {6U, 2U};
    tables[3].entries[3] = {3U, 3U};
    tables[3].entries[4] = {4U, 4U};

    // ROM 0x166B60, row 4: count 2, codes 7/10.
    tables[4].entry_count = 2U;
    tables[4].entries[0] = {7U, 3U};
    tables[4].entries[1] = {10U, 3U};

    return tables;
}();

} // namespace

const UsMagicSpellCatalog& us_magic_spell_catalog() {
    return kUsMagicSpells;
}

const UsActionEffectCatalog& us_action_effect_catalog() {
    return kUsActionEffects;
}

const UsActionEffectDefinition* find_us_action_effect(
    const std::uint8_t effect_table_index) {
    if (effect_table_index >= kUsActionEffectCount) {
        return nullptr;
    }
    return &kUsActionEffects[effect_table_index];
}

const MagicSpellDefinition* find_us_magic_spell(
    const std::uint8_t spell_code) {
    if (spell_code >= kUsMagicSpellCount) {
        return nullptr;
    }
    return &kUsMagicSpells[spell_code];
}

const UsMagicSpellTableCatalog& us_magic_spell_table_catalog() {
    return kUsMagicSpellTables;
}

const MagicSpellTable* find_us_magic_spell_table(
    const std::uint16_t table_index) {
    if (table_index >= kUsMagicSpellTableCount) {
        return nullptr;
    }
    return &kUsMagicSpellTables[table_index];
}

} // namespace shadowrun::ghidra_conversion
