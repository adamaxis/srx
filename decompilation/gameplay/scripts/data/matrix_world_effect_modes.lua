-- ROM 0x1A9CE mode records and the negative-selector fallback at 0x1AA10.
-- Cadence and frame counts are source bytes from each record, not host timing.
return {
    default = { cadence = 12, phase_count = 6 }, -- 0x1AA10
    [0] = { cadence = 8, phase_count = 14 },    -- 0x1AA04
    [1] = { cadence = 4, phase_count = 4 },     -- 0x1A9FE
    [2] = { cadence = 9, phase_count = 6 },     -- 0x1A9F8
    [3] = { cadence = 7, phase_count = 6 },     -- 0x1AA22
    [4] = { cadence = 9, phase_count = 4 },     -- 0x1AA1C
    [5] = { cadence = 9, phase_count = 6 },     -- 0x1AA16
    [6] = { cadence = 4, phase_count = 4 },     -- 0x1AA0A
    [7] = { cadence = 4, phase_count = 4 },     -- shares 0x1AA0A
    [8] = { cadence = 12, phase_count = 28 },   -- 0x1A9F2
}
