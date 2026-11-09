-- One field-service shop input screen for every data-driven shop catalog.
-- ROM entry is shared at 0x56AC0; 0x56DC4 handles the selected action.
local E = require("enums")
local T = E.MenuTransition
local Catalog = require("data.shop_catalog")

local Shop = {}

function Shop.new(args)
    return args.state or {}
end

function Shop.input(menu, controller_events)
    local definition = assert(Catalog.catalog[menu.shop_id],
        "active shop menu definition is missing")
    local function event(name)
        return { event = name }
    end
    local function move_cursor(delta)
        local maximum = menu.action_panel == 0
            and (menu.menu_result == 0 and 3 or 5)
            or 7
        local candidate = menu.action_cursor + delta
        if candidate < 0 then
            candidate = 0
        elseif candidate > maximum then
            candidate = maximum
        end
        if candidate == menu.action_cursor then return event("no_change") end
        menu.action_cursor = candidate
        if menu.action_panel == 0 or menu.menu_result == 1 then
            menu.selected_action = candidate
        end
        return event("selection_moved")
    end
    local function move_panel(delta)
        if menu.menu_result == 1 then return event("no_change") end
        local candidate = menu.action_panel + delta
        if candidate < 0 or candidate > 1 then return event("no_change") end
        if candidate == menu.action_panel then return event("no_change") end
        menu.action_panel = candidate
        menu.action_cursor = 0
        if candidate == 0 then menu.selected_action = 0 end
        return event("panel_changed")
    end

    -- ROM 0x56AC0 priority: primary cursor, panel cursor, A/B action,
    -- active-runner change, then Start return. A/B share the 0x56CF4 branch;
    -- a root category panel does not leave the shop on B.
    if (controller_events & E.Pad.LEFT) ~= 0 then
        return move_cursor(-1)
    elseif (controller_events & E.Pad.UP) ~= 0 then
        return move_panel(-1)
    elseif (controller_events & E.Pad.RIGHT) ~= 0 then
        return move_cursor(1)
    elseif (controller_events & E.Pad.DOWN) ~= 0 then
        return move_panel(1)
    elseif (controller_events & (E.Pad.A | E.Pad.B)) ~= 0 then
        if menu.action_panel == 0 then
            menu.selected_action = menu.action_cursor
            return event("no_change")
        end
        local group = definition.groups[
            menu.menu_result == 1 and 1 or menu.selected_action + 1]
        if group == nil then
            return event("source_action_boundary")
        elseif menu.menu_result == 1 then
            return event(group.equipment_upgrade_offers[
                menu.action_cursor + 1] and "equipment_upgrade_selected"
                or "offer_unavailable")
        elseif #group.cyberdeck_model_offers > 0 then
            return event(group.cyberdeck_model_offers[menu.action_cursor + 1]
                and "cyberdeck_model_selected" or "offer_unavailable")
        elseif #group.cyberdeck_stat_upgrade_offers > 0 then
            return event(group.cyberdeck_stat_upgrade_offers[
                menu.action_cursor + 1]
                and "cyberdeck_stat_upgrade_selected" or "offer_unavailable")
        elseif #group.cyberdeck_rating_upgrade_offers > 0 then
            return event(group.cyberdeck_rating_upgrade_offers[
                menu.action_cursor + 1]
                and "cyberdeck_rating_upgrade_selected" or "offer_unavailable")
        elseif group.id == "sell" then
            return event("sale_selected")
        elseif #group.progression_offers > 0 then
            return event(group.progression_offers[menu.action_cursor + 1]
                and "progression_selected" or "offer_unavailable")
        elseif #group.offers > 0 then
            return event(group.offers[menu.action_cursor + 1]
                and "purchase_selected" or "offer_unavailable")
        end
        return event("source_action_boundary")
    elseif (controller_events & E.Pad.C) ~= 0 then
        menu.action_cursor = 0
        menu.action_panel = menu.menu_result == 1 and 1 or 0
        menu.selected_action = 0
        return event("active_runner_change_requested")
    elseif (controller_events & E.Pad.START) ~= 0 then
        return { transition = T.EXIT, event = "returned_to_field" }
    end
    return event("no_change")
end

return Shop
