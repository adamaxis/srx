-- ROM 0x1A9CE mode records and the negative-selector fallback at 0x1AA10.
-- Cadence and frame counts are source bytes from each record, not host timing.
-- 0x1A4EA indexes this signed timing table with the current node's +0x10 byte.
return {
    reaction_deltas = {
        [0] = 100, 20, 10, 9, 9, 8, 7, 6, 5, 4, 3,
        2, 1, 0, -1, -2, -3, -5, -6, -7, -8,
        -- 0x1A4EA does not bound the node byte: indices 21/22 read the
        -- following 0x41F9/0x00FF code words at 0x1A538/0x1A53A.
        [21] = 0x41F9, [22] = 0x00FF,
    }, -- ROM 0x1A50E..0x1A53B, including the source's two overreads
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
