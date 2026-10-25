-- data/actor_waypoint_lists.lua: source spawn records to +0xE0 routes [C].
-- 0x142DC reads +0xE0, advances +0xE4 by four, wraps on a zero X, and
-- treats a negative initial X as no patrol list. Generated from rom.bin
-- plus the extracted default/world-activity actor manifests; regenerate, do not hand-edit.
return {
    [0xA8CC] = { -- source list
        [0] = { position_offset = 0xA8E4, group_leader = true, initial = { x = 3712, y = 1792 }, points = {  } }, -- +0xE0=0xA8E4 tagged=0x800172CA
    },
    [0x16FFC] = { -- source list
        [0] = { position_offset = 0x17012, group_leader = true, initial = { x = 4352, y = 1792 }, points = {  } }, -- +0xE0=0x17012 tagged=0x8001710E
    },
    [0x17018] = { -- source list
        [0] = { position_offset = 0x1702E, group_leader = true, initial = { x = 1792, y = 1280 }, points = {  } }, -- +0xE0=0x1702E tagged=0x800171A2
    },
    [0x17034] = { -- source list
        [0] = { position_offset = 0x1704A, group_leader = true, initial = { x = 1792, y = 1280 }, points = {  } }, -- +0xE0=0x1704A tagged=0x80017236
    },
    [0x17050] = { -- source list
        [0] = { position_offset = 0x17066, group_leader = true, initial = { x = 2816, y = 768 }, points = {  } }, -- +0xE0=0x17066 tagged=0x80017158
    },
    [0x1706C] = { -- source list
        [0] = { position_offset = 0x17082, group_leader = true, initial = { x = 4352, y = 768 }, points = {  } }, -- +0xE0=0x17082 tagged=0x800172CA
    },
    [0x17088] = { -- source list
        [0] = { position_offset = 0x1709C, group_leader = true, initial = { x = 768, y = 1664 }, points = {  } }, -- +0xE0=0x1709C tagged=0x800171EC
    },
    [0x170A2] = { -- source list
        [0] = { position_offset = 0x170B6, group_leader = true, initial = { x = 896, y = 2688 }, points = {  } }, -- +0xE0=0x170B6 tagged=0x800173A8
    },
    [0x170BC] = { -- source list
        [0] = { position_offset = 0x170D2, group_leader = true, initial = { x = 2816, y = 768 }, points = {  } }, -- +0xE0=0x170D2 tagged=0x80017280
    },
    [0x170D8] = { -- source list
        [0] = { position_offset = 0x170EE, group_leader = true, initial = { x = 3840, y = 1280 }, points = {  } }, -- +0xE0=0x170EE tagged=0x80017314
    },
    [0x170F4] = { -- source list
        [0] = { position_offset = 0x17108, group_leader = true, initial = { x = 768, y = 1664 }, points = {  } }, -- +0xE0=0x17108 tagged=0x8001735E
    },
    [0x1E552] = { -- source list
        [0] = { position_offset = 0x1E5A6, group_leader = true, initial = { x = 22912, y = 3712 }, points = {  } }, -- +0xE0=0x1E5A6 tagged=0x801D657C
        [1] = { position_offset = 0x1E5AC, group_leader = true, initial = { x = 24320, y = 7040 }, points = {  } }, -- +0xE0=0x1E5AC tagged=0x801D657C
        [2] = { position_offset = 0x1E5B2, group_leader = true, initial = { x = 24576, y = 7808 }, points = {  } }, -- +0xE0=0x1E5B2 tagged=0x801D657C
        [3] = { position_offset = 0x1E5B8, group_leader = true, initial = { x = 20224, y = 4608 }, points = {  } }, -- +0xE0=0x1E5B8 tagged=0x801D657C
        [4] = { position_offset = 0x1E5BE, group_leader = true, initial = { x = 23296, y = 2432 }, points = {  } }, -- +0xE0=0x1E5BE tagged=0x801D657C
    },
    [0x1E5C4] = { -- source list
        [0] = { position_offset = 0x1E610, group_leader = true, initial = { x = 4352, y = 1152 }, points = {  } }, -- +0xE0=0x1E610 tagged=0x801D6AB0
        [1] = { position_offset = 0x1E616, group_leader = true, initial = { x = 4736, y = 1152 }, points = {  } }, -- +0xE0=0x1E616 tagged=0x801D6532
        [2] = { position_offset = 0x1E61C, group_leader = true, initial = { x = 4352, y = 1408 }, points = {  } }, -- +0xE0=0x1E61C tagged=0x801D6532
        [3] = { position_offset = 0x1E622, group_leader = true, initial = { x = 4736, y = 1408 }, points = {  } }, -- +0xE0=0x1E622 tagged=0x801D6988
    },
    [0x1E628] = { -- source list
        [0] = { position_offset = 0x1E674, group_leader = true, initial = { x = 3840, y = 1280 }, points = {  } }, -- +0xE0=0x1E674 tagged=0x801D6AFA
        [1] = { position_offset = 0x1E67A, group_leader = true, initial = { x = 4224, y = 1280 }, points = {  } }, -- +0xE0=0x1E67A tagged=0x801D6AFA
        [2] = { position_offset = 0x1E680, group_leader = true, initial = { x = 512, y = 1280 }, points = {  } }, -- +0xE0=0x1E680 tagged=0x801D6AFA
        [3] = { position_offset = 0x1E686, group_leader = true, initial = { x = 768, y = 1920 }, points = {  } }, -- +0xE0=0x1E686 tagged=0x801D6A1C
    },
    [0x679F0] = { -- source list
        [0] = { position_offset = 0x67A24, group_leader = true, initial = { x = 1536, y = 4096 }, points = { { x = 15616, y = 4096 }, { x = 15616, y = 8704 }, { x = 19328, y = 8704 }, { x = 19328, y = 14720 }, { x = 6656, y = 14720 }, { x = 6656, y = 8704 }, { x = 9984, y = 8704 }, { x = 9984, y = 4096 } } }, -- +0xE0=0x67A24 tagged=0x801D6988
        [1] = { position_offset = 0x67A30, group_leader = true, initial = { x = 19328, y = 8704 }, points = { { x = 19328, y = 14720 }, { x = 6656, y = 14720 }, { x = 6656, y = 8704 }, { x = 9984, y = 8704 }, { x = 9984, y = 4096 } } }, -- +0xE0=0x67A30 tagged=0x801D6988
        [2] = { position_offset = 0x67A3C, group_leader = true, initial = { x = 6656, y = 8704 }, points = { { x = 9984, y = 8704 }, { x = 9984, y = 4096 } } }, -- +0xE0=0x67A3C tagged=0x801D6988
    },
    [0xD5538] = { -- source list
        [0] = { position_offset = 0xD556C, group_leader = true, initial = { x = 2048, y = 1536 }, points = { { x = 4992, y = 1536 }, { x = 4992, y = 7168 }, { x = 1536, y = 7168 }, { x = 1536, y = 11008 }, { x = 3328, y = 13312 }, { x = 6656, y = 13312 }, { x = 6656, y = 14592 }, { x = 10752, y = 14592 }, { x = 11648, y = 13824 }, { x = 15360, y = 13824 }, { x = 15488, y = 14592 }, { x = 19968, y = 14592 }, { x = 20864, y = 12800 }, { x = 25600, y = 12800 }, { x = 25600, y = 7296 }, { x = 20480, y = 7296 }, { x = 20480, y = 2176 }, { x = 5120, y = 2176 }, { x = 5120, y = 1536 } } }, -- +0xE0=0xD556C tagged=0x801D6988
        [1] = { position_offset = 0xD5584, group_leader = true, initial = { x = 6656, y = 13312 }, points = { { x = 6656, y = 14592 }, { x = 10752, y = 14592 }, { x = 11648, y = 13824 }, { x = 15360, y = 13824 }, { x = 15488, y = 14592 }, { x = 19968, y = 14592 }, { x = 20864, y = 12800 }, { x = 25600, y = 12800 }, { x = 25600, y = 7296 }, { x = 20480, y = 7296 }, { x = 20480, y = 2176 }, { x = 5120, y = 2176 }, { x = 5120, y = 1536 } } }, -- +0xE0=0xD5584 tagged=0x801D6988
        [2] = { position_offset = 0xD5594, group_leader = true, initial = { x = 15360, y = 13824 }, points = { { x = 15488, y = 14592 }, { x = 19968, y = 14592 }, { x = 20864, y = 12800 }, { x = 25600, y = 12800 }, { x = 25600, y = 7296 }, { x = 20480, y = 7296 }, { x = 20480, y = 2176 }, { x = 5120, y = 2176 }, { x = 5120, y = 1536 } } }, -- +0xE0=0xD5594 tagged=0x801D6988
    },
    [0x15B998] = { -- source list
        [0] = { position_offset = 0x15B9CC, group_leader = true, initial = { x = 384, y = 13056 }, points = { { x = 384, y = 9216 }, { x = 5248, y = 9216 }, { x = 5376, y = 7168 }, { x = 9856, y = 7168 }, { x = 9856, y = 8448 }, { x = 14592, y = 8448 }, { x = 15360, y = 9984 }, { x = 15360, y = 14592 }, { x = 11776, y = 14592 }, { x = 15360, y = 14592 }, { x = 15360, y = 9984 }, { x = 14592, y = 8448 }, { x = 9856, y = 8448 }, { x = 9856, y = 7168 }, { x = 5376, y = 7168 }, { x = 5248, y = 9216 }, { x = 384, y = 9216 } } }, -- +0xE0=0x15B9CC tagged=0x801D6988
        [1] = { position_offset = 0x15B9E4, group_leader = true, initial = { x = 14592, y = 8448 }, points = { { x = 15360, y = 9984 }, { x = 15360, y = 14592 }, { x = 11776, y = 14592 }, { x = 15360, y = 14592 }, { x = 15360, y = 9984 }, { x = 14592, y = 8448 }, { x = 9856, y = 8448 }, { x = 9856, y = 7168 }, { x = 5376, y = 7168 }, { x = 5248, y = 9216 }, { x = 384, y = 9216 } } }, -- +0xE0=0x15B9E4 tagged=0x801D6988
        [2] = { position_offset = 0x15B9F4, group_leader = true, initial = { x = 15360, y = 14592 }, points = { { x = 15360, y = 9984 }, { x = 14592, y = 8448 }, { x = 9856, y = 8448 }, { x = 9856, y = 7168 }, { x = 5376, y = 7168 }, { x = 5248, y = 9216 }, { x = 384, y = 9216 } } }, -- +0xE0=0x15B9F4 tagged=0x801D6988
    },
    [0x172D1C] = { -- source list
        [0] = { position_offset = 0x172D58, group_leader = true, initial = { x = 13440, y = 8704 }, points = { { x = 10240, y = 8704 }, { x = 10240, y = 15744 }, { x = 13440, y = 14080 }, { x = 21248, y = 14080 }, { x = 21248, y = 9472 }, { x = 22016, y = 9472 }, { x = 22016, y = 6400 }, { x = 25216, y = 6400 }, { x = 25216, y = 2944 }, { x = 29440, y = 4992 }, { x = 1792, y = 4992 } } }, -- +0xE0=0x172D58 tagged=0x801D6988
        [1] = { position_offset = 0x172D68, group_leader = true, initial = { x = 21248, y = 14080 }, points = { { x = 21248, y = 9472 }, { x = 22016, y = 9472 }, { x = 22016, y = 6400 }, { x = 25216, y = 6400 }, { x = 25216, y = 2944 }, { x = 29440, y = 4992 }, { x = 1792, y = 4992 } } }, -- +0xE0=0x172D68 tagged=0x801D6988
        [2] = { position_offset = 0x172D78, group_leader = true, initial = { x = 25216, y = 6400 }, points = { { x = 25216, y = 2944 }, { x = 29440, y = 4992 }, { x = 1792, y = 4992 } } }, -- +0xE0=0x172D78 tagged=0x801D6988
    },
    [0x17D660] = { -- source list
        [0] = { position_offset = 0x17D6B6, group_leader = true, initial = { x = 12032, y = 6400 }, points = { { x = 12032, y = 8832 }, { x = 13952, y = 9728 }, { x = 19072, y = 9728 }, { x = 19072, y = 768 }, { x = 8064, y = 6528 }, { x = 8064, y = 18176 }, { x = 12032, y = 20608 }, { x = 8448, y = 21632 }, { x = 8064, y = 16000 }, { x = 8064, y = 6400 } } }, -- +0xE0=0x17D6B6 tagged=0x801D6988
        [1] = { position_offset = 0x17D6CE, group_leader = true, initial = { x = 8064, y = 18176 }, points = { { x = 12032, y = 20608 }, { x = 8448, y = 21632 }, { x = 8064, y = 16000 }, { x = 8064, y = 6400 } } }, -- +0xE0=0x17D6CE tagged=0x801D6988
        [2] = { position_offset = 0x17D694, group_leader = true, initial = { x = 2816, y = 3584 }, points = { { x = 21888, y = 3584 }, { x = 21888, y = 6144 }, { x = 19200, y = 6144 }, { x = 22528, y = 12544 }, { x = 22528, y = 15232 }, { x = 4992, y = 15232 }, { x = 4992, y = 6528 } } }, -- +0xE0=0x17D694 tagged=0x801D6988
    },
    [0x18A73A] = { -- source list
        [0] = { position_offset = 0x18A76E, group_leader = true, initial = { x = 31232, y = 3584 }, points = { { x = 31232, y = 6144 }, { x = 27776, y = 6528 }, { x = 25216, y = 6528 }, { x = 25216, y = 20608 }, { x = 22528, y = 20608 }, { x = 22528, y = 12544 }, { x = 3584, y = 12544 }, { x = 5376, y = 3584 } } }, -- +0xE0=0x18A76E tagged=0x801D6988
        [1] = { position_offset = 0x18A786, group_leader = true, initial = { x = 22528, y = 12544 }, points = { { x = 3584, y = 12544 }, { x = 5376, y = 3584 } } }, -- +0xE0=0x18A786 tagged=0x801D6988
        [2] = { position_offset = 0x18A794, group_leader = true, initial = { x = 5248, y = 3584 }, points = { { x = 19456, y = 3584 }, { x = 19456, y = 12416 }, { x = 13824, y = 12416 }, { x = 13824, y = 20480 }, { x = 16640, y = 20480 }, { x = 16640, y = 17024 }, { x = 3328, y = 17024 } } }, -- +0xE0=0x18A794 tagged=0x801D6988
    },
    [0x1D753A] = { -- source list
        [0] = { position_offset = 0x1D7556, group_leader = true, initial = { x = 11264, y = 12544 }, points = { { x = 11392, y = 11520 } } }, -- +0xE0=0x1D7556 tagged=0x801D6A1C
        [1] = { position_offset = 0x1D7560, group_leader = true, initial = { x = 17664, y = 1536 }, points = {  } }, -- +0xE0=0x1D7560 tagged=0x801D6A1C
    },
    [0x1D7BD4] = { -- source list
        [0] = { position_offset = 0x1D7BE8, group_leader = true, initial = { x = 8960, y = 3328 }, points = { { x = 12544, y = 3328 }, { x = 12544, y = 15360 }, { x = 1024, y = 15360 } } }, -- +0xE0=0x1D7BE8 tagged=0x801D6A1C
        [1] = { position_offset = 0x1D7BE8, group_leader = false, initial = { x = 8960, y = 3328 }, points = { { x = 12544, y = 3328 }, { x = 12544, y = 15360 }, { x = 1024, y = 15360 } } }, -- +0xE0=0x1D7BE8 tagged=0x001D6A1C
    },
    [0x1D82F4] = { -- source list
        [0] = { position_offset = 0x1D8310, group_leader = true, initial = { x = 1024, y = 4096 }, points = { { x = 15616, y = 4096 }, { x = 15616, y = 9728 }, { x = 1024, y = 9728 }, { x = 15616, y = 9728 }, { x = 15616, y = 4096 } } }, -- +0xE0=0x1D8310 tagged=0x801D6A1C
        [1] = { position_offset = 0x1D8310, group_leader = false, initial = { x = 1024, y = 4096 }, points = { { x = 15616, y = 4096 }, { x = 15616, y = 9728 }, { x = 1024, y = 9728 }, { x = 15616, y = 9728 }, { x = 15616, y = 4096 } } }, -- +0xE0=0x1D8310 tagged=0x001D6A1C
        [2] = { position_offset = 0x1D8310, group_leader = false, initial = { x = 1024, y = 4096 }, points = { { x = 15616, y = 4096 }, { x = 15616, y = 9728 }, { x = 1024, y = 9728 }, { x = 15616, y = 9728 }, { x = 15616, y = 4096 } } }, -- +0xE0=0x1D8310 tagged=0x001D6A1C
    },
    [0x1D8B70] = { -- source list
        [0] = { position_offset = 0x1D8B84, group_leader = true, initial = { x = 4608, y = 13056 }, points = { { x = 13824, y = 13056 }, { x = 13824, y = 14336 }, { x = 4608, y = 14336 } } }, -- +0xE0=0x1D8B84 tagged=0x801D6A1C
        [1] = { position_offset = 0x1D8B84, group_leader = false, initial = { x = 4608, y = 13056 }, points = { { x = 13824, y = 13056 }, { x = 13824, y = 14336 }, { x = 4608, y = 14336 } } }, -- +0xE0=0x1D8B84 tagged=0x001D6A1C
    },
    [0x1D91DA] = { -- source list
        [0] = { position_offset = 0x1D9248, group_leader = true, initial = { x = 1536, y = 4352 }, points = { { x = 20352, y = 4352 } } }, -- +0xE0=0x1D9248 tagged=0x801D6A1C
        [1] = { position_offset = 0x1D9252, group_leader = true, initial = { x = 1536, y = 8960 }, points = { { x = 20352, y = 8960 } } }, -- +0xE0=0x1D9252 tagged=0x801D6A1C
        [2] = { position_offset = 0x1D925C, group_leader = true, initial = { x = 2048, y = 13568 }, points = { { x = 20352, y = 13568 } } }, -- +0xE0=0x1D925C tagged=0x801D6A1C
        [3] = { position_offset = 0x1D9266, group_leader = true, initial = { x = 21504, y = 2560 }, points = {  } }, -- +0xE0=0x1D9266 tagged=0x801D6A1C
        [4] = { position_offset = 0x1D926C, group_leader = true, initial = { x = 24704, y = 2560 }, points = {  } }, -- +0xE0=0x1D926C tagged=0x801D6A1C
        [5] = { position_offset = 0x1D922E, group_leader = true, initial = { x = 5632, y = 1408 }, points = { { x = 5632, y = 2688 } } }, -- +0xE0=0x1D922E tagged=0x801D6988
        [6] = { position_offset = 0x1D9238, group_leader = true, initial = { x = 13952, y = 6144 }, points = {  } }, -- +0xE0=0x1D9238 tagged=0x801D6988
        [7] = { position_offset = 0x1D923E, group_leader = true, initial = { x = 2816, y = 10880 }, points = { { x = 3840, y = 10880 } } }, -- +0xE0=0x1D923E tagged=0x801D6988
    },
    [0x1D9944] = { -- source list
        [0] = { position_offset = 0x1D9950, group_leader = true, initial = { x = 4864, y = 4864 }, points = { { x = 8192, y = 4864 }, { x = 8192, y = 7168 }, { x = 17408, y = 7168 }, { x = 17408, y = 14848 }, { x = 4864, y = 14848 }, { x = 4864, y = 4864 } } }, -- +0xE0=0x1D9950 tagged=0x801D6A1C
    },
    [0x1DA016] = { -- source list
        [0] = { position_offset = 0x1DA02A, group_leader = true, initial = { x = 3328, y = 15744 }, points = { { x = 10496, y = 15744 }, { x = 10496, y = 11648 }, { x = 12928, y = 11648 }, { x = 12928, y = 15744 }, { x = 20224, y = 15744 }, { x = 12928, y = 15744 }, { x = 12928, y = 11648 }, { x = 10496, y = 11648 }, { x = 10496, y = 15744 }, { x = 3328, y = 15744 } } }, -- +0xE0=0x1DA02A tagged=0x801D6A1C
        [1] = { position_offset = 0x1DA02A, group_leader = false, initial = { x = 3328, y = 15744 }, points = { { x = 10496, y = 15744 }, { x = 10496, y = 11648 }, { x = 12928, y = 11648 }, { x = 12928, y = 15744 }, { x = 20224, y = 15744 }, { x = 12928, y = 15744 }, { x = 12928, y = 11648 }, { x = 10496, y = 11648 }, { x = 10496, y = 15744 }, { x = 3328, y = 15744 } } }, -- +0xE0=0x1DA02A tagged=0x001D6A1C
    },
    [0x1DA82E] = { -- source list
        [0] = { position_offset = 0x1DA83A, group_leader = true, initial = { x = 10240, y = 2816 }, points = { { x = 10240, y = 9984 }, { x = 5632, y = 9984 }, { x = 5632, y = 2816 }, { x = 10240, y = 2816 } } }, -- +0xE0=0x1DA83A tagged=0x801D6A1C
    },
    [0x1DB010] = { -- source list
        [0] = { position_offset = 0x1DB01C, group_leader = true, initial = { x = 14848, y = 1280 }, points = { { x = 14848, y = 17152 }, { x = 5632, y = 17152 }, { x = 14848, y = 17152 }, { x = 14848, y = 1280 } } }, -- +0xE0=0x1DB01C tagged=0x801D6A1C
    },
    [0x1DB5DE] = { -- source list
        [0] = { position_offset = 0x1DB5EA, group_leader = true, initial = { x = 7680, y = 1280 }, points = { { x = 7680, y = 7168 }, { x = 7680, y = 1280 } } }, -- +0xE0=0x1DB5EA tagged=0x801D6A1C
    },
    [0x1DBBE8] = { -- source list
        [0] = { position_offset = 0x1DBBFC, group_leader = true, initial = { x = 1024, y = 9984 }, points = { { x = 1024, y = 18688 }, { x = 14848, y = 18688 }, { x = 14848, y = 9984 }, { x = 1024, y = 9984 } } }, -- +0xE0=0x1DBBFC tagged=0x801D6A1C
        [1] = { position_offset = 0x1DBBFC, group_leader = false, initial = { x = 1024, y = 9984 }, points = { { x = 1024, y = 18688 }, { x = 14848, y = 18688 }, { x = 14848, y = 9984 }, { x = 1024, y = 9984 } } }, -- +0xE0=0x1DBBFC tagged=0x001D6A1C
    },
    [0x1DC380] = { -- source list
        [0] = { position_offset = 0x1DC3C0, group_leader = true, initial = { x = 5504, y = 14208 }, points = { { x = 5504, y = 5248 }, { x = 17408, y = 5248 }, { x = 5504, y = 5248 }, { x = 5504, y = 14208 } } }, -- +0xE0=0x1DC3C0 tagged=0x801D6A1C
        [1] = { position_offset = 0x1DC3D6, group_leader = true, initial = { x = 24704, y = 19584 }, points = { { x = 19456, y = 6784 }, { x = 19456, y = 16128 }, { x = 21120, y = 16128 }, { x = 19456, y = 16128 }, { x = 19456, y = 6784 }, { x = 24704, y = 19584 } } }, -- +0xE0=0x1DC3D6 tagged=0x801D6A1C
        [2] = { position_offset = 0x1DC3B4, group_leader = true, initial = { x = 16896, y = 12032 }, points = {  } }, -- +0xE0=0x1DC3B4 tagged=0x801D6A1C
        [3] = { position_offset = 0x1DC3BA, group_leader = true, initial = { x = 12160, y = 12032 }, points = {  } }, -- +0xE0=0x1DC3BA tagged=0x801D6A1C
    },
    [0x1DCB1C] = { -- source list
        [0] = { position_offset = 0x1DCB28, group_leader = true, initial = { x = 768, y = 1152 }, points = { { x = 5632, y = 1152 }, { x = 5632, y = 5376 }, { x = 24576, y = 5376 }, { x = 24576, y = 14592 }, { x = 19200, y = 14592 }, { x = 24576, y = 14592 }, { x = 24576, y = 5376 }, { x = 5632, y = 5376 }, { x = 5632, y = 1152 }, { x = 768, y = 1152 } } }, -- +0xE0=0x1DCB28 tagged=0x801D6A1C
    },
    [0x1DD226] = { -- source list
        [0] = { position_offset = 0x1DD23A, group_leader = true, initial = { x = 896, y = 3328 }, points = { { x = 896, y = 14464 }, { x = 9600, y = 14464 }, { x = 9600, y = 5376 }, { x = 5888, y = 5376 }, { x = 5888, y = 3328 }, { x = 896, y = 3328 } } }, -- +0xE0=0x1DD23A tagged=0x801D6A1C
        [1] = { position_offset = 0x1DD258, group_leader = true, initial = { x = 17920, y = 5632 }, points = { { x = 24576, y = 5632 }, { x = 24576, y = 14464 }, { x = 18816, y = 14464 }, { x = 24576, y = 14464 }, { x = 24576, y = 5632 }, { x = 17920, y = 5632 } } }, -- +0xE0=0x1DD258 tagged=0x801D6A1C
    },
    [0x1DD9AC] = { -- source list
        [0] = { position_offset = 0x1DD9C0, group_leader = true, initial = { x = 1024, y = 4352 }, points = { { x = 1024, y = 8192 }, { x = 1024, y = 4352 } } }, -- +0xE0=0x1DD9C0 tagged=0x801D6A1C
        [1] = { position_offset = 0x1DD9CE, group_leader = true, initial = { x = 8960, y = 14336 }, points = { { x = 8960, y = 1280 }, { x = 8960, y = 14336 } } }, -- +0xE0=0x1DD9CE tagged=0x801D6A1C
    },
    [0x1DDE26] = { -- source list
        [0] = { position_offset = 0x1DDE3A, group_leader = true, initial = { x = 768, y = 3584 }, points = { { x = 4096, y = 3584 }, { x = 4096, y = 3584 } } }, -- +0xE0=0x1DDE3A tagged=0x801D6A1C
        [1] = { position_offset = 0x1DDE48, group_leader = true, initial = { x = 8960, y = 9600 }, points = { { x = 10880, y = 9600 }, { x = 10880, y = 9600 } } }, -- +0xE0=0x1DDE48 tagged=0x801D6A1C
    },
    [0x1DE38C] = { -- source list
        [0] = { position_offset = 0x1DE398, group_leader = true, initial = { x = 13312, y = 1280 }, points = { { x = 13312, y = 17152 } } }, -- +0xE0=0x1DE398 tagged=0x801D6A1C
    },
    [0x1DE998] = { -- source list
        [0] = { position_offset = 0x1DE9B4, group_leader = true, initial = { x = 768, y = 11008 }, points = { { x = 8704, y = 11008 }, { x = 768, y = 11008 } } }, -- +0xE0=0x1DE9B4 tagged=0x801D6A1C
        [1] = { position_offset = 0x1DE9C2, group_leader = true, initial = { x = 2304, y = 1280 }, points = { { x = 13440, y = 1280 }, { x = 2304, y = 1280 } } }, -- +0xE0=0x1DE9C2 tagged=0x801D6A1C
        [2] = { position_offset = 0x1DE9D0, group_leader = true, initial = { x = 5376, y = 6400 }, points = { { x = 12800, y = 6400 } } }, -- +0xE0=0x1DE9D0 tagged=0x801D6A1C
    },
    [0x1DEF12] = { -- source list
        [0] = { position_offset = 0x1DEF1E, group_leader = true, initial = { x = 12288, y = 9088 }, points = { { x = 12288, y = 17408 } } }, -- +0xE0=0x1DEF1E tagged=0x801D6A1C
    },
    [0x1E994C] = { -- source list
        [0] = { position_offset = 0x1E99B0, group_leader = true, initial = { x = 1960, y = 18176 }, points = {  } }, -- +0xE0=0x1E99B0 tagged=0x801D66EE
        [1] = { position_offset = 0x1E99B6, group_leader = true, initial = { x = 3680, y = 19200 }, points = {  } }, -- +0xE0=0x1E99B6 tagged=0x801D66EE
        [2] = { position_offset = 0x1E99BC, group_leader = true, initial = { x = 3640, y = 14592 }, points = {  } }, -- +0xE0=0x1E99BC tagged=0x801D66EE
        [3] = { position_offset = 0x1E99C2, group_leader = true, initial = { x = 1960, y = 13056 }, points = {  } }, -- +0xE0=0x1E99C2 tagged=0x801D66EE
        [4] = { position_offset = 0x1E99C8, group_leader = true, initial = { x = 2800, y = 8576 }, points = {  } }, -- +0xE0=0x1E99C8 tagged=0x801D66EE
        [5] = { position_offset = 0x1E99CE, group_leader = true, initial = { x = 880, y = 9096 }, points = {  } }, -- +0xE0=0x1E99CE tagged=0x801D66EE
    },
    [0x1EDA88] = { -- source list
        [0] = { position_offset = 0x1EDAC8, group_leader = true, initial = { x = 2560, y = 2048 }, points = {  } }, -- +0xE0=0x1EDAC8 tagged=0x801EDB00
        [1] = { position_offset = 0x1EDABC, group_leader = true, initial = { x = 1920, y = 2304 }, points = {  } }, -- +0xE0=0x1EDABC tagged=0x801D66EE
        [2] = { position_offset = 0x1EDAC2, group_leader = true, initial = { x = 3328, y = 2304 }, points = {  } }, -- +0xE0=0x1EDAC2 tagged=0x801D66EE
    },
}
