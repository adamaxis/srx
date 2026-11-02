-- Semantic contact actions for selected field activities.
-- [C] The source lists at 0xA8AE and 0xA8CC use the special branches in
-- 0xECCC. Their stable content IDs keep those addresses out of Lua policy.
local E = require("enums")

return {
    scene_34_story_contact = {
        handler = E.FieldActorContactHandler.WORLD_EFFECT,
        action = E.FieldActorContactAction.ARAGORN_CONTACT,
        mark_escort = true,
    },
    scene_20_ilene_activity = {
        handler = E.FieldActorContactHandler.PENDING_WORLD_EFFECT,
        action = E.FieldActorContactAction.ILENE_ACTIVITY,
        mark_escort = true,
    },
}
