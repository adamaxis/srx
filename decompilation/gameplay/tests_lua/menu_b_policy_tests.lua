package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Route = require("menu")
local P, S, T = E.Pad, E.MenuScreen, E.MenuTransition

local ids = {}
for name, id in pairs(S) do ids[#ids + 1] = { name = name, id = id } end
table.sort(ids, function(left, right) return left.id < right.id end)

local function fixture(policy)
    local calls = 0
    local received = nil
    local closed = 0
    local host = {
        play_sound = function() end,
        redraw = function() end,
        blank = function() end,
        set_window = function() end,
        set_screen = function() end,
        menu_closed = function() closed = closed + 1 end,
        legacy_open = function() end,
        legacy_advance = function() return true end,
        notebook_boundary = function() end,
    }
    local game = { menu_b_policy = policy }
    local route = Route.new(host, game)
    return route, {
        handler_calls = function() return calls end,
        received_events = function() return received end,
        closed = function() return closed end,
        handler = function()
            return {
                new = function() return {} end,
                input = function(_, events)
                    calls = calls + 1
                    received = events
                    return { transition = T.NONE }
                end,
            }
        end,
    }
end

local failures, checks = 0, 0
local function check(value, label)
    checks = checks + 1
    if not value then
        failures = failures + 1
        io.write("FAIL ", label, "\n")
    end
end

for _, item in ipairs(ids) do
    local rom_route, rom = fixture(E.MenuBPolicy.ROM)
    Route.register(item.id, rom.handler())
    rom_route:open(item.id)
    rom_route:frame(P.B)
    check(rom_route:is_open() and rom.handler_calls() == 1 and
          rom.received_events() == P.B and rom.closed() == 0,
          item.name .. " ROM policy forwards B unchanged")

    local always_root, root = fixture(function()
        return E.MenuBPolicy.ALWAYS_CANCEL
    end)
    Route.register(item.id, root.handler())
    always_root:open(item.id)
    always_root:frame(P.B)
    check(not always_root:is_open() and root.handler_calls() == 0 and
          root.closed() == 1,
          item.name .. " ALWAYS_CANCEL closes a root without calling its handler")

    local always_child, child = fixture(E.MenuBPolicy.ALWAYS_CANCEL)
    Route.register(S.COMMAND, {
        new = function() return {} end,
        input = function() return { transition = T.NONE } end,
    })
    Route.register(item.id, child.handler())
    always_child:open(S.COMMAND)
    always_child:apply(always_child:top(), {
        transition = T.PUSH, screen = item.id, args = {},
    })
    always_child:frame(P.B)
    check(#always_child:screens() == 1 and
          always_child:top().screen == S.COMMAND and
          child.handler_calls() == 0 and child.closed() == 0,
          item.name .. " ALWAYS_CANCEL pops one child without calling its handler")
end

-- No setting or an unrecognized older config value resolves to ROM behavior.
do
    local route, state = fixture(nil)
    Route.register(S.MESSAGE, state.handler())
    route:open(S.MESSAGE)
    route:frame(P.B)
    check(route:is_open() and state.handler_calls() == 1 and
          state.received_events() == P.B,
          "missing policy defaults to ROM")
end

do
    local route, state = fixture("older-invalid-value")
    Route.register(S.MESSAGE, state.handler())
    route:open(S.MESSAGE)
    route:frame(P.B)
    check(route:is_open() and state.handler_calls() == 1 and
          state.received_events() == P.B,
          "unknown config value defaults to ROM")
end

io.write(string.format("menu B policy tests: %d/%d passed across %d screens\n",
                       checks - failures, checks, #ids))
os.exit(failures == 0 and 0 or 1)
