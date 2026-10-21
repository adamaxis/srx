package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Inn = require("tarislar_city_inn")

local location_id = "puyallup_site_03_tarislar_city_inn"
local choices = {}
local prompts = {}
local speeches = {}
local service_calls = {}
local payment_result = "paid"
local triplet_result = "applied"

game = {
    state = {
        story_flag_bit_0 = true,
        story_flag_bit_1 = false,
        story_flag_bit_13 = false,
        story_flag_bit_30 = false,
    },
    location = { type = "building", id = location_id },
    field = { mode = 1, event_code = 1, alternate_result = 0 },
    ui = {
        prompt_text = function(id, text, available_choices)
            prompts[#prompts + 1] = {
                id = id,
                text = text,
                choices = available_choices,
            }
            local selected = table.remove(choices, 1)
            assert(selected ~= nil, "unexpected prompt: " .. id)
            local allowed = false
            for _, option in ipairs(available_choices) do
                if option == selected then allowed = true end
            end
            assert(allowed, "choice " .. selected .. " is not available at " .. id)
            return selected
        end,
        say_text = function(text, continuation)
            speeches[#speeches + 1] = {
                text = text,
                continuation = continuation == true,
            }
        end,
    },
    services = {
        call = function(domain, operation)
            service_calls[#service_calls + 1] = { domain, operation }
            if domain == "building" and operation == "charge_30" then
                return payment_result
            end
            if domain == "runner" and
               operation == "adjust_source_byte_triplet_1" then
                return triplet_result
            end
            return nil
        end,
    },
}

local function queue(...)
    choices = { ... }
    prompts = {}
    speeches = {}
    service_calls = {}
end

local function location_task_result(expected, ...)
    queue(...)
    assert(Inn.present_indexed_title_service() == expected)
    assert(#choices == 0, "unused menu choice")
end

-- The indexed title-service prerequisites come from scene/event and FFFC1E.
game.location.id = "another_building"
queue("more")
assert(Inn.present_indexed_title_service() == "denied")
assert(#prompts == 0)
game.location.id = location_id
game.field.mode = 2
assert(Inn.present_indexed_title_service() == "denied")
game.field.mode = 1

-- The ordinary selector exposes Frosty only for FFFC1E bit 13 and a clear
-- FFFC1E bit 30. It keeps the ROM's stay-here result.
queue("more", "leave")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#prompts[2].choices == 2 and prompts[2].choices[2] == "leave")
game.state.story_flag_bit_13 = true
queue("more", "frosty")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(prompts[2].choices[2] == "frosty")
assert(game.state.notebook_tip_flag_bit_23 == true)
assert(game.state.field_mode_effect_flag_bit_4 == true)
assert(game.state.field_mode_effect_flag_bit_13 == true)
assert(game.state.field_mode_effect_flag_bit_21 == true)
game.state.story_flag_bit_13 = false

-- Scene 1's signed alternate result selects the two package text nodes.
game.field.alternate_result = 1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1].text:find("package", 1, true))
game.field.alternate_result = -1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1].text:find("roots around", 1, true))
game.field.alternate_result = 0

-- Room payment uses the generic building service and returns the hotel menu
-- only after a successful 30-nuyen transaction.
payment_result = "paid"
location_task_result("hotel-options", "more", "crash", "pay")
assert(#service_calls == 1 and service_calls[1][1] == "building" and
       service_calls[1][2] == "charge_30")
assert(#speeches == 1 and speeches[1].text:find("towels", 1, true))
payment_result = "insufficient"
location_task_result("field_return", "more", "crash", "pay")
assert(#service_calls == 1 and service_calls[1][2] == "charge_30")
assert(#speeches == 1 and speeches[1].text:find("low on creds", 1, true))

-- Command 0x16 marks the active trigger before Tabatha's conversation;
-- command 0x14's typed source triplet call and story masks happen only after
-- she agrees to receive the runner.
game.state.story_flag_bit_1 = false
game.state.story_trigger_active_1 = nil
triplet_result = "applied"
location_task_result("field_return",
    "more", "shale", "more", "more", "more", "about_michael", "more",
    "caleb", "errascoe", "recruit")
assert(game.state.story_trigger_active_1 == true)
assert(game.state.story_flag_bit_1 == true)
assert(game.state.notebook_tip_flag_bit_2 == true)
assert(game.state.notebook_tip_flag_bit_4 == true)
assert(game.state.notebook_tip_flag_bit_5 == true)
assert(game.state.notebook_tip_flag_bit_6 == true)
assert(game.state.notebook_tip_flag_bit_8 == true)
assert(game.state.field_mode_effect_flag_bit_2 == true)
assert(game.state.field_mode_effect_flag_bit_4 == true)
assert(game.state.mr_johnsons_flag_bit_2 == true)
assert(game.state.mr_johnsons_flag_bit_4 == true)
assert(service_calls[1][1] == "runner" and
       service_calls[1][2] == "adjust_source_byte_triplet_1")
assert(Inn.present_indexed_title_service() == "denied",
       "the one-shot Tabatha service ran after FFFC1E bit 1 was set")

-- The source encounter commands are an explicit unresolved task result;
-- the host turns this into a visible source boundary.
game.state.story_flag_bit_1 = false
game.state.story_flag_bit_30 = false
game.state.story_flag_bit_13 = false
triplet_result = "applied"
location_task_result("tabatha_ambush_unconverted",
    "more", "shale", "more", "more", "more", "about_michael", "more",
    "caleb", "errascoe", "thanks")
assert(speeches[#speeches].continuation == true)

-- If the session cannot apply command 0x14's source byte update, the script
-- reports that failure instead of claiming acceptance completed.
game.state.story_flag_bit_1 = false
triplet_result = "unavailable"
location_task_result("tabatha_state_unavailable",
    "more", "shale", "more", "more", "more")

print("Tarislar City Inn tests passed: gates, stay payment, Tabatha state and combat boundary")
