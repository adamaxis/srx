-- world_links.lua + data/world_links_data.lua (generated from the ROM).
-- Emulator cases are GPGX runs from /tmp/s8540.state (doc 27 section 5).
local E = require("enums")
local W = require("world_links")
local D = W.data()

local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

-- 1. Every link record decodes; event-table scenes round-trip to a loadable destination.
local links, routed = 0, 0
for scene = 0, 55 do
    local s = D.scenes[scene]
    check(s ~= nil, "scene " .. scene .. " present")
    for _, link in ipairs(W.links_in_scene(scene)) do
        links = links + 1
        local c = W.decode_code(link.code)
        check(c.link, "link bit " .. string.format("%06X", link.record))
        check(c.row >= 0 and c.row <= 3 and c.event >= 0 and c.event <= 31, "decode range")
        check(link.x0 <= link.x1 and link.y0 <= link.y1, "rect order")
        check(W.link_at(scene, link.x0 << 4, link.y0 << 4) ~= nil, "link_at finds a record")
        local dest = W.event_destination(scene, c.event)
        if s.event_table then
            check(dest ~= nil, "event table byte")
            if dest < 0x38 then
                routed = routed + 1
                check(W.scene_mode(dest) ~= nil and D.scenes[dest] ~= nil, "destination loads " .. dest)
            end
        else
            check(dest == nil, "no table -> no destination")
        end
    end
end
check(links == 892, "892 link records (profile_field_map_triggers.py census), got " .. links)
check(routed > 0, "some doors route")
-- Only scenes 0..9 carry an event table (doc 19 section 2).
for scene = 0, 55 do
    check((D.scenes[scene].event_table ~= nil) == (scene <= 9), "event table presence " .. scene)
end
check(W.event_destination(2, 4) == 0x0D, "Redmond event 4 -> Jump House 0x0D")
check(W.event_destination(0, 6) == 0x0A, "Downtown event 6 -> 0x0A")

-- 2. 0xBD34 return offsets and the door contact [C ROM + emulator].
check(D.return_offsets[E.DoorRow.DOWN][1].dy == 0x140 and D.return_offsets[E.DoorRow.UP][1].dy == -0x140, "row lead dy")
check(D.return_offsets[E.DoorRow.RIGHT][1].dx == 0x1E0 and D.return_offsets[E.DoorRow.LEFT][1].dx == -0x1E0, "row lead dx")
local function contact(lx, ly, code)
    return W.door_contact{ scene_mode = E.SceneMode.NONE, lead = { x = lx, y = ly },
        camera = { x = 1, y = 2 }, probe_codes = { 0x7F, code, code } }
end
local cases = {
    -- name, pre-step lead, door code, expected party (emulator FFDF44..4E)
    { "motel run5", 15584, 10890, 0xC2, { { 15584, 11210 }, { 15904, 11210 }, { 15264, 11210 } } },
    { "Little Chiba", 28424, 9350, 0xC1, { { 28424, 9670 }, { 28744, 9670 }, { 28104, 9670 } } },
    { "Jump House", 7344, 2680, 0xC4, { { 7344, 3000 }, { 7664, 3000 }, { 7024, 3000 } } },
}
for _, c in ipairs(cases) do
    local r = contact(c[2], c[3], c[4])
    check(r.saved and r.row == E.DoorRow.DOWN, c[1] .. " row")
    for i = 1, 3 do
        check(r.party[i].x == c[5][i][1] and r.party[i].y == c[5][i][2], c[1] .. " runner " .. i)
    end
    check(r.camera.x == 1 and r.camera.y == 2, c[1] .. " camera FFDF50")
    check(r.contact_heading == E.Heading.DOWN, c[1] .. " +0xCC")
end
local inside = W.door_contact{ scene_mode = E.SceneMode.STREET, lead = { x = 1, y = 1 }, camera = { x = 0, y = 0 }, probe_codes = { 0x80, 0x80, 0x80 } }
check(not inside.saved and inside.contact_heading == E.Heading.UP, "inside a building: +0xCC only, FFDF44 kept")
check(W.probe_row({ 0x7F, 0xA0, 0xC0 }) == E.DoorRow.RIGHT, "first LINK probe wins")
check(W.probe_row({ 0x1F, 0x1F, 0x5F }) == 2, "no LINK probe: last probe")

-- 3. Return headings.
local point = contact(15584, 10890, 0xC2)
check(W.return_placement(point, E.LinkReturn.SERVICE, E.Heading.UP).heading == E.Heading.DOWN, "0x9B8 service heading")
check(W.return_placement(point, E.LinkReturn.INTERIOR, E.Heading.LEFT).heading == E.Heading.LEFT, "0x2F32 keeps heading")
check(not pcall(W.return_placement, point, 99, 0), "unknown kind errors")
local flat = W.door_contact_flat(0, 15584, 10890, 5, 6, 0xC2, 0xC2, 0xC2)
check(#flat == 11 and flat[1] == 1 and flat[4] == 15584 and flat[5] == 11210 and flat[10] == 5, "flat adapter")
check(#W.door_contact_flat(2, 1, 1, 0, 0, 0x80, 0x80, 0x80) == 3, "flat adapter, not saved")

-- 4. Cave graph (doc 19 section 3).
local c = W.cave_exit(0, 0)
check(c.room == 5 and c.scene == 0x2D and c.x == 0x5E00 and c.y == 0x2280, "room 0 dir 0")
local edges = { {0,5},{0,6},{0,7},{1,7},{1,11},{2,4},{2,5},{2,9},{2,10},{3,13},{3,15},{3,16},{4,5},{7,8},
    {10,11},{11,12},{12,13},{14,15},{16,17},{16,18},{18,21},{19,20},{20,21},{21,23},{22,23} }
local G = D.room_graph
local function linked(a, b) for dir = 0, 3 do if G.next[a][dir] == b then return true end end return false end
for _, e in ipairs(edges) do check(linked(e[1], e[2]) and linked(e[2], e[1]), "edge " .. e[1] .. "-" .. e[2]) end
local seen, queue = { [0] = true }, { 0 }
while #queue > 0 do
    local r = table.remove(queue)
    for _, e in ipairs(edges) do
        for k = 1, 2 do
            local a, b = e[k], e[3 - k]
            if a == r and not seen[b] then seen[b] = true; queue[#queue + 1] = b end
        end
    end
end
local reached = 0; for _ in pairs(seen) do reached = reached + 1 end
check(reached == 24, "all 24 rooms connected")

print(string.format("world_links: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
