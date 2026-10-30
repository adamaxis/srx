-- Field alarm policy for ROM 0x1DFC4..0x1E0DC. The native adapter supplies
-- named scene facts, tile-space regions, and a lazy normalized chance sample;
-- Lua selects the state transition and typed side effects. [C]
local E = require("enums")
local M = {}

local function is_integer(value)
    return type(value) == "number" and value % 1 == 0
end

local function is_non_negative_integer(value)
    return is_integer(value) and value >= 0
end

local function valid_probability_sample(value)
    return type(value) == "number" and value == value and
           value >= 0 and value < 1
end

local function valid_activity_id(value)
    return type(value) == "string" and #value > 0 and #value <= 128 and
           value:match("^[%w_.%-]+$") ~= nil
end

local function valid_rectangles(rectangles)
    if type(rectangles) ~= "table" then return false end
    for _, rect in ipairs(rectangles) do
        if type(rect) ~= "table" or
           not is_non_negative_integer(rect.min_tile_x) or
           not is_non_negative_integer(rect.max_tile_x) or
           not is_non_negative_integer(rect.min_tile_y) or
           not is_non_negative_integer(rect.max_tile_y) then
            return false
        end
    end
    return true
end

local function valid_facts(facts)
    return type(facts) == "table" and
        type(facts.alarm_check_due) == "boolean" and
        type(facts.building_scene) == "boolean" and
        type(facts.alarm_timeout_scene) == "boolean" and
        is_non_negative_integer(facts.player_tile_x) and
        is_non_negative_integer(facts.player_tile_y) and
        type(facts.security_disabled) == "boolean" and
        type(facts.alarm_active) == "boolean" and
        is_integer(facts.alarm_time_remaining_frames) and
        facts.alarm_time_remaining_frames >= -1 and
        facts.alarm_time_remaining_frames <= E.AiTuning.ALARM_FRAMES and
        (facts.probability_sample == nil or
         valid_probability_sample(facts.probability_sample)) and
        (facts.direct_raise == nil or type(facts.direct_raise) == "boolean") and
        (facts.clear_alarm == nil or type(facts.clear_alarm) == "boolean") and
        (facts.disable_alarm == nil or type(facts.disable_alarm) == "boolean") and
        (facts.reset == nil or type(facts.reset) == "boolean") and
        (facts.scene_exit == nil or type(facts.scene_exit) == "boolean") and
        (facts.reinforcement_activity_id == nil or
         valid_activity_id(facts.reinforcement_activity_id)) and
        valid_rectangles(facts.alarm_regions) and
        (facts.actors == nil or type(facts.actors) == "table")
end

local function state_result(facts)
    return {
        alarm_active = facts.alarm_active,
        alarm_time_remaining_frames = facts.alarm_time_remaining_frames,
        security_disabled = facts.security_disabled,
        actors = facts.actors or {},
    }
end

local function actor_flag_action(actors, action)
    for _, actor in ipairs(actors) do
        local roles = type(actor) == "table" and
            type(actor.actor_facts) == "table" and
            actor.actor_facts.roles or nil
        if type(roles) == "table" then
            if action == "clear_spotted" then
                roles.spotted = false
            elseif action == "promote_reinforcement" and
                   roles.reinforcement == true then
                roles.despawn_offscreen = true
            end
        end
    end
end

-- ROM 0x1E09A clears FFE16F only when active, promotes +0x9E bit 1 to bit 2
-- on the seven actor records, then always stops sound 0x30. [C]
function M.clear(state, actors)
    if type(state) ~= "table" or
       type(state.alarm_active) ~= "boolean" then
        return nil
    end
    local was_active = state.alarm_active
    state.alarm_active = false
    if was_active then
        actor_flag_action(actors or {}, "promote_reinforcement")
    end
    return {
        was_active = was_active,
        sound_stop = E.SoundId.ALARM,
    }
end

-- ROM 0x167AE..0x167D8 clears the alarm and security-disable states after
-- marking a projected terrain cell. It does not run the shared clear helper's
-- reinforcement promotion or sound-stop path. [C]
function M.reset(state)
    if type(state) ~= "table" then return nil end
    state.alarm_active = false
    state.security_disabled = false
    return state
end

local function raised_result(facts, result)
    -- ROM 0x1E04A refreshes the alarm duration before testing the active latch.
    -- An active alarm receives a refresh without replaying sound or spawning.
    result.alarm_time_remaining_frames = E.AiTuning.ALARM_FRAMES
    if result.alarm_active then return result end

    result.alarm_active = true
    result.sound_start = E.SoundId.ALARM
    result.clear_flags = "clear_spotted"
    actor_flag_action(result.actors, result.clear_flags)
    result.spawn_reinforcements = true
    result.reinforcement_activity_id = facts.reinforcement_activity_id
    return result
end

local function raise(facts, result)
    -- The alarm-combat scene takes 0x1DBAA's timeout path and does not set the
    -- alarm latch. Other scenes must be in a building. [C]
    if facts.alarm_timeout_scene then
        result.schedule_timeout = true
        return result
    end
    if not facts.building_scene then return result end
    return raised_result(facts, result)
end

local function contains_player(regions, x, y)
    for _, region in ipairs(regions) do
        -- ROM 0x1E030..0x1E044 uses exclusive lower and inclusive upper bounds.
        if region.min_tile_x < x and x <= region.max_tile_x and
           region.min_tile_y < y and y <= region.max_tile_y then
            return true
        end
    end
    return false
end

function M.tick(facts)
    if not valid_facts(facts) then return nil end
    local result = state_result(facts)

    if facts.scene_exit then
        -- Scene entry clears the alarm and security state. Reuse shared alarm
        -- cleanup so active reinforcement records are retired consistently.
        result.security_disabled = false
        local clear = M.clear(result, result.actors)
        result.sound_stop = clear.was_active and clear.sound_stop or nil
        result.clear_flags = clear.was_active and
            "promote_reinforcement" or nil
        return result
    end
    if facts.reset then
        return M.reset(result)
    end
    if facts.disable_alarm or facts.clear_alarm then
        if facts.disable_alarm then
            result.security_disabled = true
        end
        local clear = M.clear(result, result.actors)
        result.sound_stop = clear.sound_stop
        result.clear_flags = clear.was_active and
            "promote_reinforcement" or nil
        return result
    end
    if facts.direct_raise then
        return raise(facts, result)
    end

    -- The native field clock marks the source's 13-in-64-frame opportunity.
    if result.alarm_active then
        result.alarm_time_remaining_frames =
            result.alarm_time_remaining_frames - 1
        if result.alarm_time_remaining_frames < 0 then
            local clear = M.clear(result, result.actors)
            result.sound_stop = clear.sound_stop
            result.clear_flags = clear.was_active and
                "promote_reinforcement" or nil
            return result
        end
    end
    if result.security_disabled then return result end
    if not facts.alarm_check_due then return result end

    if facts.probability_sample == nil then
        result.request_probability_sample = true
        return result
    end
    -- The source accepts random values below 0x8000, exactly half of the
    -- unsigned 16-bit range. The adapter supplies that draw as a fraction.
    if facts.probability_sample >= 0.5 then return result end
    if not contains_player(facts.alarm_regions, facts.player_tile_x,
                          facts.player_tile_y) then
        return result
    end
    return raise(facts, result)
end

return M
