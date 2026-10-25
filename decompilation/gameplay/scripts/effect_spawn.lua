-- ROM 0x15646 spawn_entity_effect_record: thrown item (+0x57 codes 13..15,
-- grenades) launched from the party path 0xADBE and actor path 0x1408A.
-- Pure record math. The host owns slot allocation (0x15A8C entity slot,
-- 0x1A2CC effect slot), the asset meter tick (0x36FC, D1=-1), the world
-- timeout (0x1DBAA), and the target resolution (0x55848 + 0x15D3C).
local EntityMotion = require("entity_motion")
local EffectSpawn = {}

-- ROM 0x15A4C: spawn position delta, 8 directions (x, y), signed [C].
local position_offsets = {
    { 0x0000, -0x00B0 }, { 0x007C, -0x007C }, { 0x00B0, 0x0000 }, { 0x007C, 0x007C },
    { 0x0000, 0x00B0 }, { -0x007C, 0x007C }, { -0x00B0, 0x0000 }, { -0x007C, -0x007C },
}
-- ROM 0x15A6C: FFDD60/FFDD62 directional probe delta (x, y), signed [C].
local probe_offsets = {
    { 0x0040, -0x0100 }, { 0x0120, -0x00F0 }, { 0x0180, -0x0040 }, { 0x00D0, 0x0050 },
    { 0x0000, 0x0020 }, { -0x0180, 0x0040 }, { -0x0180, -0x0040 }, { -0x0100, -0x00C0 },
}

EffectSpawn.CONTINUATION = 0x15390      -- +0x3A handler
EffectSpawn.WINDOW = 0x003C             -- (+0x16 & 0x7FF) <= 0x3C
EffectSpawn.DISTANCE_CAP = 0x0BE0       -- 0x157CA

local function word(v) return v & 0xFFFF end

-- 0x156FA: ROL.W #3, +1 when bit 15 set (half-octant rounding), AND #7.
function EffectSpawn.direction_index(heading)
    local r = ((heading << 3) | (heading >> 13)) & 0xFFFF
    if r & 0x8000 ~= 0 then r = r + 1 end
    return r & 7
end

function EffectSpawn.window_open(animation_counter)
    return (animation_counter & 0x07FF) <= EffectSpawn.WINDOW
end

-- spawn(parent, target) -> record. parent = { x, y, heading, f56, f57, f87,
-- f9d }. target = nil (no +0xA2 link) or { heading, distance, marker } with
-- heading/distance measured by 0x15D3C from the parent to the 0x55848 point
-- and marker = FFF0DD.
function EffectSpawn.spawn(parent, target)
    local h = parent.heading
    local rec = {
        continuation = EffectSpawn.CONTINUATION, movement_scale = 0x2000,
        heading = h, f56 = parent.f56, f57 = parent.f57, f9d = parent.f9d,
        lifetime = 0x13, movement_offset = -0x0500, vertical_velocity = 0xF400,
        behavior_type = 2, distance_steps = 0x00B4, animation_resource = 0,
    }
    local i = EffectSpawn.direction_index(h) + 1
    rec.probe_x = word(parent.x + probe_offsets[i][1])
    rec.probe_y = word(parent.y + probe_offsets[i][2])
    rec.x = word(parent.x + position_offsets[i][1])
    rec.y = word(parent.y + position_offsets[i][2])
    local o
    if target == nil then
        rec.marker = (parent.f87 & 0xFF) >> 1                 -- 0x15750
        o = EntityMotion.scaled_heading_offsets(h, 8, 5)       -- 0x1575A
    else
        rec.marker = target.marker & 0xFF                      -- 0x157AE
        local d = math.min(target.distance, EffectSpawn.DISTANCE_CAP)
        local q = d // 0x80                                    -- 0x157E0
        rec.heading = target.heading
        rec.distance_steps = q * 2
        rec.movement_offset = -(q * 0x20)                      -- 0x1581A
        o = EntityMotion.scaled_heading_offsets(target.heading, 8, 4)
    end
    rec.offset_x, rec.offset_y = o.horizontal, o.vertical      -- +0x4C/+0x4E
    return rec
end

-- Host adapter: spawn_flat(x, y, heading, f87, linked, t_heading, t_distance,
-- marker) -> { x, y, probe_x, probe_y, heading, marker, distance_steps,
-- movement_offset, offset_x, offset_y }.
function EffectSpawn.spawn_flat(x, y, heading, f87, linked, th, td, marker)
    local r = EffectSpawn.spawn({ x = x, y = y, heading = heading, f87 = f87 },
        linked ~= 0 and { heading = th, distance = td, marker = marker } or nil)
    return { r.x, r.y, r.probe_x, r.probe_y, r.heading, r.marker,
             r.distance_steps, r.movement_offset, r.offset_x, r.offset_y }
end

return EffectSpawn
