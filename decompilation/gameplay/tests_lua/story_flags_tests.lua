-- Named story-fact model and source audit for its ROM provenance.
local E = require("enums")
local StoryFacts = require("story_facts")
local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local state = {}
game = { state = state }
local ids = {}
for name, fact_id in pairs(E.StoryFact) do
    check(type(fact_id) == "string" and
          fact_id:match("^[a-z][a-z0-9_]*$") ~= nil,
          name .. " has a semantic fact identity")
    check(not ids[fact_id], fact_id .. " is unique")
    ids[fact_id] = true
    check(StoryFacts.is_known(fact_id), fact_id .. " is recognized")
end
check(not StoryFacts.is_known("story_flag_bit_7"),
      "ROM storage positions are not gameplay facts")
check(not StoryFacts.is_known("unknown_story_fact"),
      "unknown progression names are rejected")
StoryFacts.set(E.StoryFact.MOTEL_BILL_PAID, true)
check(StoryFacts.is_set(E.StoryFact.MOTEL_BILL_PAID) == true,
      "named fact can be recorded")
StoryFacts.set(E.StoryFact.MOTEL_BILL_PAID, false)
check(StoryFacts.is_set(E.StoryFact.MOTEL_BILL_PAID) == false,
      "named fact can be cleared")
check(not pcall(StoryFacts.set, "story_flag_bit_7", true),
      "raw storage identity cannot be written")
check(not pcall(StoryFacts.is_set, "unknown_story_fact"),
      "unknown fact cannot be queried")

-- ---- live triggers (story_triggers.lua) against the ROM [C] ----------------
local T = require("story_triggers_source")
local rom = assert(io.open("decompilation/input/shadowrun_genesis_us.bin", "rb")):read("a")
local function u16(a) return rom:byte(a + 1) << 8 | rom:byte(a + 2) end
local function u32(a) return u16(a) << 16 | u16(a + 2) end
do
    local handler = u32(0x12CCB4 + 4 * 0x44)
    check(handler == 0x21FD0, "cmd44 table entry resolves to ROM handler 0x21FD0")
    check(u16(handler) == 0x2639 and u32(handler + 2) == 0xFFFC1E and
          u16(handler + 6) == 0x08C3 and u16(handler + 8) == 30 and
          u16(handler + 10) == 0x23C3 and u32(handler + 12) == 0xFFFC1E,
          "cmd44 handler reads, sets bit 30, and writes FFFC1E")
    check(u16(handler + 26) == 0x41F9 and u32(handler + 28) == 0xF57F8,
          "cmd44 handler opens the Frosty meeting pages")
end
do
    local handler = u32(0x12CCB4 + 4 * 0x34)
    check(handler == 0x21CE4,
          "cmd34 table entry resolves to the Sinsearach state selector")
    check(u16(0x21CEA) == 0x0801 and u16(0x21CEC) == 0x000D and
          u16(0x21D10) == 0x0801 and u16(0x21D12) == 0x0011 and
          u16(0x21D18) == 0x0801 and u16(0x21D1A) == 0x0012 and
          u16(0x21D20) == 0x0800 and u16(0x21D22) == 0x0005 and
          u16(0x21D28) == 0x0801 and u16(0x21D2A) == 0x0013 and
          u16(0x21D30) == 0x0801 and u16(0x21D32) == 0x0014 and
          u16(0x21D38) == 0x0801 and u16(0x21D3A) == 0x001E and
          u16(0x21D48) == 0x41F9 and u32(0x21D4A) == 0xF57E4,
          "cmd34 checks bit13, story bits17/18/19/20/30, and item bit5 before root F57E4")
    check(u32(0x12CCB4 + 4 * 0x35) == 0x21DCC,
          "cmd35 is the separate 0x21DCC response handler")
