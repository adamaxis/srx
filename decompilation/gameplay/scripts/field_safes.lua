-- Safe handlers reached from ROM 0x20C90 after a successful 0x16760 terrain
-- mark. The FFE178 branch gates only Renraku's 0x540D6 reward; 0x541B2 has
-- its own FFE187 gate and is checked afterward. [C]
local E = require("enums")
local StoryFlags = require("story_flags")

local Safes = {}

local function set_effect_flag(bit, value)
    game.state["field_mode_effect_flag_bit_" .. bit] = value
end

local function set_group_item(bit, value)
    game.state["group_item_flag_bit_" .. bit] = value
end

local function renraku_map_overlay(context)
    if context.projected_terrain_event_published ~= true or
       context.scene_id ~= E.FieldSafe.RENRAKU_SCENE or
       StoryFlags.is_set(E.StoryFlag.ITO_FIGHT_WON) ~= true or
       StoryFlags.is_set(E.StoryFlag.RENRAKU_MAP_OVERLAY) ~= false then
        return false
    end

    -- ROM 0x540D6: story bit 12, four FFFC34 clue changes, and group
    -- item 8. The typed host applies the FFFBFB/FFE191 writes around the
    -- 0x1E04A alarm service, in source order.
    StoryFlags.set(E.StoryFlag.RENRAKU_MAP_OVERLAY)
    set_effect_flag(9, true)
    set_effect_flag(16, false)
    set_effect_flag(8, false)
    set_effect_flag(17, false)
    set_group_item(E.GroupItemBit.RENRAKU_MAP_OVERLAY, true)
    return true
end

local function fuchi_prototype_cyberheart(context)
    if context.fuchi_safe_context == 0 or
       StoryFlags.is_set(E.StoryFlag.HOSPITAL_VISITED) ~= true or
       StoryFlags.is_set(E.StoryFlag.CYBERHEART_SAFE) ~= false then
        return false
    end

    -- ROM 0x541B2: mark bit 14, update the two FFFC34 flags, and give
    -- FFFC22 bit 0. The source leaves FFE187 armed after this branch.
    StoryFlags.set(E.StoryFlag.CYBERHEART_SAFE)
    set_effect_flag(19, false)
    set_effect_flag(20, true)
    set_group_item(E.GroupItemBit.PROTOTYPE_CYBERHEART, true)
    return true
end

function Safes.resolve(context)
    if type(context) ~= "table" or
       type(context.scene_id) ~= "number" or
       context.scene_id % 1 ~= 0 or context.scene_id < 0 or
       context.scene_id > E.Scene.LAST_ID or
       type(context.fuchi_safe_context) ~= "number" or
       context.fuchi_safe_context % 1 ~= 0 or
       context.fuchi_safe_context < 0 or
       context.fuchi_safe_context > 0xFF or
       type(context.projected_terrain_event_published) ~= "boolean" then
        return nil
    end

    if renraku_map_overlay(context) then
        return {
            action = "renraku_map_overlay",
            raise_alarm = true,
            reset_strike_team_counter = true,
            queue_alarm_transition = true,
            strike_team_counter_value = 0,
            alarm_transition_pending_value =
                E.FieldSafe.ALARM_TRANSITION_PENDING_VALUE,
        }
    end
    if fuchi_prototype_cyberheart(context) then
        return {
            action = "fuchi_prototype_cyberheart",
            raise_alarm = false,
            reset_strike_team_counter = false,
            queue_alarm_transition = false,
            strike_team_counter_value = 0,
            alarm_transition_pending_value = 0,
        }
    end
    return {
        action = "none",
        raise_alarm = false,
        reset_strike_team_counter = false,
        queue_alarm_transition = false,
        strike_team_counter_value = 0,
        alarm_transition_pending_value = 0,
    }
end

return Safes
