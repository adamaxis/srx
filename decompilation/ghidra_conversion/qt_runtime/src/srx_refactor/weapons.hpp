// weapons.hpp: GENERATED from data/weapons.lua; do not edit. Sources: item records 0xD4540 (+0x22..+0x27),
// sounds 0x15318/0x15336, automatic 0x38AC/0x556C6, reload mode 0x14226, accessory rules 0x5709E.
#pragma once
#include <array>
#include <string_view>
#include "enums.hpp"
namespace srx {
struct Weapon {
    ItemId id; std::string_view name; WeaponClass weapon_class;
    std::uint16_t price; std::uint8_t clip_size, damage_class, power, band;
    SoundId fire_sound, silenced_sound; bool automatic; std::uint8_t rounds_per_shot, reload_equip_mode;
    std::uint8_t accepts_mask;   // bit (1 << Accessory)
    constexpr bool accepts(Accessory a) const { return (accepts_mask >> v(a)) & 1; }
    constexpr SoundId sound(bool silenced) const { return silenced ? silenced_sound : fire_sound; }
};
inline constexpr std::array<Weapon, 15> kWeapons = {{
    {ItemId::STREETLINE_SPECIAL, "Streetline Special", WeaponClass::LIGHT_PISTOL, 100, 6, 0, 4, 0, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::MODEL_101T, "Model 101T Light Pistol", WeaponClass::LIGHT_PISTOL, 350, 12, 0, 6, 0, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::AMERICAN_L36, "American L36 Light Pistol", WeaponClass::LIGHT_PISTOL, 300, 11, 0, 6, 0, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::SECURITY_500, "Security 500 Light Pistol", WeaponClass::LIGHT_PISTOL, 450, 25, 0, 6, 0, SoundId::SHOT_LIGHT_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::WARHAWK, "Warhawk Heavy Pistol", WeaponClass::HEAVY_PISTOL, 500, 6, 1, 10, 0, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 2, 0x0B},
    {ItemId::MAX_POWER, "Max-Power Heavy Pistol", WeaponClass::HEAVY_PISTOL, 450, 10, 1, 9, 0, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::PREDATOR, "Predator Heavy Pistol", WeaponClass::HEAVY_PISTOL, 450, 15, 1, 9, 0, SoundId::SHOT_HEAVY_PISTOL, SoundId::SHOT_SILENCED, false, 1, 0, 0x0B},
    {ItemId::AK97, "AK-97 SMG", WeaponClass::SMG, 1000, 30, 1, 6, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0, 0x3D},
    {ItemId::HK227S, "HK227-S SMG", WeaponClass::SMG, 1500, 28, 1, 7, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0, 0x3D},
    {ItemId::MACH22, "Mach 22 SMG", WeaponClass::SMG, 800, 24, 1, 6, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, true, 3, 0, 0x3D},
    {ItemId::ALLEGIANCE, "Allegiance Shotgun", WeaponClass::SHOTGUN, 1400, 5, 2, 10, 0, SoundId::SHOT_SHOTGUN, SoundId::SHOT_SILENCED, false, 1, 2, 0x3D},
    {ItemId::ROOMSWEEPER, "Roomsweeper Shotgun", WeaponClass::SHOTGUN, 1000, 8, 2, 9, 0, SoundId::SHOT_SHOTGUN, SoundId::SHOT_SILENCED, false, 1, 2, 0x3D},
    {ItemId::FRAG_GRENADE, "Frag Grenade", WeaponClass::THROWN, 300, 6, 2, 8, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0, 0x3D},
    {ItemId::SCATTER_GRENADE, "Scatter Grenade", WeaponClass::THROWN, 180, 6, 1, 6, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0, 0x3D},
    {ItemId::CONCUSSION_GRENADE, "Concussion Grenade", WeaponClass::THROWN, 180, 6, 1, 10, 0, SoundId::SHOT_SMG_OR_THROWN, SoundId::SHOT_SILENCED_SMG_OR_THROWN, false, 1, 0, 0x3D},
}};
constexpr const Weapon& weapon(ItemId id) { return kWeapons[v(id) - 1]; }
}  // namespace srx
