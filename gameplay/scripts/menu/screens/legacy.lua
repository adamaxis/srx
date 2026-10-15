-- legacy.lua: TRANSITIONAL adapter for screens whose rules are still in C++ (clips, item action, trade,
-- first aid, automatic use, discard, attributes/skills, magic spell setup, notebook, dismiss, save/load).
-- The route still owns WHEN they get input; C++ advances them via host.legacy_advance until each is ported.
-- Remove a screen from here as soon as its Lua module exists (MIGRATION.md, "port queue").
local E = require("enums")
local M = {}

function M.make(screen, window)
    local M = { window = window or E.WindowPlane.SIDEBAR, legacy = true }
    function M.new(args) return { args = args } end
    function M.enter(st, ctx) ctx.host.legacy_open(screen, st.args) end
    function M.input(st, events, ctx)
        local still_open, exit_menu = ctx.host.legacy_advance(screen, events)
        if not still_open then return { transition = E.MenuTransition.POP, result = { exit_menu = exit_menu } } end
    end
    return M
end

return M
