-- rules/deck.lua: cyberdeck memory/storage/load-time rules, ROM-traced [C unless marked].
-- Deck record view (FFFBBE, docs/05 s3): rec.memory (+0x02 w, active memory), rec.storage (+0x04 w),
--   rec.load_io (+0x06 b, "Load/IO Speed" label 0xE387E block), rec.ratings[0..11] (+0x0E+p),
--   rec.active[0..4] (+0x1A+slot, -1 = empty), rec.files[0..4] = {id (+0x1F+3k, <0 empty), size (+0x20+3k)}.
-- Slot state: slots.status[0..4] = FFF0ED, slots.timer[0..4] = FFF0F2 (words), slots.phase[0..4] = FFF0E8.
-- srx cross-check: src/cyberspace/matrix_storage_conversion.cpp, matrix_program_timing_conversion.cpp (same maths).
local M = {}
M.FOOTPRINT = { [0] = 2, 4, 4, 3, 4, 4, 2, 3, 3, 2, 2, 3 }   -- 0xE2DF2 [C bytes]
M.PROGRAMS, M.SLOTS, M.FILES, M.MAX_LOAD_STEPS, M.READY = 12, 5, 5, 6, 6
M.FILE_NAME = 0xE3A85                                        -- + id*0x14 (Back-up Files, Archived Files, ...) [C]

function M.index(col, row) return row * 4 + col end                              -- 0x18A92
function M.size(p, rating) return rating * rating * M.FOOTPRINT[p] end          -- 0x1952C
local function w16(v) return v & 0xFFFF end
function M.free_memory(rec)                                                      -- 0x1957E
    local used = 0
    for s = 0, M.SLOTS - 1 do local p = rec.active[s]; if p >= 0 then used = used + M.size(p, rec.ratings[p]) end end
    return w16(rec.memory - used)
end
function M.free_storage(rec)                                                     -- 0x19558
    local used = 0
    for k = 0, M.FILES - 1 do used = used + rec.files[k].size end              -- 0x1954A (+0x20 even if id < 0)
    for p = 0, M.PROGRAMS - 1 do used = used + M.size(p, rec.ratings[p]) end
    return w16(rec.storage - used)
end
local function signed(v) return v >= 0x8000 and v - 0x10000 or v end
function M.fits(rec, p) return signed(w16(M.free_memory(rec) - M.size(p, rec.ratings[p]))) >= 0 end   -- cmp.w / bpl
function M.load_steps(size, io)                                                  -- 0x195A6: ceil, >= 7 -> 6
    local s = w16(size) // io; if w16(size) % io ~= 0 then s = s + 1 end
    if s >= 7 then s = M.MAX_LOAD_STEPS end
    return s
end
function M.loaded_slot(rec, p) for s = 0, M.SLOTS - 1 do if rec.active[s] == p then return s end end end   -- 0x19260
function M.free_slot(rec) for s = 0, M.SLOTS - 1 do if rec.active[s] < 0 then return s end end end          -- 0x192D4
-- 0x19736 + 0x195D8: install p in slot
function M.load(rec, slots, slot, p, matrix_mode)
    if matrix_mode or slots.status[slot] == M.READY then slots.status[slot] = 0 end   -- FFF10C < 0 or status == 6
    rec.active[slot] = p; slots.phase[slot] = 0
    slots.timer[slot] = M.load_steps(M.size(p, rec.ratings[p]), rec.load_io) * 10
end
-- 0x1962A: erase; program-specific RAM clears are the host's (game.program_unloaded):
--   3 Rebound: FF1592 b5, FFF0D9; 5 Shield: FFF0D2; 6 Smoke: FF1592 b2; 7 Mirrors: FF1592 b1, FFE169 [C addrs; names docs/05]
M.UNLOAD_EFFECT = { [3] = true, [5] = true, [6] = true, [7] = true }
function M.unload(rec, slots, slot) local p = rec.active[slot]; rec.active[slot] = -1; slots.status[slot] = M.READY; return p end
-- Storage erase cursor 0x18DE0: descending circular search for the next file (FFDD46); nil = none at all
function M.next_file(rec, from)
    for i = 0, M.FILES - 1 do local k = (from - i) % M.FILES; if rec.files[k].id >= 0 then return k end end
end
function M.erase_file(rec, k) rec.files[k].id = -1; rec.files[k].size = 0 end      -- 0x18F2A
function M.files(rec) local t = {}; for k = M.FILES - 1, 0, -1 do if rec.files[k].id >= 0 then t[#t + 1] = k end end; return t end  -- 0x18F7C order
-- Run Info 0x18C28: FFF0E3 < 0 on a run, 0 none, > 0 complete. Action text 0xE44C8[i]: FFFBF6 < 0 -> 1, 0 -> 0, > 0 -> 3;
-- FFFBF2 < 0 (CPU) -> index 2 [C 0x18C8C..0x18CD0]
function M.run_info(run_state, node_kind, action)
    if run_state == 0 then return "NONE" elseif run_state > 0 then return "COMPLETE" end
    local i = node_kind < 0 and 2 or (action < 0 and 1 or action == 0 and 0 or 3)
    return "ACTIVE", i
end
return M
