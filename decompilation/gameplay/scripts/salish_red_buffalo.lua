-- ROM 0x54322: scene 6/event 1 is a same-map title service. Its D6=-1
-- suppresses the event-table destination; Lua owns the story branch and its
-- completion facts, while the host presents the typed dialogue operations.
local E = require("enums")
local StoryFlags = require("story_flags")
local HarlequinDialogue = require("harlequin_dialogue")

local RedBuffalo = {}
local LOCATION_ID = "salish_red_buffalo_lodge"

local function at_lodge()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID and
           game.field.mode == 6 and game.field.event_code == 1
end

local function more(menu_id, text)
    return game.ui.prompt_text(menu_id, text, { "more" }) == "more"
end

local INTRODUCTION = table.concat({
    "<A lone AmerIndian woman is knitting here.",
    "She looks up at you, and introduces herself as Red Buffalo Woman.>",
}, "||")

local GIFT = table.concat({
    "Thank you, elf-friend.",
    "Please deliver my gift to Lady Gillian.",
    "Good luck on your journey.",
}, "||")

local NO_GIFT = table.concat({
    "I'm afraid I don't get many visitors out here, so I have nothing to offer you.",
    "But I wish you well on your journies.",
    "Good day.",
}, "||")

local NO_BUSINESS_FOLLOWUP =
    "<Thinking over how quickly they could cut you to ribbons, you agree you don't have any reason to be here, and quickly leave.>"

local NO_BUSINESS_RESPONSE = table.concat({
    "<Passing even more armed elves, you enter the penthouse.",
    "Inside you face a bizarre sight.",
    "There stands an elf, his face tatooed like a clown; however, his expression reflects his serious nature.",
    "This must be Harlequin.>",
}, "||")

function RedBuffalo.present_indexed_title_service()
    if not at_lodge() then return "denied" end

    -- The ROM selects F4DB8 only while the Ritual of Binding flag (17) is
    -- set and Red Buffalo's service bit (18) is clear. Otherwise it uses
    -- F4DCC. Both branches keep the player on scene 6.
    StoryFlags.begin_trigger(E.StoryFlag.RED_BUFFALO_WOMAN)
    if StoryFlags.is_set(E.StoryFlag.RITUAL_OF_BINDING) == true and
       StoryFlags.is_set(E.StoryFlag.RED_BUFFALO_WOMAN) == false then
        if not more("introduction", INTRODUCTION) then
            return "field_return"
        end
        if not more("gift", GIFT) then
            return "field_return"
        end

        -- 0x5435C marks FFDD64's active trigger; the adjacent writes update
        -- FFFC22 bits 5/6 and FFFC30 bits 29/30.
        StoryFlags.finish_trigger(E.StoryFlag.RED_BUFFALO_WOMAN)
        game.state.notebook_tip_flag_bit_6 = false
        game.state.notebook_tip_flag_bit_5 = true
        game.state.field_mode_effect_flag_bit_29 = false
        game.state.field_mode_effect_flag_bit_30 = true
        return "field_return"
    end

    if not more("introduction", INTRODUCTION) then
        return "field_return"
    end
    local choice = game.ui.prompt_text("no_gift", NO_GIFT,
                                       { "no_business", "more" })
    if choice == "no_business" then
        game.ui.say_text(NO_BUSINESS_FOLLOWUP, false)
    elseif choice == "more" then
        -- ROM 0xF5888 / 0xF58A4 map More to the armed-elves and penthouse
        -- response. The no-business choice alone uses the leave text.
        game.ui.say_text(NO_BUSINESS_RESPONSE, false)
        HarlequinDialogue.present_main_conversation()
    end
    return "field_return"
end

return RedBuffalo
