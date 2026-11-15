-- Port-only Pocket Secretary options row and B policy submenu behavior.
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Text = require("pocket_secretary")
local Route = require("menu")
local Screens = require("menu.screens.pocket_secretary")
local P, S = E.Pad, E.MenuScreen
local failed = false

local function check(condition, message)
    if not condition then
        failed = true
        print("FAIL " .. message)
    end
end

local root_ids, _, root_text_keys = Text.root()
check(#root_ids == 6 and root_ids[6] == "options" and
      root_text_keys[6] == "pocket_secretary.option.options",
      "Pocket Secretary root must end with the OPTIONS row")

check(type(Text.options) == "function",
      "Pocket Secretary option text keys must be defined in Lua")
check(type(Screens.Options) == "table",
      "Pocket Secretary OPTIONS screen must be registered in Lua")

if type(Text.options) == "function" and type(Screens.Options) == "table" and
   S.POCKET_SECRETARY_OPTIONS ~= nil then
    local option_ids, title_key, option_keys = Text.options()
    check(#option_ids == 2 and option_ids[1] == "b_button_rom" and
          option_ids[2] == "b_button_always_cancel" and
          title_key == "pocket_secretary.options.title" and
          option_keys[1] == "pocket_secretary.options.option.b_button_rom" and
          option_keys[2] == "pocket_secretary.options.option.b_button_always_cancel",
          "OPTIONS must define both B policy text keys")

    local policy = E.MenuBPolicy.ROM
    local game = {
        pocket_extra_row = function() return false end,
        source_random_bound = function() return 0 end,
        secretary_effect_tick = function() end,
        menu_b_policy = function() return policy end,
        set_menu_b_policy = function(value) policy = value end,
    }
    local host = {
        blank = function() end,
        redraw = function() end,
        set_window = function() end,
        set_screen = function() end,
        play_sound = function() end,
        menu_closed = function() end,
    }
    local route = Route.new(host, game)
    route:open(S.POCKET_SECRETARY)
    for _ = 1, 5 do route:frame(P.DOWN) end
    check(route:top().state.row == 5,
          "Down must reach OPTIONS after the existing root entries")
    route:frame(P.A)
    check(route:top().screen == S.POCKET_SECRETARY_OPTIONS,
          "A on OPTIONS must open the submenu")
    if route:top().screen == S.POCKET_SECRETARY_OPTIONS then
        check(route:top().state.labels[1] == "b_button_rom",
              "OPTIONS must display the current ROM policy")
        route:frame(P.B)
        check(route:top().screen == S.POCKET_SECRETARY and
              route:top().state.row == 5,
              "ROM-mode B must return to the root with OPTIONS selected")

        route:frame(P.A)
        route:frame(P.LEFT)
        check(policy == E.MenuBPolicy.ALWAYS_CANCEL and
              route:top().state.labels[1] == "b_button_always_cancel",
              "Left must apply and display ALWAYS CANCEL immediately")
        route:frame(P.RIGHT)
        check(policy == E.MenuBPolicy.ROM and
              route:top().state.labels[1] == "b_button_rom",
              "Right must toggle back to ROM")
        route:frame(P.A)
        check(policy == E.MenuBPolicy.ALWAYS_CANCEL and
              route:top().state.labels[1] == "b_button_always_cancel",
              "A must toggle the B policy")
        route:frame(P.B)
        check(route:top().screen == S.POCKET_SECRETARY and
              route:top().state.row == 5,
              "ALWAYS CANCEL B must return with OPTIONS selected")
    end
end

if failed then os.exit(1) end
print("Pocket Secretary OPTIONS root, toggle, and back behavior passed")
