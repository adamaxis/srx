-- Scene 1/event 1: Tarislar City Inn.
-- ROM 0x53B16 owns the paid-stay/Tabatha service, while 0x1F65A owns the
-- ordinary, run-result, and Frosty prompts. The link data selects each entry
-- path from source-backed story facts before this module presents it.
local E = require("enums")
local StoryFlags = require("story_flags")

local Inn = {}

local LOCATION_ID = "puyallup_site_03_tarislar_city_inn"

local function at_inn()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID
end

local function more(menu_id, text)
    return game.ui.prompt_text(menu_id, text, { "more" }) == "more"
end

local function say(text, continuation)
    game.ui.say_text(text, continuation == true)
end

local function leave()
    say("I'll be seeing you, chum.", false)
    return "field_return"
end

local function room_offer()
    local choice = game.ui.prompt_text(
        "room_payment",
        table.concat({
            "Sorry, but all we have left are rooms with views.",
            "Those are going for a meager 30$.",
            "A fair price, chummer?",
        }, "||"),
        { "pay", "decline" })
    if choice == "decline" then return leave() end
    if choice ~= "pay" then return "field_return" end

    -- ROM command 0x06 charges the active FFF10D amount (30 at this branch).
    local payment = game.services.call("building", "charge_30")
    if payment == "paid" then
        say(table.concat({
            "Hope you brought your own towels.",
            "I've been cleaned out this month.",
            "Enjoy your stay.",
        }, "||"), false)
        return "hotel-options"
    end
    if payment == "insufficient" then
        say(table.concat({
            "Your stick seems to be low on creds.",
            "I'm sorry, but we can only board paying customers.",
        }, "||"), false)
    end
    return "field_return"
end

local function frosty_information()
    -- ROM command 0xE6 at 0x2497C sets FFFC30 bit 23, FFFC34 bits 4, 13,
    -- and 21, then returns the 0x1180D2 Sinsearach-land response.
    game.state.notebook_tip_flag_bit_23 = true
    game.state.field_mode_effect_flag_bit_4 = true
    game.state.field_mode_effect_flag_bit_13 = true
    game.state.field_mode_effect_flag_bit_21 = true
    say(table.concat({
        "I haven't seen her in a couple of days, but that's not too unusual.",
        "If you're in a hurry, you might try the Sinsearach lands south of the AmerIndian Village.",
        "Story is she went through some harrowing ritual to become a tribal member.",
    }, "||"), false)
    return "field_return"
end

local function mark_tabatha_acceptance()
    -- ROM command 0x14 at 0x21862 increments the source runner-byte triplet,
    -- sets FFFC30 bits 2/4/5/6/8, FFFC34 bits 2/4, and FFFC27 bits 2/4.
    if game.services.call("runner", "adjust_source_byte_triplet_1") ~=
       "applied" then
        return false
    end
    for _, bit in ipairs({ 2, 4, 5, 6, 8 }) do
        game.state["notebook_tip_flag_bit_" .. bit] = true
    end
    game.state.field_mode_effect_flag_bit_2 = true
    game.state.field_mode_effect_flag_bit_4 = true
    game.state.mr_johnsons_flag_bit_2 = true
    game.state.mr_johnsons_flag_bit_4 = true
    return true
end

