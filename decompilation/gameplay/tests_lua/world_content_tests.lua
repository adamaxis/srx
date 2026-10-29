package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local World = require("world_content")
local E = require("enums")

game = {
    modules = {
        world_content_data = {
            world_activity_actor_lists = {
                {
                    scene = 13,
                    list_id = "scene_13_ilene_gate_script",
                    activity_kind = "scene_script",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
                    source_order = 0,
                    actor_name = "lone11",
                    visual_profile_id = E.ActorVisualProfile.LONE_STAR_MAGE,
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x1100,
                    world_y = 0x0300,
                    initial_heading = 0xC000,
                    action_kind = "item",
                    action_id = "american_l36",
                    ordered_overrides = "0x2E=0xC0,0x9E=0x10",
                    actor_facts = { roles = {}, interaction = {} },
                },
                {
                    scene = 47,
                    list_id = "scene_47_strike_team",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
                    source_order = 0,
                    actor_name = "Strike Team",
                },
                {
                    scene = 20,
                    list_id = "scene_20_ilene_pending_list",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "inactive",
                    pending_world_effect_requirement = "pending",
                    source_order = 0,
                    actor_name = "lone11",
                    visual_profile_id = E.ActorVisualProfile.LONE_STAR_MAGE,
                    profile_id = "lone11_k0a_item03",
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x0E80,
                    world_y = 0x0700,
                    initial_heading = 0x8000,
                    action_kind = "item",
                    action_id = "american_l36",
                    ordered_overrides = "0x9E=0x20,0x2E=0x80,0x9E=0x8",
                    actor_facts = { roles = {}, interaction = {} },
                },
            },
        },
    },
}

local actors = World.world_activity_actors_for_list_id(
    13, "scene_13_ilene_gate_script")
assert(#actors == 1, "selector lookup returned the wrong actor count")
assert(actors[1].id == "world_activity:13:scene_13_ilene_gate_script:0" and
       actors[1].name == "lone11" and
       actors[1].visual_profile_id == E.ActorVisualProfile.LONE_STAR_MAGE and
       actors[1].palette_line == 1 and
       actors[1].sprite == "actor-kind-10-palette-1" and
       actors[1].x == 0x1100 and actors[1].y == 0x0300 and
       actors[1].heading == 0xC000 and
       actors[1].action_id == "american_l36" and
       actors[1].activity_kind == "scene_script" and
       actors[1].ordered_overrides == nil and
       actors[1].actor_facts.roles ~= nil,
       "selector lookup lost the typed host actor facts")
local pending = World.world_activity_actors_for_list_id(
    20, "scene_20_ilene_pending_list")
assert(#pending == 1 and
       pending[1].id ==
           "world_activity:20:scene_20_ilene_pending_list:0" and
       pending[1].activity_kind == "event_driven" and
       pending[1].name == "lone11" and
       pending[1].visual_profile_id == E.ActorVisualProfile.LONE_STAR_MAGE and
       pending[1].x == 0x0E80 and pending[1].y == 0x0700 and
       pending[1].heading == 0x8000 and
       pending[1].ordered_overrides == nil and
       pending[1].actor_facts.roles ~= nil,
       "activity-id lookup did not preserve the pending Ilene actor")
assert(#World.world_activity_actors_for_list_id(
           20, "scene_47_strike_team") == 0,
       "activity-id lookup crossed scene boundaries")
assert(World.pending_world_activity_actor_list_for_scene(20, false, true) ==
           "scene_20_ilene_pending_list" and
       World.pending_world_activity_actor_list_for_scene(20, true, true) ==
           nil and
       World.pending_world_activity_actor_list_for_scene(20, false, false) ==
           nil and
       World.pending_world_activity_actor_list_for_scene(20, true, false) ==
           nil and
       World.pending_world_activity_actor_list_for_scene(47, false, true) ==
           nil,
       "pending actor-list selector ignored its extracted marker gates")
assert(#World.world_activity_actors_for_list_id("13", "scene_13_ilene_gate_script") == 0,
       "activity lookup accepted a nonnumeric scene")

-- ROM 0x1DED8 scans live field actor records and requests the 0x1E514 escort
-- cleanup path [C]. Lua selects actor state through its semantic live flag.
local DialogueActions = require("dialogue_actions")
local escorted_actor = {
    live_field_actor = true,
    actor_facts = {
        lifecycle = { active = true },
        behavior = { escort = true },
    },
}
local slot_only_actor = {
    source_record_slot = 7,
    live_field_actor = false,
    actor_facts = {
        lifecycle = { active = true },
        behavior = { escort = true },
    },
}
game.actors = {
    ["world-escort"] = escorted_actor,
    ["legacy-slot-only"] = slot_only_actor,
}
local cleanup_registry = DialogueActions.new_default_registry()
assert(cleanup_registry:execute("actor.cleanup_escort_records"))
assert(escorted_actor.source_record_cleanup_requested == true and
       slot_only_actor.source_record_cleanup_requested == nil,
       "escort cleanup relied on a numeric record slot instead of actor facts")

print("world content tests passed")
