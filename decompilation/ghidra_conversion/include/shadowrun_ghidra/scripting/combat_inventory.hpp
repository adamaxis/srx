#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::scripting {

struct ScriptCombatInventoryItemView {
    std::string_view item_id;
    std::uint8_t slot = 0;
    // Lua sees item-specific state, never the source phase byte.
    std::string_view value_property;
    std::uint8_t value = 0;
};

inline constexpr std::array<std::string_view, 50> kScriptItemIds{{
    "streetline", "model_101t", "american_l36", "security_500",
    "warhawk", "max_power", "predator", "ak_97", "hk227_s", "mach_22",
    "allegiance", "roomsweeper", "frag", "scatter", "concussion",
    "medkit", "stim_patch", "trauma_patch", "fetish", "maglock_passkey",
    "electronic_kit", "smart_goggles", "enchanted_dagger", "power_focus",
    "flame_dart_focus", "flame_bolt_focus", "hellblast_focus",
    "invisibility_focus", "sleep_focus", "stink_focus",
    "supr_barrier_focus", "mana_zap_focus", "mana_blast_focus",
    "mana_storm_focus", "rockskin_focus", "heal_wounds_focus",
    "confusion_focus", "barrier_focus", "protection_talisman",
    "combat_sense_spell_lock", "rat_totem", "gator_totem", "bear_totem",
    "armor_vest", "armor_clothing", "armor_jacket", "vest_with_plates",
    "lined_duster", "light_combat_armor", "heavy_combat_armor",
}};

inline constexpr std::array<std::string_view, 7> kScriptArmorItemIds{{
    "armor_vest", "armor_clothing", "armor_jacket", "vest_with_plates",
    "lined_duster", "light_combat_armor", "heavy_combat_armor",
}};

inline constexpr std::optional<std::string_view>
script_item_id_from_source_code(const std::uint8_t source_code) {
    return source_code >= 1U && source_code <= kScriptItemIds.size()
        ? std::optional<std::string_view>{
              kScriptItemIds[static_cast<std::size_t>(source_code - 1U)]}
        : std::nullopt;
}

inline constexpr std::optional<std::uint8_t>
script_item_source_code_from_id(const std::string_view item_id) {
    for (std::size_t index = 0U; index < kScriptItemIds.size(); ++index) {
        if (kScriptItemIds[index] == item_id) {
            return static_cast<std::uint8_t>(index + 1U);
        }
    }
    return std::nullopt;
}

inline constexpr std::optional<std::string_view>
script_armor_item_id_from_source_index(const std::uint8_t source_index) {
    return source_index >= 1U &&
                   source_index <= kScriptArmorItemIds.size()
        ? std::optional<std::string_view>{
              kScriptArmorItemIds[static_cast<std::size_t>(source_index - 1U)]}
        : std::nullopt;
}

inline constexpr std::optional<std::uint8_t>
script_armor_source_index_from_item_id(const std::string_view item_id) {
    for (std::size_t index = 0U; index < kScriptArmorItemIds.size(); ++index) {
        if (kScriptArmorItemIds[index] == item_id) {
            return static_cast<std::uint8_t>(index + 1U);
        }
    }
    return std::nullopt;
}

inline constexpr std::optional<ScriptCombatInventoryItemView>
script_combat_inventory_item(const std::uint8_t position,
                             const std::uint8_t source_code,
                             const std::uint8_t detail_value) {
    const auto item_id = script_item_id_from_source_code(source_code);
    if (!item_id.has_value() || position < 1U || position > 8U) {
        return std::nullopt;
    }

    std::string_view property;
    if (source_code <= 12U) {
        property = "loaded_rounds";
    } else if (source_code >= 13U && source_code <= 19U) {
        property = "uses_remaining";
    } else if (source_code == 24U) {
        property = "sorcery_bonus";
    } else if (source_code >= 25U && source_code <= 38U) {
        property = "spell_action_bonus";
    } else if (source_code == 39U) {
        property = "rating";
    } else if (source_code == 40U) {
        property = "command_bonus";
    }
    return ScriptCombatInventoryItemView{
        *item_id, position, property,
        property.empty() ? static_cast<std::uint8_t>(0U) : detail_value};
}

} // namespace shadowrun::ghidra_conversion::scripting
