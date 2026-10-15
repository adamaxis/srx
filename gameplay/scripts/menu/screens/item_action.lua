-- item_action.lua: inventory cell action row (0x11740 page<2 branch -> 0x11780.., selector in FFF0D0 = 2..5) plus its
-- children: discard prompt (0x12634..0x1275C, ROM-read), item trade, first aid, automatic use.
-- Rules: rules/inventory.lua + rules/first_aid.lua. Sounds: ROM where cited, otherwise srx FSR (6333-6720) [H].
local E = require("enums")
local Inv, FA = require("menu.rules.inventory"), require("menu.rules.first_aid")
local P, A, R, T, S = E.Pad, E.ItemAction, E.MenuRegion, E.MenuTransition, E.MenuSound
local DETAIL = R.SIDEBAR_SLOT | R.COMBAT_BARS | R.ITEM_PICTURE | R.ITEM_HEADER | R.RATING | R.DAMAGE_POWER | R.WEAPON_BARS | R.ARMOR

local function active_member(party, ctx)
    return party[ctx.game.active_slot()]
end
local function wrap(cur, left, right)
    -- ROM 0x117E6 tests Left first; simultaneous Left+Right therefore moves left.
    if left then return cur <= A.EQUIP_OR_USE and A.CANCEL or cur - 1 end
    if right then return cur >= A.CANCEL and A.EQUIP_OR_USE or cur + 1 end
    return cur
end

-- ---- action row ---------------------------------------------------------------------------------------------
local M = { window = E.WindowPlane.SIDEBAR }
function M.new(a) return { column = a.column, row = a.row, selector = A.EQUIP_OR_USE } end
function M.enter(st, ctx)
    st.sel = Inv.selected(active_member(ctx.game.party(), ctx),
                          st.column, st.row)
    ctx.redraw(R.BUTTON_STRIP | R.COMMAND_LABEL)
end
function M.input(st, events, ctx)
    local left, right = events & P.LEFT ~= 0, events & P.RIGHT ~= 0
    local nxt = wrap(st.selector, left, right)
    if nxt ~= st.selector then st.selector = nxt; ctx.sound(S.MOVE); ctx.redraw(R.BUTTON_STRIP | R.COMMAND_LABEL); return end
    -- ROM 0x11852..0x118A0: A/B confirm; C falls through; Start exits silently.
    if events & (P.A | P.B) == 0 then
        if events & P.START ~= 0 then return { transition = T.POP } end
        return
    end
    local party = ctx.game.party()
    local r = active_member(party, ctx)
    local sel = st.sel or Inv.selected(r, st.column, st.row)
    if not sel or st.selector == A.CANCEL then return { transition = T.POP } end
    ctx.sound(S.SELECT)
    if st.selector == A.EQUIP_OR_USE then
        local br = Inv.classify(sel.code, true)
        if br == E.ItemBranch.FIRST_AID then return { transition = T.REPLACE, screen = E.MenuScreen.FIRST_AID, args = { sel = sel } } end
        if br == E.ItemBranch.AUTOMATIC_USE then
            -- ROM 0x12168: codes 0x13/0x14/0x16/0x18..0x2B -> 0x121C0 prints "This item is automatically used." (0xD202F),
            -- waits 0xD94 and returns. The menu applies NO effect: these items act passively wherever their code is read [C]
            return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = E.MenuMessage.AUTOMATIC_USE, data = { sel = sel } } }
        end
        local cap = function(code) return ctx.game.magazine_capacity(code) end
        local m = Inv.toggle_equipment(r, sel, cap(r.weapon_code), cap(sel.code))
        if m ~= "invalid" and m ~= "rejected" then
            ctx.game.commit_party(party)
            ctx.redraw(DETAIL)
        end
        return { transition = T.POP }
    end
    if st.selector == A.DISCARD then
        if not Inv.can_discard(sel) then return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = E.MenuMessage.CANNOT_DISCARD, data = { sel = sel } } } end
        return { transition = T.REPLACE, screen = E.MenuScreen.DISCARD_CONFIRM, args = { sel = sel } }
    end
    -- TRADE
    if not Inv.can_trade(sel.code) then return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = E.MenuMessage.CANNOT_TRADE, data = { sel = sel } } } end
    local active = ctx.game.slot_active_set()
    local src = ctx.game.active_slot()
    local rec = Inv.next_recipient(active, src, src)
    if not rec then return { transition = T.POP } end
    return { transition = T.REPLACE, screen = E.MenuScreen.INVENTORY_TRADE, args = { sel = sel, source = src, recipient = rec } }
