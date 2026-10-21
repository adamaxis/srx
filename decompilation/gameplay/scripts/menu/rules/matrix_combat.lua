-- rules/matrix_combat.lua: Matrix dice, programs and IC turns, ROM-traced 2026-10-15 [C unless marked].
-- Inputs are plain tables: deck (FFFBBE view: mpcp +0, hardening +1, response +7, bod +8, evasion +9, masking +0x0A,
-- sensor +0x0B, persona +0x0C, ratings[0..11] +0x0E), node (FF1100 view: colour +5, security +6, ic_condition +0x0F,
-- ic_reaction +0x10, ic_rating/ic_rating2/ic_code), decker (leader: computer +0x88, d81c = 0xD81C value, physical +0x4C),
-- st (Matrix RAM: alert FFF0A0, mode FFF0B7, mirrors FFE169, smoke FFE16A, shield FFF0D2, karma_counter FFF0CE,
-- passive FFF0A3, trace_on FFF0E6, trace_left FFF0E7). roll(dice, tn) = 0xDBA successes (RNG stays in C++).
local MX = require("menu.rules.matrix")
local M = {}
M.PROGRAM = { ATTACK = 0, SLOW = 1, DEGRADE = 2 }
M.RESULT = { FAIL = 7, BLOCKED = 8, SUCCESS = 9 }
local function s8(v) v = v & 0xFF; return v >= 0x80 and v - 0x100 or v end
local function min2(tn) return tn < 2 and 2 or tn end

-- 0x55F44 hacking pool. base = 0xD81C + Computer + 2*Response. defense = d1 < 0 (clamped to MPCP), offense = d1 == 0.
-- FFF0B7: A (FF) offense gets base - base//3, defense base//3; C (1) offense base//3, defense base - base//3;
-- B (0) both base >> 1, defense rounds up. => FFF0B7 = attack / balanced / defend split [C].
function M.pool(deck, decker, mode, defense)
    local base = decker.d81c + decker.computer + 2 * deck.response
    local third = base // 3
    local v
    if mode > 0 then v = defense and base - third or third
    elseif mode < 0 then v = defense and third or base - third
    else v = base >> 1; if defense and base & 1 == 1 then v = v + 1 end end
    if defense and v > deck.mpcp then v = deck.mpcp end
    return v
end
-- TN modifiers: 0x562DE decker/IC-defence rolls +2*Smoke; 0x562D4 IC attack/trace rolls +Mirrors +2*Smoke.
function M.tn_decker(tn, st) return min2(tn + 2 * st.smoke) end
function M.tn_ic(tn, st) return min2(tn + st.mirrors + 2 * st.smoke) end

-- 0x55FD8 decker test for program p: dice = min(pool, rating) + rating (0 -> 1) - (colour - 1); <= 0 -> no roll.
-- Returns successes, or -1 when none (leader +0x9F).
function M.decker_test(deck, decker, node, st, p, roll)
    local r = deck.ratings[p]
    local pool = M.pool(deck, decker, st.mode, false)
    local dice = math.min(pool, r) + r; if dice == 0 then dice = 1 end
    dice = dice - (node.colour - 1)
    local s = 0
    if dice > 0 then s = roll(dice, M.tn_decker(node.security, st)) end
    return s > 0 and s or -1
end
-- 0x56078 opposed: IC rating dice vs Evasion (min 2, no Mirrors/Smoke); net = s - ic_s floored 0.
function M.opposed(deck, node, st, s, roll)
    local ic = roll(MX.ic_rating(node, st.alert), min2(deck.evasion))
    local n = s - ic; return n < 0 and 0 or n
end
-- 0x19E1A / 0x19E84 / 0x19F00 + 0x19E32..: outcome -1 fail, 0 blocked, > 0 net. Program run state FFF0E8 = 7/8/9.
function M.run_program(deck, decker, node, st, p, roll)
    local s = M.decker_test(deck, decker, node, st, p, roll)
    local net = s < 0 and -1 or M.opposed(deck, node, st, s, roll)
    return net, net < 0 and M.RESULT.FAIL or
                net == 0 and M.RESULT.BLOCKED or M.RESULT.SUCCESS
end
local function ic_defeated(st, party)                 -- shared FFF0CE rollover
    st.karma_counter = s8(st.karma_counter - 2)
    if st.karma_counter < 0 then
        st.karma_counter = 15
        for i = 0, 2 do local r = party[i]; if r then r.karma = (r.karma + 1) & 0xFF end end   -- FF018D/028D/038D
    end
end
-- Attack program table entry 0x1B734 (table 0xE2E0A). Its result path uses
-- 0x19E32 and the shared 0x56434 helper to reduce selected node +0x0F.
function M.attack(node, st, net, party)
    st.passive = 0
    node.ic_condition = s8(node.ic_condition - net)
    if node.ic_condition <= 0 then node.ic_condition = 0; ic_defeated(st, party); return true end
    return false
end
-- Slow program table entry 0x1B696 (table 0xE2E0A). Result path 0x19EA6
-- sends 2*net + floor(Slow rating/2) through 0x56482 to node +0x10; a
-- negative signed result clears +0x0F and uses the shared FFF0CE rollover.
function M.slow(deck, node, st, net, party)
    st.passive = 0
    node.ic_reaction = s8(node.ic_reaction - (2 * net + (deck.ratings[1] >> 1)))
    if node.ic_reaction < 0 then node.ic_reaction = 0; node.ic_condition = 0; ic_defeated(st, party); return true end
    return false
