-- Scene 1, event 7, indexed title service 21 (ROM 0x5447E).
-- This script implements the first authorized conversation, the repeat
-- information service, and the later gated tomb prompt. The field event does
-- not enter scene 54 until the tomb prompt's travel pages complete.
local E = require("enums")
local StoryFlags = require("story_flags")
local HarlequinDialogue = require("harlequin_dialogue")
local DialogueGraph = require("dialogue_graph")

local TarislarApartments = {}

local LOCATION_ID = "puyallup_site_02_tarislar_garden_apartments"

local function at_apartments()
    return game.location.type == "building" and
           game.location.id == LOCATION_ID
end

local function more(prompt_id, text)
    return game.ui.prompt_text(prompt_id, text, { "more" }) == "more"
end

function TarislarApartments.present_first_entry()
    if not at_apartments() or
       StoryFlags.is_set(E.StoryFlag.FROSTY_MET) ~= true or
       StoryFlags.is_set(E.StoryFlag.GARDEN_APTS_ENTERED) == true then
        return "denied"
    end

    -- ROM 0x5456E marks trigger 21, awards +2 Karma to the active party,
    -- then starts prompt record 0xF4DF4. Dismissing its first page selects
    -- command 0x45 (0x21FF2), which sets FFFC34 bits 5 and 6.
    StoryFlags.begin_trigger(E.StoryFlag.GARDEN_APTS_ENTERED)
    StoryFlags.mark_active_trigger()
    if game.services.call("party", "award_karma_2") ~= "applied" then
        return "denied"
    end

    local outcome = DialogueGraph.run(
        "tarislar_garden_apartments.first_authorized_entry", {
            ["tarislar_apartments.mark_first_entry_arrival_effects"] =
                function()
                    -- ROM command 0x45 at 0x21FF2 sets FFFC34 bits 5 and 6.
                    game.state.field_mode_effect_flag_bit_5 = true
                    game.state.field_mode_effect_flag_bit_6 = true
                    return "applied"
                end,
            ["tarislar_apartments.present_harlequin_main_conversation"] =
                function()
                    return HarlequinDialogue.present_main_conversation()
                end,
        })
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    end
    return outcome
end

function TarislarApartments.present_repeat_service()
    if not at_apartments() or
       StoryFlags.is_set(E.StoryFlag.FROSTY_MET) ~= true or
       StoryFlags.is_set(E.StoryFlag.GARDEN_APTS_ENTERED) ~= true or
       game.field.alternate_result ~= 0 then
        return "denied"
    end

    -- ROM 0x544E8..0x5456A selects F4E20, F4E34, or F4E48 from the
    -- Ito/post-Ito/map bits. The question branch at F4E20 reaches F5A4C;
    -- the information command at F4E48 reaches 0x220CC, which chooses the
    -- Renraku-only or two-map response using FFFC22 bit 7.
    local choices = { "questions" }
    if StoryFlags.is_set(E.StoryFlag.ITO_FIGHT_WON) == true then
        if StoryFlags.is_set(E.StoryFlag.POST_ITO_BRIEFED) ~= true then
            -- 0x5451A sets story bit 28 before presenting F4E34.
            StoryFlags.mark_fixed(E.StoryFlag.POST_ITO_BRIEFED)
            choices = { "ito_killed", "information" }
        else
            local renraku_map_shown =
                StoryFlags.is_set(E.StoryFlag.RENRAKU_MAP_SHOWN) == true
            local both_maps_shown =
                StoryFlags.is_set(E.StoryFlag.BOTH_MAPS_SHOWN) == true
            local information_available =
                (not renraku_map_shown and
                 game.state.group_item_flag_bit_8 == true) or
                (renraku_map_shown and not both_maps_shown and
                 game.state.group_item_flag_bit_7 == true)
            if information_available then
                choices = { "information" }
            end
        end
    end

    local choice = game.ui.prompt_text(
        "repeat_service",
        "Hello again, Joshua.||||You have bought my time with your pain.||||Spend it as you wish.",
        choices)
    if choice == "questions" then
        game.ui.say_text(table.concat({
            "I have no new information at this time.",
            "My resources are spread too thin, but that shouldn't concern you.",
            "For now, you are on your own.",
        }, "||"), false)
        return "left"
    end
    if choice == "ito_killed" then
        if not more("ito_response", table.concat({
            "You managed to have him killed?",
            "It seems I am not the only person to underestimate you, although it appears I'll be the only one around to correct my mistake.",
            "Well, with Ogami out of the way Renraku should not bother us any longer.",
        }, "||")) then
            return "left"
        end
        return HarlequinDialogue.present_map_information()
    end
    if choice == "information" then
        return HarlequinDialogue.present_map_information()
    end
    return "left"
