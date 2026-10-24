-- command.lua: the Start/Command overview, ROM 0x10D30..0x10F4E plus page helpers 0x114A0/0x11506/0x11740.
-- State mirrors the ROM bytes: page = FFF0D0, row = FFF0D1, posture_gate = FFF115, panel_dirty = FFF112.
local E = require("enums")
local Inv = require("menu.rules.inventory")
local P, Page, R, T, S = E.Pad, E.MenuPage, E.MenuRegion, E.MenuTransition, E.MenuSound

local M = { window = E.WindowPlane.SIDEBAR }   -- [C] 0x10D30 never writes VDP reg 0x11 (docs/09 s11)

-- 0x1170C + 0x111E8 after every move / refresh: details, bars, posture, buttons, label (docs/09 s10.1) [C regions]
M.REFRESH = R.ITEM_PICTURE | R.ITEM_HEADER | R.RATING | R.DAMAGE_POWER | R.COMBAT_BARS | R.WEAPON_BARS
          | R.ARMOR | R.POSTURE_BLOCK | R.BUTTON_STRIP | R.COMMAND_LABEL

local CHILD = {   -- 0xD2A0A jump table, indexed page-2 [C]
    [Page.CLIPS] = E.MenuScreen.CLIPS, [Page.ATTRIBUTES_SKILLS] = E.MenuScreen.ATTRIBUTES_SKILLS,
    [Page.CYBERWARE] = E.MenuScreen.CYBERWARE, [Page.MAGIC] = E.MenuScreen.MAGIC,
    [Page.POCKET_SECRETARY] = E.MenuScreen.POCKET_SECRETARY,
}

function M.new() return { page = Page.POCKET_SECRETARY, row = 0, posture_gate = false, panel_dirty = false } end

local function leader(ctx) return ctx.game.party()[ctx.game.active_slot()] end

-- 0x10D36/0x10D40: btst #6 / #4 on leader +0x10 (DEAD / DOWN), then 0x391A [C].
-- party().status is the +0x10 byte. The old C++ binding tested the +0x11 byte.
local function reselect_if_incapacitated(ctx)
    local r = leader(ctx)
    local down = E.RunnerStatus.DEAD | E.RunnerStatus.DOWN
    if r and r.status & down ~= 0 then ctx.game.reselect_leader() end
end

-- Full build: 0x10D30 (display off ... on) then 0x10E2E. [C]
-- 0x10D30 display-off span, emulator-measured [C emulator]: from the field the
-- display drops mid-frame 8600, then 8601..8627 black (27), reveal at line 175
-- on 8628; on a return the frame of the input is already black: 8720..8747 (28).
local BLANK_OPEN, BLANK_RETURN, BLANK_REVEAL_LINE = 27, 28, 175
-- Command OPEN masks, emulator e2 8600/8628/8629 [C]. The first field frame
-- keeps its upper rows, then the two reveal frames expose menu text in stages.
local OPEN = {
    lead = { { 0, 30, 320, 194 } },
    reveal = {
        [0] = {
            { 0, 168, 56, 8 }, { 80, 168, 40, 8 }, { 240, 168, 40, 8 },
            { 312, 168, 8, 8 }, { 16, 176, 24, 8 }, { 96, 176, 24, 8 },
            { 152, 176, 56, 8 }, { 240, 176, 24, 8 }, { 56, 192, 24, 8 },
            { 56, 200, 24, 8 }, { 112, 200, 48, 8 }, { 168, 200, 72, 8 },
        },
        [1] = {
            { 128, 24, 40, 8 }, { 176, 24, 48, 8 }, { 128, 40, 64, 8 },
            { 200, 40, 32, 8 }, { 128, 48, 48, 8 }, { 200, 48, 32, 8 },
            { 136, 64, 40, 8 }, { 184, 64, 32, 8 }, { 184, 72, 40, 8 },
            { 112, 80, 48, 8 }, { 168, 80, 56, 8 }, { 112, 104, 24, 8 },
            { 144, 104, 104, 8 }, { 112, 112, 24, 8 }, { 144, 112, 104, 8 },
        },
    },
}
-- Return from Attributes/Skills (exit 0xD4A0 already blanked two staged
-- frames) [C emulator e2]: 8962..8988 black (27), reveal at line 211 on 8989;
-- the display comes on mid-line, so row 211 is black left of x=218; on 8990
-- the item-detail block (x 112..247, rows 24..131) is not drawn yet.
local AS_RETURN = { black = 27, line = 211,
                    reveal = { [0] = { { 0, 211, 218, 1 } }, [1] = { { 112, 24, 136, 108 } } } }
local function build(st, ctx, full, black_frames, reveal_line, reveal, lead)
    if full then
        reselect_if_incapacitated(ctx)
        st.page, st.row = Page.POCKET_SECRETARY, 0          -- 0x10F50 [C]
        ctx.host.blank(E.DisplayBlank.OFF)
        ctx.redraw(R.FULL)
        for _, r in ipairs(lead or {}) do
            ctx.host.mask(E.MenuMask.LEAD, 0, r[1], r[2], r[3], r[4])
        end
        for frame, rects in pairs(reveal or {}) do
            for _, r in ipairs(rects) do ctx.host.mask(E.MenuMask.REVEAL, frame, r[1], r[2], r[3], r[4]) end
        end
        ctx.host.blank(E.DisplayBlank.ON, black_frames or BLANK_RETURN, reveal_line or BLANK_REVEAL_LINE)
    else
        ctx.redraw(M.REFRESH)                              -- 0x10D90: panels without blanking [C]
    end
    st.panel_dirty = true                                   -- FFF112=FF [C value, H meaning]
    ctx.sound(S.MOVE)                                       -- 0x10E2E plays 0x25 even on open [C]
    ctx.redraw(M.REFRESH)
