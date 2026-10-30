package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local story_fact_ids = {}
for _, fact_id in pairs(E.StoryFact) do story_fact_ids[fact_id] = true end

local host
local function set_bit(word, bit, value)
    if value then return word | (1 << bit) end
    return word & ~(1 << bit)
end

local function reset()
    host = {
        story = {},
        group_items = {},
        field_effects = 0,
        strike_team_stage = 5,
        fuchi_cyberheart_safe_armed = false,
        alarm_transition_scheduled = false,
    }
    game = {
        state = setmetatable({}, {
            __index = function(_, name)
                if story_fact_ids[name] then return host.story[name] end
                local item_id = name:match("^group_item%.([%w_]+)$")
                if item_id then
                    return host.group_items[item_id]
                end
                local bit = name:match("^field_mode_effect_flag_bit_(%d+)$")
                if bit then
                    return (host.field_effects & (1 << tonumber(bit))) ~= 0
                end
                return nil
            end,
            __newindex = function(_, name, value)
                if story_fact_ids[name] then
                    host.story[name] = value
                    return
                end
                local item_id = name:match("^group_item%.([%w_]+)$")
                if item_id then
                    host.group_items[item_id] = value
                    return
                end
                local bit = name:match("^field_mode_effect_flag_bit_(%d+)$")
                if bit then
                    host.field_effects = set_bit(
                        host.field_effects, tonumber(bit), value)
                    return
                end
                error("unexpected state fact " .. name)
            end,
        }),
    }
end

local function set_story_fact(fact_id, value)
    host.story[fact_id] = value
end

local function same_facts(first, second)
    for key, value in pairs(first) do
        if second[key] ~= value then return false end
    end
    for key, value in pairs(second) do
        if first[key] ~= value then return false end
    end
    return true
end

local function copy_facts(source)
    local copy = {}
    for key, value in pairs(source) do copy[key] = value end
    return copy
end

local Safes = require("field_safes")
local checks = 0
local function check(value, label)
    assert(value, label)
    checks = checks + 1
end

local function renraku_prerequisites()
    reset()
    set_story_fact(E.StoryFact.ITO_FIGHT_WON, true)
    set_story_fact(E.StoryFact.RENRAKU_MAP_OVERLAY, false)
    host.field_effects = set_bit(host.field_effects, 16, true)
    host.field_effects = set_bit(host.field_effects, 8, true)
    host.field_effects = set_bit(host.field_effects, 17, true)
end

local function context(scene, projected)
    return {
        scene_id = scene,
        projected_terrain_event_published = projected,
        fuchi_cyberheart_safe_armed =
            host.fuchi_cyberheart_safe_armed,
    }
end

-- Renraku safe: the FFE178 gate, scene 0x28, b16, and clear b12 are
-- independent requirements from ROM 0x540D6.
renraku_prerequisites()
check(Safes.resolve(context(0x28, false)).action == "none",
      "FFE178 clear blocks Renraku reward")
renraku_prerequisites()
check(Safes.resolve(context(0x27, true)).action == "none",
      "Renraku reward is restricted to scene 0x28")
renraku_prerequisites()
set_story_fact(E.StoryFact.ITO_FIGHT_WON, false)
check(Safes.resolve(context(0x28, true)).action == "none",
      "Renraku reward requires story bit 16")
renraku_prerequisites()
set_story_fact(E.StoryFact.RENRAKU_MAP_OVERLAY, true)
check(Safes.resolve(context(0x28, true)).action == "none",
      "Renraku reward is one shot through story bit 12")

renraku_prerequisites()
local renraku = Safes.resolve(context(0x28, true))
check(renraku.action == "renraku_map_overlay" and renraku.raise_alarm,
      "Renraku map overlay reward selected")
check(game.state.renraku_map_overlay and game.state["group_item.map_overlay"] and
      game.state.field_mode_effect_flag_bit_9 and
      not game.state.field_mode_effect_flag_bit_16 and
      not game.state.field_mode_effect_flag_bit_8 and
      not game.state.field_mode_effect_flag_bit_17,
      "Renraku story, item, and field-effect writes match 0x540D6")
check(host.strike_team_stage == 5 and not host.alarm_transition_scheduled and
      renraku.reset_strike_team_stage and renraku.defer_alarm_transition,
      "Renraku result requests escalation reset and deferred alarm")
host.strike_team_stage = 0
host.alarm_transition_scheduled = true
local renraku_story = {}
for key, value in pairs(host.story) do renraku_story[key] = value end
local renraku_items = copy_facts(host.group_items)
local renraku_effects = host.field_effects
local renraku_strike = host.strike_team_stage
local renraku_alarm = host.alarm_transition_scheduled
check(Safes.resolve(context(0x28, true)).action == "none" and
      same_facts(host.story, renraku_story) and
      same_facts(host.group_items, renraku_items) and
      host.field_effects == renraku_effects and
      host.strike_team_stage == renraku_strike and
      host.alarm_transition_scheduled == renraku_alarm,
      "Renraku safe repeat has no further effects")

-- Fuchi safe: 0x541B2 is checked after 0x540D6 and uses FFE187, b25, and
-- clear b14. It does not require the Renraku FFE178 gate.
reset()
set_story_fact(E.StoryFact.HOSPITAL_VISITED, true)
host.fuchi_cyberheart_safe_armed = false
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe requires FFE187")
host.fuchi_cyberheart_safe_armed = true
set_story_fact(E.StoryFact.HOSPITAL_VISITED, false)
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe requires story bit 25")
set_story_fact(E.StoryFact.HOSPITAL_VISITED, true)
set_story_fact(E.StoryFact.CYBERHEART_SAFE, true)
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe is one shot through story bit 14")

reset()
set_story_fact(E.StoryFact.HOSPITAL_VISITED, true)
set_story_fact(E.StoryFact.CYBERHEART_SAFE, false)
host.fuchi_cyberheart_safe_armed = true
host.field_effects = set_bit(host.field_effects, 19, true)
local fuchi = Safes.resolve(context(0x28, false))
check(fuchi.action == "fuchi_prototype_cyberheart" and
      not fuchi.raise_alarm,
      "Fuchi cyber-heart reward is independent of FFE178")
check(game.state.cyberheart_safe and
      game.state["group_item.prototype_cyberheart"] and
      not game.state.field_mode_effect_flag_bit_19 and
      game.state.field_mode_effect_flag_bit_20,
      "Fuchi story, item, and field-effect writes match 0x541B2")
local fuchi_story = {}
for key, value in pairs(host.story) do fuchi_story[key] = value end
local fuchi_items = copy_facts(host.group_items)
local fuchi_effects = host.field_effects
check(Safes.resolve(context(0x28, false)).action == "none" and
      same_facts(host.story, fuchi_story) and
      same_facts(host.group_items, fuchi_items) and
      host.field_effects == fuchi_effects,
      "Fuchi safe repeat has no further effects")

print(("Field safe tests passed: %d prerequisite and one-shot checks"):format(checks))
