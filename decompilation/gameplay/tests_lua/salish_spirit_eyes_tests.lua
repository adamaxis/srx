package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local SpiritEyes = require("salish_spirit_eyes")

local backing = {}
local active_trigger
game = {
    field = { mode = 4, event_code = 27, alternate_result = 0 },
    location = { type = "site", id = "council_spirit_eyes_site" },
    modules = { world_content_data = { entities = {}, areas = {} } },
}
game.state = setmetatable({}, {
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
            assert(value == true, "the source has no FFDD64 clear operation")
            active_trigger = tonumber(index)
            return
        end
        backing[key] = value
    end,
})

local choices = {}
local menus = {}
local dialogue = {}
game.ui = {
    prompt_text = function(menu_id, text, options)
        menus[#menus + 1] = { id = menu_id, text = text, options = options }
        local selected = table.remove(choices, 1)
        assert(selected ~= nil, "missing choice for " .. menu_id)
        for _, option in ipairs(options) do
            if option == selected then return selected end
        end
        error("invalid choice " .. selected .. " for " .. menu_id)
    end,
    say_text = function(text, continuation)
        dialogue[#dialogue + 1] = { text = text, continuation = continuation }
    end,
}

local function choose(...)
    choices = { ... }
end

local function clear_menus_and_dialogue()
    menus = {}
    dialogue = {}
end

game.location.id = "wrong_site"
assert(SpiritEyes.present() == "denied")
assert(backing.story_flag_bit_5 == nil)

game.location.id = "council_spirit_eyes_site"
choose("more")
assert(SpiritEyes.present() == "field_return")
assert(game.state.story_flag_bit_5 == true and active_trigger == 5)
assert(game.state.field_mode_effect_flag_bit_14 == false and
       game.state.field_mode_effect_flag_bit_15 == true and
       game.state.field_mode_effect_flag_bit_16 == true and
       game.state.field_auxiliary_flag_bit_0 == true)
assert(menus[1].id == "first_contact" and
       dialogue[#dialogue].continuation == false)

clear_menus_and_dialogue()
assert(SpiritEyes.present() == "field_return")
assert(dialogue[#dialogue].text:find("horn of a gargoyle", 1, true))

backing.group_item_flag_bit_4 = true
assert(SpiritEyes.present() == "field_return")
assert(game.state.group_item_flag_bit_4 == false and
       game.state.story_flag_bit_6 == true)
assert(game.state.field_mode_effect_flag_bit_16 == false and
       game.state.field_mode_effect_flag_bit_17 == true and
       game.state.field_auxiliary_flag_bit_0 == false and
       game.state.field_auxiliary_flag_bit_1 == true)

clear_menus_and_dialogue()
assert(SpiritEyes.present() == "field_return")
assert(dialogue[#dialogue].text:find("hell hound pelt", 1, true))

backing.group_item_flag_bit_3 = true
assert(SpiritEyes.present() == "field_return")
assert(game.state.group_item_flag_bit_3 == false and
       game.state.story_flag_bit_7 == true)
assert(game.state.field_mode_effect_flag_bit_17 == false and
       game.state.field_mode_effect_flag_bit_18 == true and
       game.state.field_auxiliary_flag_bit_1 == false and
       game.state.field_auxiliary_flag_bit_2 == true)

clear_menus_and_dialogue()
assert(SpiritEyes.present() == "field_return")
assert(dialogue[#dialogue].text:find("living feather", 1, true))

backing.story_flag_bit_8 = true
assert(SpiritEyes.present() == "field_return")
assert(dialogue[#dialogue].text:find("living feather", 1, true))

backing.group_item_flag_bit_2 = true
choose("more")
assert(SpiritEyes.present() == "field_return")
assert(game.state.group_item_flag_bit_2 == false and
       game.state.story_flag_bit_9 == true and active_trigger == 9)
assert(game.state.field_mode_effect_flag_bit_15 == false and
       game.state.field_mode_effect_flag_bit_18 == false and
       game.state.field_mode_effect_flag_bit_19 == true and
       game.state.field_auxiliary_flag_bit_2 == false)

-- The ROM's separate scene-47 continuation selects FFDD64=8 before the
-- encounter dialogue. Its post-dialogue 0x53F50 path marks story bit 8 and
-- grants the Feathered Scale by setting FFFC22 bit 2.
clear_menus_and_dialogue()
game.location.id = "salish_feathered_serpent_lair"
game.field.event_code = 25
game.field.mode = 47
backing.story_flag_bit_7 = false
backing.story_flag_bit_8 = false
backing.story_flag_bit_9 = false
backing.group_item_flag_bit_2 = false
choose("more", "more", "more", "scale", "thanks", "what",
       "more", "more", "more")
assert(SpiritEyes.present_serpent_encounter() == "field_return")
assert(active_trigger == 8 and game.state.story_flag_bit_8 == true and
       game.state.group_item_flag_bit_2 == true)
assert(menus[4].id == "serpent_intentions" and
       menus[5].id == "serpent_thanks" and
       menus[6].id == "serpent_task")
assert(#choices == 0)

-- Verify the item produced by Licourtrix completes the same ritual branch
-- that consumes Group Item bit 2 and advances the active story trigger to 9.
clear_menus_and_dialogue()
game.location.id = "council_spirit_eyes_site"
game.field.event_code = 27
game.field.mode = 4
backing.story_flag_bit_7 = true
assert(SpiritEyes.present() == "field_return")
assert(game.state.story_flag_bit_9 == true and active_trigger == 9 and
       game.state.group_item_flag_bit_2 == false)
assert(dialogue[#dialogue].text:find("completed your tasks", 1, true))
assert(#choices == 0)

print("Spirit Eyes tests passed: source item gates, fixed triggers, and lair return")
