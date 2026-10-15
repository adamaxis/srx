-- message.lua: one generic "show text, wait for a button" screen. Replaces six separate wait flags in srx
-- (clips message_, action_message_, automatic_use result_wait, first_aid result_wait, PS subview waits,
-- Mr. Johnson detail_wait). ROM equivalent: 0xD94 wait-for-button after a message box (docs/09 s9) [C].
-- args: { message = E.MenuMessage.*, mask = edge mask that dismisses (default any high-byte edge), data = {...} }
local E = require("enums")
local M = { window = E.WindowPlane.SIDEBAR }
function M.new(a) return { message = a.message, mask = a.mask or E.Pad.EDGES, data = a.data } end
function M.enter(st, ctx) ctx.redraw(E.MenuRegion.MESSAGE) end
function M.input(st, events, ctx)
    if events & st.mask ~= 0 then ctx.redraw(E.MenuRegion.MESSAGE); return { transition = E.MenuTransition.POP, result = { message = st.message } } end
end
function M.show(message, mask, data)   -- helper for other screens
    return { transition = E.MenuTransition.PUSH, screen = E.MenuScreen.MESSAGE, args = { message = message, mask = mask, data = data } }
end
return M
