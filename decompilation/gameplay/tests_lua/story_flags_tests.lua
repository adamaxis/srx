-- story_flags.lua: FFFC1E / FFDD64 rule (0x2196C) against a host stub that
-- mirrors FieldSceneRuntime's story-fact accessors.
local E = require("enums")

local host = { fffc1e = 0, ffdd64 = nil }
local function reset(flags, trigger) host.fffc1e, host.ffdd64 = flags, trigger end
game = { state = setmetatable({}, {
    __index = function(_, name)
        local bit = name:match("^story_flag_bit_(%d+)$")
        if bit then return (host.fffc1e >> tonumber(bit)) & 1 == 1 end
        local trig = name:match("^story_trigger_active_(%d+)$")
        if trig then
            if host.ffdd64 == nil then return nil end
            return host.ffdd64 == tonumber(trig)
        end
        return nil
    end,
    __newindex = function(_, name, value)
        local bit = name:match("^story_flag_bit_(%d+)$")
        if bit then
            local m = 1 << tonumber(bit)
            host.fffc1e = value and (host.fffc1e | m) or (host.fffc1e & ~m)
            return
        end
        local trig = name:match("^story_trigger_active_(%d+)$")
        if trig and value then host.ffdd64 = tonumber(trig); return end
        error("unwritable fact " .. name)
    end,
}) }

local S = require("story_flags")
local pass, total = 0, 0
local function check(cond, label)
    total = total + 1
    if cond then pass = pass + 1 else print("FAIL " .. label) end
end

reset(0, 5)
check(S.active_trigger() == 5, "FFDD64 read back")
check(S.apply_dialogue_command(0x15) == E.StoryFlag.SHAMAN_MET, "cmd15 sets the active trigger's bit")
check(host.fffc1e == 1 << 5, "only bit 5 set")

reset(0, 0)
check(S.apply_dialogue_command(0x05) == nil, "cmd05 unpaid branch does not mark")
check(S.apply_dialogue_command(0x05, { paid = true }) == E.StoryFlag.MOTEL_BILL_PAID, "cmd05 paid marks trigger 0")
check(S.is_set(E.StoryFlag.MOTEL_BILL_PAID) == true, "is_set reads FFFC1E")

reset(0, 3)
check(S.apply_dialogue_command(0x1C) == E.StoryFlag.MICHAEL_MISSION_TOLD, "cmd1C uses fixed 0x0D")
check(host.ffdd64 == 13, "cmd1C overwrites FFDD64")
check(S.apply_dialogue_command(0x1D) == 13, "later cmd1D sees the overwritten FFDD64")
check(host.fffc1e & (1 << 3) == 0, "original trigger bit untouched")

reset(0, 1)
check(S.apply_dialogue_command(0x46, { branch = 2 }) == E.StoryFlag.BOTH_MAPS_SHOWN, "cmd46 second branch")
check(S.apply_dialogue_command(0x61) == E.StoryFlag.HOSPITAL_VISITED, "cmd61 fixed 0x19")
check(S.apply_dialogue_command(0x30) == nil, "unrelated command leaves flags alone")

reset(0, nil)
check(not pcall(S.mark_active_trigger), "missing FFDD64 is an error, not bit 0")
check(S.is_set(31) == false and S.name(0) == "MOTEL_BILL_PAID", "name lookup")
check(not pcall(S.set, 32), "bit range is checked")

for cmd, mark in pairs(S.DIALOGUE_COMMAND_MARKS) do
    if type(mark) == "number" then
        check(S.name(mark) ~= nil, string.format("cmd %02X fixed index is a named StoryFlag", cmd))
    end
end

-- ---- live triggers (story_triggers.lua) against the ROM [C] ----------------
local T = require("story_triggers")
local rom = assert(io.open("decompilation/input/shadowrun_genesis_us.bin", "rb")):read("a")
local function u16(a) return rom:byte(a + 1) << 8 | rom:byte(a + 2) end
local function u32(a) return u16(a) << 16 | u16(a + 2) end
do
    local ok, live_count = true, 0
    for i = 0, T.COUNT - 1 do
        local h = u32(T.TABLE + 4 * i)
        if T.LIVE[i] then live_count = live_count + 1; if T.LIVE[i].handler ~= h then ok = false end
        elseif h ~= T.EMPTY_HANDLER then ok = false end
    end
    check(ok and live_count == 13, "live set 0-4,10,12,14,15,18,19,21,22 == ROM 0x15FD5C")
end
-- lea.l abs.l,a0 (41F9) occurrences in a byte range
local function leas(from, to)
    local out = {}
    for a = from, to - 6, 2 do if u16(a) == 0x41F9 then out[u32(a + 2)] = true end end
    return out
