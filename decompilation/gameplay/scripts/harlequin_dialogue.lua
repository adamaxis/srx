-- Dialogue graph shared by the Tarislar Apartments and the Red Buffalo
-- no-gift continuation. The calling location supplies its ROM-backed speaker
-- and menu-label profile; story and quest facts remain authoritative in Lua.
local E = require("enums")
local StoryFlags = require("story_flags")

local HarlequinDialogue = {}

local function more(prompt_id, text)
    return game.ui.prompt_text(prompt_id, text, { "more" }) == "more"
end

local function choose(prompt_id, text, choices, expected)
    return game.ui.prompt_text(prompt_id, text, choices) == expected
end

function HarlequinDialogue.present_map_information()
    -- ROM command 0x48 selects F5990 or F59FC from FFFC22 bit 7. The next
    -- command path applies map-completion bits 26 and 27 as the response is
    -- accepted.
    if game.state.group_item_flag_bit_8 ~= true then
        game.state.field_mode_effect_flag_bit_8 = true
        game.ui.say_text(table.concat({
            "Thon turned on Renraku when he thought he no longer needed them.",
            "I believe his actions were a bit premature; I still feel him growing weaker.",
            "However, I suggest a run against Renraku to find out exactly what they do have.",
        }, "||"), false)
        return "left"
    end

    StoryFlags.mark_fixed(E.StoryFlag.RENRAKU_MAP_SHOWN)
    local map_response
    if game.state.group_item_flag_bit_7 == true then
        -- Both group-item map bits send command 0x46 directly to 0xF59FC.
        StoryFlags.mark_fixed(E.StoryFlag.BOTH_MAPS_SHOWN)
        map_response = "<You show Harlequin the map you retrieved from " ..
                       "Renraku and the map you took from Vigore and Jarl.>"
    else
        map_response = "<You show Harlequin the map you retrieved from Renraku.>"
    end
    if not more("renraku_map", map_response) then
        return "left"
    end

    if game.state.group_item_flag_bit_7 ~= true then
        if not more("renraku_map_meaning", table.concat({
            "Hmmmm.  This shows the location of Ellisia's tomb, but the reference points have been removed.",
            "This map and its counterpart must be what Thon is after.",
        }, "||")) then
            return "left"
        end
        -- Command 0x47 (0x22072) has no second map: set FFFC34 bit 12,
        -- mark story bit 26, and continue with the Vigore lead.
        game.state.field_mode_effect_flag_bit_12 = true
        StoryFlags.mark_fixed(E.StoryFlag.RENRAKU_MAP_SHOWN)
        if not more("vigore_lead", table.concat({
            "Thon killed your brother.",
            "If you really want the revenge you've been after, you must thwart his plans.",
            "Earlier, I mentioned Vigore.  Have you heard of him?",
        }, "||")) then
            return "left"
        end
        if not more("vigore_report", table.concat({
            "<You tell Harlequin what you know of Vigore and Jarl.",
            "Harlequin frowns when he hears of your contact with them.>",
        }, "||")) then
            return "left"
        end
        if not more("vigore_explanation", table.concat({
            "Vigore is Thon's most trusted lieutenant and the front man for Thon's operations.",
            "Jarl is less well known, but just as important to the spirit.",
            "Both of them have been using you for information.",
        }, "||")) then
            return "left"
        end
        game.ui.say_text(table.concat({
            "Seek out Vigore and Jarl; they are the weak link in the chain.",
            "See if you can break it!",
        }, "||"), false)
        return "left"
    end

    if not more("both_maps_meaning", table.concat({
        "Impressive!  This shows the exact location of Ellisia's tomb, if one is familiar with these landmarks.",
        "If Vigore had these maps then Thon must be very close to finding the tomb!",
    }, "||")) then
        return "left"
    end
    if not more("tomb_artifacts", table.concat({
        "There is no telling what artifacts are in that tomb.",
        "I will not stand idle and let that mockery desecrate the final resting place of such a noble warrior!!.",
    }, "||")) then
        return "left"
    end
    if not more("harlequin_resolves", table.concat({
        "<Harlequin is half standing, his knuckles bone-white where he is clenching the chair.",
        "Suddenly, his animated features go slack as he slowly sinks back into his seat.",
        "He speaks with quiet determination.>",
    }, "||")) then
        return "left"
    end
    game.state.field_mode_effect_flag_bit_14 = true
    StoryFlags.mark_fixed(E.StoryFlag.BOTH_MAPS_SHOWN)
    game.ui.say_text(table.concat({
        "As we sit here, Thon may already be at Ellisia's tomb.",
        "Joshua, you must go now to assemble a team to destroy Thon.",
        "We now have very little time, so equip your team well and when you return here I will have arranged for your transport to the Tomb.",
    }, "||"), true)
    game.ui.say_text(table.concat({
        "<Harlequin fixes you with his piercing gaze.>",
        "I beseech you.  Thon must be stopped and an elven myth preserved.",
        "I'll prepare things on this end for your return.",
    }, "||"), false)
    return "left"
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
