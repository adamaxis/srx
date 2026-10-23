-- Scene 1/event 1: Tarislar City Inn.
-- ROM 0x53B16 owns the paid-stay/Tabatha service, while 0x1F65A owns the
-- ordinary, run-result, and Frosty prompts. The link data selects each entry
-- path from source-backed story facts before this module presents it.
local E = require("enums")
local StoryFlags = require("story_flags")
local DialogueGraph = require("dialogue_graph")

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

local function apply_tabatha_acceptance()
    -- ROM command 0x14 at 0x21862 increments the source runner-byte triplet,
    -- sets FFFC30 bits 2/4/5/6/8, FFFC34 bits 2/4, and FFFC27 bits 2/4.
    if game.services.call("runner", "adjust_source_byte_triplet_1") ~=
       "applied" then
        return "service_unavailable"
    end
    for _, bit in ipairs({ 2, 4, 5, 6, 8 }) do
        game.state["notebook_tip_flag_bit_" .. bit] = true
    end
    game.state.field_mode_effect_flag_bit_2 = true
    game.state.field_mode_effect_flag_bit_4 = true
    game.state.mr_johnsons_flag_bit_2 = true
    game.state.mr_johnsons_flag_bit_4 = true
    return "applied"
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
    -- 0x53B16. The intro, manager options, and paid-stay branch are represented
    -- by the reusable JSON graph. Its page and choice permutations follow the
    -- ROM prompt records; the runner source-byte-triplet update is a narrow
    -- registered host action, and the post-attack encounter remains a boundary.
    return DialogueGraph.run(
        "tarislar_city_inn.paid_stay_and_shale_service", {
            ["tarislar_city_inn.apply_tabatha_acceptance"] = function()
                return apply_tabatha_acceptance()
            end,
        })
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
