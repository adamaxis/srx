-- Field menu route profile. Command, Posture, Attributes/Skills, and the
-- wait-only Cyberware view are Lua-owned; other children remain adapters.
local E = require("enums")
local Route = require("menu.route")

Route.register(E.MenuScreen.COMMAND, require("menu.screens.command"))
Route.register(E.MenuScreen.POSTURE, require("menu.screens.posture"))
Route.register(E.MenuScreen.ATTRIBUTES_SKILLS,
               require("menu.screens.attributes").Attributes)
local wait_any = require("menu.screens.wait_any")
Route.register(E.MenuScreen.CYBERWARE,
               wait_any.make(E.MenuRegion.FULL))

local legacy = require("menu.screens.legacy")
for screen = E.MenuScreen.ITEM_ACTION, E.MenuScreen.DECK_MAP do
    if screen ~= E.MenuScreen.ATTRIBUTES_SKILLS and
       screen ~= E.MenuScreen.CYBERWARE then
        local window = E.WindowPlane.SIDEBAR
        if screen == E.MenuScreen.CYBERDECK then
            window = E.WindowPlane.OFF
        end
        Route.register(screen, legacy.make(screen, window))
    end
end

return Route
