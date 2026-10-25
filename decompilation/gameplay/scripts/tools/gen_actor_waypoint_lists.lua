-- Regenerates data/actor_waypoint_lists.lua from actor spawn records and
-- their +0xE0 coordinate lists. Usage:
-- lua tools/gen_actor_waypoint_lists.lua <rom.bin> <default-actors.tsv> <world-activity-actors.tsv> > data/actor_waypoint_lists.lua
local function read_file(path)
    local file = assert(io.open(path, "rb"), "cannot open " .. path)
    local value = file:read("a")
    file:close()
    return value
end

local rom = read_file(assert(arg[1], "ROM path"))
local function W(address)
    assert(address >= 0 and address + 2 <= #rom, "word outside ROM")
    return string.unpack(">I2", rom, address + 1)
end
local function L(address)
    assert(address >= 0 and address + 4 <= #rom, "long outside ROM")
    return string.unpack(">I4", rom, address + 1)
end

local function tsv(path)
    local rows = {}
    local lines = read_file(path):gmatch("([^\r\n]*)\r?\n")
    local header = {}
    local first = true
    for line in lines do
        if line ~= "" then
            local fields = {}
            for field in (line .. "\t"):gmatch("(.-)\t") do
                fields[#fields + 1] = field
            end
            if first then
                for index, field in ipairs(fields) do
                    header[field] = index
                end
                first = false
            else
                local row = {}
                for name, index in pairs(header) do
                    row[name] = fields[index]
                end
                rows[#rows + 1] = row
            end
        end
    end
    assert(not first, "TSV has no header: " .. path)
    return rows
end

local function number(row, field)
    local result = tonumber(row[field])
    assert(result ~= nil, "invalid " .. field .. " in actor source table")
    return result
end

local sources = {}
local function add_source(list_offset, source_order, position_offset,
                          tagged_template)
    assert(position_offset > 0, "actor spawn has no +0xE0 pointer")
    local list = sources[list_offset]
    if list == nil then
        list = {}
        sources[list_offset] = list
    end
    local existing = list[source_order]
    if existing ~= nil then
        assert(existing.position_offset == position_offset and
               existing.tagged_template == tagged_template,
               "actor source tables disagree for one spawn identity")
        return
    end
    list[source_order] = {
        position_offset = position_offset,
        tagged_template = tagged_template,
    }
end

-- Default actor rows preserve their +0xE0 pointers but not the tagged
-- template dword. Rewalk the source spawn-record list, including the ordered
-- +0x08 override block, to recover the leader marker read at 0x44A8 [C].
local default_rows = tsv(assert(arg[2], "default actor TSV path"))
local default_lists = {}
for _, row in ipairs(default_rows) do
    local list_offset = number(row, "list_offset")
    local source_order = number(row, "source_order")
    local position_offset = number(row, "position_offset")
    local list = default_lists[list_offset]
    if list == nil then
        list = {}
        default_lists[list_offset] = list
    end
    local existing = list[source_order]
    if existing ~= nil then
        assert(existing.position_offset == position_offset,
               "default actor scenes disagree on a shared source record")
    else
        list[source_order] = {
            source_order = source_order,
            position_offset = position_offset,
        }
    end
end

for list_offset, rows in pairs(default_lists) do
    local ordered = {}
    for _, row in pairs(rows) do
        ordered[#ordered + 1] = row
    end
    table.sort(ordered, function(left, right)
        return left.source_order < right.source_order
    end)
    local cursor = list_offset
    for _, row in ipairs(ordered) do
        assert(L(cursor) ~= 0,
               string.format("actor source list 0x%X ended before TSV order %d at 0x%X",
                             list_offset, row.source_order, cursor))
        local tagged_template = L(cursor)
        local position_offset = L(cursor + 4)
        assert(position_offset == row.position_offset,
               "default actor +0xE0 pointer disagrees with ROM")
        add_source(list_offset, row.source_order, position_offset,
                   tagged_template)

        local next_record = cursor + 8
        if L(next_record) == 2 then
            local pair_offset = next_record + 4
            local terminated = false
            for _ = 1, 128 do
                if W(pair_offset) == 0 then
                    pair_offset = pair_offset + 2
                    terminated = true
                    break
                end
                pair_offset = pair_offset + 2
            end
            assert(terminated, "default actor override list has no zero pair")
            next_record = pair_offset
        end
        cursor = next_record
    end
end

-- Conditional scene actor records already retain the tagged template and
-- coordinate-list pointer in the extracted manifest [C].
for _, row in ipairs(tsv(assert(arg[3], "world-activity actor TSV path"))) do
    add_source(number(row, "list_offset"), number(row, "source_order"),
               number(row, "position_offset"),
               number(row, "tagged_template"))
end

local function signed_word(value)
    return value >= 0x8000 and value - 0x10000 or value
end
local function route_at(position_offset)
    local initial_x = signed_word(W(position_offset))
    local initial_y = signed_word(W(position_offset + 2))
    local result = {
        position_offset = position_offset,
        initial = { x = initial_x, y = initial_y },
    }
    if initial_x < 0 then
        result.no_patrol = true
        return result
    end

    local points = {}
    local cursor = position_offset + 4
    local terminated = false
    for _ = 1, 64 do
        local x = signed_word(W(cursor))
        local y = signed_word(W(cursor + 2))
        if x == 0 then
            terminated = true
            break
        end
        points[#points + 1] = { x = x, y = y }
        cursor = cursor + 4
    end
    assert(terminated, "actor waypoint list has no zero-X terminator")
    result.points = points
    return result
end

local list_offsets = {}
for list_offset in pairs(sources) do
    list_offsets[#list_offsets + 1] = list_offset
end
table.sort(list_offsets)

local out = {
    "-- data/actor_waypoint_lists.lua: source spawn records to +0xE0 routes [C].",
    "-- 0x142DC reads +0xE0, advances +0xE4 by four, wraps on a zero X, and",
    "-- treats a negative initial X as no patrol list. Generated from rom.bin",
    "-- plus the extracted default/world-activity actor manifests; regenerate, do not hand-edit.",
    "return {",
}
for _, list_offset in ipairs(list_offsets) do
    out[#out + 1] = string.format("    [0x%X] = { -- source list", list_offset)
    local orders = {}
    for source_order in pairs(sources[list_offset]) do
        orders[#orders + 1] = source_order
    end
    table.sort(orders)
    for _, source_order in ipairs(orders) do
        local source = sources[list_offset][source_order]
        local route = route_at(source.position_offset)
        local fields = {
            string.format("position_offset = 0x%X", route.position_offset),
            string.format("group_leader = %s",
                tostring((source.tagged_template & 0x80000000) ~= 0)),
            string.format("initial = { x = %d, y = %d }",
                route.initial.x, route.initial.y),
        }
        if route.no_patrol then
            fields[#fields + 1] = "no_patrol = true"
        else
            local points = {}
            for _, point in ipairs(route.points) do
                points[#points + 1] = string.format(
                    "{ x = %d, y = %d }", point.x, point.y)
            end
            fields[#fields + 1] = "points = { " ..
                table.concat(points, ", ") .. " }"
        end
        out[#out + 1] = string.format("        [%d] = { %s }, -- +0xE0=0x%X tagged=0x%08X",
            source_order, table.concat(fields, ", "),
            source.position_offset, source.tagged_template)
    end
    out[#out + 1] = "    },"
end
out[#out + 1] = "}"
io.write(table.concat(out, "\n"), "\n")
