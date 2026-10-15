-- rules/matrix_programs.lua: Rebound, Medic, Sleaze, Deception, Relocate, Analyze + trace bar, ROM-traced 2026-10-15.
-- state = 0x17C84 d1 result-panel code; fail = 0x19B34 fail animation. Same table views as matrix_combat. ic_type(node, alert) = 0x19BAE raw type. [C unless marked]
local MX = require("menu.rules.matrix")
local C = require("menu.rules.matrix_combat")
local M = {}
local P = { REBOUND = 3, MEDIC = 4, SLEAZE = 8, DECEPTION = 9, RELOCATE = 10, ANALYZE = 11 }   -- names 0xE2F46, dispatch 0xE2E0A
M.P = P
local function itype(node, st) return MX.ic_type(node, st.alert, true, 1) end

-- Rebound 0x1B8BC: fail -> state 7, unload, FFF0B6 = FF, sfx 0x20. Success -> charges FFF0D9 = successes.
function M.rebound(deck, decker, node, st, roll)
    local s = C.decker_test(deck, decker, node, st, P.REBOUND, roll)
    if s <= 0 then return { state = 7, unload = true } end
    st.rebound = s; return { active = true }
end
-- 0x56658 on an incoming IC attack (hits FFF0A4): charges > hits -> -1 charge, reflect; == -> reflect + unload;
-- < -> unload, attack lands. Reflect = 0x5668C: attack(IC, hits >> 1).
function M.rebound_hit(node, st, hits, party)
    local c = st.rebound or 0
    if c > hits then st.rebound = c - 1; C.attack(node, st, hits >> 1, party); return "reflected" end
    st.rebound = 0
    if c == hits then C.attack(node, st, hits >> 1, party); return "reflected_unload" end
    return "failed_unload"
end
-- Medic 0x19C76: dice = Medic rating, TN by persona: >= 8 -> 4, >= 5 -> 5, else 6. Heal persona += successes (cap 10).
function M.medic(deck, roll)
    local tn = deck.persona >= 8 and 4 or deck.persona >= 5 and 5 or 6
    local s = roll(deck.ratings[P.MEDIC], tn)
    if s <= 0 then return { fail = true } end
    deck.persona = math.min(10, deck.persona + s); return { state = 6, healed = s }
end
-- Sleaze 0x19FD0: refused (state 11) once the encounter is on (FFDF84). Else test + opposed vs Evasion:
-- fail, net > 0 -> FFF0A9 = FF (sneaks past) and FFE16B = net; net 0 -> state 10.
function M.sleaze(deck, decker, node, st, roll, encounter)
    if encounter then return { state = 11 } end
    local s = C.decker_test(deck, decker, node, st, P.SLEAZE, roll)
    if s < 0 then return { fail = true, unload = true } end
    local net = C.opposed(deck, node, st, s, roll)
    if net > 0 then st.sleazed = true; st.sleaze = net; return { ok = true, frames = 0x78 } end
    return { state = 10 }
end
-- Deception 0x1A030: refused once encountered (11); Barrier (1) / Black (8) immune (12). Net > 0 -> IC crashed (0x1A154).
function M.deception(deck, decker, node, st, roll, encounter)
    if encounter then return { state = 11 } end
    local t = itype(node, st); if t == 1 or t == 8 then return { state = 12 } end
    local s = C.decker_test(deck, decker, node, st, P.DECEPTION, roll)
    if s < 0 then return { fail = true, unload = true } end
    local net = C.opposed(deck, node, st, s, roll)
    if net > 0 then node.ic_condition = 0; return { state = 6, crashed = true } end
    return { state = 10 }
