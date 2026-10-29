-- Semantic contact actions for selected field activities.
-- [C] The source lists at 0xA8AE and 0xA8CC use the special branches in
-- 0xECCC. Their stable content IDs keep those addresses out of Lua policy.
return {
    scene_34_story_contact = {
        handler = "world_effect",
        action = "aragorn_contact",
        mark_escort = true,
    },
    scene_20_ilene_pending_list = {
        handler = "pending_world_effect",
        action = "ilene_activity",
        mark_escort = true,
    },
}
