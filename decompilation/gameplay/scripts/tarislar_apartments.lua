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
    local outcome = DialogueGraph.run(
        "tarislar_garden_apartments.repeat_service", {
            ["tarislar_apartments.mark_post_ito_briefed"] = function()
                -- ROM 0x5451A selects command 0x40; handler 0x21F32 marks
                -- FFFC1E bit 28 before the repeat-service choices appear.
                StoryFlags.mark_fixed(E.StoryFlag.POST_ITO_BRIEFED)
                return "applied"
            end,
            ["tarislar_apartments.present_map_information"] = function()
                return HarlequinDialogue.present_map_information()
            end,
        })
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    end
    return outcome
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
    -- The JSON graph branches to text groups 0x15FF28 / 0x15FE5D and
    -- 0x15FF20 / 0x15FDB8. Both one-page routes suppress scene 13.
    local outcome = DialogueGraph.run(
        "tarislar_garden_apartments.signed_alternate_result")
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    end
    return outcome
end

function TarislarApartments.present_tomb_antechamber_arrival()
    return DialogueGraph.run("tarislar_tomb.antechamber_arrival")
end

function TarislarApartments.present_tomb_departure()
    -- The graph owns the story gate and prompt/page sequence. Its typed
    -- enter_field_scene outcome is translated to the entity task's stable
    -- continuation result so the ordinary area-event route service performs
    -- the actual scene handoff after the last page.
    local outcome = DialogueGraph.run(
        "tarislar_garden_apartments.tomb_departure")
    if type(outcome) == "table" and
       outcome.type == "continue_in_place" then
        return "left"
    elseif type(outcome) == "table" and
           outcome.type == "enter_field_scene" and
           outcome.sceneId == 54 and outcome.arrivalId == "rom_default" then
        return "enter_scene_54"
    end
    return outcome
end

return TarislarApartments
