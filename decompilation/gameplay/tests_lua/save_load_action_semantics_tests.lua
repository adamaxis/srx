local E = require("enums")
local Screens = require("menu.screens.save_load")
local P = E.Pad

local function check(condition, message)
    if not condition then error(message, 2) end
end

local function context()
    local calls = {}
    return {
        calls = calls,
        redraw = function() end,
        game = {
            scene_mode = function() return E.SceneMode.NONE end,
            in_combat = function() return false end,
            save_game = function(slot)
                calls.save = slot
                return true
            end,
            load_game = function(slot)
                calls.load = slot
                return true
            end,
        },
    }
end

local function open_slot(action, events)
    local ctx = context()
    local menu = Screens.SaveLoad.new()
    Screens.SaveLoad.enter(menu, ctx)
    if events ~= 0 then Screens.SaveLoad.input(menu, events, ctx) end
    local expected_row = action == "save" and 0 or 1
    check(menu.row == expected_row and menu.selected_action == action,
          "cursor row and semantic action agree at the visible choice")

    local route = Screens.SaveLoad.input(menu, P.A, ctx)
    check(route and route.args.operation == action,
          "save/load route carries its named operation")
    check(route.args.op == nil,
          "save/load route does not carry the old numeric op selector")

    local slot = Screens.Slot.new(route.args)
    Screens.Slot.enter(slot, ctx)
    check(slot.operation == action, "slot state retains the semantic operation")
    check(slot.title == (action == "save" and "save_game" or "load_game"),
          "slot title resolves from the semantic operation")
    local result = Screens.Slot.input(slot, P.A, ctx)
    return ctx, slot, result
end

-- ROM 0xC2600 establishes the displayed Save then Load order [C]. The row
-- remains only the cursor position; Lua carries operation identity by name.
local initial = Screens.SaveLoad.new()
check(initial.row == 0 and initial.selected_action == "save",
      "initial save selection separates cursor position from action identity")

local save_ctx, save_slot, save_result = open_slot("save", 0)
check(save_slot.row == 0 and save_ctx.calls.save == 0 and
      save_ctx.calls.load == nil,
      "save action selects the host save operation for the selected slot")
check(save_result.args.message == E.MenuMessage.GAME_SAVED and
      save_result.args.data.done,
      "successful save returns the save completion message")

local load_ctx, load_slot, load_result = open_slot("load", P.DOWN)
check(load_slot.row == 0 and load_ctx.calls.load == 0 and
      load_ctx.calls.save == nil,
      "load action selects the host load operation for the selected slot")
check(load_result.args.message == E.MenuMessage.LOAD_OK and
      load_result.args.data.done,
      "successful load returns the load completion message")

local invalid_ctx = context()
local invalid_slot = Screens.Slot.new({ operation = "unknown" })
Screens.Slot.enter(invalid_slot, invalid_ctx)
local invalid_result = Screens.Slot.input(invalid_slot, P.A, invalid_ctx)
check(invalid_result == nil and invalid_ctx.calls.save == nil and
      invalid_ctx.calls.load == nil,
      "unknown operation cannot fall through to loading")

print("save/load semantic action tests passed")
