-- ROM scene-2/event-0 indexed service 19 (0x543A2) prepares the Ilene
-- actor handoff before the event-table transition to scene 20. The service
-- only arms FFE182; it does not consume or replace the selected route.
local E = require("enums")
local StoryFlags = require("story_flags")

local Ilene = {}
local LOCATION_ID = "hollywood_correctional_facility"

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

return Ilene
