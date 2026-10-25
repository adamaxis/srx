package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")

local host
local function set_bit(word, bit, value)
    if value then return word | (1 << bit) end
    return word & ~(1 << bit)
end

local function reset()
    host = {
        story = 0,
        group_items = 0,
        field_effects = 0,
        bytes = {
            [E.StoryByte.STRIKE_TEAM] = 5,
            [E.StoryByte.FUCHI_SAFE_CONTEXT] = 0,
            [E.StoryByte.ALARM_TRANSITION_PENDING] = 0,
        },
    }
    game = {
        state = setmetatable({}, {
            __index = function(_, name)
                local bit = name:match("^story_flag_bit_(%d+)$")
                if bit then return (host.story & (1 << tonumber(bit))) ~= 0 end
                bit = name:match("^group_item_flag_bit_(%d+)$")
                if bit then
                    return (host.group_items & (1 << tonumber(bit))) ~= 0
                end
                bit = name:match("^field_mode_effect_flag_bit_(%d+)$")
                if bit then
                    return (host.field_effects & (1 << tonumber(bit))) ~= 0
                end
                return nil
            end,
            __newindex = function(_, name, value)
                local bit = name:match("^story_flag_bit_(%d+)$")
                if bit then
                    host.story = set_bit(host.story, tonumber(bit), value)
                    return
                end
                bit = name:match("^group_item_flag_bit_(%d+)$")
                if bit then
                    host.group_items = set_bit(
                        host.group_items, tonumber(bit), value)
                    return
                end
                bit = name:match("^field_mode_effect_flag_bit_(%d+)$")
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

local Safes = require("field_safes")
local checks = 0
local function check(value, label)
    assert(value, label)
    checks = checks + 1
end

local function renraku_prerequisites()
    reset()
    host.story = set_bit(host.story, E.StoryFlag.ITO_FIGHT_WON, true)
    host.field_effects = set_bit(host.field_effects, 16, true)
    host.field_effects = set_bit(host.field_effects, 8, true)
    host.field_effects = set_bit(host.field_effects, 17, true)
end

local function context(scene, projected)
    return {
        scene_id = scene,
        projected_terrain_event_published = projected,
        fuchi_safe_context =
            host.bytes[E.StoryByte.FUCHI_SAFE_CONTEXT] or 0,
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
host.story = set_bit(host.story, E.StoryFlag.ITO_FIGHT_WON, false)
check(Safes.resolve(context(0x28, true)).action == "none",
      "Renraku reward requires story bit 16")
renraku_prerequisites()
host.story = set_bit(
    host.story, E.StoryFlag.RENRAKU_MAP_OVERLAY, true)
check(Safes.resolve(context(0x28, true)).action == "none",
      "Renraku reward is one shot through story bit 12")

renraku_prerequisites()
local renraku = Safes.resolve(context(0x28, true))
check(renraku.action == "renraku_map_overlay" and renraku.raise_alarm,
      "Renraku map overlay reward selected")
check(game.state.story_flag_bit_12 and game.state.group_item_flag_bit_8 and
      game.state.field_mode_effect_flag_bit_9 and
      not game.state.field_mode_effect_flag_bit_16 and
      not game.state.field_mode_effect_flag_bit_8 and
      not game.state.field_mode_effect_flag_bit_17,
      "Renraku story, item, and field-effect writes match 0x540D6")
check(host.bytes[E.StoryByte.STRIKE_TEAM] == 5 and
      host.bytes[E.StoryByte.ALARM_TRANSITION_PENDING] == 0 and
      renraku.reset_strike_team_counter and
      renraku.queue_alarm_transition and
      renraku.strike_team_counter_value == 0 and
      renraku.alarm_transition_pending_value ==
          E.FieldSafe.ALARM_TRANSITION_PENDING_VALUE,
      "Renraku result requests the 0x540D6 strike reset and deferred alarm")
host.bytes[E.StoryByte.STRIKE_TEAM] = 0
host.bytes[E.StoryByte.ALARM_TRANSITION_PENDING] = 1
local renraku_story = host.story
local renraku_items = host.group_items
local renraku_effects = host.field_effects
local renraku_strike = host.bytes[E.StoryByte.STRIKE_TEAM]
local renraku_alarm = host.bytes[E.StoryByte.ALARM_TRANSITION_PENDING]
check(Safes.resolve(context(0x28, true)).action == "none" and
      host.story == renraku_story and host.group_items == renraku_items and
      host.field_effects == renraku_effects and
      host.bytes[E.StoryByte.STRIKE_TEAM] == renraku_strike and
      host.bytes[E.StoryByte.ALARM_TRANSITION_PENDING] == renraku_alarm,
      "Renraku safe repeat has no further effects")

-- Fuchi safe: 0x541B2 is checked after 0x540D6 and uses FFE187, b25, and
-- clear b14. It does not require the Renraku FFE178 gate.
reset()
host.story = set_bit(host.story, E.StoryFlag.HOSPITAL_VISITED, true)
host.bytes[E.StoryByte.FUCHI_SAFE_CONTEXT] = 0
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe requires FFE187")
host.bytes[E.StoryByte.FUCHI_SAFE_CONTEXT] = E.FieldSafe.FUCHI_SAFE_CONTEXT_VALUE
host.story = set_bit(host.story, E.StoryFlag.HOSPITAL_VISITED, false)
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe requires story bit 25")
host.story = set_bit(host.story, E.StoryFlag.HOSPITAL_VISITED, true)
host.story = set_bit(host.story, E.StoryFlag.CYBERHEART_SAFE, true)
check(Safes.resolve(context(0x28, false)).action == "none",
      "Fuchi safe is one shot through story bit 14")

reset()
host.story = set_bit(host.story, E.StoryFlag.HOSPITAL_VISITED, true)
host.bytes[E.StoryByte.FUCHI_SAFE_CONTEXT] = E.FieldSafe.FUCHI_SAFE_CONTEXT_VALUE
host.field_effects = set_bit(host.field_effects, 19, true)
local fuchi = Safes.resolve(context(0x28, false))
check(fuchi.action == "fuchi_prototype_cyberheart" and
      not fuchi.raise_alarm,
      "Fuchi cyber-heart reward is independent of FFE178")
check(game.state.story_flag_bit_14 and
      game.state.group_item_flag_bit_0 and
      not game.state.field_mode_effect_flag_bit_19 and
      game.state.field_mode_effect_flag_bit_20,
      "Fuchi story, item, and field-effect writes match 0x541B2")
local fuchi_story = host.story
local fuchi_items = host.group_items
local fuchi_effects = host.field_effects
check(Safes.resolve(context(0x28, false)).action == "none" and
      host.story == fuchi_story and host.group_items == fuchi_items and
      host.field_effects == fuchi_effects,
      "Fuchi safe repeat has no further effects")

print(("Field safe tests passed: %d prerequisite and one-shot checks"):format(checks))
