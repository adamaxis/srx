-- Semantic field links and cave travel. The offline importer converts ROM
-- collision records into named sides, named events, and world-space bounds.
-- Runtime logic consumes those concepts directly.
local Data = require("data.world_links_data")

local WorldLinks = {}

function WorldLinks.data() return Data end

function WorldLinks.links_in_scene(scene_id)
    local scene = Data.scenes[scene_id]
    return scene and scene.links or {}
end

function WorldLinks.scene_mode(scene_id)
    local scene = Data.scenes[scene_id]
    return scene and scene.mode or nil
end

-- Link bounds and actor positions are already in the same world-space units.
function WorldLinks.link_at(scene_id, world_x, world_y)
    for _, link in ipairs(WorldLinks.links_in_scene(scene_id)) do
        local bounds = link.bounds
        if world_x >= bounds.left and world_x <= bounds.right and
           world_y >= bounds.top and world_y <= bounds.bottom then
            return link
        end
    end
    return nil
end

-- Resolve a doorway directly to its gameplay transition. No event selector
-- or lookup-table index crosses into script logic.
function WorldLinks.transition_at(scene_id, world_x, world_y)
    local link = WorldLinks.link_at(scene_id, world_x, world_y)
    return link and link.transition or nil
end

-- `return_side` is one of north/east/south/west. The host has already
-- translated its collision result into this named doorway fact.
function WorldLinks.door_contact(context)
    local formation = Data.return_offsets[context.return_side]
    if formation == nil then
        error("world_links.door_contact: unknown return side " ..
              tostring(context.return_side))
    end
    if context.outdoor_field ~= true then
        return { saved = false }
    end

    local party = {}
    for index, offset in ipairs(formation) do
        party[index] = {
            x = context.lead.x + offset.x,
            y = context.lead.y + offset.y,
        }
    end
    return {
        saved = true,
        party = party,
        camera = { x = context.camera.x, y = context.camera.y },
    }
end

-- Return behavior is stated as a gameplay path and compass facing.
function WorldLinks.return_placement(point, kind, current_facing)
    local facing
    if kind == "same_map_service" then
        facing = "south"
    elseif kind == "building_interior" then
        facing = current_facing
    else
        error("world_links.return_placement: unknown return kind " ..
              tostring(kind))
    end
    return { party = point.party, facing = facing, camera = point.camera }
end

function WorldLinks.exit_bearing(context)
    local facing_policy = context.facing_policy
    local entry_bearing = context.entry_bearing_degrees % 360
    if facing_policy == "face_away_from_entry" then
        return { bearing_degrees = (entry_bearing + 180) % 360 }
    elseif facing_policy == "preserve_entry" then
        return { bearing_degrees = entry_bearing }
    end
    error("world_links.exit_bearing: unknown facing policy " ..
          tostring(facing_policy))
end

function WorldLinks.service_at(scene_id, world_x, world_y)
    local transition = WorldLinks.transition_at(scene_id, world_x, world_y)
    if transition and transition.kind == "same_map_service" then
        return { kind = transition.service }
    end
    return nil
end

-- A cave room contains fully resolved exits, including their destination and
-- arrival position. No ROM zone code or table index crosses the script API.
function WorldLinks.cave_exit(context)
    local source = Data.cave_rooms[context.room]
    if not source then return nil end
    return source.exits[context.direction]
end

local entry_rooms = { [6] = 0, [9] = 3 }

function WorldLinks.entry_room(scene_id)
    return entry_rooms[scene_id]
end

return WorldLinks
