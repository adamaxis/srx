-- data/weapons.lua: ONE record per weapon, merging every ROM table that describes weapons.
-- Sources (all [C]):
--   name, price, clip_size, damage_class, power, band : item record 0xD4540 + 0x30*(code-1), fields +0x00/+0x22/+0x24/+0x25/+0x26/+0x27 (doc 08)
--   fire_sound / silenced_sound : 0x15318 / 0x15336, indexed by code-1 (fire_shot 0x1514E)
--   automatic, rounds_per_shot : 0x38AC (3 rounds for codes 8..10), FFDD3A burst (0x15154), TN +3 at 0x556C6
--   reload_equip_mode          : 0x14226 (mode 2 for codes 5, 11, 12)
--   accepts (accessories)      : shop rules 0x5709E-0x57108 (pistols < 8: Smartgun/Silencer/Laser; others: no Silencer)
-- Generated from rom.bin by tools/gen_weapons.py; regenerate rather than hand-edit.
local E = require("enums")
local ItemId, WeaponClass, Accessory = E.ItemId, E.WeaponClass, E.Accessory

---@class Weapon
---@field id integer               ItemId
---@field name string
---@field class integer            WeaponClass
---@field price integer            +0x22 buy price base
---@field clip_size integer        +0x24 (grenades: quantity class)
---@field damage_class integer     +0x25 -> TN table 0x166AD4
---@field power integer            +0x26
---@field band integer             +0x27 -> AI distance band 0x13E7C
---@field fire_sound integer       SoundId
---@field silenced_sound integer   SoundId
---@field automatic boolean
---@field rounds_per_shot integer  party members only (0x38AC)
---@field reload_equip_mode integer
---@field accepts table<integer, boolean>  Accessory -> allowed
local Weapon = {}
Weapon.__index = Weapon
setmetatable(Weapon, { __call = function(_, t) return setmetatable(t, Weapon) end })

function Weapon:is_thrown() return self.class == WeaponClass.THROWN end
function Weapon:sound(silenced) return silenced and self.silenced_sound or self.fire_sound end
-- Target-number modifier from accessories (0x556C6) [C]
function Weapon:tn_modifier(accessory_bits, has_smartlink, has_goggles)
    local m = self.automatic and 3 or 0
    local function has(a) return (accessory_bits & (1 << a)) ~= 0 end
    if self.automatic then
        if has(Accessory.GAS_VENT_2) then m = m - 2 elseif has(Accessory.GAS_VENT_3) then m = m - 3 end
    end
    if has(Accessory.LASER_SIGHT) then m = m - 1 end
    if has(Accessory.SMARTGUN) then
        if has_smartlink then m = m - 2 elseif has_goggles then m = m - 1 end
    end
    return m
end
-- Accessory purchase check (0x57070-0x57108); returns ok, refusal text address [C]
function Weapon:can_fit(accessory, accessory_bits)
    if (accessory_bits & (1 << accessory)) ~= 0 then return false, 0x1AA6E4 end      -- already has it
    if not self.accepts[accessory] then
        return false, (accessory == Accessory.SILENCER) and 0x1AA683 or 0x1AA65C
    end
    if accessory == Accessory.LASER_SIGHT and (accessory_bits & (1 << Accessory.SMARTGUN)) ~= 0 then
        return false, 0x1AA6B2                                                        -- not needed on a smartgun
    end
    return true
end

local Weapons = {
    [ItemId.STREETLINE_SPECIAL] = Weapon{
        id = ItemId.STREETLINE_SPECIAL, name = "Streetline Special", class = WeaponClass.LIGHT_PISTOL,
        price = 100, clip_size = 6, damage_class = 0, power = 4, band = 0,
        fire_sound = 0x1C, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.MODEL_101T] = Weapon{
        id = ItemId.MODEL_101T, name = "Model 101T Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        price = 350, clip_size = 12, damage_class = 0, power = 6, band = 0,
        fire_sound = 0x1C, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.AMERICAN_L36] = Weapon{
        id = ItemId.AMERICAN_L36, name = "American L36 Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        price = 300, clip_size = 11, damage_class = 0, power = 6, band = 0,
        fire_sound = 0x1C, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.SECURITY_500] = Weapon{
        id = ItemId.SECURITY_500, name = "Security 500 Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        price = 450, clip_size = 25, damage_class = 0, power = 6, band = 0,
        fire_sound = 0x1C, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.WARHAWK] = Weapon{
        id = ItemId.WARHAWK, name = "Warhawk Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        price = 500, clip_size = 6, damage_class = 1, power = 10, band = 0,
        fire_sound = 0x51, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 2,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.MAX_POWER] = Weapon{
        id = ItemId.MAX_POWER, name = "Max-Power Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        price = 450, clip_size = 10, damage_class = 1, power = 9, band = 0,
        fire_sound = 0x51, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.PREDATOR] = Weapon{
        id = ItemId.PREDATOR, name = "Predator Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        price = 450, clip_size = 15, damage_class = 1, power = 9, band = 0,
        fire_sound = 0x51, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    [ItemId.AK97] = Weapon{
        id = ItemId.AK97, name = "AK-97 SMG", class = WeaponClass.SMG,
        price = 1000, clip_size = 30, damage_class = 1, power = 6, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = true, rounds_per_shot = 3,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.HK227S] = Weapon{
        id = ItemId.HK227S, name = "HK227-S SMG", class = WeaponClass.SMG,
        price = 1500, clip_size = 28, damage_class = 1, power = 7, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = true, rounds_per_shot = 3,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.MACH22] = Weapon{
        id = ItemId.MACH22, name = "Mach 22 SMG", class = WeaponClass.SMG,
        price = 800, clip_size = 24, damage_class = 1, power = 6, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = true, rounds_per_shot = 3,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.ALLEGIANCE] = Weapon{
        id = ItemId.ALLEGIANCE, name = "Allegiance Shotgun", class = WeaponClass.SHOTGUN,
        price = 1400, clip_size = 5, damage_class = 2, power = 10, band = 0,
        fire_sound = 0x1D, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 2,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.ROOMSWEEPER] = Weapon{
        id = ItemId.ROOMSWEEPER, name = "Roomsweeper Shotgun", class = WeaponClass.SHOTGUN,
        price = 1000, clip_size = 8, damage_class = 2, power = 9, band = 0,
        fire_sound = 0x1D, silenced_sound = 0x3F,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 2,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.FRAG_GRENADE] = Weapon{
        id = ItemId.FRAG_GRENADE, name = "Frag Grenade", class = WeaponClass.THROWN,
        price = 300, clip_size = 6, damage_class = 2, power = 8, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.SCATTER_GRENADE] = Weapon{
        id = ItemId.SCATTER_GRENADE, name = "Scatter Grenade", class = WeaponClass.THROWN,
        price = 180, clip_size = 6, damage_class = 1, power = 6, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    [ItemId.CONCUSSION_GRENADE] = Weapon{
        id = ItemId.CONCUSSION_GRENADE, name = "Concussion Grenade", class = WeaponClass.THROWN,
        price = 180, clip_size = 6, damage_class = 1, power = 10, band = 0,
        fire_sound = 0x1E, silenced_sound = 0x4A,
        automatic = false, rounds_per_shot = 1,
        reload_equip_mode = 0,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
}
Weapons.Weapon = Weapon
function Weapons.get(code) return Weapons[code] end
return Weapons
