-- Harlequin's Lua conversation is shared by the Tarislar Apartments and the
-- Red Buffalo no-gift continuation. Its map-information branch runs through a
-- shared JSON graph; the active location supplies presentation while Lua
-- remains authoritative for story and quest facts.
local E = require("enums")
local StoryFlags = require("story_flags")
local DialogueGraph = require("dialogue_graph")

local HarlequinDialogue = {}

local function more(prompt_id, text)
    return game.ui.prompt_text(prompt_id, text, { "more" }) == "more"
end

local function choose(prompt_id, text, choices, expected)
    return game.ui.prompt_text(prompt_id, text, choices) == expected
end

function HarlequinDialogue.present_map_information()
    -- The same ROM conversation serves both Scene 1 and Scene 6. The active
    -- location supplies its speaker and menu profile to the shared graph.
    local location = game.location
    assert(type(location.speaker_id) == "string" and
           type(location.text_profile) == "string",
        "Harlequin map dialogue needs its location presentation context")
    local outcome = DialogueGraph.run("harlequin.map_information", nil, {
        presentation = {
            speakerId = location.speaker_id,
            textProfile = location.text_profile,
        },
    })
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    end
    return outcome
end

function HarlequinDialogue.present_main_conversation()
    if not more("meeting", table.concat({
        "So we meet.",
        "I guess you would eventually have found me, given your resources and abilities.",
        "As you have probably guessed, I've been keeping an eye on you since you first arrived in Seattle.",
    }, "||")) then
        return "left"
    end

    if not choose("michael_topic", table.concat({
        "You've acquired quite a reputation in your short time here, yet your original mission remains unfulfilled.",
        "I will help you if I can.",
    }, "||"), { "ask_about_michael" }, "ask_about_michael") then
        return "left"
    end

    if not choose("thon_topic", table.concat({
        "Your brother's death is made worse by the fact that it was wholly avoidable.",
        "I underestimated Thon, and he punished me by destroying those in my employ.",
        "I apologize for Michael's death.",
    }, "||"), { "who_is_thon" }, "who_is_thon") then
        return "left"
    end

    if not more("thon_explanation", table.concat({
        "Thon is a what, not a who.",
        "It is a free spirit, an elemental who's summoner was killed.",
        "Thon is older than most and with age comes power, considerable power in Thon's case.",
    }, "||")) then
        return "left"
    end

    if not more("thon_curse", table.concat({
        "But Thon is cursed.",
        "He must replenish his life force or his form in this world will eventually cease to exist.",
        "Thon seeks out magical artifacts and, in a ritual of destruction, absorbs their energies!",
    }, "||")) then
        return "left"
    end

    if not choose("renraku_topic", table.concat({
        "Currently, Thon's energy is waning.",
        "But make no mistake!  He is still very powerful and quite dangerous.",
        "But to save on expenditure of his energies, he now uses much outside help and avoids direct confrontations.",
    }, "||"), { "why_renraku" }, "why_renraku") then
        return "left"
    end

    if not more("renraku_explanation", table.concat({
        "Renraku became involved with Thon through Vigore, who was casting about for corporate support.",
        "Renraku assigned Mako to the project, but he was easily blinded by the promise of power and became Thon's unwitting pawn.",
    }, "||")) then
        return "left"
    end

    local ito_won = StoryFlags.is_set(E.StoryFlag.ITO_FIGHT_WON) == true
    if ito_won then
        -- The 0x21F32 branch writes FFDD64=0x1C and marks bit 28, then
        -- presents prompt 0xF5968's Ito answer.
        StoryFlags.mark_fixed(E.StoryFlag.POST_ITO_BRIEFED)
    else
        game.state.field_mode_effect_flag_bit_7 = true
    end

    local renraku_update = table.concat({
        "After Thon fragged his relationship with Renraku, Mako was replaced.",
        "Ito Ogami is a legendary player of hardball.",
        "I've managed to keep him and Renraku off my trail so far.",
    }, "||")
    if not ito_won then
        -- FFFC34 bit 7 returns from prompt 0xF594C after this page.
        game.ui.say_text(renraku_update, false)
        return "left"
    end

    if not choose("ito_update", renraku_update,
                  { "ito_killed" }, "ito_killed") then
        return "left"
    end
    if not more("ito_response", table.concat({
        "You managed to have him killed?",
        "It seems I am not the only person to underestimate you, although it appears I'll be the only one around to correct my mistake.",
        "Well, with Ogami out of the way Renraku should not bother us any longer.",
    }, "||")) then
        return "left"
    end

    return HarlequinDialogue.present_map_information()
end

return HarlequinDialogue
