-- ROM scene-2/event-0 indexed service 19 (0x543A2) prepares the Ilene
-- actor handoff before the event-table transition to scene 20. The service
-- only arms FFE182; it does not consume or replace the selected route.
local E = require("enums")
local StoryFlags = require("story_flags")

local Ilene = {}
local LOCATION_ID = "hollywood_correctional_facility"

local ACTIVITY_TEXT = table.concat({
    "Ilene gracefully leaps into a cat-like fighting stance as you enter.",
    "You briefly explain your involvement with the Sinsearach and your purpose for being here.",
    "She smiles and reaches under her cot, pulling out a crude, but workable homemade pistol.",
}, "||")

function Ilene.prepare_scene_20_entry()
    if game.location.type ~= "building" or
       game.location.id ~= LOCATION_ID or
       game.field.mode ~= 2 or game.field.event_code ~= 0 or
       game.field.alternate_result ~= 0 then
        return "continue"
    end

    if StoryFlags.is_set(18) ~= true or
       StoryFlags.is_set(E.StoryFlag.ILENE_MET) ~= false or
       game.state.field_mode_effect_flag_bit_0 ~= false or
       game.state.notebook_tip_flag_bit_12 ~= true then
        return "continue"
    end

    -- FFE182 is a transient pending-actor-list marker, distinct from the
    -- persistent FFFC34 field-mode-effect flags exposed above.
    game.state.pending_world_effect = true
    return "continue"
end

-- ROM 0x54402 applies these state changes before displaying group 0x115ECC.
-- The field host reaches this task only after the ROM-backed +0x9E/+0x10
-- actor-pair gate consumes the pending FFE182 marker.
function Ilene.complete_scene_20_activity()
    if game.location.type ~= "field_map" or
       game.location.id ~= "scene_20_ilene" then
        return "skip"
    end

    local reward = game.services.call("party", "award_karma_2")
    if reward ~= "applied" then
        return "reward_unavailable"
    end

    StoryFlags.set(E.StoryFlag.ILENE_MET)
    game.state.field_mode_flag_bit_31 = false
    game.state.field_mode_effect_flag_bit_0 = false
    game.state.field_mode_effect_flag_bit_1 = true
    game.state.field_auxiliary_flag_bit_12 = false
    game.state.field_auxiliary_flag_bit_13 = false

    game.ui.say_text(ACTIVITY_TEXT, false)
    return "completed"
end

return Ilene
