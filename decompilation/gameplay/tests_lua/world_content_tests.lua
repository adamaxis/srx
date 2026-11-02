package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local World = require("world_content")
local E = require("enums")

game = {
    modules = {
        world_content_data = {
            world_activity_actors = {
                {
                    id = "field_activity_scene_13_ilene_gate_script_actor_0",
                    scene = 13,
                    activity_id = "scene_13_ilene_gate_script",
                    activity_kind = "scene_script",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
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
                    id = "field_activity_scene_47_strike_team_actor_0",
                    scene = 47,
                    activity_id = "scene_47_strike_team",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
                    actor_name = "Strike Team",
                },
                {
                    id = "field_activity_scene_20_ilene_activity_actor_0",
                    scene = 20,
                    activity_id = "scene_20_ilene_activity",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "inactive",
                    pending_world_effect_requirement = "pending",
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

local actors = World.world_activity_actors_for_activity_id(
    "scene_13_ilene_gate_script", 13)
assert(#actors == 1, "selector lookup returned the wrong actor count")
assert(actors[1].id ==
       "field_activity_scene_13_ilene_gate_script_actor_0" and
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
local pending = World.world_activity_actors_for_activity_id(
    "scene_20_ilene_activity", 20)
assert(#pending == 1 and
       pending[1].id ==
           "field_activity_scene_20_ilene_activity_actor_0" and
       pending[1].activity_kind == "event_driven" and
       pending[1].name == "lone11" and
       pending[1].visual_profile_id == E.ActorVisualProfile.LONE_STAR_MAGE and
       pending[1].x == 0x0E80 and pending[1].y == 0x0700 and
       pending[1].heading == 0x8000 and
       pending[1].ordered_overrides == nil and
       pending[1].actor_facts.roles ~= nil,
       "activity-id lookup did not preserve the pending Ilene actor")
assert(#World.world_activity_actors_for_activity_id(
           "scene_47_strike_team", 20) == 0,
       "activity-id lookup crossed scene boundaries")
assert(#World.world_activity_actors_for_activity_id(
           "scene_13_ilene_gate_script") == 1,
       "semantic activity lookup unexpectedly requires a scene number")
assert(World.pending_world_activity_id_for_scene(20, false, true) ==
           "scene_20_ilene_activity" and
       World.pending_world_activity_id_for_scene(20, true, true) ==
           nil and
       World.pending_world_activity_id_for_scene(20, false, false) ==
           nil and
       World.pending_world_activity_id_for_scene(20, true, false) ==
           nil and
       World.pending_world_activity_id_for_scene(47, false, true) ==
           nil,
       "pending actor-list selector ignored its extracted marker gates")
assert(#World.world_activity_actors_for_activity_id(
           "scene_13_ilene_gate_script", "13") == 0,
       "activity lookup accepted a nonnumeric scene")

-- ROM 0x1DED8 scans live field actor records and requests the 0x1E514 escort
-- cleanup path [C]. Lua selects actor state through its semantic live flag.
local DialogueActions = require("dialogue_actions")
local E = require("enums")
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
assert(cleanup_registry:execute("actor.conclude_escort_relationship"))
assert(escorted_actor.pending_lifecycle_event ==
           E.ActorLifecycleEvent.ESCORT_CONCLUDED and
       slot_only_actor.pending_lifecycle_event == nil,
       "escort conclusion relied on a numeric record slot instead of actor facts")

-- ROM 0x17034/0x17236 is the confirmed Scene-10 default +0x9E bit-4 actor.
-- 0xECCC selects variant 4 in this route; 0x21174 uses signed FFFC12 to
-- select the 0xFE640 subrow (negative +0xC, >=4 +8, otherwise +0).
local generic_contact_action = World.field_actor_contact_task(
    E.Scene.FRIENDSHIP_RESTAURANT,
    "field_activity_scene_10_default_script_actor_0",
    "scene_10_default_script", E.FieldActorContactHandler.ORDINARY)
assert(generic_contact_action ==
           E.FieldActorContactAction.GENERIC_DIALOGUE,
       "ROM-confirmed Scene-10 bit-4 contact did not select generic dialogue")
assert(World.generic_field_contact_row_offset(-1) ==
           E.GenericFieldContact.ROW_OFFSET_NEGATIVE and
       World.generic_field_contact_row_offset(4) ==
           E.GenericFieldContact.ROW_OFFSET_HIGH and
       World.generic_field_contact_row_offset(0) ==
           E.GenericFieldContact.ROW_OFFSET_DEFAULT,
       "generic contact row did not follow the signed FFFC12 bands")
assert(World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_default_script",
           E.GenericFieldContact.VARIANT_STANDARD) ==
           E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL,
       "Scene-10 variant 4 did not resolve its confirmed neutral row")
assert(World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_default_script",
           E.GenericFieldContact.VARIANT_ALTERNATE) == nil and
       World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_event_6_script",
           E.GenericFieldContact.VARIANT_ALTERNATE) ==
               "scene_10_group_2_default" and
       World.field_actor_contact_task(
           E.Scene.FRIENDSHIP_RESTAURANT,
           "field_activity_scene_10_default_script_actor_0",
           "scene_10_default_script",
           E.FieldActorContactHandler.WORLD_EFFECT) ==
               E.FieldActorContactAction.START_ACTIVITY,
       "generic dialogue escaped its source-confirmed actor/row/handler")
local generic_text = require("data.field_contact_dialogue_text")
assert(generic_text[E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL] ==
           "<A huge female ork dressed in AmerIndian garb is here." ..
               "||She seems occupied with some fetishes.>",
       "Scene-10 neutral text no longer matches ROM 0x101E24")
local presented_contact_text, presented_continuation
game.location = {
    type = "field_map",
    id = E.GenericFieldContact.LOCATION_ID,
    event_id = E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL,
}
game.ui = {
    say_text = function(text, continuation)
        presented_contact_text = text
        presented_continuation = continuation
    end,
}
local GenericFieldContacts = require("field_generic_contacts")
assert(GenericFieldContacts.present() == "presented" and
       presented_contact_text == generic_text[
           E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL] and
       presented_continuation == false,
       "generic contact presentation did not open the selected Lua row")

print("world content tests passed")
