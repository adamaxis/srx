-- Semantic item definitions derived from the US ROM item catalog.
-- ROM identity/layout is resolved by the offline importer; gameplay uses named
-- item IDs, prices, armor modifiers, totem bonuses, art availability, and ranges.
-- Generated rows come from tools/gen_records.py items; regenerate, do not hand edit.
local ItemKind = require("enums").ItemKind

---@class ItemDefinition
---@field id string stable gameplay identity
---@field name string
---@field kind integer ItemKind
---@field purchase_base integer? price for purchasable items
---@field target_motion_range table world-distance bounds for target approach
---@field has_detail_art boolean whether a named detail image exists
---@field armor table? combat armor modifiers
---@field totem_spell_modifiers table? per-spell totem bonuses
local Item = {}
Item.__index = Item
setmetatable(Item, { __call = function(_, definition)
    return setmetatable(definition, Item)
end })

local M = { Item = Item, by_id = {
    ["streetline"] = Item{ id = "streetline", name = "Streetline Special", kind = ItemKind.FIREARM, purchase_base = 100, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["model_101t"] = Item{ id = "model_101t", name = "Model 101T Light Pistol", kind = ItemKind.FIREARM, purchase_base = 350, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["american_l36"] = Item{ id = "american_l36", name = "American L36 Light Pistol", kind = ItemKind.FIREARM, purchase_base = 300, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["security_500"] = Item{ id = "security_500", name = "Security 500 Light Pistol", kind = ItemKind.FIREARM, purchase_base = 450, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["warhawk"] = Item{ id = "warhawk", name = "Warhawk Heavy Pistol", kind = ItemKind.FIREARM, purchase_base = 500, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["max_power"] = Item{ id = "max_power", name = "Max-Power Heavy Pistol", kind = ItemKind.FIREARM, purchase_base = 450, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["predator"] = Item{ id = "predator", name = "Predator Heavy Pistol", kind = ItemKind.FIREARM, purchase_base = 450, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["ak_97"] = Item{ id = "ak_97", name = "AK-97 SMG", kind = ItemKind.FIREARM, purchase_base = 1000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["hk227_s"] = Item{ id = "hk227_s", name = "HK227-S SMG", kind = ItemKind.FIREARM, purchase_base = 1500, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["mach_22"] = Item{ id = "mach_22", name = "Mach 22 SMG", kind = ItemKind.FIREARM, purchase_base = 800, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["allegiance"] = Item{ id = "allegiance", name = "Allegiance Shotgun", kind = ItemKind.FIREARM, purchase_base = 1400, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["roomsweeper"] = Item{ id = "roomsweeper", name = "Roomsweeper Shotgun", kind = ItemKind.FIREARM, purchase_base = 1000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["frag"] = Item{ id = "frag", name = "Frag Grenade", kind = ItemKind.GRENADE, purchase_base = 300, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["scatter"] = Item{ id = "scatter", name = "Scatter Grenade", kind = ItemKind.GRENADE, purchase_base = 180, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["concussion"] = Item{ id = "concussion", name = "Concussion Grenade", kind = ItemKind.GRENADE, purchase_base = 180, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["medkit"] = Item{ id = "medkit", name = "Medkit", kind = ItemKind.MEDICAL_KIT, purchase_base = 200, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["stim_patch"] = Item{ id = "stim_patch", name = "Stim Patch", kind = ItemKind.STIM_PATCH, purchase_base = 300, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["trauma_patch"] = Item{ id = "trauma_patch", name = "Trauma Patch", kind = ItemKind.TRAUMA_PATCH, purchase_base = 300, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["fetish"] = Item{ id = "fetish", name = "Fetish", kind = ItemKind.FETISH, purchase_base = 500, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["maglock_passkey"] = Item{ id = "maglock_passkey", name = "Maglock Passkey", kind = ItemKind.PASSKEY, purchase_base = 10000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["electronic_kit"] = Item{ id = "electronic_kit", name = "Electronic Kit", kind = ItemKind.ELECTRONIC_KIT, purchase_base = 1000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["smart_goggles"] = Item{ id = "smart_goggles", name = "Smart Goggles", kind = ItemKind.SMART_GOGGLES, purchase_base = 3000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["enchanted_dagger"] = Item{ id = "enchanted_dagger", name = "Enchanted Dagger", kind = ItemKind.ENCHANTED_DAGGER, purchase_base = 50000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["power_focus"] = Item{ id = "power_focus", name = "Power Focus", kind = ItemKind.POWER_FOCUS, purchase_base = 15000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["flame_dart_focus"] = Item{ id = "flame_dart_focus", name = "Flame Dart Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["flame_bolt_focus"] = Item{ id = "flame_bolt_focus", name = "Flame Bolt Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["hellblast_focus"] = Item{ id = "hellblast_focus", name = "Hellblast Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["invisibility_focus"] = Item{ id = "invisibility_focus", name = "Invisibility Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["sleep_focus"] = Item{ id = "sleep_focus", name = "Sleep Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["stink_focus"] = Item{ id = "stink_focus", name = "Stink Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["supr_barrier_focus"] = Item{ id = "supr_barrier_focus", name = "Supr Barrier Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["mana_zap_focus"] = Item{ id = "mana_zap_focus", name = "Mana Zap Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["mana_blast_focus"] = Item{ id = "mana_blast_focus", name = "Mana Blast Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["mana_storm_focus"] = Item{ id = "mana_storm_focus", name = "Mana Storm Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["rockskin_focus"] = Item{ id = "rockskin_focus", name = "Rockskin Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["heal_wounds_focus"] = Item{ id = "heal_wounds_focus", name = "Heal Wounds Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["confusion_focus"] = Item{ id = "confusion_focus", name = "Confusion Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["barrier_focus"] = Item{ id = "barrier_focus", name = "Barrier Spell Focus", kind = ItemKind.SPELL_FOCUS, purchase_base = 4000, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["protection_talisman"] = Item{ id = "protection_talisman", name = "Protection Talisman", kind = ItemKind.TALISMAN, purchase_base = 17500, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["combat_sense_spell_lock"] = Item{ id = "combat_sense_spell_lock", name = "Combat Sense Spell Lock", kind = ItemKind.SPELL_LOCK, purchase_base = 17500, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["rat_totem"] = Item{ id = "rat_totem", name = "Rat Totem", kind = ItemKind.TOTEM, totem_spell_modifiers = { flame_dart = -1, flame_bolt = -1, hellblast = -1, invisibility = 2, sleep = -1, stink = 2, mana_zap = -1, mana_blast = -1, mana_storm = -1, heal_wounds = 0, confusion = 2 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = false },
    ["gator_totem"] = Item{ id = "gator_totem", name = "Gator Totem", kind = ItemKind.TOTEM, totem_spell_modifiers = { flame_dart = 2, flame_bolt = 2, hellblast = 2, invisibility = -1, sleep = 2, stink = -1, mana_zap = 2, mana_blast = 2, mana_storm = 2, heal_wounds = 0, confusion = -1 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = false },
    ["bear_totem"] = Item{ id = "bear_totem", name = "Bear Totem", kind = ItemKind.TOTEM, totem_spell_modifiers = { flame_dart = 0, flame_bolt = 0, hellblast = 0, invisibility = 0, sleep = 0, stink = 0, mana_zap = 0, mana_blast = 0, mana_storm = 0, heal_wounds = 2, confusion = 0 }, target_motion_range = { minimum_distance = 1600, maximum_distance = 2240 }, has_detail_art = false },
    ["armor_vest"] = Item{ id = "armor_vest", name = "Armor Vest", kind = ItemKind.ARMOR, purchase_base = 200, armor = { primary_modifier = 2, secondary_modifier = 1 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["armor_clothing"] = Item{ id = "armor_clothing", name = "Armor Clothing", kind = ItemKind.ARMOR, purchase_base = 500, armor = { primary_modifier = 1, secondary_modifier = 2 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["armor_jacket"] = Item{ id = "armor_jacket", name = "Armor Jacket", kind = ItemKind.ARMOR, purchase_base = 900, armor = { primary_modifier = 5, secondary_modifier = 3 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["vest_with_plates"] = Item{ id = "vest_with_plates", name = "Vest w/Plates", kind = ItemKind.ARMOR, purchase_base = 600, armor = { primary_modifier = 4, secondary_modifier = 3 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["lined_duster"] = Item{ id = "lined_duster", name = "Lined Duster", kind = ItemKind.ARMOR, purchase_base = 700, armor = { primary_modifier = 3, secondary_modifier = 2 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["light_combat_armor"] = Item{ id = "light_combat_armor", name = "Light Combat Armor", kind = ItemKind.ARMOR, purchase_base = 10000, armor = { primary_modifier = 6, secondary_modifier = 4 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
    ["heavy_combat_armor"] = Item{ id = "heavy_combat_armor", name = "Heavy Combat Armor", kind = ItemKind.ARMOR, purchase_base = 20000, armor = { primary_modifier = 8, secondary_modifier = 6 }, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 }, has_detail_art = true },
} }
M.catalog_order = { "streetline", "model_101t", "american_l36", "security_500", "warhawk", "max_power", "predator", "ak_97", "hk227_s", "mach_22", "allegiance", "roomsweeper", "frag", "scatter", "concussion", "medkit", "stim_patch", "trauma_patch", "fetish", "maglock_passkey", "electronic_kit", "smart_goggles", "enchanted_dagger", "power_focus", "flame_dart_focus", "flame_bolt_focus", "hellblast_focus", "invisibility_focus", "sleep_focus", "stink_focus", "supr_barrier_focus", "mana_zap_focus", "mana_blast_focus", "mana_storm_focus", "rockskin_focus", "heal_wounds_focus", "confusion_focus", "barrier_focus", "protection_talisman", "combat_sense_spell_lock", "rat_totem", "gator_totem", "bear_totem", "armor_vest", "armor_clothing", "armor_jacket", "vest_with_plates", "lined_duster", "light_combat_armor", "heavy_combat_armor" }
return M
