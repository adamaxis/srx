-- Semantic spell definitions derived from the US ROM spell tables.
-- Runtime rules consume named spells and resolved values, not ROM row fields.
-- Generated rows come from tools/gen_records.py spells; regenerate, do not hand edit.
local Spell = {}
Spell.__index = Spell
setmetatable(Spell, { __call = function(_, definition)
    return setmetatable(definition, Spell)
end })

function Spell:drain(force, stat_reduction)
    local target = ((force >> 1) + (force & 1)) + self.force_drain_modifier
    if target < 2 then target = 2 end
    return self.base_drain - (stat_reduction or 0), target
end

local M = { Spell = Spell, by_id = {
    ["flame_dart"] = Spell{ id = "flame_dart", name = "Flame Dart", base_drain = 1, force_drain_modifier = 1, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["flame_bolt"] = Spell{ id = "flame_bolt", name = "Flame Bolt", base_drain = 3, force_drain_modifier = 1, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["hellblast"] = Spell{ id = "hellblast", name = "Hellblast", base_drain = 10, force_drain_modifier = 3, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["invisibility"] = Spell{ id = "invisibility", name = "Invisibility", base_drain = 3, force_drain_modifier = 0, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["sleep"] = Spell{ id = "sleep", name = "Sleep", base_drain = 6, force_drain_modifier = 0, target_motion_range = nil },
    ["stink"] = Spell{ id = "stink", name = "Stink", base_drain = 6, force_drain_modifier = 0, target_motion_range = nil },
    ["super_barrier"] = Spell{ id = "super_barrier", name = "Super Barrier", base_drain = 6, force_drain_modifier = 2, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["mana_zap"] = Spell{ id = "mana_zap", name = "Mana Zap", base_drain = 1, force_drain_modifier = 0, target_motion_range = nil },
    ["mana_blast"] = Spell{ id = "mana_blast", name = "Mana Blast", base_drain = 3, force_drain_modifier = 0, target_motion_range = nil },
    ["mana_storm"] = Spell{ id = "mana_storm", name = "Mana Storm", base_drain = 6, force_drain_modifier = 1, target_motion_range = nil },
    ["rockskin"] = Spell{ id = "rockskin", name = "Rockskin", base_drain = 3, force_drain_modifier = 2, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["heal_wounds"] = Spell{ id = "heal_wounds", name = "Heal Wounds", base_drain = 3, force_drain_modifier = 0, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
    ["confusion"] = Spell{ id = "confusion", name = "Confusion", base_drain = 6, force_drain_modifier = 0, target_motion_range = nil },
    ["barrier"] = Spell{ id = "barrier", name = "Barrier", base_drain = 3, force_drain_modifier = 2, target_motion_range = { minimum_distance = 1280, maximum_distance = 1600 } },
} }
M.catalog_order = { "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep", "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm", "rockskin", "heal_wounds", "confusion", "barrier" }
return M
