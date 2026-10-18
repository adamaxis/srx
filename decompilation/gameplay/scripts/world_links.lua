-- World links: doors, building exits and the cave room graph.
-- Data comes from data/world_links_data.lua, generated from the ROM by
-- ghidra_conversion/tools/rom_research/extract_world_links.py. This module is
-- the only place that interprets it; the host only supplies positions and the
-- raw probe codes and applies the result. See docs 19 and 27.
--
-- ROM flow [C]:
--   contact 0xB7B8: >= 2 of the 4 probe bytes FFEF10..13 carry LINK (bit 7)
--     -> 0xBC36: row = bits 6:5 of the first LINK probe of FFEF10..12;
--        lead +0xCC = row << 14 (always); when FFDF40 == 0 (outdoor field)
--        FFDF44..4E = lead pre-step position + 0xBD34[row], FFDF50/52 = camera.
--   the door auto-walk faces +0xCC + 0x8000 (0xACB8), i.e. into the door.
--   leave a same-map service (hotel, hospital, shop; d6 != 0, 0x9B8):
--     party at FFDF44.., heading and target heading 0x8000 for all runners.
--   leave an interior scene (FFDF42 -> 0x2F32): scene block restored from
--     FFDE26, camera = FFDF50/52, party at FFDF44..; heading is NOT written,
--     the party keeps the heading it walked out of the interior with.
local E = require("enums")
local Data = require("data.world_links_data")

local WorldLinks = {}

local RUNNERS = 3

function WorldLinks.data() return Data end

-- Collision record code byte -> { link, row, event }.
function WorldLinks.decode_code(code)
    local C = E.LinkCode
    return {
        link = (code & C.LINK) ~= 0,
        row = (code >> C.ROW_SHIFT) & C.ROW_MASK,
        event = code & C.EVENT_MASK,
    }
end

function WorldLinks.links_in_scene(scene_id)
    local scene = Data.scenes[scene_id]
    return scene and scene.links or {}
end

-- Event-table destination for (scene, event): 0x167AE reads
-- byte [profile+0x4E][FFFBF9]. nil when the scene has no table.
function WorldLinks.event_destination(scene_id, event)
    local scene = Data.scenes[scene_id]
    if not scene or not scene.event_table then return nil end
    return scene.event_table[event]
end

-- 0x167AE scene mode of a destination (FFDF40).
function WorldLinks.scene_mode(dest)
    if dest < 0x15 then return E.SceneMode.STREET end
    if dest < 0x2C then return E.SceneMode.BUILDING end
    if dest < 0x38 then return E.SceneMode.CAVE end
    return nil   -- 0x167AE halts (stop #$2700)
end

-- Link record whose inclusive pixel rect holds the world position.
function WorldLinks.link_at(scene_id, world_x, world_y)
    local px, py = world_x >> 4, world_y >> 4
    for _, link in ipairs(WorldLinks.links_in_scene(scene_id)) do
        if px >= link.x0 and px <= link.x1 and py >= link.y0 and py <= link.y1 then
            return link
        end
    end
    return nil
end

-- 0xBC36/0xBC52: the first probe byte (of up to three) with LINK set
-- selects the row; when none has it the last one is used.
function WorldLinks.probe_row(probe_codes)
    local selected = probe_codes[#probe_codes]
    for _, code in ipairs(probe_codes) do
        if (code & E.LinkCode.LINK) ~= 0 then selected = code; break end
    end
    return WorldLinks.decode_code(selected).row
end

-- 0xB838 ror.w #2: lead +0xCC.
function WorldLinks.row_heading(row) return (row << 14) & 0xFFFF end

-- Door contact. ctx = { scene_mode, lead = {x, y}, camera = {x, y},
-- probe_codes = {c0, c1, c2} }. Returns { row, contact_heading, saved,
-- party = { {x, y} x3 }, camera = {x, y} }; saved is false away from the
-- outdoor field (FFDF40 ~= 0), where the previous return point is kept.
function WorldLinks.door_contact(ctx)
    local row = WorldLinks.probe_row(ctx.probe_codes)
    local result = { row = row, contact_heading = WorldLinks.row_heading(row), saved = false }
    if ctx.scene_mode ~= E.SceneMode.NONE then return result end
    result.saved = true
    result.party = {}
    for i = 1, RUNNERS do
        local off = Data.return_offsets[row][i]
        result.party[i] = { x = (ctx.lead.x + off.dx) & 0xFFFF, y = (ctx.lead.y + off.dy) & 0xFFFF }
    end
    result.camera = { x = ctx.camera.x, y = ctx.camera.y }
    return result
end

-- Leaving a building. point = the saved door_contact result; kind is
-- E.LinkReturn; current_heading is the lead's heading at the exit (kept by
-- the interior path). Returns { party, heading, camera }.
function WorldLinks.return_placement(point, kind, current_heading)
    local heading
    if kind == E.LinkReturn.SERVICE then
        heading = Data.service_return_heading
    elseif kind == E.LinkReturn.INTERIOR then
        heading = current_heading
    else
        error("world_links.return_placement: unknown return kind " .. tostring(kind))
    end
    return { party = point.party, heading = heading, camera = point.camera }
end

-- Cave/wilderness graph (0x2CE0): leaving `room` by exit dir 0..3
-- (zone code 8..11). Returns { room, scene, x, y }.
function WorldLinks.cave_exit(room, dir)
    local G = Data.room_graph
    local next_room = G.next[room][dir]
    local slot = G.arrival_slot[room][dir]
    local scene = G.scene[next_room]
    local row = scene < 0x2C and scene - 6 or scene - 0x2C + 4
    local xy = G.arrival_xy[row][slot]
    return { room = next_room, scene = scene, x = xy.x, y = xy.y }
end

-- Host adapters: flat integer arrays across the C++ boundary.
-- door_contact_flat(scene_mode, lead_x, lead_y, camera_x, camera_y, p0, p1, p2)
--   -> { saved(0/1), row, contact_heading, x1, y1, x2, y2, x3, y3, cam_x, cam_y }
function WorldLinks.door_contact_flat(scene_mode, lx, ly, cx, cy, p0, p1, p2)
    local r = WorldLinks.door_contact{ scene_mode = scene_mode, lead = { x = lx, y = ly },
        camera = { x = cx, y = cy }, probe_codes = { p0, p1, p2 } }
    if not r.saved then return { 0, r.row, r.contact_heading } end
    local p = r.party
    return { 1, r.row, r.contact_heading, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y,
             r.camera.x, r.camera.y }
end

-- return_heading_flat(kind, current_heading) -> { heading }
function WorldLinks.return_heading_flat(kind, current_heading)
    local placement = WorldLinks.return_placement({}, kind, current_heading)
    return { placement.heading }
end

return WorldLinks
