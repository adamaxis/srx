-- ROM 0x53D5A handles Scene-4 event 27 (Spirit Eyes). The independent
-- Scene-47 event-25 continuation at 0x53EFA handles Licourtrix's lair.
-- The field host supplies the source route and speaker assets; Lua owns the
-- story gates, item handoffs, and prompt graph.
local E = require("enums")
local StoryFlags = require("story_flags")
local DialogueGraph = require("dialogue_graph")

local SpiritEyes = {}

local SERPENT_SITE = "salish_feathered_serpent_lair"

local function at_site(location_id, event_code)
    return game.location.id == location_id and
           game.field.event_code == event_code
end

local function say(text, continuation)
    game.ui.say_text(text, continuation == true)
end

local function more(menu_id, text)
    return game.ui.prompt_text(menu_id, text, { "more" }) == "more"
end

local function reminder(text)
    say(text, false)
    return "field_return"
end

function SpiritEyes.present()
    return DialogueGraph.run("salish.spirit_eyes.ritual")
end

function SpiritEyes.present_serpent_encounter()
    if not at_site(SERPENT_SITE, 25) then
        return "denied"
    end

    if StoryFlags.is_set(E.StoryFlag.SHAMAN_PELT_GIVEN) == true then
        return reminder(table.concat({
            "You glimpse a feathered serpent taking flight through a large opening in the ceiling.",
            "Most likely, it was alerted by the commotion of your battle with the Renraku Strike Team.",
        }, "||"))
    end
    if StoryFlags.is_set(E.StoryFlag.SERPENT_SCALE_TAKEN) == true then
        return reminder(table.concat({
            "Licourtrix's lair is empty except for a few decaying bodies from Renraku's attack.",
            "It looks as if he doesn't plan on returning.",
        }, "||"))
    end

    -- Event 25 reaches 0x53EFA only on the context-0x13 route. The first
    -- conversation root identifies the Renraku team and carries the player
    -- through the source text/choice nodes before its 0x53F50 state update.
    -- ROM 0x53F00 stores FFDD64=8 before it opens Licourtrix's profile and
    -- the F4C44 conversation. Keep the active index separate from story bit 8,
    -- which is only marked by the post-dialogue 0x2196C call.
    StoryFlags.select_fixed_trigger(E.StoryFlag.SERPENT_SCALE_TAKEN)
    if not more("serpent_more", table.concat({
        "Entering this antechamber, you see an Renraku Strike Team attempting to kill a feathered serpent.",
        "Knowing the evil that Renraku seems to breed, you join sides with the dragon and quickly finish them off.",
    }, "||")) then
        return "field_return"
    end
    if not more("serpent_more", table.concat({
        "With the last of the Renraku Strike Team dealt with, you find yourself face to face with the feathered serpent.",
        "As you stand wondering what it will do next, an ethereal voice fills your head.",
    }, "||")) then
        return "field_return"
    end
    if not more("serpent_more", table.concat({
        "I thank you, human, for assisting me.",
        "It seems the Renraku Corporation has it in for my kind.",
        "They now most likely have it in for you also.",
    }, "||")) then
        return "field_return"
    end

    local answer = game.ui.prompt_text("serpent_intentions", table.concat({
        "But tell me, how did you happen to come here?",
        "Before you answer, remember, you deal with a dragon.",
    }, "||"), { "neighborhood", "scale" })
    while answer == "neighborhood" do
        if not more("serpent_more", table.concat({
            "You try my patience, human!",
            "I know you sought me.",
            "Now be honest about your intentions, before my gratuitous patience wanes!",
        }, "||")) then
            return "field_return"
        end
        answer = game.ui.prompt_text("serpent_lie", "How did you happen to come here?", {
            "neighborhood", "scale",
        })
    end
    if answer ~= "scale" then
        return "field_return"
    end
    if game.ui.prompt_text("serpent_thanks", table.concat({
            "The feathered serpent gazes at you for what seems like an eternity.",
            "Honesty is so rare in humans.",
            "Understand it is appreciated, even by non-humans.",
        }, "||"), { "thanks" }) ~= "thanks" then
        return "field_return"
    end
    if game.ui.prompt_text("serpent_task", table.concat({
            "I would be glad to help with such a small task.",
            "I feel, however, that it is not enough.",
        }, "||"), { "what" }) ~= "what" then
        return "field_return"
    end
    if not more("serpent_more", table.concat({
        "I owe my life to you, and I sense you have many trials ahead of you.",
        "One of these trials I doubt you will survive.",
    }, "||")) then
        return "field_return"
    end
    if not more("serpent_more", table.concat({
        "Thus I will bond myself to protect you in your quest for the truth.",
        "Know that I will appear only once to aid you, and only when that situation is dire enough for my intervention.",
    }, "||")) then
        return "field_return"
    end
    if not more("serpent_more", table.concat({
        "My name is Licourtrix, and now we are bound.",
        "Fare thee well, human, until fate brings us together again.",
        "Licourtrix shakes loose a scale from his hide as he flies away.",
        "Excitement barely contained, you snatch up the final item for the ritual.",
    }, "||")) then
        return "field_return"
    end

    -- 0x53F00 sets FFDD64=8. After 0x1F246, 0x53F50 calls 0x2196C to set
    -- story bit 8, then BSETs FFFC22 bit 2 for the Feathered Scale. The
    -- Spirit Eyes route consumes that item at 0x53E98 on return.
    StoryFlags.set(E.StoryFlag.SERPENT_SCALE_TAKEN)
    game.state.group_item_flag_bit_2 = true
    return "field_return"
end

return SpiritEyes
