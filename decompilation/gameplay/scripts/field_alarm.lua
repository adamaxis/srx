-- Field alarm policy for ROM 0x1DFC4..0x1E0DC. C++ supplies scene facts,
-- profile rectangles, actor flags, and lazy source RNG draws; Lua selects the
-- state transition and typed side effects. [C]
local E = require("enums")
local M = {}

local function is_integer(value, minimum, maximum)
    return type(value) == "number" and value % 1 == 0 and
           value >= minimum and value <= maximum
end

local function valid_rectangles(rectangles)
    if type(rectangles) ~= "table" then return false end
    for _, rect in ipairs(rectangles) do
        if type(rect) ~= "table" or
           not is_integer(rect.x_min, 0, E.FieldAlarm.WORD_MAX) or
           not is_integer(rect.x_max, 0, E.FieldAlarm.WORD_MAX) or
           not is_integer(rect.y_min, 0, E.FieldAlarm.WORD_MAX) or
           not is_integer(rect.y_max, 0, E.FieldAlarm.WORD_MAX) then
            return false
        end
    end
    return true
end

local function valid_facts(facts)
    return type(facts) == "table" and
        is_integer(facts.scene_id, 0, E.SceneId.FINAL) and
        is_integer(facts.frame_counter, 0, E.FieldAlarm.WORD_MAX) and
        is_integer(facts.scene_mode, E.SceneMode.NONE, E.SceneMode.CAVE) and
        is_integer(facts.player_tile_x, 0, E.FieldAlarm.WORD_MAX) and
        is_integer(facts.player_tile_y, 0, E.FieldAlarm.WORD_MAX) and
        type(facts.disabled) == "boolean" and
        type(facts.alarm) == "boolean" and
        is_integer(facts.timer, E.FieldAlarm.TIMER_MIN,
                   E.FieldAlarm.TIMER_MAX) and
        (facts.rng == nil or
         is_integer(facts.rng, 0, E.FieldAlarm.WORD_MAX)) and
        (facts.direct_raise == nil or type(facts.direct_raise) == "boolean") and
        (facts.clear_alarm == nil or type(facts.clear_alarm) == "boolean") and
        (facts.disable_alarm == nil or type(facts.disable_alarm) == "boolean") and
        (facts.reset == nil or type(facts.reset) == "boolean") and
        (facts.reinforcement_list_address == nil or
         is_integer(facts.reinforcement_list_address, 0,
                    E.FieldAlarm.LIST_ADDRESS_MAX)) and
        valid_rectangles(facts.rects) and
        (facts.actors == nil or type(facts.actors) == "table")
end

local function state_result(facts)
    return {
        alarm = facts.alarm,
        timer = facts.timer,
        disabled = facts.disabled,
        actors = facts.actors or {},
    }
end

local function actor_flag_action(actors, action)
    for _, actor in ipairs(actors) do
        if type(actor) == "table" and
           is_integer(actor.flags9e, 0, E.FieldAlarm.ACTOR_FLAGS_MAX) then
            if action == "clear_spotted" then
                actor.flags9e = actor.flags9e & ~E.ActorFlag.SPOTTED
            elseif action == "promote_reinforcement" and
                   (actor.flags9e & E.ActorFlag.REINFORCEMENT) ~= 0 then
                actor.flags9e = actor.flags9e | E.ActorFlag.DESPAWN_OFFSCREEN
            end
        end
    end
end

-- ROM 0x1E09A clears FFE16F only when active, promotes +0x9E bit 1 to bit 2
-- on the seven actor records, then always stops sound 0x30. [C]
function M.clear(state, actors)
    if type(state) ~= "table" or type(state.alarm) ~= "boolean" then
        return nil
    end
    local was_active = state.alarm
    state.alarm = false
    if was_active then
        actor_flag_action(actors or {}, "promote_reinforcement")
    end
    return {
        was_active = was_active,
        sound_stop = E.SoundId.ALARM,
    }
end

-- ROM 0x167AE..0x167D8 clears FFE16F and FFE170 after marking a projected
-- terrain cell. It does not run 0x1E09A's reinforcement promotion or sound
-- stop path. [C]
function M.reset(state)
    if type(state) ~= "table" then return nil end
    state.alarm = false
    state.disabled = false
    return state
end

local function raised_result(facts, result)
    -- ROM 0x1E04A refreshes FFE11E before testing the alarm latch. An already
    -- active alarm therefore gets the full source timer again without a new
    -- sound, actor-flag clear, or reinforcement pass. [C]
    result.timer = E.AiTuning.ALARM_FRAMES
    if result.alarm then return result end

    result.alarm = true
    result.sound_start = E.SoundId.ALARM
    result.clear_flags = "clear_spotted"
    actor_flag_action(result.actors, result.clear_flags)
    result.spawn_reinforcements = true
    result.reinforcement_list_address = facts.reinforcement_list_address
    result.reinforcement_flag = E.ActorFlag.REINFORCEMENT
    return result
end

local function raise(facts, result)
    -- The alarm-combat scene takes 0x1DBAA's timeout path and does not set the
    -- alarm latch; other scenes require FFDF40 == building mode 2. [C]
    if facts.scene_id == E.SceneId.ALARM_COMBAT then
        result.schedule_timeout = true
        return result
    end
    if facts.scene_mode ~= E.SceneMode.BUILDING then return result end
    return raised_result(facts, result)
end

local function contains_player(rectangles, x, y)
    for _, rect in ipairs(rectangles) do
        -- 0x1E030..0x1E044 use unsigned compares: lower bounds are exclusive,
        -- upper bounds inclusive. [C]
        if rect.x_min < x and x <= rect.x_max and
           rect.y_min < y and y <= rect.y_max then
            return true
        end
    end
    return false
end

function M.tick(facts)
    if not valid_facts(facts) then return nil end
    local result = state_result(facts)

    if facts.reset then
        return M.reset(result)
    end
    if facts.disable_alarm or facts.clear_alarm then
        if facts.disable_alarm then
            result.disabled = true -- FFE170, set by a successful security disable. [C]
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

    -- ROM subtracts the active timer before checking FFE170. A negative result
    -- enters the shared clear path even when security is disabled. [C]
    if result.alarm then
        result.timer = result.timer - 1
        if result.timer < 0 then
            local clear = M.clear(result, result.actors)
            result.sound_stop = clear.sound_stop
            result.clear_flags = clear.was_active and
                "promote_reinforcement" or nil
            return result
        end
    end
    if result.disabled then return result end

    -- 0x1DFC4 samples randomness only on the 13th frame of each 64-frame
    -- period. The host draws only after Lua requests it. [C]
    if (facts.frame_counter & E.FieldAlarm.FRAME_MASK) ~=
       E.FieldAlarm.FRAME_VALUE then
        return result
    end
    if facts.rng == nil then
        result.sample_rng = true
        return result
    end
    if facts.rng >= E.FieldAlarm.RANDOM_REJECT_FROM then return result end
    if not contains_player(facts.rects, facts.player_tile_x,
                           facts.player_tile_y) then
        return result
    end
    return raise(facts, result)
end

return M
