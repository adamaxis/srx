-- Generates weapons.hpp (C++ mirror of data/weapons.lua):  lua5.4 gen_weapons_hpp.lua > weapons.hpp
package.path = "./?.lua;" .. package.path
local W, E = require("data.weapons"), require("enums")
local function key(tbl, val) for k, v in pairs(tbl) do if v == val then return k end end end
local o = {
"// weapons.hpp: GENERATED from data/weapons.lua; do not edit. Sources: item records 0xD4540 (+0x22..+0x27),",
"// sounds 0x15318/0x15336, automatic 0x38AC/0x556C6, reload mode 0x14226, accessory rules 0x5709E.",
"#pragma once", "#include <array>", "#include <string_view>", '#include "enums.hpp"', "namespace srx {",
"struct Weapon {",
"    ItemId id; std::string_view name; WeaponClass weapon_class;",
"    std::uint16_t price; std::uint8_t clip_size, damage_class, power, band;",
"    SoundId fire_sound, silenced_sound; bool automatic; std::uint8_t rounds_per_shot, reload_equip_mode;",
"    std::uint8_t accepts_mask;   // bit (1 << Accessory)",
"    constexpr bool accepts(Accessory a) const { return (accepts_mask >> v(a)) & 1; }",
"    constexpr SoundId sound(bool silenced) const { return silenced ? silenced_sound : fire_sound; }",
"};",
"inline constexpr std::array<Weapon, 15> kWeapons = {{" }
for code = 1, 15 do
    local w = W[code]
    local mask = 0
    for a, ok in pairs(w.accepts) do if ok then mask = mask | (1 << a) end end
    o[#o + 1] = string.format('    {ItemId::%s, "%s", WeaponClass::%s, %d, %d, %d, %d, %d, SoundId::%s, SoundId::%s, %s, %d, %d, 0x%02X},',
        key(E.ItemId, w.id), w.name, key(E.WeaponClass, w.class), w.price, w.clip_size, w.damage_class, w.power, w.band,
        key(E.SoundId, w.fire_sound), key(E.SoundId, w.silenced_sound), tostring(w.automatic), w.rounds_per_shot,
        w.reload_equip_mode, mask)
end
o[#o + 1] = "}};"
o[#o + 1] = "constexpr const Weapon& weapon(ItemId id) { return kWeapons[v(id) - 1]; }"
o[#o + 1] = "}  // namespace srx"
print(table.concat(o, "\n"))
