-- clips.lua: Clips page 0x11B58, ROM-traced (out/menu_sound_sites.txt; disasm 0x11B58..0x12062) [C unless marked].
-- ONE-SHOT: every outcome prints a text, waits for any button edge 0xD94 (mask $FF00, silent) and returns to the
-- Command page via 0x11CB6 -> 0x11928. Cancel and the trade Start exit print "Function Aborted!" (0x11C9C, 0xD1ED8).
--   main loop 0x11BC6: Left/Right 0x25 (cycle FFF0D0 2..5); A/B/C 0x24 -> gate 0x12062 -> dispatch; NO Start check.
--   reload prompt 0x11D2A ("You still have ammo left. Reload?"): Left (else Right) 0x25 toggles FFF114; only A
--     confirms (0x24); FFF114 < 0 ("No") -> rts: silent exit, no text. No B/C/Start.
--   trade loop 0x11EA4: Left/Right 0x25; A/B/C 0x24 -> Increase 0x120E4(+1) / Decrease (-1) / Trade 0x11F90 /
--     Change recipient 0x1209A; Start -> 0x11C9C Function Aborted.
-- Rules: rules/clips.lua.
local E = require("enums")
local Clips = require("menu.rules.clips")
local P, C, TC, R, T, S, MM = E.Pad, E.ClipsCommand, E.ClipTradeCommand, E.MenuRegion, E.MenuTransition, E.MenuSound, E.MenuMessage
local AFTER = R.SIDEBAR_SLOT | R.COMBAT_BARS
local CONFIRM = P.A | P.B | P.C

local function active(ctx) return ctx.game.party()[ctx.game.active_slot()] end
local function cap(ctx) return ctx.game.magazine_capacity(active(ctx).weapon_code) end
local function commit(ctx, slot, record)
    ctx.game.set_runner_ammo(slot, record.clips, record.rounds)
end
local function commit_party(ctx, party)
    for slot = 0, 2 do
        local record = party[slot]
        if record then
            commit(ctx, slot, record)
        end
    end
end
-- 0x11CB6: text + 0xD94 wait (any edge, no sound), then back to Command
local function finish(msg) return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = msg, mask = P.EDGES } } end

local M = { window = E.WindowPlane.SIDEBAR }
function M.new() return { selection = C.RELOAD } end
function M.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function M.input(st, events, ctx)
    if events & P.LEFT ~= 0 or events & P.RIGHT ~= 0 then             -- Left tested first (0x11BD8) [C]
        st.selection = Clips.cycle(st.selection, events & P.LEFT ~= 0 and -1 or 1)
        ctx.sound(S.MOVE); ctx.redraw(R.SUBMENU); return
    end
    if events & CONFIRM == 0 then return end                           -- Start ignored [C]
    ctx.sound(S.SELECT)
    local rec = active(ctx)
    if st.selection == C.CANCEL then return finish(MM.FUNCTION_ABORTED) end
    local gate, reserve_cleared = Clips.gate(rec, st.selection)
    if gate then
        if reserve_cleared then commit(ctx, ctx.game.active_slot(), rec) end
        return finish(gate)
    end
    if st.selection == C.RELOAD then
        if rec.rounds ~= 0 then return { transition = T.PUSH, screen = E.MenuScreen.RELOAD_CONFIRM } end
        Clips.reload(rec, cap(ctx)); commit(ctx, ctx.game.active_slot(), rec)
        ctx.redraw(AFTER); return finish(MM.RELOADED)
    elseif st.selection == C.TRADE then
        local list = Clips.recipients(ctx.game.party(), ctx.game.active_slot())
        if #list == 0 then return finish(MM.NO_PARTY) end
        return { transition = T.PUSH, screen = E.MenuScreen.CLIP_TRADE, args = { recipients = list, quantity = rec.clips } }
    end
    local party = ctx.game.party()
    local distributed, changed = Clips.distribute(party)
    if not distributed then return finish(MM.NO_PARTY) end
    if changed then commit_party(ctx, party) end
    ctx.redraw(AFTER); return finish(MM.EXCHANGE_COMPLETE)
end
-- children report how the clips menu ends; nil message = silent exit (reload "No")
function M.child_returned(st, child, res, ctx)
    if res.message then return finish(res.message) end
    return { transition = T.POP }
end

local Confirm = { window = E.WindowPlane.SIDEBAR }
-- 0x11D00 clr FFF114, then the first draw 0x127B0 does eori #$FF => FFF114 < 0 => default "No" [C]
function Confirm.new() return { yes = false } end
function Confirm.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Confirm.input(st, events, ctx)
    if events & (P.LEFT | P.RIGHT) ~= 0 then st.yes = not st.yes; ctx.sound(S.MOVE); ctx.redraw(R.SUBMENU); return end
    if events & P.A == 0 then return end                               -- B/C/Start ignored [C 0x11D58..0x11D6E]
    ctx.sound(S.SELECT)
    if not st.yes then return { transition = T.POP, result = {} } end  -- rts, no text [C 0x11D84]
    local slot = ctx.game.active_slot()
    local rec = active(ctx)
    Clips.reload(rec, cap(ctx)); commit(ctx, slot, rec)
    ctx.redraw(AFTER)
    return { transition = T.POP, result = { message = MM.RELOADED } }
end

local Trade = { window = E.WindowPlane.SIDEBAR }
function Trade.new(a) return { recipients = a.recipients, index = 1, command = TC.INCREASE, quantity = a.quantity } end
function Trade.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Trade.input(st, events, ctx)
    if events & (P.LEFT | P.RIGHT) ~= 0 then
        if events & P.LEFT ~= 0 then st.command = st.command <= TC.INCREASE and TC.CHANGE_RECIPIENT or st.command - 1
        else st.command = st.command >= TC.CHANGE_RECIPIENT and TC.INCREASE or st.command + 1 end
        ctx.sound(S.MOVE); ctx.redraw(R.SUBMENU); return
    end
    if events & CONFIRM == 0 then
        if events & P.START ~= 0 then return { transition = T.POP, result = { message = MM.FUNCTION_ABORTED } } end  -- branches to 0x11C9C [C 0x11F7A]
        return
    end
    ctx.sound(S.SELECT)
    local party = ctx.game.party(); local src = party[ctx.game.active_slot()]
    if st.command == TC.INCREASE then st.quantity = st.quantity >= src.clips and 1 or st.quantity + 1          -- 0x120E4 wrap [srx H]
    elseif st.command == TC.DECREASE then st.quantity = st.quantity <= 1 and src.clips or st.quantity - 1
    elseif st.command == TC.CHANGE_RECIPIENT then st.index = st.index % #st.recipients + 1                   -- 0x1209A FFDD28 toggle
    else
        local dst = party[st.recipients[st.index]]
        if st.quantity == 0 or st.quantity > src.clips then return end
        src.clips = src.clips - st.quantity; dst.clips = (dst.clips + st.quantity) & 0xFF
        commit(ctx, ctx.game.active_slot(), src)
        commit(ctx, st.recipients[st.index], dst)
        ctx.redraw(AFTER)
        return { transition = T.POP, result = { message = MM.EXCHANGE_COMPLETE } }
    end
    ctx.redraw(R.SUBMENU)
end

return { Clips = M, Confirm = Confirm, Trade = Trade }
