-- attributes.lua: Attributes/Skills 0xD01C (window OFF: $9100 at 0xD024 [C]; no SFX in the routine [C docs/09 s10.6]).
-- Rules: srx attributes_skills_menu_conversion.cpp. From the Command overview the screen is VIEW-ONLY
-- (srx AttributesSkillsMenuMode::command_view_only); karma upgrades are the hotel mode (args.karma = true).
local E = require("enums")
local P, R, T, MM = E.Pad, E.MenuRegion, E.MenuTransition, E.MenuMessage
local M = { window = E.WindowPlane.OFF }

-- species attribute maxima: ROM table 0xC22A6, 5 x 6 bytes, row = +0x9A species, col = attribute index;
-- 0xD884..0xD8B8: next = attr(+0x77+i)+1, cmp > table -> "at limit" (0xD9DE); i >= 6 -> not raisable (0xD87A) [C]
M.MAX = { [0] = { 6, 6, 6, 6, 6, 6 }, { 6, 7, 6, 8, 6, 6 }, { 7, 5, 8, 6, 6, 7 }, { 9, 6, 8, 5, 5, 6 }, { 11, 5, 10, 4, 4, 5 } }
M.ROWS, M.COLUMNS = 10, 2

-- cell -> (section, index): rows 0..3 attributes (col*4 + row), rows 4..9 skills ((row-4) + col*6)
function M.cell(col, row)
    if row < 4 then return "attribute", row + col * 4 end
    return "skill", (row - 4) + col * 6
end
-- returns status, next_value, cost
function M.preview(r, col, row)
    local sec, i = M.cell(col, row)
    if sec == "attribute" then
        if i >= 6 then return "not_raisable" end
        local mx = M.MAX[r.species]; if not mx then return "unknown_species" end
        local nxt = r.attributes[i] + 1
        if nxt > mx[i + 1] then return "at_limit" end
        return "available", nxt, nxt
    end
    if i == E.Skill.SORCERY and r.archetype >= E.Archetype.CASTER_LIMIT then return "class_restricted" end
    local base = r.skills[i]
    local reduced = col == 0 and i >= E.Skill.PISTOLS and i <= E.Skill.SHOTGUNS   -- weapon specialities default to Firearms
    if reduced and base == 0 then base = r.skills[E.Skill.FIREARMS] end
    local nxt = base + 1
    if nxt >= 0x0D then return "at_limit" end
    return "available", nxt, reduced and (nxt * 15 + 9) // 10 or nxt * 2
end
function M.apply(r, col, row)
    local status, nxt, cost = M.preview(r, col, row)
    if status ~= "available" then return status end
    if r.karma < cost then return "insufficient_karma" end
    r.karma = r.karma - cost
    local sec, i = M.cell(col, row)
    if sec == "attribute" then r.attributes[i] = nxt else r.skills[i] = nxt end
    return "upgraded"
end

local function rec(ctx) return ctx.game.party()[ctx.game.active_slot()] end
function M.new(a) return { col = 0, row = 0, karma = a.karma or false } end
function M.enter(st, ctx)
    ctx.host.blank(E.DisplayBlank.OFF); ctx.redraw(R.FULL); ctx.host.blank(E.DisplayBlank.ON)   -- 0xD01C build [C]
end
local function moved(st, ctx)
    ctx.redraw(R.SUBMENU)
    if st.karma and M.preview(rec(ctx), st.col, st.row) == "at_limit" then return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE, args = { message = MM.MAX_LEVEL } } end
end
-- Exit 0xD4A0 [C]: VDP reg 0x11=$91 (window = field sidebar from x>=272),
-- then CPU fills of tile $07FF over plane FFDD16 (0xFFE) and FFDD14 (0x102A)
-- race the raster. Emulator e2 [C]: input frame 8960 shows the screen with
-- its text cleared; 8961 only the sidebar above line 127; then black.
local LEAD_TEXT = { { 80, 24, 104, 16 }, { 24, 56, 240, 32 }, { 264, 56, 8, 8 },
                    { 24, 104, 240, 48 }, { 120, 160, 8, 8 }, { 24, 168, 240, 24 } }
