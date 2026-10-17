-- rules/save.lua: save-game format 0x596C4 / load 0x59360 [C unless marked]. C++ only moves bytes to/from SRAM.
-- SRAM: byte n lives at 0x200000 + 2n + 1 (odd bytes, 0x14CE read / 0x14E8 write / 0x1502 copy).
-- Each slot has two 0x800-byte copies: slot 0 = 0x0000 (primary) / 0x0800 (backup); slot 1 = 0x1000 / 0x1800.
-- Block layout (0x800 bytes, image of FFF1E8..):
--   0x000 20 B header "    GAME            " (0x59972); magic "GAME" checked at +4..+7
--   0x014 roster: 13 x 0x4A runner records; leader FF0100 -> index 0, FF0200 -> FFFC0C, FF0300 -> FFFC0D (<0 = none)
--   0x3D6 0x248 B copy of FFF9FA..FFFC41 (spells, deck FFFBBE, nuyen FFFBFE, party ids FFFC0C/D, ...)
--   0x61E 9 words: FFDDC4, FFDDC6, FFDDC8, FF0100, FF0104, FF0200, FF0204, FF0300, FF0304
--   0x630 u32 BE checksum = sum of bytes 0x014..0x62F
--   0x634..0x7FF: not written by the save; whatever the buffer held (stale) [C]
-- Quirk: roster entries not in the party are written from the stale buffer (last load), not from RAM [C].
-- Save is refused silently (rts) when FFDF40 != 0 (0x596C4), on top of the menu gate.
local M = {}
M.SIZE, M.HEADER, M.ROSTER, M.RECORD, M.ROSTER_N, M.GLOBALS, M.GLOBALS_LEN, M.WORDS, M.SUM_LO, M.SUM_AT =
    0x800, "    GAME            ", 0x14, 0x4A, 13, 0x3D6, 0x248, 0x61E, 0x14, 0x630
M.OFFSETS = { [0] = { 0x0000, 0x0800 }, [1] = { 0x1000, 0x1800 } }   -- d5 = 0x08000000 / 0x18001000
-- 0x597EE: the 0x4A-byte runner record = these source offsets in order (w = word, l = long, else byte)
M.RECORD_FIELDS = { {0x0E,"w"},{0x2C,"w"},{0x4C},{0x4D},{0x4E},{0x4F},{0x56},{0x57},{0x58,"l"},{0x5C,"l"},{0x60,"l"},{0x64,"w"},
    {0x66,"l"},{0x6A,"l"},{0x6E,"l"},{0x72,"l"} }
for o = 0x76, 0x8E do M.RECORD_FIELDS[#M.RECORD_FIELDS + 1] = { o } end
for _, f in ipairs({ {0x9A},{0x9B},{0x9C},{0x90,"w"},{0xA0,"w"},{0x8F},{0x9D} }) do M.RECORD_FIELDS[#M.RECORD_FIELDS + 1] = f end

function M.checksum(block)          -- block: 0-based byte array (0x59786: 0x61C bytes from +0x14)
    local s = 0; for i = M.SUM_LO, M.SUM_AT - 1 do s = s + block[i] end; return s & 0xFFFFFFFF
end
function M.valid(block)             -- 0x593FC / 0x5961C
    if block[4] ~= 0x47 or block[5] ~= 0x41 or block[6] ~= 0x4D or block[7] ~= 0x45 then return false end
    local c = (block[M.SUM_AT] << 24) | (block[M.SUM_AT + 1] << 16) | (block[M.SUM_AT + 2] << 8) | block[M.SUM_AT + 3]
    return c == M.checksum(block)
end
function M.seal(block)              -- header + checksum (0x596E6..0x5975C)
    for i = 1, #M.HEADER do block[i - 1] = M.HEADER:byte(i) end
    local c = M.checksum(block)
    for k = 0, 3 do block[M.SUM_AT + k] = (c >> (24 - 8 * k)) & 0xFF end
    return block
end
-- Load decision (0x59456 then 0x59372): returns ok, which copy to use, repair {dst, src} or nil.
-- ROM BUG [C 0x593B2 / 0x593E2 / 0x59496 / 0x594C6]: the repair copy uses fixed slot-0 offsets (0x0000 <-> 0x0800)
-- whatever slot is loaded. M.ROM_REPAIR_QUIRK = true reproduces it; default false repairs the loaded slot (MIGRATION D29).
M.ROM_REPAIR_QUIRK = false
function M.load_plan(slot, primary_ok, backup_ok)
    local o = M.ROM_REPAIR_QUIRK and M.OFFSETS[0] or M.OFFSETS[slot]
    if primary_ok then
        if backup_ok then return true, M.OFFSETS[slot][1] end
        return true, M.OFFSETS[slot][1], { dst = o[2], src = o[1] }    -- backup <- primary
    end
    if backup_ok then return true, M.OFFSETS[slot][2], { dst = o[1], src = o[2] } end   -- primary <- backup
    return false                                                        -- carry: "no game saved here"
end
return M
