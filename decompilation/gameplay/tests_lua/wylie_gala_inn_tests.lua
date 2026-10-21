package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Inn = require("wylie_gala_inn")

local choices = {}
local prompts = {}
local speeches = {}
local service_calls = {}
local payment_result = "paid"
local location_id = "penumbra_site_09_wylies_gala_inn"

game = {
    state = { runner_feature_flag_bit_2 = false },
    location = { type = "building", id = location_id },
    field = { mode = 3, event_code = 1, alternate_result = 0 },
    ui = {
        prompt_text_key = function(id, key, available_choices)
            prompts[#prompts + 1] = {
                id = id,
                key = key,
                choices = available_choices,
            }
            local selected = table.remove(choices, 1)
            assert(selected ~= nil, "unexpected prompt: " .. id)
            assert(key == "wylie.gala_inn.entry" or
                   key == "wylie.gala_inn.room")
            local allowed = false
            for _, option in ipairs(available_choices) do
                if option == selected then allowed = true end
            end
            assert(allowed, "choice " .. selected .. " is not available at " .. id)
            return selected
        end,
        say = function(event_id)
            speeches[#speeches + 1] = event_id
        end,
    },
    services = {
        call = function(domain, operation)
            service_calls[#service_calls + 1] = { domain, operation }
            assert(domain == "building" and operation == "charge_50")
            return payment_result
        end,
    },
}

local function queue(...)
    choices = { ... }
    prompts = {}
    speeches = {}
    service_calls = {}
end

game.field.mode = 2
queue()
assert(not pcall(Inn.present_field_exit_prompt))
assert(#prompts == 0)
game.field.mode = 3

-- Normal arrival offers the source's 50-nuyen room branch.
queue("crash", "pay")
assert(Inn.present_field_exit_prompt() == "hotel-options")
assert(prompts[1].key == "wylie.gala_inn.entry")
assert(prompts[2].key == "wylie.gala_inn.room")
assert(#service_calls == 1 and service_calls[1][2] == "charge_50")
assert(#speeches == 1 and speeches[1] == "room_paid")

payment_result = "insufficient"
queue("crash", "pay")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1] == "room_insufficient")

-- FF018F bit 2 selects the source free-stay page and Hotel Options.
payment_result = "paid"
game.state.runner_feature_flag_bit_2 = true
queue("crash")
assert(Inn.present_field_exit_prompt() == "hotel-options")
assert(#prompts == 1 and #service_calls == 0)
assert(#speeches == 1 and speeches[1] == "free_stay")

-- Unknown restored runner-status data stays an explicit source boundary.
game.state.runner_feature_flag_bit_2 = nil
queue("crash")
assert(Inn.present_field_exit_prompt() == "wylie_feature_state_unavailable")
assert(#service_calls == 0 and #speeches == 0)

-- The signed run result selects the two separately traced ROM text roots.
game.field.alternate_result = 1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1] == "package_after")
game.field.alternate_result = -1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1] == "package_before")

game.field.alternate_result = 0
game.state.runner_feature_flag_bit_2 = false
queue("leave")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#service_calls == 0 and #speeches == 0)

print("Wylie's Gala Inn ROM branches passed")
