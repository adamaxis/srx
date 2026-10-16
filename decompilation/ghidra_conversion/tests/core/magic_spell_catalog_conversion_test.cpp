#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"

#include <array>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto& spells = us_magic_spell_catalog();
    if (spells.size() != kUsMagicSpellCount ||
        spells.front().display_name != "Flame Dart" ||
        spells.back().display_name != "Confusion") {
        return 1;
    }

    constexpr std::array<std::int16_t, kUsMagicSpellCount> kEffects{
        1, 3, 10, 3, 6, 6, 6, 1, 3, 6, 3, 3, 6};
    constexpr std::array<std::uint8_t, kUsMagicSpellCount> kDifficulty{
        1, 1, 3, 0, 0, 0, 2, 0, 0, 1, 2, 0, 0};
    constexpr std::array<std::int8_t, kUsMagicSpellCount> kAvailability{
        1, 2, 3, -1, 1, -1, -1, 0, 1, 2, -1, -1, -1};
    constexpr std::array<std::int8_t, kUsMagicSpellCount> kSelectors{
        0, 0, 0, 0, -1, -1, 0, -1, -1, -1, 0, 0, -1};

    for (std::size_t index = 0; index < spells.size(); ++index) {
        const auto& spell = spells[index];
        if (spell.spell_code != index || spell.base_effect != kEffects[index] ||
            spell.difficulty_modifier != kDifficulty[index] ||
            spell.availability_byte != kAvailability[index] ||
            spell.attribute_selector != kSelectors[index]) {
            return 2;
        }
    }

    const auto* mana_storm = find_us_magic_spell(9U);
    if (mana_storm == nullptr || mana_storm->display_name != "Mana Storm" ||
        mana_storm->effect_table_index != 2U ||
        find_us_magic_spell(13U) != nullptr) {
        return 3;
    }

    const auto& effects = us_action_effect_catalog();
    if (effects.size() != kUsActionEffectCount ||
        effects[0].source_byte != 0xFFU ||
        effects[0].negated_source_byte != 0x01U ||
        effects[1].source_byte != 0xFDU ||
        effects[1].negated_source_byte != 0x03U ||
        effects[3].negated_source_byte != 0x0AU ||
        effects[5].negated_source_byte != 0xFDU ||
        find_us_action_effect(15U) != &effects[15] ||
        find_us_action_effect(16U) != nullptr) {
        return 5;
    }

    const auto& tables = us_magic_spell_table_catalog();
    if (tables.size() != kUsMagicSpellTableCount ||
        tables[0].entry_count != 4U ||
        tables[0].entries[0].spell_code != 5U ||
        tables[0].entries[0].force_limit != 3U ||
        tables[0].entries[2].spell_code != 3U ||
        tables[0].entries[3].spell_code != 10U ||
        tables[1].entry_count != 3U ||
        tables[1].entries[0].spell_code != 0U ||
        tables[1].entries[2].spell_code != 10U ||
        tables[1].entries[2].force_limit != 3U ||
        tables[2].entry_count != 5U ||
        tables[2].entries[1].spell_code != 9U ||
        tables[2].entries[1].force_limit != 4U ||
        tables[2].entries[4].spell_code != 12U ||
        tables[3].entry_count != 6U ||
        tables[3].entries[4].spell_code != 4U ||
        tables[3].entries[5].spell_code != 11U ||
        tables[4].entry_count != 3U ||
        tables[4].entries[1].spell_code != 10U ||
        tables[4].entries[2].spell_code != 11U ||
        find_us_magic_spell_table(4U) != &tables[4] ||
        find_us_magic_spell_table(5U) != nullptr) {
        return 4;
    }
    return 0;
}
