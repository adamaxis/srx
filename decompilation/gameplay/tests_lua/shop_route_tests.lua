-- ROM-backed shop input through the shared menu route layer.
-- 0x56AC0 sends B through the A branch at 0x56D6C -> 0x56CF4;
-- 0x56DC4 acts only on the action panel. Down/Up switch the panels through
-- 0x58CDE. C calls 0xD4C8 and restarts 0x56AC0; result 1 re-enters its direct
-- selector at 0x56C2C. Start, not B, returns from the field service loop.

local E = require("enums")
local Shop = require("shop")
require("menu.field_init")

game = {}

local shop_ids = {}
for shop_id in pairs(Shop.catalog) do
    shop_ids[#shop_ids + 1] = shop_id
end
table.sort(shop_ids)

local checks = 0
local failures = 0
local function check(condition, label)
    checks = checks + 1
    if not condition then
        failures = failures + 1
        io.write("FAIL ", label, "\n")
    end
end

check(#shop_ids == 6, "the source shop catalog contains six menus")

for selected_tab = 0, 5 do
    Shop.open_menu("ares_weapon_emporium")
    for _ = 1, selected_tab do
        Shop.advance_menu(E.Pad.RIGHT, E.MenuBPolicy.ROM)
    end
    local result = Shop.advance_menu(E.Pad.A, E.MenuBPolicy.ROM)
    check(result.open and result.event == "tab_selected" and
          result.action_panel == 0 and
          result.action_cursor == selected_tab and
          result.selected_action == selected_tab,
          "A latches Ares source tab " .. selected_tab ..
          " and stays on the tab row (ROM 0x56D64 -> 0x587DA)")
    local entered = Shop.advance_menu(E.Pad.DOWN, E.MenuBPolicy.ROM)
    check(entered.open and entered.event == "panel_changed" and
          entered.action_panel == 1 and entered.action_cursor == 0 and
          entered.selected_action == selected_tab,
          "Down enters the latched Ares tab " .. selected_tab .. " list")
end

Shop.open_menu("ares_weapon_emporium")
local start_result = Shop.advance_menu(E.Pad.START, E.MenuBPolicy.ROM)
check(not start_result.open and start_result.event == "returned_to_field",
      "ROM Start exits the shop while C remains the runner-change button")

local function open(shop_id)
    local state = Shop.open_menu(shop_id)
    check(state.open and state.shop_id == shop_id,
          shop_id .. " opens its Lua menu state")
    return state
end

local function rom_root_action(shop_id)
    local direct_selector = Shop.catalog[shop_id].menu_result == 1
    local result = Shop.advance_menu(E.Pad.A, E.MenuBPolicy.ROM)
    check(result.open and
          result.event == (direct_selector
              and "equipment_upgrade_selected" or "tab_selected") and
          (direct_selector or (result.action_panel == 0 and
                               result.action_cursor == 0)),
          shop_id .. " ROM A selects the highlighted tab")
    Shop.open_menu(shop_id)
    result = Shop.advance_menu(E.Pad.B, E.MenuBPolicy.ROM)
    check(result.open and
          result.event == (direct_selector
              and "equipment_upgrade_selected" or "tab_selected"),
          shop_id .. " ROM B follows the A tab-confirm branch")
end

local function action_panel_event(shop_id)
    local group = Shop.catalog[shop_id].groups[1]
    if #group.progression_offers > 0 then
        return "progression_selected"
    end
    if #group.offers > 0 then
        return "purchase_selected"
    end
    if #group.equipment_upgrade_offers > 0 then
        return "equipment_upgrade_selected"
    end
    if #group.cyberdeck_rating_upgrade_offers > 0 then
        return "cyberdeck_rating_upgrade_selected"
    end
    return "source_action_boundary"
end

for _, shop_id in ipairs(shop_ids) do
    local direct_selector = Shop.catalog[shop_id].menu_result == 1

    open(shop_id)
    rom_root_action(shop_id)

    Shop.open_menu(shop_id)
    local down = Shop.advance_menu(E.Pad.DOWN, E.MenuBPolicy.ROM)
    if direct_selector then
        check(down.open and down.event == "no_change" and
              down.action_panel == 1,
              shop_id .. " ROM Down keeps its direct selector")
    else
        check(down.open and down.event == "panel_changed" and
              down.action_panel == 1,
              shop_id .. " ROM Down opens its action panel")
        local up = Shop.advance_menu(E.Pad.UP, E.MenuBPolicy.ROM)
        check(up.open and up.event == "panel_changed" and
              up.action_panel == 0,
              shop_id .. " ROM Up returns to the category panel")
        Shop.advance_menu(E.Pad.DOWN, E.MenuBPolicy.ROM)
    end

    local rom_action = Shop.advance_menu(E.Pad.B, E.MenuBPolicy.ROM)
    check(rom_action.open and rom_action.event == action_panel_event(shop_id),
          shop_id .. " ROM B reaches its selected action in one update")

    Shop.open_menu(shop_id)
    local cancelled = Shop.advance_menu(
        E.Pad.B, E.MenuBPolicy.ALWAYS_CANCEL)
    check(not cancelled.open and cancelled.event == "returned_to_field" and
          game.shop_menu == nil,
          shop_id .. " ALWAYS_CANCEL closes a root before its handler")

    Shop.open_menu(shop_id)
    if not direct_selector then
        Shop.advance_menu(E.Pad.DOWN, E.MenuBPolicy.ROM)
    end
    cancelled = Shop.advance_menu(E.Pad.B, E.MenuBPolicy.ALWAYS_CANCEL)
    check(not cancelled.open and cancelled.event == "returned_to_field" and
          game.shop_menu == nil,
          shop_id .. " ALWAYS_CANCEL closes its action panel before its handler")

    Shop.open_menu(shop_id)
    cancelled = Shop.advance_menu(
        E.Pad.B, E.MenuBPolicy.ALWAYS_CANCEL, true)
    check(not cancelled.open and cancelled.event == "returned_to_field" and
          game.shop_menu == nil,
          shop_id .. " ALWAYS_CANCEL preempts its transaction prompt")

    Shop.open_menu(shop_id)
    local transaction_rom = Shop.advance_menu(
        E.Pad.B, E.MenuBPolicy.ROM, true)
    check(transaction_rom.open and transaction_rom.event == "no_change",
          shop_id .. " ROM policy leaves transaction B to its host service")

    Shop.open_menu(shop_id)
    local c = Shop.advance_menu(E.Pad.C, E.MenuBPolicy.ROM)
    check(c.open and c.event == "active_runner_change_requested" and
          c.action_cursor == 0 and c.selected_action == 0 and
          c.action_panel == (direct_selector and 1 or 0),
          shop_id .. " C changes runner and restores its source panel")
end

io.write("shop_route_tests: ", checks - failures, "/", checks, " passed; shops=",
         #shop_ids, "\n")
if failures > 0 then os.exit(1) end
