// weapons.hpp: GENERATED from semantic data/weapons.lua; do not edit.
#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include "enums.hpp"
namespace srx {
struct Weapon {
    std::string_view id, name; WeaponClass weapon_class;
    std::uint16_t purchase_base;
    std::uint8_t magazine_capacity, damage_effect, power_rating;
    SoundId fire_sound, silenced_sound; bool automatic;
    std::uint8_t rounds_per_shot; std::uint8_t accepts_mask;
    constexpr bool accepts(Accessory a) const { return (accepts_mask >> v(a)) & 1; }
    constexpr SoundId sound(bool silenced) const { return silenced ? silenced_sound : fire_sound; }
};
inline constexpr std::array<Weapon, 15> kWeapons = {{
    {"streetline", "Streetline Special", WeaponClass::LIGHT_PISTOL, 100, 6, 1, 4, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"model_101t", "Model 101T Light Pistol", WeaponClass::LIGHT_PISTOL, 350, 12, 1, 6, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"american_l36", "American L36 Light Pistol", WeaponClass::LIGHT_PISTOL, 300, 11, 1, 6, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"security_500", "Security 500 Light Pistol", WeaponClass::LIGHT_PISTOL, 450, 25, 1, 6, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"warhawk", "Warhawk Heavy Pistol", WeaponClass::HEAVY_PISTOL, 500, 6, 3, 10, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"max_power", "Max-Power Heavy Pistol", WeaponClass::HEAVY_PISTOL, 450, 10, 3, 9, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"predator", "Predator Heavy Pistol", WeaponClass::HEAVY_PISTOL, 450, 15, 3, 9, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0x0B},
    {"ak_97", "AK-97 SMG", WeaponClass::SMG, 1000, 30, 3, 6, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0x3D},
    {"hk227_s", "HK227-S SMG", WeaponClass::SMG, 1500, 28, 3, 7, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0x3D},
    {"mach_22", "Mach 22 SMG", WeaponClass::SMG, 800, 24, 3, 6, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0x3D},
    {"allegiance", "Allegiance Shotgun", WeaponClass::SHOTGUN, 1400, 5, 6, 10, SoundId::SHOT_SHOTGUN, SoundId::SHOT_SILENCED, false, 1, 0x3D},
    {"roomsweeper", "Roomsweeper Shotgun", WeaponClass::SHOTGUN, 1000, 8, 6, 9, SoundId::SHOT_SHOTGUN, SoundId::SHOT_SILENCED, false, 1, 0x3D},
    {"frag", "Frag Grenade", WeaponClass::THROWN, 300, 6, 6, 8, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0x3D},
    {"scatter", "Scatter Grenade", WeaponClass::THROWN, 180, 6, 3, 6, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0x3D},
    {"concussion", "Concussion Grenade", WeaponClass::THROWN, 180, 6, 3, 10, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0x3D},
}};
constexpr const Weapon* weapon(std::string_view id) {
    for (const auto& record : kWeapons) if (record.id == id) return &record;
    return nullptr;
}
}  // namespace srx