local LEAD_SIDEBAR = { { 0, 0, 272, 224 }, { 272, 127, 48, 97 } }
local function exit(ctx)
    ctx.host.set_window(E.WindowPlane.SIDEBAR)
    for _, r in ipairs(LEAD_TEXT) do ctx.host.mask(E.MenuMask.LEAD, 0, r[1], r[2], r[3], r[4]) end
    for _, r in ipairs(LEAD_SIDEBAR) do ctx.host.mask(E.MenuMask.LEAD, 1, r[1], r[2], r[3], r[4]) end
    return { transition = T.POP, result = { exit_as = true } }
end
function M.input(st, events, ctx)
    if events & P.LEFT ~= 0 then st.col = 0; return moved(st, ctx) end
    if events & P.RIGHT ~= 0 then st.col = 1; return moved(st, ctx) end
    if events & P.UP ~= 0 then if st.row > 0 then st.row = st.row - 1 end; return moved(st, ctx) end
    if events & P.DOWN ~= 0 then if st.row < M.ROWS - 1 then st.row = st.row + 1 end; return moved(st, ctx) end
    if events & (P.A | P.B) ~= 0 then
        if not st.karma then return exit(ctx) end    -- view-only (0xD33A -> 0xD4A0): upgrade key leaves (selection_data_unavailable)
        local r = rec(ctx)
        if st.col == 1 and (st.row == 2 or st.row == 3) then return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE, args = { message = MM.NOT_RAISABLE } } end
        if st.col == 0 and st.row == 4 and r.archetype >= E.Archetype.CASTER_LIMIT then return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE, args = { message = MM.MAGE_ONLY } } end
        local status, _, cost = M.preview(r, st.col, st.row)
        if status ~= "available" then return end
        if r.karma < cost then return { transition = T.PUSH, screen = E.MenuScreen.MESSAGE, args = { message = MM.INSUFFICIENT_KARMA } } end
        return { transition = T.PUSH, screen = E.MenuScreen.UPGRADE_PROMPT, args = { col = st.col, row = st.row } }
    end
    if events & P.C ~= 0 then st.col = 0; ctx.game.reselect_leader(); ctx.redraw(R.SUBMENU | R.SIDEBAR_SLOT); return end
    if events & P.START ~= 0 then return exit(ctx) end
end
function M.child_returned(st, child, res, ctx) ctx.redraw(R.SUBMENU | R.COMBAT_BARS) end

-- Yes/No upgrade prompt: Up/Down wrap, A/B/C confirm; Yes = option 0 [srx C]
local Prompt = { window = E.WindowPlane.OFF }
function Prompt.new(a) return { col = a.col, row = a.row, option = 0 } end
function Prompt.enter(st, ctx) ctx.redraw(R.SUBMENU) end
function Prompt.input(st, events, ctx)
    if events & (P.UP | P.DOWN) ~= 0 then st.option = 1 - st.option; ctx.redraw(R.SUBMENU); return end
    if events & (P.A | P.B | P.C) == 0 then return end
    local res = "declined"
    if st.option == 0 then
        -- game.party() hands out record copies; write the upgraded record back explicitly.
        local party = ctx.game.party()
        res = M.apply(party[ctx.game.active_slot()], st.col, st.row)
        if res == "upgraded" then ctx.game.commit_party(party) end
    end
    if res == "insufficient_karma" then return { transition = T.REPLACE, screen = E.MenuScreen.MESSAGE, args = { message = MM.INSUFFICIENT_KARMA } } end
    return { transition = T.POP, result = { upgrade = res } }
end

return { Attributes = M, Prompt = Prompt }
