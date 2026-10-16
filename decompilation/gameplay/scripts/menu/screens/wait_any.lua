-- wait_any.lua: display-then-wait screens (Cyberware 0x11950, Magical Defense fallback of 0xFBA6):
-- draw once, then 0xD94 waits for any new high-byte edge, then return. No cursor, no sound [C docs/09 s10.4/10.5].
local E = require("enums")
local function make(region_on_enter)
    local M = { window = E.WindowPlane.SIDEBAR }
    function M.new(args)
        return { args = args, magic_view = args and args.magic_view }
    end
    function M.enter(st, ctx) ctx.redraw(region_on_enter) end
    function M.input(st, events, ctx)
        if events & E.Pad.EDGES ~= 0 then
            ctx.redraw(E.MenuRegion.BUTTON_STRIP | E.MenuRegion.COMMAND_LABEL)  -- 0x11928 row restore [C]
            return { transition = E.MenuTransition.POP }
        end
    end
    return M
end
return { make = make }
