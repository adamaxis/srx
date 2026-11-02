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
    -- [C] Default Scene-10 list 0x17034 installs template 0x17236 at
    -- (0x0700,0x0500) with +0x9E bit 4. 0xECCC sets FFF0AE=4 on the normal
    -- Scene-10 path. The only response configured here is the ROM-confirmed
    -- neutral FFFC12 row: 0xFE680 -> 0xFF12C -> text 0x102424.
    scene_10_default_script = {
        scene_id = E.Scene.FRIENDSHIP_RESTAURANT,
        handler = E.FieldActorContactHandler.ORDINARY,
        action = E.FieldActorContactAction.GENERIC_DIALOGUE,
        generic_dialogue = {
            signed_selector = E.GenericFieldContact.SIGNED_SELECTOR_NEUTRAL,
            rows_by_variant = {
                [E.GenericFieldContact.VARIANT_STANDARD] = {
                    [E.GenericFieldContact.ROW_OFFSET_DEFAULT] =
                        E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL,
                },
            },
        },
    },
}
