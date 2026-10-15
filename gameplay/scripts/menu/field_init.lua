-- Field menu route profile. Command and Posture are Lua-owned now; child
-- screens remain explicit adapters until their host services are migrated.
local E = require("enums")
local Route = require("menu.route")

Route.register(E.MenuScreen.COMMAND, require("menu.screens.command"))
Route.register(E.MenuScreen.POSTURE, require("menu.screens.posture"))
Route.register(E.MenuScreen.ATTRIBUTES_SKILLS,
               require("menu.screens.attributes").Attributes)

local legacy = require("menu.screens.legacy")
for screen = E.MenuScreen.ITEM_ACTION, E.MenuScreen.DECK_MAP do
    if screen ~= E.MenuScreen.ATTRIBUTES_SKILLS then
        local window = E.WindowPlane.SIDEBAR
        if screen == E.MenuScreen.CYBERDECK then
            window = E.WindowPlane.OFF
        end
        Route.register(screen, legacy.make(screen, window))
    end
end

return Route
