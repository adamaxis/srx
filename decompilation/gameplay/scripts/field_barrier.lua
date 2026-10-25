-- Directional five-cell field barrier placement from ROM 0x49FA..0x4A86 [C].
local E = require("enums")
local I = require("lib.int68k")

local FieldBarrier = {}

local function snap_axis_to_cell_center(value)
    -- ROM 0x4A18/0x4A2A wraps the source word, clears the low cell bits, then
    -- adds E.Grid.CELL_CENTER [C].
    local wrapped = I.u16(value)
    local cell_mask = ~((1 << E.Grid.CELL_SHIFT) - 1)
    return I.u16((wrapped & cell_mask) + E.Grid.CELL_CENTER)
end

function FieldBarrier.place_flat(caster_x, caster_y, heading_word, action_code)
    -- 0x49FA rotates the +0x2E word left three bits and masks the octant;
    -- its low three bits equal the original u16's high three bits [C].
    local heading = I.u16(heading_word)
    local octant = (heading >> E.FieldBarrier.HEADING_OCTANT_SHIFT) &
        E.FieldBarrier.OCTANT_MASK
    local center_offset = E.FieldBarrier.CENTER_OFFSETS[octant]
    local center_x = snap_axis_to_cell_center(
        I.u16(caster_x) + center_offset[1])
    local center_y = snap_axis_to_cell_center(
        I.u16(caster_y) + center_offset[2])

    -- ROM 0x4A34..0x4A42 computes row*E.Grid.ROW_CELLS+column [C].
    local base = (center_y >> E.Grid.CELL_SHIFT) * E.Grid.ROW_CELLS +
        (center_x >> E.Grid.CELL_SHIFT)
    local cells = {}
    for index = 1, E.FieldBarrier.CELL_COUNT do
        -- ROM 0x4A68 adds each signed cell offset to the base word [C].
        cells[index] = I.u16(base + E.FieldBarrier.CELL_OFFSETS[octant][index])
    end

    -- 0x4A74..0x4A7E selects the stronger byte through action code 9 [C].
    local code = I.u8(action_code) <= E.FieldBarrier.SUPER_MAX_ACTION_CODE
        and E.FieldBarrier.CELL_CODE_SUPER_BARRIER
        or E.FieldBarrier.CELL_CODE_BARRIER

    -- Integer array layout is octant, center X, center Y, five cell indices,
    -- then cell code. The host uses this same Lua result for the collision map.
    return {
        octant, center_x, center_y,
        cells[1], cells[2], cells[3], cells[4], cells[5], code,
    }
end

return FieldBarrier
