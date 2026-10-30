package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local DialogueActions = require("dialogue_actions")
local SpiritEyes = require("salish_spirit_eyes")
local E = require("enums")

local backing = {}
local choices = {}
local menus = {}
local dialogue = {}
game = {
    field = { package_run_status = "none" },
    location = { type = "site", id = "salish_feathered_serpent_lair" },
    state = setmetatable({}, {
        __index = function(_, key) return backing[key] end,
        __newindex = function(_, key, value) backing[key] = value end,
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
assert(actions:execute("story.set_fact", {
    factId = E.StoryFact.SERPENT_SCALE_TAKEN, value = true,
}))
assert(game.state.serpent_scale_taken == true and
       game.state.shaman_tasks_done == nil)
assert(actions:execute("item.set_group_item", {
    itemId = E.GroupItemId.FEATHERED_SCALE, value = true,
}))
assert(game.state["group_item.feathered_scale"] == true)
assert(actions:execute("field.set_auxiliary_flag", {
    flagBit = 2, value = false,
}))
assert(game.state.field_auxiliary_flag_bit_2 == false)

-- The Licourtrix conversation is content-catalog owned now; this Lua-only
-- adapter suite has no loaded content package, so the graph remains an
-- explicit boundary here. ROM state permutations are covered by the C++
-- GameScriptRuntime integration test using the production JSON catalogs.
game.state.shaman_pelt_given = true
game.state.serpent_scale_taken = false
game.state.shaman_tasks_done = false
game.state["group_item.feathered_scale"] = false
local licourtrix_unavailable = SpiritEyes.present_serpent_encounter()
assert(type(licourtrix_unavailable) == "table" and
       licourtrix_unavailable.type == "source_boundary" and
       licourtrix_unavailable.reason ==
           "dialogue_graph:content_catalog_missing" and
       game.state.serpent_scale_taken == false and
       game.state["group_item.feathered_scale"] == false)

print("Spirit Eyes tests passed: graph adapter boundaries and shared Lua actions")
