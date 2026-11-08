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
local HAGGARD_HELP_RESULT =
    "The patient is restrained and taken away. The doctor pays you 50 nuyen."
local HAGGARD_THREATEN_RESULT =
    "The doctor turns to his two assistants. <Let's go.> " ..
    "This one's about dry, anyway."
local HAGGARD_CHOICES = {
    { id = "attack", label = "Attack!" },
    { id = "threaten", label = "Threaten to leave the patient alone." },
    { id = "help", label = "Help them." },
} -- Longplay section 11, frame 930; source dispatch 0x23BB4 [C].

local function event_opening(event_id)
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
        if row.event_id == event_id then
            opening = row.opening_text
            break
        end
    end
    local choice_count = 0
    for _, row in ipairs(choices) do
        if row.event_id == event_id then
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

    local opening = event_opening("hidden_camera")
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

local function source_boundary(reason)
    return { type = "source_boundary", reason = reason }
end

function RandomEvents.present_haggard_man()
    local location = game.location
    if type(location) ~= "table" or location.type ~= "field_map" or
       location.id ~= "random_field_event" or
       location.event_id ~= "haggard_man_collapse" then
        return nil
    end

    local opening = event_opening("haggard_man_collapse")
    if opening == nil then
        return source_boundary(
            "ROM 0x20FA2 doctor opening data is unavailable")
    end

    game.ui.say_text(opening, true)
    local choice = game.ui.prompt_text(
        "haggard_man_action", "", HAGGARD_CHOICES)
    if choice == "threaten" then
        -- Command 0xAB reaches 0x23CB4. Its 0-49 result adds 0x64 to
        -- FFFBFE and displays 0x1211A0; 50-99 enters the patrol prompt
        -- at 0x12115C, which remains an explicit boundary.
        local roll = random_die(100)
        if roll == nil then
            return source_boundary(
                "ROM 0x23CB4 needs a valid 0-99 random result")
        end
        if roll >= 50 then
            return source_boundary(
                "ROM 0x23CB4 patrol prompt at 0x12115C is not implemented")
        end

        local funds = type(game.nuyen) == "function" and game.nuyen() or nil
        if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 then
            return source_boundary(
                "ROM 0x23CB4 reward needs the current nuyen value")
        end
        game.set_nuyen(funds + 100)
        game.ui.say_text(HAGGARD_THREATEN_RESULT, false)
        return "resolved"
    end

    if choice ~= "help" then
        -- Attack includes the random/patrol branch at 0x23C38 and the
        -- separate dice/spawn branch at 0x23C52, still explicit boundaries.
        return source_boundary(
            "ROM 0x23BB4 choice continuation is not implemented: " ..
            tostring(choice))
    end

    local funds = type(game.nuyen) == "function" and game.nuyen() or nil
    if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 then
        return source_boundary(
            "ROM 0x23CF2 reward needs the current nuyen value")
    end
    game.set_nuyen(funds + 50) -- 0x23CF2 adds 50; result text at 0x121188.
    game.ui.say_text(HAGGARD_HELP_RESULT, false)
    return "resolved"
end

return RandomEvents
