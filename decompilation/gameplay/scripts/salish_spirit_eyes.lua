-- ROM 0x53D5A handles Scene-4 event 27 (Spirit Eyes). The independent
-- Scene-47 event-25 continuation at 0x53EFA handles Licourtrix's lair.
-- The field host supplies the source route and speaker assets; Lua owns the
-- story gates, item handoffs, and prompt graph.
local E = require("enums")
local StoryFlags = require("story_flags")

local SpiritEyes = {}

local SPIRIT_SITE = "council_spirit_eyes_site"
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

local FIRST_CONTACT = table.concat({
    "You come upon an ancient AmerIndian sitting cross-legged near an open fire pit.",
    "Your breath catches in your throat as his eyes slide open and he turns to face you.",
}, "||")

local NEED_HORN = table.concat({
    "I cannot instruct you further until I have the horn of a gargoyle.",
    "Seek them in the caves or abandoned areas.",
}, "||")

local SECOND_ITEM = table.concat({
    "Well done, young warrior.  The second item required is the pelt of a hell hound.",
    "These foul animals travel in packs, attacking lone travelers or defenseless groups.",
    "Remember, the pelt must be removed by your own hand.",
}, "||")

local NEED_PELT = table.concat({
    "The hell hound pelt is critical for the preparation of the gargoyle horn.",
    "Quickly, bring a pelt to me so that I may begin the final process!",
}, "||")

local FINAL_ITEM = table.concat({
    "The final task is at hand. Return with the living scale of a feathered serpent.",
    "The feather must be given freely, not taken, from the dragon to be of use.",
    "Return with this final item and we will learn your opponent's name.",
}, "||")

local NEED_SCALE = table.concat({
    "All is ready but for the living feather.",
    "I know not where to find a dragon, but they are solitary creatures with vast intelligence and powers!",
    "All I await is this final item.",
}, "||")

local TASKS_DONE = table.concat({
    "You have completed your tasks, young warrior.  Rest while I call upon the spirits.",
    "In return for the gifts you provided, they will guide me to the answer you seek.",
}, "||")

function SpiritEyes.present()
    if not at_site(SPIRIT_SITE, 27) then
        return "denied"
    end

    if StoryFlags.is_set(E.StoryFlag.SHAMAN_MET) ~= true then
        -- 0x53D5A sets story bit 5, FFFC30 bits 15/16, FFFC40 bit 0,
        -- and FFDD64=5 before opening F4BD8. Command 0x15 in that node marks
        -- the same active story index.
        StoryFlags.mark_fixed(E.StoryFlag.SHAMAN_MET)
        game.state.field_mode_effect_flag_bit_14 = false
        game.state.field_mode_effect_flag_bit_15 = true
        game.state.field_mode_effect_flag_bit_16 = true
        game.state.field_auxiliary_flag_bit_0 = true
        if not more("first_contact", FIRST_CONTACT) then
            return "field_return"
        end
        StoryFlags.apply_dialogue_command(0x15)
        return reminder(NEED_HORN)
    end

    if StoryFlags.is_set(E.StoryFlag.SHAMAN_ITEM4_GIVEN) ~= true then
        if game.state.group_item_flag_bit_4 ~= true then
            return reminder(NEED_HORN)
        end
        -- 0x53DD0 consumes Group Item 4 and sets story bit 6 before F4C34.
        game.state.group_item_flag_bit_4 = false
        StoryFlags.set(E.StoryFlag.SHAMAN_ITEM4_GIVEN)
        game.state.field_mode_effect_flag_bit_14 = false
        game.state.field_mode_effect_flag_bit_16 = false
        game.state.field_mode_effect_flag_bit_17 = true
        game.state.field_auxiliary_flag_bit_0 = false
        game.state.field_auxiliary_flag_bit_1 = true
        return reminder(SECOND_ITEM)
    end

    if StoryFlags.is_set(E.StoryFlag.SHAMAN_PELT_GIVEN) ~= true then
        if game.state.group_item_flag_bit_3 ~= true then
            return reminder(NEED_PELT)
        end
        -- 0x53E38 consumes Group Item 3 and sets story bit 7 before F4C3C.
        game.state.group_item_flag_bit_3 = false
        StoryFlags.set(E.StoryFlag.SHAMAN_PELT_GIVEN)
        game.state.field_mode_effect_flag_bit_17 = false
        game.state.field_mode_effect_flag_bit_18 = true
        game.state.field_auxiliary_flag_bit_1 = false
        game.state.field_auxiliary_flag_bit_2 = true
        return reminder(FINAL_ITEM)
    end

    if StoryFlags.is_set(E.StoryFlag.SERPENT_SCALE_TAKEN) ~= true then
        return reminder(NEED_SCALE)
    end

    if StoryFlags.is_set(E.StoryFlag.SHAMAN_TASKS_DONE) ~= true then
        if game.state.group_item_flag_bit_2 ~= true then
            return reminder(NEED_SCALE)
        end
        -- 0x53EA8 consumes Group Item 2, writes story bit 9, and advances
        -- the FFFC30 ritual marker from bits 15/18 to bit 19 before F4C04.
        game.state.group_item_flag_bit_2 = false
        -- The direct branch sets the fixed active trigger and story bit
        -- before F4C04. Its 0x1A command then marks that same trigger after
        -- the completion dialogue.
        StoryFlags.select_fixed_trigger(E.StoryFlag.SHAMAN_TASKS_DONE)
        StoryFlags.set(E.StoryFlag.SHAMAN_TASKS_DONE)
        game.state.field_mode_effect_flag_bit_15 = false
        game.state.field_mode_effect_flag_bit_18 = false
        game.state.field_mode_effect_flag_bit_19 = true
        game.state.field_auxiliary_flag_bit_2 = false
        say(TASKS_DONE, true)
        StoryFlags.apply_dialogue_command(0x1A)
        return "field_return"
    end

    -- F4C18 is the post-ritual answer root.
    say(table.concat({
        "Warrior, why do you waste time here with me?",
        "There are no more answers I can provide.",
        "You must seek out Thon!",
        "You must find the clues yourself!",
    }, "||"), true)
    say(table.concat({
        "Evil walks the land!",
        "It is more powerful than I expected and dedicated to the pursuit of power.",
    }, "||"), true)
    return reminder(table.concat({
        "This is the true face of your enemy, and his name is",
        "THON!",
    }, "||"))
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

    -- 0x53F00 sets FFDD64=8. After 0x1F246, 0x53F50 calls 0x2196C and
    -- clears FFFC22 bit 2; no group-item addition is present in this handler.
    StoryFlags.set(E.StoryFlag.SERPENT_SCALE_TAKEN)
    game.state.group_item_flag_bit_2 = false
    return "field_return"
end

return SpiritEyes
