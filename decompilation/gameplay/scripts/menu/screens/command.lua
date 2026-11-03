-- command.lua: the Start/Command overview, with named section and cursor state.
-- ROM addresses below document recovered behavior; source selectors stay in
-- the native adapter and are not part of this Lua state.
local E = require("enums")
local Inv = require("menu.rules.inventory")
local Items = require("items").by_id
local ItemDefinitions = require("data.items").by_id
local P, Section, R, T, S = E.Pad, E.CommandSection, E.MenuRegion, E.MenuTransition, E.MenuSound
local SECTION_ORDER = {
    Section.INVENTORY_LEFT, Section.INVENTORY_RIGHT, Section.CLIPS,
    Section.ATTRIBUTES_SKILLS, Section.CYBERWARE, Section.MAGIC,
    Section.POCKET_SECRETARY,
}
local SECTION_POSITION = {}
for position, section in ipairs(SECTION_ORDER) do
    SECTION_POSITION[section] = position
end
local INVENTORY_COLUMN = {
    [Section.INVENTORY_LEFT] = 0,
    [Section.INVENTORY_RIGHT] = 1,
}
local function is_inventory_section(section)
    return INVENTORY_COLUMN[section] ~= nil
end

local M = { window = E.WindowPlane.SIDEBAR }   -- [C] 0x10D30 never writes VDP reg 0x11 (docs/09 s11)

-- 0x1170C + 0x111E8 after every move / refresh: details, bars, posture, buttons, label (docs/09 s10.1) [C regions]
M.REFRESH = R.ITEM_PICTURE | R.ITEM_HEADER | R.RATING | R.DAMAGE_POWER | R.COMBAT_BARS | R.WEAPON_BARS
          | R.ARMOR | R.POSTURE_BLOCK | R.BUTTON_STRIP | R.COMMAND_LABEL

local CHILD = {   -- recovered section-to-screen routes; the ROM uses a jump table
    [Section.CLIPS] = E.MenuScreen.CLIPS,
    [Section.ATTRIBUTES_SKILLS] = E.MenuScreen.ATTRIBUTES_SKILLS,
    [Section.CYBERWARE] = E.MenuScreen.CYBERWARE,
    [Section.MAGIC] = E.MenuScreen.MAGIC,
    [Section.POCKET_SECRETARY] = E.MenuScreen.POCKET_SECRETARY,
}

function M.new()
    return {
        section = Section.POCKET_SECRETARY,
        selected_inventory_row = 0,
        suppress_next_posture_focus = false,
        inventory_item_hold_frames = 0,
        inventory_picture_bank = nil,
        inventory_cursor_hidden_frames = 0, inventory_cursor_hidden = false,
    }
end

local function leader(ctx) return ctx.game.party()[ctx.game.active_slot()] end

local function picture_bank(item_id)
    local item = item_id and Items[item_id]
    if not item then return nil end
    return item.kind == "firearm" and "weapon" or "non_weapon"
end

local function mark_picture_bank(st, item_id)
    local bank = picture_bank(item_id)
    if bank then st.inventory_picture_bank = bank end
end

local function current_picture_item_id(st, ctx)
    if is_inventory_section(st.section) then
        local selected = Inv.selected(
            leader(ctx), INVENTORY_COLUMN[st.section], st.selected_inventory_row)
        return selected and selected.id or nil
    end
    return leader(ctx).equipped_weapon_id
end

-- 0x10D36/0x10D40: btst #6 / #4 on leader +0x10 (DEAD / DOWN), then 0x391A [C].
-- The host translates record +0x10 into runner_facts.lifecycle; Lua does not
-- receive the status byte. The earlier native check read +0x11 by mistake.
local function reselect_if_incapacitated(ctx)
    local r = leader(ctx)
    local lifecycle = r and r.runner_facts and r.runner_facts.lifecycle
    if lifecycle and (lifecycle.dead or lifecycle.downed) then
        ctx.game.reselect_leader()
    end
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
        st.section, st.selected_inventory_row = Section.POCKET_SECRETARY, 0 -- 0x10F50 [C]
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
    ctx.sound(S.MOVE)                                       -- 0x10E2E plays 0x25 even on open [C]
    ctx.redraw(M.REFRESH)
    -- 0x12A54 compares FFF100 with the selected picture bank and calls
    -- 0x1107E to upload only when the bank changes.
    mark_picture_bank(st, current_picture_item_id(st, ctx))
end

function M.enter(st, ctx) build(st, ctx, true, BLANK_OPEN, BLANK_REVEAL_LINE, OPEN.reveal, OPEN.lead) end

-- 0x11592: inventory cursor sprite at (page*0x28, row*0x18) + item detail [C]
local function inventory_cursor(st, ctx)
    ctx.redraw(R.INVENTORY_CURSOR | R.ITEM_PICTURE | R.ITEM_HEADER)
    mark_picture_bank(st, current_picture_item_id(st, ctx))
end