end

function TarislarApartments.present_guarded_entry()
    local outcome = DialogueGraph.run(
        "tarislar_garden_apartments.guarded_entry", {
            ["tarislar_apartments.present_harlequin_main_conversation"] =
                function()
                    return HarlequinDialogue.present_main_conversation()
                end,
        })
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        -- The entry task contract uses a non-matching result to suppress the
        -- scene-13 event-table transition after this same-map conversation.
        return "left"
    end
    return outcome
end

function TarislarApartments.present_alternate_result()
    if not at_apartments() or game.field.mode ~= 1 or
       game.field.event_code ~= 7 or game.field.alternate_result == 0 then
        return "denied"
    end

    -- ROM 0x5447E tests signed FFF0D5 before the Frosty and visit flags.
    -- Positive selects group 0x15FF28 / text 0x15FE5D; negative selects
    -- group 0x15FF20 / text 0x15FDB8. Both prompt chains are one page and
    -- leave D6 at -1, so the scene-1 event-table destination is suppressed.
    if game.field.alternate_result > 0 then
        game.ui.say_text(table.concat({
            "Heavily armed and armored elves train their weapons on you.",
            "After confirming you as the pick-up man, you are given a package and pushed back out into the street.",
        }, "||"), false)
    else
        game.ui.say_text(table.concat({
            "The metallic clacking of weapons being readied stops you in your tracks!",
            "A lone elf approaches and you hand him the package.",
            "He tosses you a grin and a credstick, and walks you to the door.",
        }, "||"), false)
    end
    return "left"
end

function TarislarApartments.present_tomb_departure()
    if not at_apartments() or
       StoryFlags.is_set(E.StoryFlag.FROSTY_MET) ~= true or
       StoryFlags.is_set(E.StoryFlag.GARDEN_APTS_ENTERED) ~= true or
       StoryFlags.is_set(E.StoryFlag.ITO_FIGHT_WON) ~= true or
       StoryFlags.is_set(E.StoryFlag.POST_ITO_BRIEFED) ~= true or
       StoryFlags.is_set(E.StoryFlag.RENRAKU_MAP_SHOWN) ~= true or
       StoryFlags.is_set(E.StoryFlag.BOTH_MAPS_SHOWN) ~= true then
        return "denied"
    end

    -- ROM root 0xF4E08 is reached only after the Harlequin, Ito, and map
    -- progress gates. Command 0xE4 enters 0x24954, which sets FFF0CC before
    -- returning to the trip sequence. The matching Lua task result lets the
    -- entity-owned link resume scene 54 through the ordinary route service.
    local choice = game.ui.prompt_text(
        "tomb_offer",
        "Are you prepared for the trip to Ellisia's tomb?",
        { "yes", "no" })
    if choice == "no" then
        if not more("tomb_declined", table.concat({
            "As we sit here, Thon may already be at Ellisia's tomb.",
            "Joshua, you must go now to assemble a team to destroy Thon.",
            "We now have very little time, so equip your team well and when you return here I will have arranged for your transport to the Tomb.",
        }, "||")) then
            return "left"
        end
        game.ui.say_text(table.concat({
            "<Harlequin fixes you with his piercing gaze.>",
            "I beseech you.  Thon must be stopped and an elven myth preserved.",
            "I'll prepare things on this end for your return.",
        }, "||"), false)
        return "left"
    end
    if choice ~= "yes" then return "left" end

    if not more("tomb_flight", table.concat({
        "<Harlequin takes you to a sleek VTOL craft whose body bristles with communications and jamming devices.",
        "A harried ride with Harlequin at the stick takes you to the entrance of Ellisia's tomb.>",
    }, "||")) then
        return "left"
    end
    if not more("tomb_walkie", table.concat({
        "<Harlequin gives you a hands-free walkie-talkie.>",
        "What happens in the tomb is up to you.  I'll take care of things out here.",
        "<Harlequin holds up a hand to stifle your questions and ushers you to the tomb.>",
    }, "||")) then
        return "left"
    end
    if not more("tomb_good_luck", table.concat({
        "Good luck, my friend.  May we meet again.",
        "<You take a deep breath of fresh air and a last look at the forest.",
        "Nodding to Harlequin, you turn and walk into the tomb's antechamber, noticing that the stone doors were literally ripped from the entrance...",
    }, "||")) then
        return "left"
    end
    return "enter_scene_54"
end

return TarislarApartments
