-- ROM 0xD4456 maps a current field region to its cab-map cursor position.
-- ROM 0xD445D maps that cursor position back to the destination region.
local M = {}

M.cursor_to_region = {
    "salish_shidhe", "council_island", "redmond_barrens",
    "renraku_arcology", "downtown_seattle", "penumbra_district",
    "puyallup_barrens",
}
M.region_to_cursor = {
    downtown_seattle = 4,
    puyallup_barrens = 6,
    redmond_barrens = 2,
    penumbra_district = 5,
    council_island = 1,
    renraku_arcology = 3,
    salish_shidhe = 0,
}
-- Stable game-facing region IDs are translated to the field selector's source
-- values by the native adapter. Each fare comes from record byte +0x1D in the
-- 30-byte records beginning at 0xD4464 [C].
M.Region = {
    DOWNTOWN_SEATTLE = "downtown_seattle",
    PUYALLUP_BARRENS = "puyallup_barrens",
    REDMOND_BARRENS = "redmond_barrens",
    PENUMBRA_DISTRICT = "penumbra_district",
    COUNCIL_ISLAND = "council_island",
    RENRAKU_ARCOLOGY = "renraku_arcology",
    SALISH_SHIDHE = "salish_shidhe",
}

M.regions = {
    [M.Region.DOWNTOWN_SEATTLE] = {
        name = "Downtown Seattle", fare = 20 },
    [M.Region.PUYALLUP_BARRENS] = {
        name = "Puyallup Barrens", fare = 40 },
    [M.Region.REDMOND_BARRENS] = {
        name = "Redmond Barrens", fare = 40 },
    [M.Region.PENUMBRA_DISTRICT] = {
        name = "Penumbra District", fare = 20 },
    [M.Region.COUNCIL_ISLAND] = {
        name = "Council Island", fare = 75 },
    [M.Region.RENRAKU_ARCOLOGY] = {
        name = "Renraku Arcology", fare = 20 },
    [M.Region.SALISH_SHIDHE] = {
        name = "Salish-Shidhe", fare = 100 },
}

-- 0x207B0 selects one of three source fare comments [C]. The 50-nuyen
-- comment is retained even though none of the seven region records uses it.
function M.fare_comment(fare)
    if fare == 20 then return "Null perspiration, chum." end
    if fare == 50 then return "Rough neighborhood, pal!" end
    return "That's a little out of my way, chummer."
end

return M
