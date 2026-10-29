-- Named campaign progress and world-activity hooks.
-- ROM addresses in comments identify recovered rules; Lua never addresses
-- the source's work-RAM layout.
local E = require("enums")

local StoryState = {}

StoryState.MAX_HEAT_LEVEL = 8
StoryState.MAX_STRIKE_TEAM_STAGE = 8

local function read_level(operation, prefix, maximum)
    local result = game.services.call("story", operation)
    if type(result) ~= "string" then return nil end
    local value = tonumber(result:match("^" .. prefix .. "_(%d+)$"))
    if value == nil or value < 0 or value > maximum then
        return nil
    end
    return value
end

local function write_level(operation, value, maximum)
    if type(value) ~= "number" or value % 1 ~= 0 or
       value < 0 or value > maximum then
        return false
    end
    return game.services.call("story", operation .. "_" .. value) == "applied"
end

function StoryState.heat_level()
    return read_level("get_heat_level", "level", StoryState.MAX_HEAT_LEVEL)
end

function StoryState.set_heat_level(level)
    return write_level("set_heat_level", level, StoryState.MAX_HEAT_LEVEL)
end

function StoryState.strike_team_stage()
    return read_level("get_strike_team_stage", "stage",
                      StoryState.MAX_STRIKE_TEAM_STAGE)
end

function StoryState.set_strike_team_stage(stage)
    return write_level("set_strike_team_stage", stage,
                       StoryState.MAX_STRIKE_TEAM_STAGE)
end

function StoryState.increase_strike_team_stage(amount)
    amount = amount or 1
    if type(amount) ~= "number" or amount % 1 ~= 0 or amount < 0 then
        return false
    end
    local stage = StoryState.strike_team_stage()
    if stage == nil then return false end
    return StoryState.set_strike_team_stage(math.min(
        StoryState.MAX_STRIKE_TEAM_STAGE, stage + amount))
end

-- ROM 0x1F1E0 [C]: after FFFBFB advances in scene 2 it queues actor list
-- 0x1D6FD0 (Boris's two map-placed Strike Team actors).
StoryState.STRIKE_TEAM_FOLLOWUPS = {
    [2] = { sequence_id = "boris_strike_team", actors = 2 },
}

function StoryState.scene()
    local result = game.services.call("field", "scene_reference")
    return type(result) == "string" and
        tonumber(result:match("^scene_(%d+)$")) or nil
end

function StoryState.queue_world_effect(scene, effect)
    return game.services.call("story", string.format(
        "queue_world_effect_%d_%d_%s",
        scene, effect.actors, effect.sequence_id)) == "queued"
end

-- ROM 0x1F1E0: advance Strike Team escalation (maximum stage 8) and queue
-- the scene's follow-up list when this activity is in Scene 2.
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
    if not StoryState.increase_strike_team_stage(1) then
        return nil
    end
    if followup ~= nil and not StoryState.queue_world_effect(scene, followup) then
        return nil
    end
    return "applied"
end

return StoryState
