-- Enter Passcode choices from the US ROM's pointer/text table at 0xC19C2.
-- The selection byte indexes the target-system table at 0xC195D after the
-- source's one-based choice decrement. FFFC28 bit numbers remain source bits.
local entries = {
    { id = "run_passcode", flag_bit = 0, current_target = true },
    { id = "aztechnology", flag_bit = 1, system_id = 7 },
    { id = "club_penumbra", flag_bit = 2, system_id = 12 },
    { id = "seattle_general_hospital", flag_bit = 3, system_id = 13 },
    { id = "ares", flag_bit = 4, system_id = 24 },
    { id = "gates_undersound", flag_bit = 5, system_id = 17 },
    { id = "hollywood", flag_bit = 6, system_id = 19 },
    { id = "mitsuhama", flag_bit = 7, system_id = 20 },
    { id = "renraku", flag_bit = 8, system_id = 21 },
    { id = "fuchi", flag_bit = 9, system_id = 22 },
    { id = "lone_star", flag_bit = 10, system_id = 23 },
    { id = "ito_passcode", flag_bit = 11, system_id = 18 },
}

-- 0xE1E6 chooses one byte from the 14-entry table beginning at 0xC1968.
local search_system_ids = {
    0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 14, 15, 16,
}

return { passcodes = entries, search_system_ids = search_system_ids }