end
-- Degrade program table entry 0x1B81A (table 0xE2E0A). Result handler
-- 0x19F18 passes positive net to the shared 0x564D6 current-node helper.
function M.degrade(node, net)
    local d = net >> 1; if d <= 0 then d = 1 end
    node.security = node.security - d; if node.security < 2 then node.security = 2 end
end
-- Mirrors 0x1B7EC / Smoke 0x1C1C0: value = rating, -1 every 0xF0 frames (0x1C1F6 / 0x1B45C); at 0 the program unloads.
M.SHROUD_PERIOD = 0xF0
function M.tick_shroud(value, timer)                  -- returns value, timer, expired
    timer = timer - 1
    if timer >= 0 then return value, timer, false end
    value = value - 1
    return value, M.SHROUD_PERIOD, value == 0
end

-- IC turn 0x561E8. Reaction recovers +1 toward the ROM value. Kind by +0x13 code:
--   < 2 passive; 2..4 attack; 5..6 trace; 7..10 trace if alert ACTIVE else passive; 11..16 attack if ACTIVE else passive;
--   17..20 passive; >= 21 attack.
function M.ic_kind(code, alert)
    if code < 2 then return "passive" elseif code < 5 then return "attack" elseif code < 7 then return "trace"
    elseif code < 11 then return alert > 0 and "trace" or "passive"
    elseif code < 17 then return alert > 0 and "attack" or "passive"
    elseif code < 21 then return "passive" end
    return "attack"
end
-- IC action interval 0x17E78: (0x1B - (reaction + alert bonus)) * 20 frames (gated by FFDF84 and FFE076 == 0, 0x1A396).
function M.ic_interval(node, alert) return (0x1B - (node.ic_reaction + (alert < 0 and 0 or alert == 0 and 1 or 2))) * 20 end

-- Passive IC 0x56116 (FFF0A3: 0 idle, <0 probing, >0 reset). Returns "none" | "probe" | resolved table.
function M.passive_step(deck, decker, node, st, roll)
    if st.passive > 0 then st.passive = 0; return "none" end
    if st.passive == 0 then
        if st.alert == MX.ACTIVE then return "none" end
        st.passive = -1; return "probe"                                             -- d0 = 2: probe animation
    end
    local ic = roll(MX.ic_rating(node, st.alert), M.tn_decker(deck.masking, st))
    local d = roll(M.pool(deck, decker, st.mode, true) + decker.computer, M.tn_decker(node.security, st))
    local raised, overload = false, false
    if ic - d > 0 then raised = true; st.alert, overload = MX.raise_alert(st.alert, 1, st.overload_armed) end
    st.passive = 0
    return { raised = raised, overload = overload, reencounter = st.alert > 0 }      -- > 0: 0x1A908; else FFDF84 = 0
end
-- Trace 0x560AA: start if idle: s = roll(rating, Evasion + Mirrors + 2*Smoke); s >= 1 -> trace_left = 11 - s, bar speed
-- 0x2800 / (trace_left * 0xF0) (0x1A3F0); running: trace_left -= 1. Completion: see matrix_programs.trace_frame [C].
function M.trace_step(deck, node, st, roll)
    if st.trace_on then st.trace_left = st.trace_left - 1; return "running" end
    local s = roll(MX.ic_rating(node, st.alert), M.tn_ic(deck.evasion, st))
    if s - 1 < 0 then return "none" end
    st.trace_left = 10 - (s - 1); st.trace_on = true
    return "started", 0x2800 // (math.max(st.trace_left, 1) * 0xF0)
end
-- Attack IC 0x56268 roll + 0x562EC resolve. Returns events table.
function M.ic_attack(deck, decker, node, st, roll)
    local black = MX.ic_type(node, st.alert, true, 1) == 8
    local hits = roll(MX.ic_rating(node, st.alert), M.tn_ic(deck.bod, st))
    if hits <= 0 then return { miss = true } end
    local def = roll(M.pool(deck, decker, st.mode, true) + deck.mpcp, M.tn_decker(node.security, st))
    local net = hits - def
    if net == 0 then return { even = true } end
    if net < 0 then return { dodged = true } end                                       -- FFE07E = 1
    local dmg = deck.hardening - net
    if dmg >= 0 then return { absorbed = true } end
    if black then                                                                        -- 0x17D90 leader physical
        decker.physical = math.max(0, math.min(10, decker.physical + dmg)); return { physical = dmg }
    end
    if st.shield > 0 then                                                                -- 0x1A12E
        dmg = dmg + st.shield; st.shield = st.shield - 1
        if st.shield <= 0 then st.shield = 0; st.shield_gone = true end
        if dmg >= 0 then return { shielded = true } end
    end
    deck.persona = math.max(0, math.min(10, deck.persona + dmg))                         -- 0x17DB8
    local ev = { persona = dmg }
    if deck.persona == 0 and MX.ic_type(node, st.alert, true, 1) == 3 then             -- Blaster: MPCP burn 0x563CC
        local b = roll(MX.ic_rating(node, st.alert), min2(deck.mpcp)) - deck.hardening
        if b > 0 then
            deck.mpcp = deck.mpcp - b; if deck.mpcp < 0 then deck.mpcp = 0; deck.response = 0 end
            if deck.response > 0 then deck.response = deck.response - 1 end
            ev.mpcp = b
        end
    end
    return ev
end
-- 0x1AF3E: persona 0 or leader physical 0 -> FFE084 dump sequence (0x1B302) then logout 0x1B1B8.
function M.crashed(deck, decker) return deck.persona == 0 or decker.physical == 0 end
return M
