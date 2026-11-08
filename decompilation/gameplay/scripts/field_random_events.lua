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
    "<You grab the man and hold him while the henchmen straight-jacket " ..
    "him and drag him off.>||Here's fifty nuyen. Now get lost."
local HAGGARD_CREDENTIALS_RESULT =
    "<They explain that they work for an insane asylum and that this is one " ..
    "of their patients.||After showing the proper credentials they give you " ..
    "50$ for your help, take their patient, and leave.>"
local HAGGARD_THREATEN_RESULT =
    "<The doctor turns to his two assistants.>||Let's go.||" ..
    "This one's about dry, anyway."
local HAGGARD_INTRO_TYPICAL =
    "<The man looks like a typical doctor to you.>"
local HAGGARD_INTRO_UNSURE =
    "<You're not sure what to make of this strange group.>"
local HAGGARD_INTRO_MALIGNANT =
    "<The well-dressed man has a malignant air about him.||" ..
    "The ragged victim seems terrified.>"
local HAGGARD_CHOICES = {
    { id = "attack", label = "Attack!" },
    { id = "threaten", label = "Threaten to leave the patient alone." },
    { id = "help", label = "Help them." },
} -- Longplay section 11, frame 930; source dispatch 0x23BB4 [C].
-- The event artwork in section 11 frames 000618/000930/000990 matches this
-- extracted image; give this route its own binding because field_narrator has
-- no portrait.
local HAGGARD_SPEAKER = "haggard_man_event"
local HAGGARD_TEXT_PROFILE = "field.world_event"

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

local function first_runner_attribute(attribute)
    if type(game.party) ~= "function" then return nil end
    local party = game.party()
    local runner = type(party) == "table" and party[0] or nil
    local attributes = type(runner) == "table" and runner.attributes or nil
    local value = type(attributes) == "table" and attributes[attribute] or nil
    if type(value) ~= "number" or value % 1 ~= 0 or value < 0 or
       value > 0xFF then
        return nil
    end
    return value
end

local function haggard_intro_text(successes, selector_roll)
    if selector_roll < 50 then
        if successes == 0 then return HAGGARD_INTRO_TYPICAL end
        if successes == 1 then return HAGGARD_INTRO_UNSURE end
        return HAGGARD_INTRO_MALIGNANT
    end
    if successes == 0 then return HAGGARD_INTRO_MALIGNANT end
    if successes == 1 then return HAGGARD_INTRO_UNSURE end
    return HAGGARD_INTRO_TYPICAL
end

local function haggard_reward(amount, result_text, missing_funds_reason)
    local funds = type(game.nuyen) == "function" and game.nuyen() or nil
    if type(funds) ~= "number" or funds % 1 ~= 0 or funds < 0 then
        return source_boundary(missing_funds_reason)
    end
    game.set_nuyen(funds + amount)
    game.ui.say_text(result_text, false, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)
    return "resolved"
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

    game.ui.say_text(opening, true, HAGGARD_SPEAKER,
                     HAGGARD_TEXT_PROFILE)

    local intelligence = first_runner_attribute("intelligence")
    local modules = type(game.modules) == "table" and game.modules or nil
    local combat = type(modules) == "table" and modules.combat or nil
    if intelligence == nil or type(combat) ~= "table" or
       type(combat.roll_thresholded_successes) ~= "function" then
        return source_boundary(
            "ROM 0x23BB4 needs the first runner's Intelligence and dice service")
    end
    -- FF017B is the first runner record's Intelligence (+0x7B). The ROM
    -- checks that pool against 6 before its separate 0-99 prompt roll.
    local successes = combat.roll_thresholded_successes(
        intelligence, 6, random_die)
    if successes == nil then
        return source_boundary("ROM 0x23BB4 Intelligence roll is unavailable")
    end
    local selector_roll = random_die(100)
    if selector_roll == nil then
        return source_boundary("ROM 0x23BB4 prompt roll is unavailable")
    end
    local prompt_variant = selector_roll < 50 and "alternate" or "standard"
    local choice = game.ui.prompt_text(
        "haggard_man_action", haggard_intro_text(successes, selector_roll),
        HAGGARD_CHOICES, HAGGARD_SPEAKER, HAGGARD_TEXT_PROFILE)

    if prompt_variant == "standard" then
        if choice == "attack" then
            -- The first command at 0x12111C returns data pointer 0x121180;
            -- the record's next interaction is still an explicit boundary.
            return source_boundary(
                "ROM 0x12111C Attack returns through prompt data 0x121180")
        end
        if choice == "threaten" then
            -- Command 0xA9 reaches 0x23C38. Its 0-49 result pays 50 via
            -- 0x23CE0; 50-99 enters the police prompt at 0x12115C.
            local roll = random_die(100)
            if roll == nil then
                return source_boundary(
                    "ROM 0x23C38 needs a valid 0-99 random result")
            end
            if roll >= 50 then
                return source_boundary(
                    "ROM 0x23C38 patrol prompt at 0x12115C is not implemented")
            end
            return haggard_reward(
                50, HAGGARD_CREDENTIALS_RESULT,
                "ROM 0x23CE0 reward needs the current nuyen value")
        end
        if choice == "help" then
            -- Command 0xAC directly reaches 0x23CE0 in this prompt variant.
            return haggard_reward(
                50, HAGGARD_CREDENTIALS_RESULT,
                "ROM 0x23CE0 reward needs the current nuyen value")
        end
    elseif choice == "attack" then
        -- Command 0xAA reaches 0x23C52 and checks first-runner Body at
        -- FF0177 against target 6. Its <=1-success combat lists remain an
        -- explicit boundary; the >1 path pays 100 and uses 0x1211A0.
        local body = first_runner_attribute("body")
        if body == nil then
            return source_boundary(
                "ROM 0x23C52 needs the first runner's Body")
        end
        local attack_successes = combat.roll_thresholded_successes(
            body, 6, random_die)
        if attack_successes == nil then
            return source_boundary("ROM 0x23C52 Body roll is unavailable")
        end
        if attack_successes <= 1 then
            return source_boundary(
                "ROM 0x23C52 combat actor lists 0x1D704C/0x1D7050 are not implemented")
        end
        return haggard_reward(
            100, HAGGARD_THREATEN_RESULT,
            "ROM 0x23C74 reward needs the current nuyen value")
    elseif choice == "threaten" then
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

        return haggard_reward(
            100, HAGGARD_THREATEN_RESULT,
            "ROM 0x23CCE reward needs the current nuyen value")
    elseif choice == "help" then
        -- Command 0xAD reaches 0x23CF2 and pays 50 nuyen.
        return haggard_reward(
            50, HAGGARD_HELP_RESULT,
            "ROM 0x23CF2 reward needs the current nuyen value")
    end

    return source_boundary(
        "ROM 0x23BB4 choice continuation is not implemented: " ..
        tostring(choice))
end

return RandomEvents
