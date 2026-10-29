-- Host timing for the weapon-effect frame lists used by field projectiles.
-- The 0x8003 list reaches completion after three updates. The 0x800B list
-- reaches frames 4 and 9 after five updates each because the signed source
-- clock at 0x5FDC wraps before 0x600E marks the last frame. Store the visible
-- result as frame durations instead of reproducing that clock at runtime.
return {
    [3] = { frame_durations = { 1, 1, 1 } },
    [11] = { frame_durations = { 1, 1, 1, 1, 5, 1, 1, 1, 1, 5, 1 } },
}
