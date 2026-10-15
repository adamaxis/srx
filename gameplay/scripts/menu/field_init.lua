-- Field menu route profile. The command shell, inventory actions, and Clips
-- children are Lua-owned; screens without a live port remain adapters.
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
local inventory = require("menu.screens.item_action")
Route.register(E.MenuScreen.ITEM_ACTION, inventory.ItemAction)
Route.register(E.MenuScreen.DISCARD_CONFIRM, inventory.Discard)
Route.register(E.MenuScreen.INVENTORY_TRADE, inventory.Trade)
Route.register(E.MenuScreen.FIRST_AID, inventory.FirstAid)
local secretary = require("menu.screens.pocket_secretary")
Route.register(E.MenuScreen.POCKET_SECRETARY, secretary.PocketSecretary)
Route.register(E.MenuScreen.GROUP_ITEMS, secretary.GroupItems)
Route.register(E.MenuScreen.DISMISS_RUNNER, secretary.Dismiss)

local legacy = require("menu.screens.legacy")
for screen = E.MenuScreen.ITEM_ACTION, E.MenuScreen.DECK_MAP do
    if screen ~= E.MenuScreen.ATTRIBUTES_SKILLS and
       screen ~= E.MenuScreen.CYBERWARE and
       screen ~= E.MenuScreen.CLIPS and
       screen ~= E.MenuScreen.RELOAD_CONFIRM and
       screen ~= E.MenuScreen.CLIP_TRADE and
       screen ~= E.MenuScreen.ITEM_ACTION and
       screen ~= E.MenuScreen.DISCARD_CONFIRM and
       screen ~= E.MenuScreen.INVENTORY_TRADE and
       screen ~= E.MenuScreen.FIRST_AID and
       screen ~= E.MenuScreen.MESSAGE and
       screen ~= E.MenuScreen.POCKET_SECRETARY and
       screen ~= E.MenuScreen.GROUP_ITEMS and
       screen ~= E.MenuScreen.DISMISS_RUNNER then
        local window = E.WindowPlane.SIDEBAR
        if screen == E.MenuScreen.CYBERDECK then
            window = E.WindowPlane.OFF
        end
        Route.register(screen, legacy.make(screen, window))
    end
end

return Route
