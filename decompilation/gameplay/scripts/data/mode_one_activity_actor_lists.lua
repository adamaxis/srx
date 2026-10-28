-- US ROM 0x1684C pending actor selection. The pointer table at 0x16FD4
-- supplies the per-scene lists; marker values are the 0xEEFE byte plus one
-- as tested against FFFC0C/FFFC0D at 0x168EE-0x16900. Scene 10's two
-- FFFBF9-dependent lists are selected directly at 0x1686C-0x16880.
-- Scene 13 and 15 retain the adjacent FFFC1E bit gates from 0x168CE-0x168EA.
return {
    [10] = {
        { event_code = 6, runner_marker = 3, list_offset = 0x17018 },
        { event_code_not = 6, runner_marker = 5, list_offset = 0x17034 },
    },
    [11] = {
        { runner_marker = 2, list_offset = 0x17050 },
    },
    -- Scene 12's 0x16FD4 pointer-table row is null.
    [13] = {
        { runner_marker = 7, required_world_flag = 19,
          list_offset = 0x1706C },
    },
    [14] = {
        { runner_marker = 4, list_offset = 0x17088 },
    },
    [15] = {
        { runner_marker = 10, required_world_flag = 29,
          list_offset = 0x170A2 },
    },
    [16] = {
        { runner_marker = 6, list_offset = 0x170BC },
    },
    [17] = {
        { runner_marker = 8, list_offset = 0x170D8 },
    },
    [18] = {
        { runner_marker = 9, list_offset = 0x170F4 },
    },
    [19] = {
        { runner_marker = 1, list_offset = 0x16FFC },
    },
}