end

function M.enter(st, ctx) build(st, ctx, true, BLANK_OPEN, BLANK_REVEAL_LINE, OPEN.reveal, OPEN.lead) end

-- 0x11592: inventory cursor sprite at (page*0x28, row*0x18) + item detail [C]
local function inventory_cursor(st, ctx) ctx.redraw(R.INVENTORY_CURSOR | R.ITEM_PICTURE | R.ITEM_HEADER) end

-- 0x114A0 [C]
function M.step_page(st, d, ctx)
    local p = st.page + d
    if p < 0 then p = Page.POCKET_SECRETARY; st.panel_dirty = true end
    if p == Page.COUNT then p = Page.INVENTORY_LEFT end
    if p == Page.CLIPS and d >= 0 then st.panel_dirty = true end
    st.page = p
    if p < Page.CLIPS then inventory_cursor(st, ctx) else ctx.redraw(R.BUTTON_STRIP) end
end

-- 0x11506 [C]; returns a PUSH of the posture focus when the ROM would enter 0x119CE
function M.step_row(st, d, events, ctx)
    if st.page < Page.CLIPS then
        local r = st.row + d
        if r < 0 then r = 0 elseif r >= E.InventoryGrid.ROWS then r = E.InventoryGrid.ROWS - 1 end
        st.row = r
        inventory_cursor(st, ctx)
        return nil
    end
    if events & (P.HELD_DOWN | P.HELD_LEFT | P.HELD_RIGHT) ~= 0 then return nil end
    if st.posture_gate then st.posture_gate = false; return nil end   -- consumed by the C path [C]
    return { transition = T.PUSH, screen = E.MenuScreen.POSTURE }
end

local function after_move(st, ctx) ctx.sound(S.MOVE); ctx.redraw(M.REFRESH) end

function M.input(st, events, ctx)
    -- priority order of 0x10DD8..0x10F2A [C]
    if events & P.LEFT ~= 0 then M.step_page(st, -1, ctx); after_move(st, ctx); return end
    if events & P.UP ~= 0 then
        local r = M.step_row(st, -1, events, ctx); if r then return r end
        after_move(st, ctx); return
    end
    if events & P.RIGHT ~= 0 then M.step_page(st, 1, ctx); after_move(st, ctx); return end
    if events & P.DOWN ~= 0 then
        local r = M.step_row(st, 1, events, ctx); if r then return r end
        after_move(st, ctx); return
    end
    if events & (P.A | P.B) ~= 0 then
        ctx.sound(S.SELECT)                                  -- 0x10E4E [C]
        local child = CHILD[st.page]
        if child then return { transition = T.PUSH, screen = child, args = { page = st.page } } end
        if Inv.selected(leader(ctx), st.page, st.row) then   -- 0x11740 -> 0x12148 [C]
            return { transition = T.PUSH, screen = E.MenuScreen.ITEM_ACTION,
                     args = { column = st.page, row = st.row } }
        end
        inventory_cursor(st, ctx); return                   -- empty slot: 0x11592 only [C]
    end
    if events & P.C ~= 0 then                               -- 0x10EC8.. [C]
        ctx.game.reselect_leader()                          -- 0x391A (plays 0x25 itself, docs/09 s4)
        st.page, st.row = Page.POCKET_SECRETARY, 0
        M.step_page(st, 0, ctx)
        st.posture_gate = true
        M.step_row(st, 0, events, ctx)                      -- consumes the gate, never opens posture
        build(st, ctx, false); return
    end
    if events & P.START ~= 0 then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end
end

-- What 0x10D30's loop does after the child routine returns; FFF0D0 was saved/restored around it [C]
function M.child_returned(st, child, res, ctx)
    if child == E.MenuScreen.POSTURE then
        if res.exit_menu then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end  -- FFF0B1 [C]
        -- 0x119CE returns through 0x11506 into the command loop. It redraws
        -- the normal posture row and bars but does not request 0x25 here.
        -- The 0x25 in 0x10D90 is for a full command refresh after other
        -- submenu returns, not the inline posture focus helper.
        ctx.redraw(M.REFRESH); return
    end
    local p = st.page
    if p < Page.CLIPS then inventory_cursor(st, ctx); return end
    if p == Page.ATTRIBUTES_SKILLS then build(st, ctx, true, AS_RETURN.black, AS_RETURN.line, AS_RETURN.reveal); return end   -- re-init: 0x10F50 resets page to 6 [C]
    if p == Page.POCKET_SECRETARY then
        if res.exit_menu then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end   -- FFF0CD [C]
        build(st, ctx, true); return
    end
    if ctx.game.leader_archetype() <= 1 and p == Page.MAGIC then build(st, ctx, true); return end   -- re-init: 0x10F50 resets page to 6 [C]
    build(st, ctx, false)
end

return M