end
-- Relocate 0x1A0C4: only vs Trace IC (types 6/7, else 12). Net > 0 -> trace bar rewinds (FFE0C8) and IC crashed.
function M.relocate(deck, decker, node, st, roll)
    local t = itype(node, st); if t ~= 6 and t ~= 7 then return { state = 12 } end
    local s = C.decker_test(deck, decker, node, st, P.RELOCATE, roll)
    if s < 0 then return { fail = true, unload = true } end
    local net = C.opposed(deck, node, st, s, roll)
    if net > 0 then node.ic_condition = 0; st.trace_rewind = true; return { state = 6, crashed = true } end
    return { state = 10 }
end
-- Analyze 0x19D26: decker test (fail -> 0x19B34 code 11); then 0x56064: roll(node security +6, max(Sensor,2)),
-- net = s - that (floor 0); FFFB30[FFF102] |= min(net, 4) << 4 (high nibble = analyze level 0..4) [C].
function M.analyze(deck, decker, node, st, roll, fffb30, idx)
    local s = C.decker_test(deck, decker, node, st, P.ANALYZE, roll)
    if s < 0 then return { fail = true, unload = true } end
    local net = s - roll(node.security, math.max(deck.sensor, 2)); if net < 0 then net = 0 end
    local lv = net >= 5 and 4 or net
    if fffb30 then fffb30[idx] = (fffb30[idx] or 0) | (lv << 4) end
    return { ok = true, net = net, level = lv, frames = 0x3C }
end
-- Dump 0x1B302 (FFE084): 60 frames (FFE086) of screen shake: each frame 0x58 groups of 4 scroll words at FFCB40
-- get += (r & 0x1F, sign bit 7) x2 and ((r >> 8) & 0x1F, sign bit 15) x2, r = 0xD44 (seed*0x20E5+1); then
-- 0x1B5A6 zeroes the 0x1C0 words, FFE084 = 0 and 0x1B1B8 logs out [C]. Presenter-only: lives in C++.
M.DUMP_FRAMES = 0x3C
function M.dump_jitter(r)
    local a, b = r & 0x1F, (r >> 8) & 0x1F
    if r & 0x80 ~= 0 then a = -a end; if r & 0x8000 ~= 0 then b = -b end
    return a, b
end
-- Trace bar (0x1A958, 0x1A3F0, 0x1AB6E / 0x1AC6A). Type 7 -> bar A (FFE0B6, start 0x4800), type 6 -> bar B
-- (FFE0B8, start 0x47C0). 1 = shown, 2 = running at 0x2800 / (max(trace_left,1) * 0xF0) per frame.
-- pos >= 0x7140 -> FFE084 (dump: 0x1B302 then logout). Bar A also runs the 0x563CC MPCP burn first.
-- Relocate (FFE0C8): pos = FFE056 * 4 each frame; >= 0x6B00 -> bar cleared, FFE0F4 = 1.
M.TRACE_START, M.TRACE_END, M.REWIND_END = { [7] = 0x4800, [6] = 0x47C0 }, 0x7140, 0x6B00
function M.trace_frame(bar, st, rewind_pos)
    if st.trace_rewind then
        bar.pos = rewind_pos * 4
        if bar.pos >= M.REWIND_END then st.trace_rewind = false; bar.state = 0; return "cancelled" end
        return "rewinding"
    end
    if bar.state ~= 2 then return "idle" end
    bar.pos = bar.pos + bar.speed
    if bar.pos >= M.TRACE_END then return bar.type == 7 and "burn_dump" or "dump" end
    return "running"
end
-- 0x563CC burn: roll(ic_rating, max(MPCP,2)) - hardening -> MPCP loss; MPCP < 0 -> 0 and response 0; response -1.
function M.burn(deck, node, st, roll)
    local b = roll(MX.ic_rating(node, st.alert), math.max(deck.mpcp, 2))
    if b <= 0 then return 0 end
    b = b - deck.hardening; if b <= 0 then return 0 end
    deck.mpcp = deck.mpcp - b; if deck.mpcp < 0 then deck.mpcp = 0; deck.response = 0 end
    if deck.response > 0 then deck.response = deck.response - 1 end
    return b
end
return M
