-- Whitelist for gameplay effects used by JSON dialogue graphs.
-- JSON can select a named operation and data; it cannot call Lua functions.
local DialogueActions = {}
local Registry = {}
Registry.__index = Registry
local StoryFlags = require("story_flags")

local function is_identifier(value)
    return type(value) == "string" and value ~= "" and
        value:match("^[%w_%-%.]+$") ~= nil
end

local function is_integer(value, minimum, maximum)
    return type(value) == "number" and value % 1 == 0 and
        value >= minimum and value <= maximum
end

function Registry.new()
    return setmetatable({ operations = {} }, Registry)
end

function Registry:register(action_id, handler)
    assert(is_identifier(action_id), "dialogue action id is invalid")
    assert(type(handler) == "function", "dialogue action needs a handler")
    assert(self.operations[action_id] == nil,
        "dialogue action is already registered: " .. action_id)
    self.operations[action_id] = handler
    return self
end

function Registry:execute(action_id, arguments, context)
    local handler = self.operations[action_id]
    if handler == nil then
        return false, nil, "unregistered action: " .. tostring(action_id)
    end
    local ok, result = pcall(handler, arguments or {}, context)
    if not ok then return false, nil, tostring(result) end
    if result == nil or result == true then return true end
    if is_identifier(result) then return true, result end
    return false, nil, "action returned an unsupported result"
end

local function register_story_actions(registry)
    registry:register("story.begin_trigger", function(arguments)
        assert(is_integer(arguments.triggerId, 0, 31),
            "story.begin_trigger needs triggerId 0..31")
        StoryFlags.begin_trigger(arguments.triggerId)
        return true
    end)

    registry:register("story.finish_trigger", function(arguments)
        assert(is_integer(arguments.triggerId, 0, 31),
            "story.finish_trigger needs triggerId 0..31")
        StoryFlags.finish_trigger(arguments.triggerId)
        return true
    end)

    registry:register("story.set_flag", function(arguments)
        assert(is_integer(arguments.flagBit, 0, 31) and
               type(arguments.value) == "boolean",
            "story.set_flag needs a flagBit and boolean value")
        game.state["story_flag_bit_" .. arguments.flagBit] = arguments.value
        return true
    end)

    registry:register("field.set_mode_effect_flag", function(arguments)
        assert(is_integer(arguments.flagBit, 0, 31) and
               type(arguments.value) == "boolean",
            "field.set_mode_effect_flag needs a flagBit and boolean value")
        game.state["field_mode_effect_flag_bit_" .. arguments.flagBit] =
            arguments.value
        return true
    end)
end

function DialogueActions.new_default_registry()
    local registry = Registry.new()
    register_story_actions(registry)
    return registry
end

DialogueActions.Registry = Registry
return DialogueActions
