package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local DialogueActions = require("dialogue_actions")
local SpiritEyes = require("salish_spirit_eyes")

local backing = {}
local active_trigger
local choices = {}
local menus = {}
local dialogue = {}
game = {
    field = { mode = 47, event_code = 25, alternate_result = 0 },
    location = { type = "site", id = "salish_feathered_serpent_lair" },
    state = setmetatable({}, {
        __index = function(_, key)
            local index = type(key) == "string" and
                key:match("^story_trigger_active_(%d+)$")
            if index ~= nil then return active_trigger == tonumber(index) end
            return backing[key]
        end,
        __newindex = function(_, key, value)
            local index = type(key) == "string" and
                key:match("^story_trigger_active_(%d+)$")
            if index ~= nil then
                active_trigger = tonumber(index)
                return
            end
            backing[key] = value
        end,
    }),
    ui = {
        prompt_text = function(menu_id, text, options)
            menus[#menus + 1] = { id = menu_id, text = text, options = options }
            local selected = table.remove(choices, 1)
            assert(selected ~= nil, "missing test choice for " .. menu_id)
            for _, option in ipairs(options) do
                if option == selected then return selected end
            end
            error("invalid test choice " .. selected .. " for " .. menu_id)
        end,
        say_text = function(text, continuation)
            dialogue[#dialogue + 1] = {
                text = text, continuation = continuation,
            }
        end,
    },
    services = { call = function() return "applied" end },
}

-- The production ritual graph comes from JSON and is exercised through the
-- C++ GameScriptRuntime integration test. This Lua-only suite checks that its
-- adapter preserves a missing-catalog boundary and that shared effect actions
-- mutate the same Lua-owned facts used by that graph.
local unavailable = SpiritEyes.present()
assert(unavailable.type == "source_boundary" and
       unavailable.reason == "dialogue_graph:content_catalog_missing")

local actions = DialogueActions.new_default_registry()
assert(actions:execute("story.select_fixed_trigger", { triggerId = 9 }))
assert(game.state.story_trigger_active_9 == true and
       game.state.story_flag_bit_9 == nil)
assert(actions:execute("item.set_group_flag", {
    flagBit = 2, value = true,
}))
assert(game.state.group_item_flag_bit_2 == true)
assert(actions:execute("field.set_auxiliary_flag", {
    flagBit = 2, value = false,
}))
assert(game.state.field_auxiliary_flag_bit_2 == false)

-- Keep the still-procedural Licourtrix branch covered until its complete ROM
-- choice/effect graph is independently traced and migrated to JSON.
game.state.story_flag_bit_7 = false
game.state.story_flag_bit_8 = false
game.state.story_flag_bit_9 = false
game.state.group_item_flag_bit_2 = false
choices = {
    "more", "more", "more", "scale", "thanks", "what",
    "more", "more", "more",
}
assert(SpiritEyes.present_serpent_encounter() == "field_return")
assert(active_trigger == 8 and game.state.story_flag_bit_8 == true and
       game.state.group_item_flag_bit_2 == true)
assert(menus[4].id == "serpent_intentions" and
       menus[5].id == "serpent_thanks" and menus[6].id == "serpent_task")
assert(#choices == 0)

print("Spirit Eyes tests passed: JSON graph adapters and Licourtrix item grant")
