-- Field menu route profile. Command, Posture, Attributes/Skills, Cyberware,
-- Clips, and Clips' message/confirmation/trade children are Lua-owned; other
-- children remain adapters.
local E = require("enums")
local Route = require("menu.route")

Route.register(E.MenuScreen.COMMAND, require("menu.screens.command"))
Route.register(E.MenuScreen.POSTURE, require("menu.screens.posture"))
Route.register(E.MenuScreen.ATTRIBUTES_SKILLS,
               require("menu.screens.attributes").Attributes)
local wait_any = require("menu.screens.wait_any")
Route.register(E.MenuScreen.CYBERWARE,
               wait_any.make(E.MenuRegion.FULL))
local clips = require("menu.screens.clips")
Route.register(E.MenuScreen.CLIPS, clips.Clips)
Route.register(E.MenuScreen.RELOAD_CONFIRM, clips.Confirm)
Route.register(E.MenuScreen.CLIP_TRADE, clips.Trade)
Route.register(E.MenuScreen.MESSAGE, require("menu.screens.message"))

local legacy = require("menu.screens.legacy")
for screen = E.MenuScreen.ITEM_ACTION, E.MenuScreen.DECK_MAP do
    if screen ~= E.MenuScreen.ATTRIBUTES_SKILLS and
       screen ~= E.MenuScreen.CYBERWARE and
       screen ~= E.MenuScreen.CLIPS and
       screen ~= E.MenuScreen.RELOAD_CONFIRM and
       screen ~= E.MenuScreen.CLIP_TRADE and
       screen ~= E.MenuScreen.MESSAGE then
        local window = E.WindowPlane.SIDEBAR
        if screen == E.MenuScreen.CYBERDECK then
            window = E.WindowPlane.OFF
        end
        Route.register(screen, legacy.make(screen, window))
    end
end

return Route
