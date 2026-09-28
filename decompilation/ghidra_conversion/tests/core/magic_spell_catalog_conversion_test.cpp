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
    return 0;
}
