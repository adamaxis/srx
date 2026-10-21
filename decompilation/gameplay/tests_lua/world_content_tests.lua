package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local World = require("world_content")

game = {
    modules = {
        world_content_data = {
            world_activity_actor_lists = {
                {
                    scene = 13,
                    script_index = 3,
                    source_order = 0,
                    actor_name = "lone11",
                    entity_kind = 10,
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x1100,
                    world_y = 0x0300,
                    initial_heading = 0xC000,
                    action_kind = "item",
                    action_code = 3,
                    ordered_overrides = "0x2E=0xC0,0x9E=0x10",
                    template_offset = 0x172CA,
                    list_offset = 0x1706C,
                },
                {
                    scene = 47,
                    script_index = -1,
                    source_order = 0,
                    list_offset = 0x1E5C4,
                    actor_name = "Strike Team",
                },
                {
                    scene = 20,
                    script_index = -1,
                    source_order = 0,
                    list_offset = 0xA8CC,
                    actor_name = "lone11",
                    entity_kind = 10,
                    profile_id = "lone11_k0a_item03",
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x0E80,
                    world_y = 0x0700,
                    initial_heading = 0x8000,
                    action_kind = "item",
                    action_code = 3,
                    ordered_overrides = "0x9E=0x20,0x2E=0x80,0x9E=0x8",
                    template_offset = 0x172CA,
                    position_offset = 0xA8E4,
                },
            },
        },
    },
}

local actors = World.world_activity_actors_for_script(13, 3)
assert(#actors == 1, "selector lookup returned the wrong actor count")
assert(actors[1].id == "world_activity:13:3:0" and
       actors[1].name == "lone11" and
       actors[1].entity_kind == 10 and actors[1].palette_line == 1 and
       actors[1].sprite == "actor-kind-10-palette-1" and
       actors[1].x == 0x1100 and actors[1].y == 0x0300 and
       actors[1].heading == 0xC000 and actors[1].action_code == 3 and
       actors[1].ordered_overrides == "0x2E=0xC0,0x9E=0x10",
       "selector lookup lost the typed host actor facts")
local pending = World.world_activity_actors_for_source_list(20, 0xA8CC)
assert(#pending == 1 and
       pending[1].id == "world_activity:20:list:43212:0" and
       pending[1].script_index == -1 and
       pending[1].name == "lone11" and pending[1].entity_kind == 10 and
       pending[1].x == 0x0E80 and pending[1].y == 0x0700 and
       pending[1].heading == 0x8000 and
       pending[1].ordered_overrides ==
           "0x9E=0x20,0x2E=0x80,0x9E=0x8",
       "source-list lookup did not preserve the pending Ilene actor")
assert(#World.world_activity_actors_for_source_list(20, 0x1E5C4) == 0,
       "source-list lookup crossed scene/list boundaries")
assert(#World.world_activity_actors_for_script(47, -1) == 0,
       "event-driven actor lists leaked into script selection")
assert(#World.world_activity_actors_for_script("13", 3) == 0,
       "selector lookup accepted a nonnumeric scene")

print("world content tests passed")
