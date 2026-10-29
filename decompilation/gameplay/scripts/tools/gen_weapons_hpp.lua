-- Generate a typed C++ view of semantic weapon data:
-- lua5.4 tools/gen_weapons_hpp.lua > weapons.hpp
package.path = "./?.lua;" .. package.path
local Weapons, E = require("data.weapons"), require("enums")
local function key(tbl, val)
    for k, v in pairs(tbl) do if v == val then return k end end
end
local output = {
    "// weapons.hpp: GENERATED from semantic data/weapons.lua; do not edit.",
    "#pragma once", "#include <array>", "#include <cstdint>",
    "#include <string_view>", '#include "enums.hpp"', "namespace srx {",
    "struct Weapon {",
    "    std::string_view id, name; WeaponClass weapon_class;",
    "    std::uint16_t purchase_base;",
    "    std::uint8_t magazine_capacity, damage_effect, power_rating;",
    "    SoundId fire_sound, silenced_sound; bool automatic;",
    "    std::uint8_t rounds_per_shot; std::uint8_t accepts_mask;",
    "    constexpr bool accepts(Accessory a) const { return (accepts_mask >> v(a)) & 1; }",
    "    constexpr SoundId sound(bool silenced) const { return silenced ? silenced_sound : fire_sound; }",
    "};",
    "inline constexpr std::array<Weapon, 15> kWeapons = {{",
}
for _, id in ipairs(Weapons.catalog_order) do
    local weapon = Weapons.get(id)
    local mask = 0
    for accessory, accepted in pairs(weapon.accepts) do
        if accepted then mask = mask | (1 << accessory) end
    end
    output[#output + 1] = string.format(
        '    {"%s", "%s", WeaponClass::%s, %d, %d, %d, %d, SoundId::%s, SoundId::%s, %s, %d, 0x%02X},',
        weapon.id, weapon.name, key(E.WeaponClass, weapon.class),
        weapon.purchase_base, weapon.magazine_capacity, weapon.damage_effect,
        weapon.power_rating, key(E.SoundId, weapon.fire_sound),
        key(E.SoundId, weapon.silenced_sound), tostring(weapon.automatic),
        weapon.rounds_per_shot, mask)
end
output[#output + 1] = "}};"
output[#output + 1] = "constexpr const Weapon* weapon(std::string_view id) {"
output[#output + 1] = "    for (const auto& record : kWeapons) if (record.id == id) return &record;"
output[#output + 1] = "    return nullptr;"
output[#output + 1] = "}"
output[#output + 1] = "}  // namespace srx"
print(table.concat(output, "\n"))
