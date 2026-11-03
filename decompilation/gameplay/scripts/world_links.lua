-- Semantic field links and cave travel. The offline importer converts ROM
-- collision records into named sides, named events, and world-space bounds.
-- Runtime logic consumes those concepts directly.
local Data = require("data.world_links_data")
local E = require("enums")

local WorldLinks = {}

-- The source return branch (0xBC36/0xBD34 and 0x09B8/0x2FA4) is shared by
-- ordinary building doors and same-map services. Add an entry only when a
-- specific ROM exit has a documented exception; values are typed enums.
local EXIT_RETURN_EXCEPTIONS = {
    -- Scene 4/event 6 at 0x15BDB0 selects Scene 0. The route matrix's
    -- source/site audit has not established this side event as a doorway.
    ["council_medicine_lodge.rom_event_06"] = {
        rom_address = "0x15BDB0",
        return_anchor = E.FieldLocationReturnAnchor.NONE,
        return_facing = E.FieldLocationReturnFacing.PRESERVE_ENTRY,
        return_music_policy = E.FieldLocationMusicReturnPolicy.NONE,
    },
    -- Scene 18 record 9 reaches movement/mode handler 0xB7B8; its branch at
    -- 0xB88A stores 0x20990 as the same-map dialogue continuation.
    -- Dismissing it does not restore a building doorway.
    ["julius_strouther_booth.rom_event_30"] = {
        rom_address = "0x1D5E38",
        return_anchor = E.FieldLocationReturnAnchor.NONE,
        return_facing = E.FieldLocationReturnFacing.PRESERVE_ENTRY,
        return_music_policy = E.FieldLocationMusicReturnPolicy.NONE,
    },
}

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

function WorldLinks.building_service_exit_cases()
    local cases = {}
    for scene_id = 0, 55 do
        local scene = Data.scenes[scene_id]
        if scene ~= nil and scene.mode == "street" then
            for link_index, link in ipairs(scene.links) do
                local transition = link.transition
                local destination = transition.kind == "scene_change" and
                    Data.scenes[transition.destination_scene_id] or nil
                if transition.kind == "same_map_service" or
                   (destination ~= nil and
                    destination.mode == "building") then
                    cases[#cases + 1] = {
                        exit_id = string.format("scene_%02X_link_%02d",
                            scene_id, link_index),
                        scene_id = scene_id,
                        link_index = link_index,
                        return_side = link.return_side,
                        contact = {
                            x = math.floor(
                                (link.bounds.left + link.bounds.right) / 2),
                            y = math.floor(
                                (link.bounds.top + link.bounds.bottom) / 2),
                        },
                        transition_kind = transition.kind,
                        destination_scene_id = transition.destination_scene_id,
                        service = transition.service,
                    }
                end
            end
        end
    end
    return cases
end

local function validate_exit_return_policy(policy)
    assert(type(policy) == "table" and
           (policy.return_anchor ==
                E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD or
            policy.return_anchor == E.FieldLocationReturnAnchor.NONE),
        "world_links.exit_return: unsupported return anchor")
    assert(policy.return_facing ==
               E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY or
           policy.return_facing ==
               E.FieldLocationReturnFacing.PRESERVE_ENTRY,
        "world_links.exit_return: unsupported return facing")
    assert(policy.return_music_policy ==
               E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE or
           policy.return_music_policy == E.FieldLocationMusicReturnPolicy.NONE,
        "world_links.exit_return: unsupported return music policy")
    return policy
end

function WorldLinks.building_service_exit_policy(context)
    local scene_outcome = context.scene_outcome
    if scene_outcome ~= nil then
        assert(scene_outcome == E.FieldLocationSceneOutcome.SAME_SCENE or
               scene_outcome == E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN or
               scene_outcome == E.FieldLocationSceneOutcome.SCENE_CHANGE,
            "world_links.exit_return: unknown scene outcome " ..
                tostring(scene_outcome))
        if scene_outcome == E.FieldLocationSceneOutcome.SAME_SCENE then
            -- ROM 0xB34 calls the prompt callback, clears it at 0xB52, then
            -- resumes the live field redraw at 0x2BD2. Only a true scene
            -- entry runs 0x2F32/0x2FA4 and restores a saved doorway point.
            return {
                return_anchor = E.FieldLocationReturnAnchor.NONE,
                return_facing = E.FieldLocationReturnFacing.PRESERVE_ENTRY,
                return_music_policy = E.FieldLocationMusicReturnPolicy.NONE,
            }
        end
        -- A source service may refresh the same map and then restore its
        -- recorded doorway. Stoker's mode-2 route does this at 0x1F65A;
        -- 0xBC36/0xBD4C select the probe and 0x09B8 restores the bearing.
        -- Keep this distinct from a same-scene dialogue continuation.
    end
    local exception = EXIT_RETURN_EXCEPTIONS[context.exit_id]
    if exception == nil then
        return {
            return_anchor =
                E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
            return_facing =
                E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
            return_music_policy =
                E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
        }
    end
    assert(type(exception.rom_address) == "string" and
           exception.rom_address:match("^0x[0-9A-Fa-f]+$") ~= nil,
        "world_links.exit_return_exceptions: exception needs a ROM address")
    local policy = validate_exit_return_policy(exception)
    return {
        return_anchor = policy.return_anchor,
        return_facing = policy.return_facing,
        return_music_policy = policy.return_music_policy,
        rom_address = exception.rom_address,
    }
end

function WorldLinks.building_service_exit_exceptions()
    local result = {}
    for exit_id, exception in pairs(EXIT_RETURN_EXCEPTIONS) do
        result[#result + 1] = {
            exit_id = exit_id,
            rom_address = exception.rom_address,
            return_anchor = exception.return_anchor,
            return_facing = exception.return_facing,
            return_music_policy = exception.return_music_policy,
        }
    end
    table.sort(result, function(left, right)
        return left.exit_id < right.exit_id
    end)
    return result
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

function WorldLinks.exit_bearing(context)
    local facing_policy = context.facing_policy
    local entry_bearing = context.entry_bearing_degrees % 360
    if facing_policy == E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY then
        return { bearing_degrees = (entry_bearing + 180) % 360 }
    elseif facing_policy == E.FieldLocationReturnFacing.PRESERVE_ENTRY then
        return { bearing_degrees = entry_bearing }
    end
    error("world_links.exit_bearing: unknown facing policy " ..
          tostring(facing_policy))
end

-- One return path handles building doors and services. The host supplies the
-- saved ROM probe triplet; Lua applies the shared policy and resolves its
-- outward bearing through exit_bearing.
function WorldLinks.building_service_exit_return(context)
    local policy = validate_exit_return_policy{
        return_anchor = context.return_anchor or
            E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
        return_facing = context.return_facing or
            E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
        return_music_policy = context.return_music_policy or
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    }
    local point = context.point
    assert(type(point) == "table" and type(point.party) == "table" and
           type(point.camera) == "table",
        "world_links.building_service_exit_return: missing saved point")
    local bearing = WorldLinks.exit_bearing{
        facing_policy = policy.return_facing,
        entry_bearing_degrees = context.entry_bearing_degrees,
    }
    return {
        party = point.party,
        camera = point.camera,
        bearing_degrees = bearing.bearing_degrees,
        return_anchor = policy.return_anchor,
        return_facing = policy.return_facing,
        return_music_policy = policy.return_music_policy,
        rom_address = context.exception_rom_address,
    }
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