end
local starts = {}
for _, i in ipairs(T.live_indices()) do starts[#starts + 1] = T.LIVE[i].handler end
table.sort(starts)
local function next_start(h) for _, s in ipairs(starts) do if s > h then return s end end; return 0x54700 end
-- command handler continuations: lea abs,a0 up to rts / jmp / bra.w 0x2196C
local function cmd_leas(h)
    local out = {}
    for a = h, h + 0x200, 2 do
        local w = u16(a)
        if w == 0x41F9 then out[#out + 1] = u32(a + 2) end
        if w == 0x4E75 or w == 0x4EF9 or w == 0x4ED0 then break end
        if w == 0x6000 and a + 2 + ((u16(a + 2) ~ 0x8000) - 0x8000) == 0x2196C then break end
    end
    return out
end
local function walk(roots)
    local seen, cmds, stack = {}, {}, {}
    for _, r in ipairs(roots) do stack[#stack + 1] = r end
    while #stack > 0 do
        local a = table.remove(stack)
        if not seen[a] and a >= 0xE0000 and a < 0x160000 then
            seen[a] = true
            if u32(a + 4) ~= 0xFFFFFFFF then
                local ob, oc = u32(a + 8), u32(a + 12)
                local nexts = { u32(a + 16) }
                if ob ~= 0 then nexts[#nexts + 1] = u32(a + 20) end
                if ob ~= 0 and oc ~= 0 then nexts[#nexts + 1] = u32(a + 24) end
                for _, v in ipairs(nexts) do
                    if v < 0xFA then
                        if not cmds[v] then
                            cmds[v] = true
                            for _, n in ipairs(cmd_leas(u32(0x12CCB4 + 4 * v))) do stack[#stack + 1] = n end
                        end
                    else stack[#stack + 1] = v end
                end
            end
        end
    end
    return cmds
end
for _, i in ipairs(T.live_indices()) do
    local L = T.LIVE[i]
    local in_body = leas(L.handler, next_start(L.handler))
    local roots_ok = true
    for _, r in ipairs(L.roots) do if not in_body[r] then roots_ok = false end end
    check(roots_ok, string.format("trigger %d roots are lea a0 in handler %06X", i, L.handler))
    local got, want, same = walk(L.roots), {}, true
    for _, c in ipairs(L.commands) do want[c] = true end
    for c in pairs(got) do if not want[c] then same = false end end
    for c in pairs(want) do if not got[c] then same = false end end
    check(same, string.format("trigger %d reachable commands == ROM graph", i))
    for _, site in ipairs(L.tail or {}) do
        check(u16(site) == 0x4EB9 and u32(site + 2) == 0x2196C, string.format("trigger %d tail %06X is jsr 0x2196C", i, site))
    end
    for _, p in ipairs(L.preset or {}) do
        check(u16(p.site - 10) == 0x23FC and u32(p.site - 8) == p.index and u32(p.site - 4) == 0xFFDD64
              and u16(p.site) == 0x4EB9 and u32(p.site + 2) == 0x2196C,
              string.format("trigger %d preset FFDD64=%d then jsr 0x2196C at %06X", i, p.index, p.site))
    end
end
-- Every live trigger: begin_trigger publishes FFDD64; each reachable story
-- command and the handler tail mark exactly the expected FFFC1E bits [C].
local F = E.StoryFlag
local expected = {
    [0]  = { commands = { [0x05] = F.MOTEL_BILL_PAID } },
    [1]  = { commands = { [0x16] = F.TARISLAR_ASKED_SHALE } },
    [2]  = { commands = { [0x17] = F.OWLFEATHER_ASKED } },
    [3]  = { commands = {} },
    [4]  = { commands = { [0x19] = F.ARAGORN_FREED_TOLD }, tail = F.ARAGORN_FREED_TOLD },
    [10] = { commands = {} }, [12] = { commands = {} }, [14] = { commands = {} },
    [15] = { commands = {}, tail = F.OGAMI_LEFT },
    [18] = { commands = {}, tail = F.RED_BUFFALO_WOMAN },
    [19] = { commands = {} },
    [21] = { commands = { [0x40] = F.POST_ITO_BRIEFED, [0x46] = { F.RENRAKU_MAP_SHOWN, F.BOTH_MAPS_SHOWN },
                          [0x47] = { F.RENRAKU_MAP_SHOWN, F.BOTH_MAPS_SHOWN } },
             preset = { F.POST_ITO_BRIEFED, F.GARDEN_APTS_ENTERED } },
    [22] = { commands = {}, tail = F.LANDMARK_MAP_TAKEN },
}
for _, i in ipairs(T.live_indices()) do
    local x = expected[i]
    local story = T.story_commands(i, S.DIALOGUE_COMMAND_MARKS)
    local n_expected = 0; for _ in pairs(x.commands) do n_expected = n_expected + 1 end
    check(#story == n_expected, string.format("trigger %d: %d story commands reachable", i, n_expected))
    for _, c in ipairs(story) do
        local want = x.commands[c]
        if type(want) == "table" then
            for branch = 1, 2 do
                reset(0, nil); S.begin_trigger(i)
                local bit = S.apply_dialogue_command(c, { branch = branch })
                check(bit == want[branch] and host.fffc1e == 1 << want[branch],
                      string.format("trigger %d cmd %02X branch %d -> bit %d", i, c, branch, want[branch]))
            end
        else
            reset(0, nil); S.begin_trigger(i)
            local bit = S.apply_dialogue_command(c, { paid = true })
            check(bit == want and host.fffc1e == 1 << want, string.format("trigger %d cmd %02X -> bit %s", i, c, tostring(want)))
        end
    end
    reset(0, nil); S.begin_trigger(i)
    check(host.ffdd64 == i, string.format("begin_trigger(%d) publishes FFDD64", i))
    local tail = S.finish_trigger(i)
    check(tail == x.tail and host.fffc1e == (x.tail and 1 << x.tail or 0), string.format("trigger %d tail marks %s", i, tostring(x.tail)))
    for k, p in ipairs(T.LIVE[i].preset or {}) do
        reset(0, nil); S.begin_trigger(i)
        check(S.mark_fixed(p.index) == x.preset[k] and host.ffdd64 == x.preset[k], string.format("trigger %d preset %d", i, p.index))
    end
end
check(not pcall(S.begin_trigger, 5) and not pcall(S.begin_trigger, 23), "begin_trigger rejects no-op / out-of-table indices")

print(string.format("story_flags tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
