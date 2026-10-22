-- route.lua: the menu route coordinator. Owns an explicit screen stack and is the ONLY thing that
-- changes which screen receives input. C++ calls route.frame(events) once per VBlank, right after
-- 0xBD2/0xBF2/0x195C sampling (docs/09 section 1) [C: ROM menu loops sample once per frame].
-- Replaces the FieldSceneRuntime priority chain of ~12 independent "child open" booleans
-- (FSR 10498-10960); see MIGRATION.md D1.
local E = require("enums")
local T = E.MenuTransition

local Route = {}
Route.__index = Route

local registry = {}                     -- MenuScreen id -> screen module
function Route.register(id, module) registry[id] = module end

-- host: C++ presenter/input binding (menu_binding.hpp). game: rule-side game services.
function Route.new(host, game)
    local r = setmetatable({ host = host, game = game, stack = {}, frame_no = 0 }, Route)
    r.ctx = {
        host = host, game = game,
        sound = function(id) host.play_sound(id) end,
        redraw = function(mask) if mask and mask ~= 0 then host.redraw(mask) end end,
    }
    return r
end

function Route:top() return self.stack[#self.stack] end
function Route:root() return self.stack[1] end
function Route:is_open() return #self.stack > 0 end

-- One screen-local presentation contract: the window plane follows the TOP screen.
local function apply_presentation(self)
    local top = self:top()
    if not top then self.host.set_window(E.WindowPlane.SIDEBAR); return end
    local m = registry[top.screen]
    self.host.set_window(m.window or E.WindowPlane.SIDEBAR)
    self.host.set_screen(top.screen, top.state)
end

local function push(self, screen, args)
    local m = assert(registry[screen], "unregistered screen " .. tostring(screen))
    local entry = {
        screen = screen,
        state = m.new(args or {}),
        debug_enter_callback_present = m.enter ~= nil,
        debug_enter_called = false,
        debug_enter_transition = T.NONE,
    }
    self.stack[#self.stack + 1] = entry
    apply_presentation(self)
    if m.enter then
        entry.debug_enter_called = true
        local result = m.enter(entry.state, self.ctx)
        if result and result.transition ~= nil then
            entry.debug_enter_transition = result.transition
        end
        self:apply(entry, result)
    end
end

-- Apply a screen result: { transition = E.MenuTransition.*, screen = id, args = {}, result = {} }
function Route:apply(entry, res)
    if not res or res.transition == nil or res.transition == T.NONE then return end
    local t = res.transition
    if t == T.PUSH then
        push(self, res.screen, res.args)
    elseif t == T.POP then
        assert(self:top() == entry, "only the top screen may pop")
        self.stack[#self.stack] = nil
        apply_presentation(self)
        local parent = self:top()
        if parent then
            local pm = registry[parent.screen]
            if pm.child_returned then
                self:apply(parent, pm.child_returned(parent.state, entry.screen, res.result or {}, self.ctx))
            end
        end
    elseif t == T.REPLACE then
        self.stack[#self.stack] = nil
        push(self, res.screen, res.args)
    elseif t == T.REBUILD then
        local screen = res.screen or (self.stack[1] and self.stack[1].screen)
        assert(screen ~= nil, "rebuild requires a root screen")
        self.stack = {}
        push(self, screen, res.args)
    elseif t == T.EXIT then        -- leave the whole menu tree (Start, FFF0B1, FFF0CD)
        self.stack = {}
        apply_presentation(self)
        local result = res.result or {}
        self.host.menu_closed(
            result.matrix_logout_transition_complete == true)
    end
end

function Route:open(screen, args)
    assert(#self.stack == 0, "menu already open")   -- FSR open_command_overview guard (FSR 5390)
    push(self, screen, args)
end

-- Scene changes can cancel the in-game menu without manufacturing an input
-- edge. The host still uses menu_closed() as the single cleanup boundary.
function Route:close()
    if #self.stack == 0 then return end
    self.stack = {}
    apply_presentation(self)
    self.host.menu_closed(false)
end

function Route:frame(events)
    self.frame_no = self.frame_no + 1
    local top = self:top()
    if not top then return end
    local m = registry[top.screen]
    self:apply(top, m.input(top.state, events, self.ctx))
end

function Route:screens()   -- for tests / debug overlay
    local out = {}
    for i, e in ipairs(self.stack) do out[i] = e.screen end
    return out
end

return Route