end

-- ---- discard prompt, ROM 0x12694..0x12758: default No; Left/Right toggle;
-- A/B/C confirm; Yes discards via 0x1275C. Start is not handled in the loop.
local Discard = { window = E.WindowPlane.SIDEBAR }
function Discard.new(a) return { sel = a.sel, yes = false } end
function Discard.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Discard.input(st, events, ctx)
    if events & (P.LEFT | P.RIGHT) ~= 0 then st.yes = not st.yes; ctx.sound(S.MOVE); ctx.redraw(R.SUBMENU); return end
    if events & (P.A | P.B | P.C) == 0 then return end
    ctx.sound(S.SELECT)
    if st.yes then
        local party = ctx.game.party()
        if Inv.discard(active_member(party, ctx), st.sel) then
            ctx.game.commit_party(party)
            ctx.redraw(DETAIL)
        end
    end
    return { transition = T.POP }
end

-- ---- item trade (srx inventory_action_conversion advance_inventory_trade; 0x127B0 default No) ----------------
local Trade = { window = E.WindowPlane.SIDEBAR }
local function pick(ctx, st, slot)
    st.recipient = slot
    st.dst_slot = Inv.free_slot(ctx.game.party()[slot])
    st.full = st.dst_slot == nil
    st.yes = false
end
function Trade.new(a) return { sel = a.sel, source = a.source, recipient = a.recipient } end
function Trade.enter(st, ctx) pick(ctx, st, st.recipient); ctx.redraw(R.SUBMENU) end
function Trade.input(st, events, ctx)
    local lr, confirm = events & (P.LEFT | P.RIGHT) ~= 0, events & (P.A | P.B | P.C) ~= 0
    if lr then ctx.sound(S.MOVE); st.yes = not st.yes; ctx.redraw(R.SUBMENU); return end
    if not confirm then return end
    ctx.sound(S.SELECT)
    if st.full or not st.yes then
        local nxt = Inv.next_recipient(ctx.game.slot_active_set(), st.source, st.recipient)
        if not nxt then return { transition = T.POP } end
        pick(ctx, st, nxt); ctx.redraw(R.SUBMENU); return
    end
    local party = ctx.game.party()
    if Inv.transfer(party[st.source], st.sel, party[st.recipient], st.dst_slot) then
        ctx.game.commit_party(party)
        ctx.redraw(DETAIL)
    end
    return { transition = T.POP }
end

-- ---- First Aid target list (ROM 0x1235C..0x1253C; effects at 0x1253E) -----------------------------
local FirstAid = { window = E.WindowPlane.SIDEBAR }
function FirstAid.new(a) return { sel = a.sel, row = 0 } end
function FirstAid.enter(st, ctx) st.targets = FA.targets(ctx.game.party()); ctx.redraw(R.SUBMENU) end
function FirstAid.input(st, events, ctx)
    if events & (P.UP | P.DOWN) ~= 0 then
        st.row = FA.cursor(st.row, #st.targets, events & P.UP ~= 0 and -1 or 1); ctx.sound(S.MOVE); ctx.redraw(R.SUBMENU); return
    end
    -- ROM 0x12402, 0x1250C, and 0x1251C route A/B/C to the same confirmation;
    -- Start returns silently at 0x1252C.
    if events & (P.A | P.B | P.C) == 0 then
        if events & P.START ~= 0 then return { transition = T.POP } end
        return
    end
    ctx.sound(S.SELECT)
    if st.row >= #st.targets then return { transition = T.POP } end            -- Cancel row
    local party = ctx.game.party()
    local target_slot = st.targets[st.row + 1]
    local msg = FA.use(active_member(party, ctx), st.sel.slot, party[target_slot],
                       function(n, tn) return ctx.game.roll_successes(n, tn) end)
    if not msg then return { transition = T.POP } end
    if msg ~= E.MenuMessage.NOT_INJURED then
        ctx.game.commit_party(party)
        ctx.redraw(DETAIL)
    end
    return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = msg, data = { target = target_slot, targets = st.targets, sel = st.sel } } }
end

return { ItemAction = M, Discard = Discard, Trade = Trade, FirstAid = FirstAid, wrap = wrap }