end
do
    check(u32(0x15FD5C + 4 * 19) == 0x543A2,
          "indexed title service 19 resolves to the Ilene prerequisite gate")
    check(u16(0x543A2) == 0x0801 and u16(0x543A4) == 0x0012 and
          u16(0x543AA) == 0x0801 and u16(0x543AC) == 0x0013 and
          u16(0x543B2) == 0x0C39 and u16(0x543B4) == 0x0002 and
          u32(0x543B6) == 0xFFFBF8 and
          u16(0x543BE) == 0x0C39 and u16(0x543C0) == 0x0000 and
          u32(0x543C2) == 0xFFFBF9 and u16(0x543CA) == 0x4A39 and
          u32(0x543CC) == 0xFFF0D5 and
          u16(0x543D4) == 0x2239 and u32(0x543D6) == 0xFFFC34 and
          u16(0x543DA) == 0x0801 and u16(0x543DC) == 0x0000 and
          u16(0x543E2) == 0x2239 and u32(0x543E4) == 0xFFFC38 and
          u16(0x543E8) == 0x0801 and u16(0x543EA) == 0x000C and
          u16(0x543F8) == 0x13FC and u16(0x543FA) == 0x00FF and
          u32(0x543FC) == 0xFFE182,
          "service19 gates on bits18/19, Scene2/event0, FFF0D5, FFFC34 bit0, and FFFC38 bit12 before arming FFE182")
    check(u16(0xED3C) == 0x0829 and u16(0xED3E) == 0x0003 and
          u16(0xED40) == 0x009E and u16(0xED46) == 0x0829 and
          u16(0xED48) == 0x0007 and u16(0xED4A) == 0x00B0 and
          u16(0xED50) == 0x0828 and u16(0xED52) == 0x0001 and
          u16(0xED54) == 0x0010 and u16(0xEDB4) == 0x0828 and
          u16(0xEDB6) == 0x0003 and u16(0xEDB8) == 0x009E and
          u16(0xEDC8) == 0x0829 and u16(0xEDCA) == 0x0001 and
          u16(0xEDCC) == 0x0010,
          "0xECCC requires pair member +0x9E bit3 and partner +0x10 bit1 before this one-shot branch")
    check(u16(0xA82C) == 0x4A39 and u32(0xA82E) == 0xFFE182 and
          u16(0xA836) == 0x267C and u32(0xA838) == 0x0000A8CC and
          u16(0xA83C) == 0x23CB and u32(0xA83E) == 0xFFE124 and
          u16(0xA842) == 0x23D3 and u32(0xA844) == 0xFFE110,
          "pending marker FFE182 selects static entity list A8CC")
    check(u32(0xA8CC) == 0x800172CA and u32(0xA8D0) == 0x0000A8E4 and
          u32(0xA8D4) == 0x00000002 and u16(0xA8D8) == 0x9E20 and
          u16(0xA8DA) == 0x2E80 and u16(0xA8DC) == 0x9E08 and
          u16(0xA8DE) == 0x0000 and u16(0xA8E4) == 0x0E80 and
          u16(0xA8E6) == 0x0700 and u16(0x172CA) == 0x000A and
          u16(0x172D2) == 0x0003,
          "FFE182 list spawns the Scene-20 lone11 actor at 0E80/0700 with final +9E bit3")
    check(u16(0xED92) == 0x4A39 and u32(0xED94) == 0xFFE182 and
          u16(0xED9C) == 0x4239 and u32(0xED9E) == 0xFFE182 and
          u16(0xEDA2) == 0x23FC and u32(0xEDA4) == 0x54402 and
          u32(0xEDA8) == 0xFFDD2C,
          "eligible actor-pair contact consumes FFE182 and selects activity 0x54402")
    check(u16(0x54402) == 0x303C and u16(0x54404) == 2 and
          u16(0x54406) == 0x4EB9 and u32(0x54408) == 0x59C34 and
          u16(0x54412) == 0x08C1 and u16(0x54414) == 0x0013 and
          u16(0x54416) == 0x23C1 and u32(0x54418) == 0xFFFC1E and
          u16(0x5441C) == 0x2639 and u32(0x5441E) == 0xFFFC30 and
          u16(0x54426) == 0x23C3 and u32(0x54428) == 0xFFFC30 and
          u16(0x54432) == 0x0883 and u16(0x54434) == 0x0000 and
          u16(0x54436) == 0x08C3 and u16(0x54438) == 0x0001 and
          u16(0x5443A) == 0x23C3 and u32(0x5443C) == 0xFFFC34 and
          u16(0x54446) == 0x0883 and u16(0x54448) == 0x000C and
          u16(0x5444A) == 0x0883 and u16(0x5444C) == 0x000D and
          u16(0x5444E) == 0x23C3 and u32(0x54450) == 0xFFFC38 and
          u16(0x54464) == 0x41F9 and u32(0x54466) == 0x115ECC and
          u16(0x5446A) == 0x4BF9 and u32(0x5446C) == 0x122048,
          "Ilene activity awards two Karma, marks bit19, updates ritual facts, and opens group 115ECC")
end
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
-- The content interface records specific outcomes by name. These checks
-- prevent the old generic ROM command/trigger API from returning to gameplay.
local Json = require("json")
local conversation_files = {
    "fuchi_offices.json", "gates_undersound.json",
    "harlequin_main_conversation.json", "harlequin_map_information.json",
    "renraku_offices.json", "salish_owlfeather.json",
    "salish_sinsearach_gate.json", "salish_spirit_eyes.json",
    "stoker_motel.json", "tarislar_city_inn.json",
}
local fact_actions = {}
for _, filename in ipairs(conversation_files) do
    local file = assert(io.open(
        "decompilation/gameplay/dialogue/conversations/" .. filename, "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    for _, graph in pairs(document.graphs) do
        for _, node in pairs(graph.nodes) do
            if type(node.actionId) == "string" and
               node.actionId:match("^story%.") then
                check(node.actionId == "story.set_fact",
                      filename .. " uses only named story actions")
                local arguments = node.arguments or {}
                check(StoryFacts.is_known(arguments.factId) and
                      type(arguments.value) == "boolean",
                      filename .. " action names a known boolean fact")
                if arguments.factId then fact_actions[arguments.factId] = true end
                check(arguments.flagBit == nil and arguments.commandId == nil and
                      arguments.triggerId == nil,
                      filename .. " carries no ROM storage or command index")
            end
        end
    end
end
local required_content_facts = {
    "motel_bill_paid", "tarislar_asked_shale", "owlfeather_asked",
    "aragorn_found", "aragorn_freed_told", "shaman_met",
    "shaman_item4_given", "shaman_pelt_given", "serpent_scale_taken",
    "shaman_tasks_done", "frosty_met", "ogami_left", "post_ito_briefed",
    "renraku_map_shown", "both_maps_shown",
}
for _, fact_id in ipairs(required_content_facts) do
    check(fact_actions[fact_id] == true,
          fact_id .. " is an explicit dialogue outcome")
end

print(string.format("story fact tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