-- 0x114A0 [C]
function M.step_section(st, direction, ctx)
    local position = SECTION_POSITION[st.section]
    assert(position ~= nil, "unknown command section: " .. tostring(st.section))
    local next_position = position + direction
    if next_position < 1 then next_position = #SECTION_ORDER end
    if next_position > #SECTION_ORDER then next_position = 1 end
    st.section = SECTION_ORDER[next_position]
    if is_inventory_section(st.section) then
        inventory_cursor(st, ctx)
    else
        ctx.redraw(R.BUTTON_STRIP)
    end
end

-- 0x11506 [C]; returns a PUSH of the posture focus when the ROM would enter 0x119CE
function M.step_row(st, d, events, ctx)
    if is_inventory_section(st.section) then
        local r = st.selected_inventory_row + d
        if r < 0 then r = 0 elseif r >= E.InventoryGrid.ROWS then r = E.InventoryGrid.ROWS - 1 end
        st.selected_inventory_row = r
        inventory_cursor(st, ctx)
        return nil
    end
    if events & (P.HELD_DOWN | P.HELD_LEFT | P.HELD_RIGHT) ~= 0 then return nil end
    if st.suppress_next_posture_focus then
        st.suppress_next_posture_focus = false
        return nil
    end
    return { transition = T.PUSH, screen = E.MenuScreen.POSTURE }
end

-- ROM 0x12A54 updates FFF100 and calls 0x1107E only when the picture bank
-- changes. Emulator measurements: weapon-bank upload holds eight frames;
-- non-weapon-bank upload holds seven. Same-bank selections draw immediately.
local PICTURE_BANK_HOLD_FRAMES = { weapon = 8, non_weapon = 7 }

local function selected_inventory_item(st, ctx)
    if not is_inventory_section(st.section) then return nil end
    return Inv.selected(
        leader(ctx), INVENTORY_COLUMN[st.section], st.selected_inventory_row)
end

local function update_inventory_ammo_count(st, ctx)
    st.inventory_ammo_count = nil
    local selected = selected_inventory_item(st, ctx)
    local definition = selected and ItemDefinitions[selected.id]
    if definition and definition.kind == E.ItemKind.FIREARM then
        -- ROM 0x12A8C draws the selected firearm's D4540 +0x24 magazine
        -- capacity in the main detail row. The portrait HUD independently
        -- shows live +0x4E through 0x36FC.
        st.inventory_ammo_count = ctx.game.magazine_capacity(selected.id)
    end
end

local function finish_move(st, old_section, old_row, previous_picture_bank, ctx)
    local moved_inventory_item = is_inventory_section(old_section) and
        is_inventory_section(st.section) and
        (old_section ~= st.section or old_row ~= st.selected_inventory_row)
    local previous = moved_inventory_item and
        Inv.selected(leader(ctx), INVENTORY_COLUMN[old_section], old_row) or nil
    local selected = moved_inventory_item and selected_inventory_item(st, ctx) or nil
    if previous and selected then
        local next_picture_bank = picture_bank(selected.id)
        local loaded_picture_bank = previous_picture_bank or
            picture_bank(previous.id)
        if loaded_picture_bank == next_picture_bank then
            st.inventory_item_hold_frames = 0
            st.inventory_detail_pending = false
            st.inventory_picture_partial = false
            -- [C emulator e7 8720] Same-bank move: detail and MOVE land on the
            -- press frame; the slot cursor is hidden that frame and shown at
            -- the new slot on 8721.
            st.inventory_cursor_hidden_frames = 1
            st.inventory_cursor_hidden = true
            ctx.sound(S.MOVE)
            ctx.redraw(M.REFRESH)
            mark_picture_bank(st, selected.id)
        else
            st.inventory_item_hold_frames =
                PICTURE_BANK_HOLD_FRAMES[next_picture_bank]
            st.inventory_item_hold_total = st.inventory_item_hold_frames
            st.inventory_detail_pending = true
            st.inventory_cursor_hidden_frames = 0
            st.inventory_cursor_hidden = true
            ctx.redraw(M.REFRESH)
            mark_picture_bank(st, selected.id)
        end
    else
        st.inventory_item_hold_frames = 0
        st.inventory_detail_pending = false
        st.inventory_picture_partial = false
        -- [C emulator] On entry, the inventory art and MOVE cue appear on the
        -- input frame; only the yellow slot cursor waits one frame.
        st.inventory_cursor_hidden_frames =
            (not is_inventory_section(old_section) and
             is_inventory_section(st.section)) and 1 or 0
        st.inventory_cursor_hidden =
            st.inventory_cursor_hidden_frames > 0
        ctx.sound(S.MOVE)
        ctx.redraw(M.REFRESH)
        mark_picture_bank(st, current_picture_item_id(st, ctx))
    end
    update_inventory_ammo_count(st, ctx)
end

