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

local function register_core_actions(registry)
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

    registry:register("story.select_fixed_trigger", function(arguments)
        assert(is_integer(arguments.triggerId, 0, 31),
            "story.select_fixed_trigger needs triggerId 0..31")
        StoryFlags.select_fixed_trigger(arguments.triggerId)
        return true
    end)

    registry:register("story.apply_dialogue_command", function(arguments)
        assert(is_integer(arguments.commandId, 0, 255),
            "story.apply_dialogue_command needs commandId 0..255")
        local marked = StoryFlags.apply_dialogue_command(
            arguments.commandId, arguments.options)
        return marked == nil and "command_unresolved" or true
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

    registry:register("field.set_auxiliary_flag", function(arguments)
        assert(is_integer(arguments.flagBit, 0, 31) and
               type(arguments.value) == "boolean",
            "field.set_auxiliary_flag needs a flagBit and boolean value")
        game.state["field_auxiliary_flag_bit_" .. arguments.flagBit] =
            arguments.value
        return true
    end)

    registry:register("item.set_group_flag", function(arguments)
        assert(is_integer(arguments.flagBit, 0, 14) and
               type(arguments.value) == "boolean",
            "item.set_group_flag needs a group-item flagBit and boolean value")
        game.state["group_item_flag_bit_" .. arguments.flagBit] =
            arguments.value
        return true
    end)

    registry:register("runner.adjust_source_byte_triplet", function(arguments)
        assert(is_integer(arguments.amount, 1, 0xFF),
            "runner.adjust_source_byte_triplet needs amount 1..255")
        local result = game.services.call(
            "runner", "adjust_source_byte_triplet_" ..
                tostring(arguments.amount))
        return result == "applied" and "applied" or "service_unavailable"
    end)

    registry:register("building.charge_amount", function(arguments)
        assert(is_integer(arguments.amount, 0, 999999),
            "building.charge_amount needs an amount from 0 to 999999")
        local result = game.services.call(
            "building", "charge_" .. tostring(arguments.amount))
        if result == "paid" or result == "insufficient" then
            return result
        end
        return "service_unavailable"
    end)

    registry:register("field.queue_mako_actor_list", function()
        local result = game.services.call(
            "field", "queue_mako_actor_list")
        return result == "queued" and result or "service_unavailable"
    end)

    -- Shared FFFBFB progression step implemented by ROM helper 0x1F1E0.
    registry:register("field.advance_strike_team_counter", function()
        local result = game.services.call(
            "field", "advance_strike_team_counter")
        return result == "applied" and result or "service_unavailable"
    end)

    registry:register("field.apply_mako_callback", function()
        local result = require("club_penumbra").apply_mako_callback()
        return result == "applied" and result or "service_unavailable"
    end)
end

function DialogueActions.new_default_registry(registered_actions)
    local registry = Registry.new()
    register_core_actions(registry)
    registered_actions = registered_actions or {}
    assert(type(registered_actions) == "table",
        "registered dialogue actions must be a table")
    local action_ids = {}
    for action_id in pairs(registered_actions) do
        action_ids[#action_ids + 1] = action_id
    end
    table.sort(action_ids)
    for _, action_id in ipairs(action_ids) do
        registry:register(action_id, registered_actions[action_id])
    end
    return registry
end

DialogueActions.Registry = Registry
return DialogueActions
