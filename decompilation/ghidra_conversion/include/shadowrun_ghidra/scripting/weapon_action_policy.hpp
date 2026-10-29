#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shadowrun::ghidra_conversion::scripting {

// Stable game item identities for the twelve firearm actions. Source item
// codes are translated at the host boundary and are not part of Lua policy.
enum class ScriptWeaponActionId : std::uint8_t {
    streetline_special,
    model_101t,
    american_l36,
    security_500,
    warhawk,
    max_power,
    predator,
    ak_97,
    hk227_s,
    mach_22,
    allegiance,
    roomsweeper,
};

enum class ScriptWeaponSkillGroup : std::uint8_t {
    pistols,
    smgs,
    shotguns,
};

struct ScriptWeaponActionPolicy {
    ScriptWeaponSkillGroup skill_group = ScriptWeaponSkillGroup::pistols;
    std::int8_t difficulty_adjustment = 0;
    bool benefits_from_targeting_assist = false;
};

inline constexpr std::array<ScriptWeaponActionId, 12>
    kScriptWeaponActionIds{{
        ScriptWeaponActionId::streetline_special,
        ScriptWeaponActionId::model_101t,
        ScriptWeaponActionId::american_l36,
        ScriptWeaponActionId::security_500,
        ScriptWeaponActionId::warhawk,
        ScriptWeaponActionId::max_power,
        ScriptWeaponActionId::predator,
        ScriptWeaponActionId::ak_97,
        ScriptWeaponActionId::hk227_s,
        ScriptWeaponActionId::mach_22,
        ScriptWeaponActionId::allegiance,
        ScriptWeaponActionId::roomsweeper,
    }};

inline constexpr std::string_view script_weapon_action_name(
    const ScriptWeaponActionId id) {
    switch (id) {
    case ScriptWeaponActionId::streetline_special: return "streetline";
    case ScriptWeaponActionId::model_101t: return "model_101t";
    case ScriptWeaponActionId::american_l36: return "american_l36";
    case ScriptWeaponActionId::security_500: return "security_500";
    case ScriptWeaponActionId::warhawk: return "warhawk";
    case ScriptWeaponActionId::max_power: return "max_power";
    case ScriptWeaponActionId::predator: return "predator";
    case ScriptWeaponActionId::ak_97: return "ak_97";
    case ScriptWeaponActionId::hk227_s: return "hk227_s";
    case ScriptWeaponActionId::mach_22: return "mach_22";
    case ScriptWeaponActionId::allegiance: return "allegiance";
    case ScriptWeaponActionId::roomsweeper: return "roomsweeper";
    }
    return {};
}

inline constexpr std::string_view script_weapon_skill_group_name(
    const ScriptWeaponSkillGroup group) {
    switch (group) {
    case ScriptWeaponSkillGroup::pistols: return "pistols";
    case ScriptWeaponSkillGroup::smgs: return "smgs";
    case ScriptWeaponSkillGroup::shotguns: return "shotguns";
    }
    return {};
}

inline constexpr std::optional<ScriptWeaponActionId>
script_weapon_action_from_source_code(const std::uint8_t code) {
    return code >= 1U && code <= kScriptWeaponActionIds.size()
        ? std::optional<ScriptWeaponActionId>{
              kScriptWeaponActionIds[static_cast<std::size_t>(code - 1U)]}
        : std::nullopt;
}

inline constexpr ScriptWeaponSkillGroup script_weapon_skill_group(
    const ScriptWeaponActionId id) {
    const auto index = static_cast<std::size_t>(id);
    return index < 7U ? ScriptWeaponSkillGroup::pistols
        : index < 10U ? ScriptWeaponSkillGroup::smgs
                      : ScriptWeaponSkillGroup::shotguns;
}

// ROM 0x556C6 indexes a consecutive set of weapon modifiers from actor
// +0x6E by item code minus one. Decode those source flags once so gameplay
// scripts receive the actual policy: skill family, difficulty change, and
// whether targeting hardware can improve the roll.
inline constexpr ScriptWeaponActionPolicy script_weapon_action_policy(
    const ScriptWeaponActionId id, const std::uint8_t source_modifier) {
    const auto source_code = static_cast<std::uint8_t>(id) + 1U;
    auto adjustment = static_cast<std::int8_t>(
        source_code >= 8U && source_code <= 10U ? 3 : 0);
    if (source_code >= 8U && source_code <= 10U) {
        if ((source_modifier & 0x10U) != 0U) {
            adjustment = static_cast<std::int8_t>(adjustment - 2);
        } else if ((source_modifier & 0x20U) != 0U) {
            adjustment = static_cast<std::int8_t>(adjustment - 3);
        }
    }
    if ((source_modifier & 0x08U) != 0U) {
        adjustment = static_cast<std::int8_t>(adjustment - 1);
    }
    return {script_weapon_skill_group(id), adjustment,
            (source_modifier & 0x01U) != 0U};
}

inline constexpr std::array<ScriptWeaponActionPolicy, 12>
default_script_weapon_action_policies() {
    std::array<ScriptWeaponActionPolicy, 12> policies{};
    for (std::size_t index = 0U; index < policies.size(); ++index) {
        policies[index].skill_group =
            script_weapon_skill_group(kScriptWeaponActionIds[index]);
    }
    return policies;
}

} // namespace shadowrun::ghidra_conversion::scripting