local function tabatha_conversation()
    StoryFlags.apply_dialogue_command(0x16)
    if not more("shale_more", table.concat({
        "Tabatha Shale?  Yeah, she's here.",
        "She's been locked in her room for days and won't let anybody in, even to clean up.",
        "If you think you can talk some sense into her, be my guest.",
    }, "||")) then
        return "field_return"
    end
    if not more("shale_more", table.concat({
        "The manager connects you to her room via the vid-phone.",
        "The screen remains blank, but the woman's wary voice identifies itself as Tabatha.",
        "Words tumble from your mouth in a desperate rush to convince Tabatha of your sincerity.",
    }, "||")) then
        return "field_return"
    end
    if not more("shale_more",
                "After some deliberation, she concedes that you are Michael's brother and agrees to let you enter her apartment.") then
        return "field_return"
    end
    if not mark_tabatha_acceptance() then
        return "tabatha_state_unavailable"
    end

    local choice = game.ui.prompt_text(
        "shale_arrival",
        table.concat({
            "Tabatha Shale opens the door as you arrive.",
            "She looks as if she hasn't slept in a week.",
            "She invites you in, but her eyes nervously dart around the room.",
        }, "||"),
        { "about_michael" })
    if choice ~= "about_michael" then return "field_return" end

    if not more("shale_more", table.concat({
        "I don't know that much about it.  We were hired by an unknown Mr. Johnson through Caleb to track down a location in Salish-Shidhe.",
        "I backed out when he made the offer, so I wasn't included in the deal.",
        "Michael said one stop at Errascoe's and he'd be set.",
    }, "||")) then
        return "field_return"
    end

    local topic = game.ui.prompt_text(
        "shale_topics",
        table.concat({
            "Y'know, Caleb's not our normal Johnson.",
            "We usually use a Johnson named Mortimer Reed.",
            "He sets up in the Penumbra District, in a restaurant called The Big Rhino.",
        }, "||"),
        { "caleb", "errascoe" })
    if topic == "caleb" then
        say(table.concat({
            "He's a Johnson Michael and I used a couple of times.",
            "I don't trust him, though.  Who knows what those elves are up to anyway.",
            "You might find him at Icarus Descending; that's where we always met him.",
        }, "||"), false)
        local other = game.ui.prompt_text(
            "shale_other_topic",
            table.concat({
                "He's a Johnson Michael and I used a couple of times.",
                "I don't trust him, though.  Who knows what those elves are up to anyway.",
                "You might find him at Icarus Descending; that's where we always met him.",
            }, "||"),
            { "errascoe" })
        if other ~= "errascoe" then return "field_return" end
        say(table.concat({
            "Boris Errascoe is a small time fixer who works out of an abandoned greenhouse in Redmond.",
            "Anything for a buck, that's Boris.",
            "He was probably the last person in the sprawl to see Michael alive.",
        }, "||"), false)
    elseif topic == "errascoe" then
        say(table.concat({
            "Boris Errascoe is a small time fixer who works out of an abandoned greenhouse in Redmond.",
            "Anything for a buck, that's Boris.",
            "He was probably the last person in the sprawl to see Michael alive.",
        }, "||"), false)
        local other = game.ui.prompt_text(
            "shale_other_topic",
            table.concat({
                "Boris Errascoe is a small time fixer who works out of an abandoned greenhouse in Redmond.",
                "Anything for a buck, that's Boris.",
                "He was probably the last person in the sprawl to see Michael alive.",
            }, "||"),
            { "caleb" })
        if other ~= "caleb" then return "field_return" end
        say(table.concat({
            "He's a Johnson Michael and I used a couple of times.",
            "I don't trust him, though.  Who knows what those elves are up to anyway.",
            "You might find him at Icarus Descending; that's where we always met him.",
        }, "||"), false)
    else
        return "field_return"
    end

    local followup = game.ui.prompt_text(
        "shale_followup",
        "What do you need from me?",
        { "recruit", "thanks" })
    if followup == "recruit" then
        say(table.concat({
            "No way, ace!",
            "I loved Michael, but I don't plan on joining him.",
            "Somebody powerful wanted Michael's team out of the way.",
            "I don't want any part of something that large.",
        }, "||"), false)
        return "field_return"
    elseif followup == "thanks" then
        say(table.concat({
            "As if on cue, the door to the room explodes inward, leaving you momentarily stunned!",
            "A flash of automatic gunfire rakes Tabatha through the obscuring smoke and sends you diving for cover.",
        }, "||"), true)
        -- The continuation after 0xF617F dispatches encounter commands 0x4B,
        -- 0x4C, and 0x20. Keep the unconverted combat handoff visible.
        return "tabatha_ambush_unconverted"
    end
    return "field_return"
end

function Inn.present_indexed_title_service()
    if not at_inn() or
       StoryFlags.is_set(E.StoryFlag.MOTEL_BILL_PAID) ~= true or
       StoryFlags.is_set(E.StoryFlag.TARISLAR_ASKED_SHALE) ~= false or
       game.field.mode ~= 1 or game.field.event_code ~= 1 or
       game.field.alternate_result ~= 0 then
        return "denied"
    end

    -- ROM 0x53A78 publishes indexed title service 1 in FFDD64 before calling
    -- 0x53B16. The intro/options root is 0xF49E8/0xF49FC.
    StoryFlags.begin_trigger(1)
    if not more("shale_more", table.concat({
        "The Tarislar City Inn caters mostly to elves, as the word Tarislar is elvish for Remembrance.",
        "Despite its run-down appearance, this apartment has a great rep for being the safest place to stay in the Barrens.",
    }, "||")) then
        return "field_return"
    end

    local choice = game.ui.prompt_text(
        "inn_service",
        table.concat({
            "You need a place to stay?",
            "Well you've come to the right place.",
            "This is the safest hotel in the city, for the price.",
            "Of course, payment is required in advance.",
        }, "||"),
        { "crash", "leave", "shale" })
    if choice == "crash" then return room_offer() end
    if choice == "shale" then return tabatha_conversation() end
    return leave()
end

function Inn.present_field_exit_prompt()
    if not at_inn() or game.field.mode ~= 1 or
       game.field.event_code ~= 1 then
        return "denied"
    end

    if game.field.alternate_result > 0 then
        say(table.concat({
            "Oh, is that the package we've been waiting for?",
            "You hand him the package and he updates your credstick.",
        }, "||"), false)
        return "field_return"
    elseif game.field.alternate_result < 0 then
        say(table.concat({
            "You tell the proprietor that you are here for a package.",
            "He roots around, looking, and finds it.",
            "Handing it over, he wishes you a great day.",
        }, "||"), false)
        return "field_return"
    end

    if not more("shale_more", table.concat({
        "The Tarislar City Inn caters mostly to elves, as the word Tarislar is elvish for Remembrance.",
        "Despite its run-down appearance, this apartment has a great rep for being the safest place to stay in the Barrens.",
    }, "||")) then
        return "field_return"
    end
    local choices = { "crash" }
    if StoryFlags.is_set(E.StoryFlag.MICHAEL_MISSION_TOLD) == true and
       StoryFlags.is_set(E.StoryFlag.FROSTY_MET) == false then
        choices[#choices + 1] = "frosty"
    end
    choices[#choices + 1] = "leave"
    local choice = game.ui.prompt_text(
        "inn_service",
        table.concat({
            "You need a place to stay?",
            "Well you've come to the right place.",
            "This is the safest hotel in the city, for the price.",
            "Of course, payment is required in advance.",
        }, "||"),
        choices)
    if choice == "crash" then return room_offer() end
    if choice == "frosty" then return frosty_information() end
    return leave()
end

return Inn
