-- Source-backed handlers for the weighted field-event selector.
-- ROM 0x21004 opens the hidden-camera prompt; its MORE command at 0x244D2
-- rolls the party's average Quickness against a party-size target. Source
-- addresses stay here as provenance; Lua works with named runner facts.
local RandomEvents = {}

local ALARM_RESULT =
    "<As the alarm goes off, you realize you must have just walked in front " ..
    "of a hidden security camera.>"
local NERVES_RESULT =
    "<Nothing happens, though. Maybe it's just your nerves.>"

local function hidden_camera_opening()
    local data = game.modules.world_content_data
    local content = type(data) == "table" and
        data.world_activity_event_content or nil
    local choices = type(data) == "table" and
        data.world_activity_event_choices or nil
    if type(content) ~= "table" or type(choices) ~= "table" then
        return nil
    end

    local opening
    for _, row in ipairs(content) do
        if row.event_id == "hidden_camera" then
            opening = row.opening_text
            break
        end
    end
    local choice_count = 0
    for _, row in ipairs(choices) do
        if row.event_id == "hidden_camera" then
            choice_count = choice_count + 1
            if row.choice_order ~= 1 or row.choice_id ~= "continue" then
                return nil
            end
        end
    end
    if type(opening) ~= "string" or opening == "" or choice_count ~= 1 then
        return nil
    end
    return opening
end

local function party_check()
    if type(game.party) ~= "function" then return nil end
    local party = game.party()
    if type(party) ~= "table" then return nil end

    local quickness_total = 0
    local active_count = 0
    for _, member in pairs(party) do
        local lifecycle = type(member) == "table" and
            type(member.runner_facts) == "table" and
            member.runner_facts.lifecycle or nil
        if type(lifecycle) == "table" and lifecycle.active == true then
            local quickness = type(member.attributes) == "table" and
                member.attributes.quickness or nil
            if type(quickness) ~= "number" or quickness % 1 ~= 0 or
               quickness < 0 or quickness > 0xFF then
                return nil
            end
            quickness_total = quickness_total + quickness
            active_count = active_count + 1
        end
    end
    if active_count == 0 or active_count > 3 then return nil end

    return math.floor(quickness_total / active_count), 4 + active_count - 1
end

local function random_die(bound)
    local response = game.services.call("random", "bounded_" .. bound)
    local digits = type(response) == "string" and
        response:match("^roll_(%d+)$") or nil
    local value = digits and tonumber(digits) or nil
    if value == nil or value % 1 ~= 0 or value < 0 or value >= bound then
        return nil
    end
    return value
end

function RandomEvents.present_hidden_camera()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "hidden_camera" then
        return nil
    end

    local opening = hidden_camera_opening()
    local dice_pool, target = party_check()
    if opening == nil or dice_pool == nil then return nil end

    -- ROM prompt 0x12130C has one MORE command. The generic dialogue host
    -- renders the source text and forwards its single continuation input.
    game.ui.say_text(opening, true)

    local combat = game.modules.combat
    if type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return nil
    end
    local successes = combat.roll_thresholded_successes(
        dice_pool, target, random_die)
    if successes == nil then return nil end

    if successes == 0 then
        -- ROM 0x244D2 calls the shared alarm routine 0x1E04A before its
        -- result record. Scene 20's timeout and map-specific reinforcements
        -- stay inside the existing typed field alarm service.
        if game.services.call("field", "raise_alarm") ~= "applied" then
            return nil
        end
        game.ui.say_text(ALARM_RESULT, false)
    else
        game.ui.say_text(NERVES_RESULT, false)
    end
    return "resolved"
end

return RandomEvents