function M.input(st, events, ctx)
    if st.inventory_item_hold_frames > 0 then
        st.inventory_item_hold_frames = st.inventory_item_hold_frames - 1
        if st.inventory_item_hold_frames == 0 then
            -- [C emulator e6 + zsnd] MOVE is requested after the bank upload:
            -- weapon-bank changes take eight frames and non-weapon changes
            -- take seven. The seven-frame path reveals text/bars and picture
            -- rows y>=56 here; the eight-frame path reveals its full panel on
            -- the next frame.
            ctx.sound(S.MOVE)
            st.inventory_cursor_hidden = false
            if (st.inventory_item_hold_total or 7) < 8 then
                st.inventory_detail_pending = false
                st.inventory_picture_partial = true
                update_inventory_ammo_count(st, ctx)
            end
            ctx.redraw(M.REFRESH)
            st.inventory_detail_reveal_frames = 1
        end
        return
    end
    if (st.inventory_detail_reveal_frames or 0) > 0 then
        st.inventory_detail_reveal_frames = 0
        st.inventory_picture_partial = false
        if st.inventory_detail_pending then
            st.inventory_detail_pending = false
            update_inventory_ammo_count(st, ctx)
        end
        ctx.redraw(M.REFRESH)
        return
    end
    if st.inventory_cursor_hidden_frames > 0 then
        st.inventory_cursor_hidden_frames =
            st.inventory_cursor_hidden_frames - 1
        st.inventory_cursor_hidden =
            st.inventory_cursor_hidden_frames > 0
    end
    update_inventory_ammo_count(st, ctx)
    local previous_picture_bank = st.inventory_picture_bank
    -- priority order of 0x10DD8..0x10F2A [C]
    if events & P.LEFT ~= 0 then
        local old_section, old_row = st.section, st.selected_inventory_row
        M.step_section(st, -1, ctx); finish_move(st, old_section, old_row, previous_picture_bank, ctx); return
    end
    if events & P.UP ~= 0 then
        local old_section, old_row = st.section, st.selected_inventory_row
        local r = M.step_row(st, -1, events, ctx); if r then return r end
        finish_move(st, old_section, old_row, previous_picture_bank, ctx); return
    end
    if events & P.RIGHT ~= 0 then
        local old_section, old_row = st.section, st.selected_inventory_row
        M.step_section(st, 1, ctx); finish_move(st, old_section, old_row, previous_picture_bank, ctx); return
    end
    if events & P.DOWN ~= 0 then
        local old_section, old_row = st.section, st.selected_inventory_row
        local r = M.step_row(st, 1, events, ctx); if r then return r end
        finish_move(st, old_section, old_row, previous_picture_bank, ctx); return
    end
    if events & (P.A | P.B) ~= 0 then
        ctx.sound(S.SELECT)                                  -- 0x10E4E [C]
        local child = CHILD[st.section]
        if child then
            return { transition = T.PUSH, screen = child,
                     args = { command_section = st.section } }
        end
        if Inv.selected(leader(ctx), INVENTORY_COLUMN[st.section],
                        st.selected_inventory_row) then      -- 0x11740 -> 0x12148 [C]
            return { transition = T.PUSH, screen = E.MenuScreen.ITEM_ACTION,
                     args = { command_section = st.section,
                              row = st.selected_inventory_row } }
        end
        inventory_cursor(st, ctx); return                   -- empty slot: 0x11592 only [C]
    end
    if events & P.C ~= 0 then                               -- 0x10EC8.. [C]
        ctx.game.reselect_leader()                          -- 0x391A (plays 0x25 itself, docs/09 s4)
        st.section, st.selected_inventory_row = Section.POCKET_SECRETARY, 0
        M.step_section(st, 0, ctx)
        st.suppress_next_posture_focus = true
        M.step_row(st, 0, events, ctx)                      -- consumes the gate, never opens posture
        build(st, ctx, false); return
    end
    if events & P.START ~= 0 then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end
end

-- What 0x10D30's loop does after the child routine returns; FFF0D0 was saved/restored around it [C]
function M.child_returned(st, child, res, ctx)
    update_inventory_ammo_count(st, ctx)
    if child == E.MenuScreen.POSTURE then
        if res.exit_menu then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end  -- FFF0B1 [C]
        -- 0x119CE returns through 0x11506 into the command loop. It redraws
        -- the normal posture row and bars but does not request 0x25 here.
        -- The 0x25 in 0x10D90 is for a full command refresh after other
        -- submenu returns, not the inline posture focus helper.
        ctx.redraw(M.REFRESH); return
    end
    local section = st.section
    if is_inventory_section(section) then inventory_cursor(st, ctx); return end
    if section == Section.ATTRIBUTES_SKILLS then
        build(st, ctx, true, AS_RETURN.black, AS_RETURN.line, AS_RETURN.reveal)
        return
    end   -- re-init: 0x10F50 returns to the pocket secretary [C]
    if section == Section.POCKET_SECRETARY then
        if res.exit_menu then reselect_if_incapacitated(ctx); return { transition = T.EXIT } end   -- FFF0CD [C]
        build(st, ctx, true); return
    end
    if E.RunnerClass.CASTERS[ctx.game.leader_class()] and
       section == Section.MAGIC then build(st, ctx, true); return end
    build(st, ctx, false)
end

return M
