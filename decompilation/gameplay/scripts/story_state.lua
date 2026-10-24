-- Numeric story bytes and the story world-effect hooks built on them.
--
-- The host only exposes primitives:
--   story.byte_get_<hex>            -> "value_<n>" or nil (unknown/unrestored)
--   story.byte_set_<hex>_<n>        -> "applied" or nil
--   story.world_effect_idle         -> "idle" or "busy"
--   story.queue_world_effect_<scene>_<list hex>_<actors>_<name> -> "queued"
--   field.scene_reference           -> "scene_<n>"
-- Story rules (caps, follow-up lists, which scene queues what) live here.
local E = require("enums")

local StoryState = {}

StoryState.BYTE_CAP = 8

-- ROM 0x1F1E0 [C]: after FFFBFB advances in scene 2 it queues actor list
-- 0x1D6FD0 (Boris's two map-placed Strike Team actors).
StoryState.STRIKE_TEAM_FOLLOWUPS = {
    [2] = { name = "boris_strike_team", list = 0x1D6FD0, actors = 2 },
}

function StoryState.byte(address)
    local result = game.services.call(
        "story", string.format("byte_get_%x", address))
    if type(result) ~= "string" then
        return nil
    end
    return tonumber(result:match("^value_(%d+)$"))
end

function StoryState.set_byte(address, value)
    return game.services.call(
        "story", string.format("byte_set_%x_%d", address, value)) == "applied"
end

-- Adds delta to a story byte, capped. False when the byte is unknown.
function StoryState.add_capped(address, delta, cap)
    local value = StoryState.byte(address)
    if value == nil then
        return false
    end
    return StoryState.set_byte(address, math.min(cap or StoryState.BYTE_CAP,
                                                 value + delta))
end

function StoryState.scene()
    local result = game.services.call("field", "scene_reference")
    return type(result) == "string" and
        tonumber(result:match("^scene_(%d+)$")) or nil
end

function StoryState.queue_world_effect(scene, effect)
    return game.services.call("story", string.format(
        "queue_world_effect_%d_%x_%d_%s",
        scene, effect.list, effect.actors, effect.name)) == "queued"
end

-- ROM 0x1F1E0: advance FFFBFB (cap 8) and queue the scene's follow-up list.
-- Returns "applied" or nil when the host state cannot take the step.
function StoryState.advance_strike_team()
    local scene = StoryState.scene()
    if scene == nil then
        return nil
    end
    local followup = StoryState.STRIKE_TEAM_FOLLOWUPS[scene]
    if followup ~= nil and
       game.services.call("story", "world_effect_idle") ~= "idle" then
        return nil
    end
    if not StoryState.add_capped(E.StoryByte.STRIKE_TEAM, 1) then
        return nil
    end
    if followup ~= nil and not StoryState.queue_world_effect(scene, followup) then
        return nil
    end
    return "applied"
end

return StoryState
