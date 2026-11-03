-- Source-mode input actions for prompts whose presenter lives outside the
-- stacked field menu. menu.route owns the optional B cancel policy.
local E = require("enums")
local P, A = E.Pad, E.MenuInputAction

local External = {}

local function input_action(profile, state, events)
    -- The source medical page can contain no treatment rows. Its legacy
    -- controller path accepts B only in that state.
    if profile.empty_choices_b_only and state.has_choices == false then
        return (events & P.B) ~= 0 and (profile.empty_b or A.BACK)
            or A.IGNORE
    end

    local order = profile.order or { "up", "down", "c", "b", "a", "start" }
    for _, input in ipairs(order) do
        if input == "up" and profile.directions ~= false and
           (events & P.UP) ~= 0 then
            return profile.up or A.UP
        elseif input == "down" and profile.directions ~= false and
               (events & P.DOWN) ~= 0 then
            return profile.down or A.DOWN
        elseif input == "a_or_b" and
               (events & (P.A | P.B)) ~= 0 then
            return profile.a_or_b or A.CONFIRM
        elseif input == "a" and (events & P.A) ~= 0 then
            return profile.a or A.CONFIRM
        elseif input == "b" and (events & P.B) ~= 0 then
            return profile.b or A.BACK
        elseif input == "c" and (events & P.C) ~= 0 then
            return profile.c or A.PAGE
        elseif input == "start" and (events & P.START) ~= 0 then
            return profile.start or A.CANCEL
        end
    end
    return A.IGNORE
end

local function make(profile)
    return {
        new = function() return {} end,
        input = function(state, events)
            return { action = input_action(profile, state, events) }
        end,
    }
end

External.startup_options = {
    new = function() return {} end,
    input = function() return { action = A.FORWARD } end,
}
External.hospital_options = make({ b = A.IGNORE, c = A.BACK,
                                   start = A.IGNORE })
External.hospital_medical_attention = make({ empty_choices_b_only = true })
External.hospital_leave_confirmation = make({ b = A.BACK })
External.dialogue_button_choices = make({
    order = { "a", "b", "c", "start" },
    directions = false, a = A.BUTTON_A, b = A.BUTTON_B, c = A.BUTTON_C,
    start = A.BUTTON_A,
})
External.dialogue_cursor_choices = make({})
External.dialogue_vid_phone_contacts = make({
    order = { "up", "down", "a_or_b", "start", "c" },
    a_or_b = A.CONFIRM, c = A.IGNORE,
})
External.dialogue_hosted_choices = make({
    b = A.CONFIRM, c = A.CONFIRM, start = A.IGNORE,
})

return External
