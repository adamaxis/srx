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

print(string.format("story_flags tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
