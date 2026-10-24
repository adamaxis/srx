-- ROM 0xD4456 maps a current field region to its cab-map cursor position.
-- ROM 0xD445D maps that cursor position back to the destination region.
local M = {}

M.cursor_to_region = { 6, 4, 2, 5, 0, 3, 1 }
M.region_to_cursor = { 4, 6, 2, 5, 1, 3, 0 }
M.region_names = {
    [0] = "Downtown Seattle",
    [1] = "Puyallup Barrens",
    [2] = "Redmond Barrens",
    [3] = "Penumbra District",
    [4] = "Council Island",
    [5] = "Renraku Arcology",
    [6] = "Salish-Shidhe",
}

-- Region indices come from the cursor table at 0xD445D [C]. Each fare is
-- record byte +0x1D in the 30-byte records beginning at 0xD4464 [C].
M.Region = {
    DOWNTOWN_SEATTLE = 0, -- Region table index from 0xD445D [C]
    PUYALLUP_BARRENS = 1, -- Region table index from 0xD445D [C]
    REDMOND_BARRENS = 2, -- Region table index from 0xD445D [C]
    PENUMBRA_DISTRICT = 3, -- Region table index from 0xD445D [C]
    COUNCIL_ISLAND = 4, -- Region table index from 0xD445D [C]
    RENRAKU_ARCOLOGY = 5, -- Region table index from 0xD445D [C]
    SALISH_SHIDHE = 6, -- Region table index from 0xD445D [C]
}

M.regions = {
    [M.Region.DOWNTOWN_SEATTLE] = { name = M.region_names[0], fare = 20 },
    [M.Region.PUYALLUP_BARRENS] = { name = M.region_names[1], fare = 40 },
    [M.Region.REDMOND_BARRENS] = { name = M.region_names[2], fare = 40 },
    [M.Region.PENUMBRA_DISTRICT] = { name = M.region_names[3], fare = 20 },
    [M.Region.COUNCIL_ISLAND] = { name = M.region_names[4], fare = 75 },
    [M.Region.RENRAKU_ARCOLOGY] = { name = M.region_names[5], fare = 20 },
    [M.Region.SALISH_SHIDHE] = { name = M.region_names[6], fare = 100 },
}

-- 0x207B0 selects one of three source fare comments [C]. The 50-nuyen
-- comment is retained even though none of the seven region records uses it.
function M.fare_comment(fare)
    if fare == 20 then return "Null perspiration, chum." end
    if fare == 50 then return "Rough neighborhood, pal!" end
    return "That's a little out of my way, chummer."
end

return M
