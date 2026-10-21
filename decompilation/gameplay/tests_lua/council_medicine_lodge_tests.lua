package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Lodge = require("council_medicine_lodge")

local location_id = "council_medicine_lodge"
local selected_choice
local prompts = {}
local speeches = {}

game = {
    state = {
        story_flag_bit_4 = false,
        runner_information_bit_4 = false,
    },
    location = { type = "building", id = location_id },
    field = { mode = 4, event_code = 3, alternate_result = 0 },
    ui = {
        prompt_text_key = function(id, text_key, choices)
            prompts[#prompts + 1] = {
                id = id,
                text_key = text_key,
                choices = choices,
            }
            assert(selected_choice ~= nil, "unexpected prompt: " .. id)
            for _, choice in ipairs(choices) do
                if choice == selected_choice then
                    local result = selected_choice
                    selected_choice = nil
                    return result
                end
            end
            error("choice " .. selected_choice .. " is not available at " .. id)
        end,
        say = function(text_key)
            speeches[#speeches + 1] = text_key
        end,
    },
}

local function reset(choice)
    selected_choice = choice
    prompts = {}
    speeches = {}
end

reset("hire_shaman")
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 1 and
       prompts[1].text_key == "council_medicine_lodge.entry.default" and
       prompts[1].choices[1] == "hire_shaman" and
       prompts[1].choices[2] == "browse_store" and
       prompts[1].choices[3] == "talk_later")
assert(game.state.runner_information_bit_4 == true,
       "command 0xF0 did not set the runner-information flag")
assert(#speeches == 1 and speeches[1] == "hire_shaman_response")

game.state.story_flag_bit_4 = true
game.state.runner_information_bit_4 = false
reset("wise_one")
assert(Lodge.present_field_exit_prompt() ==
       "medicine_lodge_wise_one_followup")
assert(prompts[1].choices[3] == "wise_one")
assert(#speeches == 1 and speeches[1] == "wise_one_followup")

game.state.story_flag_bit_4 = false
reset("browse_store")
assert(Lodge.present_field_exit_prompt() ==
       "medicine_lodge_store_requested")
assert(#speeches == 0)

reset("talk_later")
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1] == "farewell")

game.field.alternate_result = -1
reset(nil)
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 0 and #speeches == 1 and
       speeches[1] == "package_handoff")

game.field.alternate_result = 1
reset(nil)
assert(Lodge.present_field_exit_prompt() == "field_return")
assert(#prompts == 0 and #speeches == 1 and
       speeches[1] == "package_payment")

game.field.alternate_result = 0
game.field.event_code = 4
assert(not pcall(Lodge.present_field_exit_prompt),
       "the Medicine Lodge prompt accepted the wrong ROM event")
game.field.event_code = 3
game.location.id = "another_building"
assert(not pcall(Lodge.present_field_exit_prompt),
       "the Medicine Lodge prompt accepted the wrong location")

print("Council Medicine Lodge tests passed: ROM prompt, quest choice, command and alternate-result branches")
