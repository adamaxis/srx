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

} // namespace

const UsMagicSpellCatalog& us_magic_spell_catalog() {
    return kUsMagicSpells;
}

const MagicSpellDefinition* find_us_magic_spell(
    const std::uint8_t spell_code) {
    if (spell_code >= kUsMagicSpellCount) {
        return nullptr;
    }
    return &kUsMagicSpells[spell_code];
}

} // namespace shadowrun::ghidra_conversion
