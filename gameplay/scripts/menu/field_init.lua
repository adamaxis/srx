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
local magic = require("menu.screens.magic")
Route.register(E.MenuScreen.MAGIC, magic.Magic)
Route.register(E.MenuScreen.MAGICAL_DEFENSE,
               wait_any.make(E.MenuRegion.FULL))
Route.register(E.MenuScreen.MAGIC_FORCE, magic.Force)
Route.register(E.MenuScreen.MAGIC_POSTURE, magic.Posture)
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
Route.register(E.MenuScreen.NOTEBOOK, secretary.Notebook)
Route.register(E.MenuScreen.NOTEBOOK_TIPS, secretary.Tips)
Route.register(E.MenuScreen.MR_JOHNSONS, secretary.Johnsons)
Route.register(E.MenuScreen.NOTEBOOK_CONTACTS, secretary.Contacts)
Route.register(E.MenuScreen.NOTEBOOK_SHADOWRUNNERS, secretary.Shadowrunners)
Route.register(E.MenuScreen.GROUP_ITEMS, secretary.GroupItems)
Route.register(E.MenuScreen.DISMISS_RUNNER, secretary.Dismiss)
local debug_menu = require("menu.screens.debug_menu")
Route.register(E.MenuScreen.DEBUG_MENU, debug_menu.Debug)
Route.register(E.MenuScreen.DEBUG_PICK_SYSTEM, debug_menu.Pick)
local save_load = require("menu.screens.save_load")
Route.register(E.MenuScreen.SAVE_LOAD, save_load.SaveLoad)
Route.register(E.MenuScreen.SAVE_SLOT, save_load.Slot)
local cyberdeck = require("menu.screens.cyberdeck")
Route.register(E.MenuScreen.CYBERDECK, cyberdeck.Cyberdeck)
Route.register(E.MenuScreen.PULL_PLUG, cyberdeck.PullPlug)
local deck_screens = require("menu.screens.deck_screens")
Route.register(E.MenuScreen.DECK_PROGRAM, deck_screens.Program)
Route.register(E.MenuScreen.DECK_CYBERINFO, deck_screens.CyberInfo)
Route.register(E.MenuScreen.DECK_STORAGE, deck_screens.Storage)
Route.register(E.MenuScreen.DECK_MAP, deck_screens.Map)

local legacy = require("menu.screens.legacy")
for screen = E.MenuScreen.ITEM_ACTION, E.MenuScreen.DECK_MAP do
    if screen ~= E.MenuScreen.ATTRIBUTES_SKILLS and
       screen ~= E.MenuScreen.CYBERWARE and
       screen ~= E.MenuScreen.MAGIC and
       screen ~= E.MenuScreen.MAGICAL_DEFENSE and
       screen ~= E.MenuScreen.MAGIC_FORCE and
       screen ~= E.MenuScreen.MAGIC_POSTURE and
       screen ~= E.MenuScreen.CLIPS and
       screen ~= E.MenuScreen.RELOAD_CONFIRM and
       screen ~= E.MenuScreen.CLIP_TRADE and
       screen ~= E.MenuScreen.ITEM_ACTION and
       screen ~= E.MenuScreen.DISCARD_CONFIRM and
       screen ~= E.MenuScreen.INVENTORY_TRADE and
       screen ~= E.MenuScreen.FIRST_AID and
       screen ~= E.MenuScreen.MESSAGE and
       screen ~= E.MenuScreen.POCKET_SECRETARY and
       screen ~= E.MenuScreen.NOTEBOOK and
       screen ~= E.MenuScreen.NOTEBOOK_TIPS and
       screen ~= E.MenuScreen.MR_JOHNSONS and
        screen ~= E.MenuScreen.NOTEBOOK_CONTACTS and
        screen ~= E.MenuScreen.NOTEBOOK_SHADOWRUNNERS and
       screen ~= E.MenuScreen.GROUP_ITEMS and
       screen ~= E.MenuScreen.DISMISS_RUNNER and
       screen ~= E.MenuScreen.DEBUG_MENU and
       screen ~= E.MenuScreen.DEBUG_PICK_SYSTEM and
       screen ~= E.MenuScreen.SAVE_LOAD and
       screen ~= E.MenuScreen.SAVE_SLOT and
       screen ~= E.MenuScreen.CYBERDECK and
       screen ~= E.MenuScreen.PULL_PLUG and
       screen ~= E.MenuScreen.DECK_PROGRAM and
       screen ~= E.MenuScreen.DECK_CYBERINFO and
       screen ~= E.MenuScreen.DECK_STORAGE and
       screen ~= E.MenuScreen.DECK_MAP then
        Route.register(screen, legacy.make(screen, E.WindowPlane.SIDEBAR))
    end
end

return Route
