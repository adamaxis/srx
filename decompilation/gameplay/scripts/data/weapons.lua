-- data/weapons.lua: ONE record per weapon, merging every ROM table that describes weapons.
-- Sources (all [C]):
--   name, purchase base, magazine capacity, damage effect, and power rating : item record
--   0xD4540 + 0x30*(code-1); +0x25 is resolved through 0x166AD4 offline.
--   fire_sound / silenced_sound : 0x15318 / 0x15336, indexed by code-1 (fire_shot 0x1514E)
--   fire_mode, rounds_per_shot : 0x15154 + 0x38AC (three-round burst for codes 8..10); TN modifier at 0x556C6
--   trigger_behavior, attack_delay_scale : 0xACAE + 0xB39A + 0xABFC; held-A repeats after actor-derived delay
--   reload_delay_scale : 0x14226 helper mode translated to normal/double; 0xACAE later applies the shared delay
--   accepts (accessories)      : shop rules 0x5709E-0x57108 (pistols < 8: Smartgun/Silencer/Laser; others: no Silencer)
-- Source data is kept on each weapon record; gameplay/items.lua projects it into the live item API.
local E = require("enums")
local WeaponClass, Accessory, SoundId = E.WeaponClass, E.Accessory, E.SoundId

---@class Weapon
---@field id string                 Stable gameplay item identity
---@field name string
---@field class integer            WeaponClass
---@field purchase_base integer   Buy price before shop modifiers
---@field magazine_capacity integer
---@field damage_effect integer resolved base hit effect (1, 3, or 6)
---@field power_rating integer
---@field fire_sound integer       SoundId
---@field silenced_sound integer   SoundId
---@field automatic boolean         0x556C6 three-round-burst TN branch (codes 8..10)
---@field rounds_per_shot integer  party members only (0x38AC)
---@field accepts table<integer, boolean>  Accessory -> allowed
---@field fire_mode string?        single_round or three_round_burst
---@field trigger_behavior string? repeat_while_held for firearm codes 1..12
---@field attack_cadence string?   runner_attribute for firearm codes 1..12
---@field attack_delay_scale string? half for codes 1..10, normal for 11..12
---@field reload_delay_scale string? 0x14226 mode: double for codes 5, 11, 12
---@field source_action table?     item-specific hit values consumed by combat.lua
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

local Records = {
    streetline = Weapon{
        id = "streetline", name = "Streetline Special", class = WeaponClass.LIGHT_PISTOL,
        purchase_base = 100, magazine_capacity = 6, damage_effect = 1, power_rating = 4,
        fire_sound = SoundId.SHOT_LIGHT_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 1, difficulty = 4 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    model_101t = Weapon{
        id = "model_101t", name = "Model 101T Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        purchase_base = 350, magazine_capacity = 12, damage_effect = 1, power_rating = 6,
        fire_sound = SoundId.SHOT_LIGHT_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 1, difficulty = 6 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    american_l36 = Weapon{
        id = "american_l36", name = "American L36 Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        purchase_base = 300, magazine_capacity = 11, damage_effect = 1, power_rating = 6,
        fire_sound = SoundId.SHOT_LIGHT_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 1, difficulty = 6 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    security_500 = Weapon{
        id = "security_500", name = "Security 500 Light Pistol", class = WeaponClass.LIGHT_PISTOL,
        purchase_base = 450, magazine_capacity = 25, damage_effect = 1, power_rating = 6,
        fire_sound = SoundId.SHOT_LIGHT_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 1, difficulty = 6 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    warhawk = Weapon{
        id = "warhawk", name = "Warhawk Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        purchase_base = 500, magazine_capacity = 6, damage_effect = 3, power_rating = 10,
        fire_sound = SoundId.SHOT_HEAVY_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "double",
        source_action = { effect_value = 3, difficulty = 10 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    max_power = Weapon{
        id = "max_power", name = "Max-Power Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        purchase_base = 450, magazine_capacity = 10, damage_effect = 3, power_rating = 9,
        fire_sound = SoundId.SHOT_HEAVY_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 3, difficulty = 9 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    predator = Weapon{
        id = "predator", name = "Predator Heavy Pistol", class = WeaponClass.HEAVY_PISTOL,
        purchase_base = 450, magazine_capacity = 15, damage_effect = 3, power_rating = 9,
        fire_sound = SoundId.SHOT_HEAVY_PISTOL, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 3, difficulty = 9 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SILENCER] = true, [Accessory.LASER_SIGHT] = true },
    },
    ak_97 = Weapon{
        id = "ak_97", name = "AK-97 SMG", class = WeaponClass.SMG,
        purchase_base = 1000, magazine_capacity = 30, damage_effect = 3, power_rating = 6,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = true, fire_mode = "three_round_burst", rounds_per_shot = 3,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 3, difficulty = 6 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    hk227_s = Weapon{
        id = "hk227_s", name = "HK227-S SMG", class = WeaponClass.SMG,
        purchase_base = 1500, magazine_capacity = 28, damage_effect = 3, power_rating = 7,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = true, fire_mode = "three_round_burst", rounds_per_shot = 3,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 3, difficulty = 7 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    mach_22 = Weapon{
        id = "mach_22", name = "Mach 22 SMG", class = WeaponClass.SMG,
        purchase_base = 800, magazine_capacity = 24, damage_effect = 3, power_rating = 6,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = true, fire_mode = "three_round_burst", rounds_per_shot = 3,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "half", reload_delay_scale = "normal",
        source_action = { effect_value = 3, difficulty = 6 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    allegiance = Weapon{
        id = "allegiance", name = "Allegiance Shotgun", class = WeaponClass.SHOTGUN,
        purchase_base = 1400, magazine_capacity = 5, damage_effect = 6, power_rating = 10,
        fire_sound = SoundId.SHOT_SHOTGUN, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "normal", reload_delay_scale = "double",
        source_action = { effect_value = 6, difficulty = 10 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    roomsweeper = Weapon{
        id = "roomsweeper", name = "Roomsweeper Shotgun", class = WeaponClass.SHOTGUN,
        purchase_base = 1000, magazine_capacity = 8, damage_effect = 6, power_rating = 9,
        fire_sound = SoundId.SHOT_SHOTGUN, silenced_sound = SoundId.SHOT_SILENCED,
        automatic = false, fire_mode = "single_round", rounds_per_shot = 1,
        trigger_behavior = "repeat_while_held", attack_cadence = "runner_attribute",
        attack_delay_scale = "normal", reload_delay_scale = "double",
        source_action = { effect_value = 6, difficulty = 9 },
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    frag = Weapon{
        id = "frag", name = "Frag Grenade", class = WeaponClass.THROWN,
        purchase_base = 300, magazine_capacity = 6, damage_effect = 6, power_rating = 8,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = false, rounds_per_shot = 1,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    scatter = Weapon{
        id = "scatter", name = "Scatter Grenade", class = WeaponClass.THROWN,
        purchase_base = 180, magazine_capacity = 6, damage_effect = 3, power_rating = 6,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = false, rounds_per_shot = 1,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
    concussion = Weapon{
        id = "concussion", name = "Concussion Grenade", class = WeaponClass.THROWN,
        purchase_base = 180, magazine_capacity = 6, damage_effect = 3, power_rating = 10,
        fire_sound = SoundId.SHOT_SMG_OR_THROWN, silenced_sound = SoundId.SHOT_SILENCED_SMG_OR_THROWN,
        automatic = false, rounds_per_shot = 1,
        accepts = { [Accessory.SMARTGUN] = true, [Accessory.SOUND_SUPPRESSOR] = true, [Accessory.LASER_SIGHT] = true, [Accessory.GAS_VENT_2] = true, [Accessory.GAS_VENT_3] = true },
    },
}
local Weapons = {
    Weapon = Weapon,
    by_id = Records,
    catalog_order = {
        "streetline", "model_101t", "american_l36", "security_500",
        "warhawk", "max_power", "predator", "ak_97", "hk227_s",
        "mach_22", "allegiance", "roomsweeper", "frag", "scatter",
        "concussion",
    },
}
function Weapons.get(id) return Records[id] end
return Weapons
