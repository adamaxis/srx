package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Apartments = require("tarislar_apartments")

local location_id = "puyallup_site_02_tarislar_garden_apartments"
local choices = {}
local prompts = {}
local speeches = {}

game = {
    state = {
        story_flag_bit_16 = false,
        story_flag_bit_21 = true,
        story_flag_bit_26 = false,
        story_flag_bit_27 = false,
        story_flag_bit_28 = false,
        story_flag_bit_30 = true,
        group_item_flag_bit_7 = false,
        group_item_flag_bit_8 = false,
    },
    location = { type = "building", id = location_id },
    field = { alternate_result = 0 },
    ui = {
        prompt_text = function(id, text, available_choices)
            local prompt = { id = id, text = text, choices = {} }
            for index, choice in ipairs(available_choices) do
                prompt.choices[index] = choice
            end
            prompts[#prompts + 1] = prompt
            local selected = table.remove(choices, 1)
            assert(selected ~= nil, "unexpected prompt: " .. id)
            local allowed = false
            for _, choice in ipairs(available_choices) do
                if choice == selected then allowed = true end
            end
            assert(allowed,
                "choice " .. selected .. " is not available at " .. id)
            return selected
        end,
        say_text = function(text, continuation)
            speeches[#speeches + 1] = {
                text = text,
                continuation = continuation == true,
            }
        end,
    },
}

local function queue(...)
    choices = { ... }
    prompts = {}
    speeches = {}
end

local function set_story(bit, value)
    game.state["story_flag_bit_" .. bit] = value
end

local function expect_repeat(allowed_choices, ...)
    queue(...)
    assert(Apartments.present_repeat_service() == "left")
    assert(#choices == 0, "unused prompt choice")
    assert(prompts[1].id == "repeat_service")
    assert(prompts[1].text ==
        "Hello again, Joshua.||||You have bought my time with your pain.||||Spend it as you wish.")
    assert(#prompts[1].choices == #allowed_choices)
    for index, expected in ipairs(allowed_choices) do
        assert(prompts[1].choices[index] == expected)
    end
end

-- Service 21 is a same-map route, gated by the first-visit story and the
-- FFF0D5 alternate result supplied by the field exit.
game.field.alternate_result = 1
queue("questions")
assert(Apartments.present_repeat_service() == "denied")
assert(#prompts == 0)
game.field.alternate_result = 0
game.location.id = "another_building"
queue("questions")
assert(Apartments.present_repeat_service() == "denied")
assert(#prompts == 0)
game.location.id = location_id
set_story(30, false)
queue("questions")
assert(Apartments.present_repeat_service() == "denied")
assert(#prompts == 0)
set_story(30, true)
set_story(21, false)
queue("questions")
assert(Apartments.present_repeat_service() == "denied")
assert(#prompts == 0)
set_story(21, true)

-- Before Ito is defeated, 0x544F0 chooses the questions-only root F4E20.
expect_repeat({ "questions" }, "questions")
assert(#speeches == 1 and speeches[1].text == table.concat({
    "I have no new information at this time.",
    "My resources are spread too thin, but that shouldn't concern you.",
    "For now, you are on your own.",
}, "||"))
assert(speeches[1].continuation == false)

-- Ito's win with bit 28 clear selects F4E34, marks the post-Ito trigger,
-- and offers the two source options before the map inventory is checked.
set_story(16, true)
queue("ito_killed", "more")
assert(Apartments.present_repeat_service() == "left")
assert(game.state.story_flag_bit_28 == true)
assert(#prompts == 2 and prompts[1].choices[1] == "ito_killed" and
       prompts[1].choices[2] == "information")
assert(prompts[2].id == "ito_response")
assert(prompts[2].text:find(
    "Well, with Ogami out of the way", 1, true))
assert(#speeches == 1 and speeches[1].text:find(
    "Thon turned on Renraku", 1, true))
assert(game.state.field_mode_effect_flag_bit_8 == true)

-- With the Renraku map available, post-Ito root F4E48 offers information.
-- Selecting it shows that map, follows the missing-counterpart branch, and
-- sets the source's FFFC34 bit 12 before the Vigore lead pages.
game.state.group_item_flag_bit_8 = true
expect_repeat({ "information" }, "information", "more", "more", "more",
              "more", "more")
assert(prompts[2].id == "renraku_map")
assert(prompts[2].text ==
       "<You show Harlequin the map you retrieved from Renraku.>")
assert(prompts[3].id == "renraku_map_meaning")
assert(prompts[6].id == "vigore_explanation")
assert(game.state.story_flag_bit_26 == true)
assert(game.state.field_mode_effect_flag_bit_12 == true)
assert(game.state.story_flag_bit_27 ~= true)

-- Once map 26 is shown, bit 7 is the remaining information gate. The two-map
-- response takes the other 0x220CC/0x22018 branch and commits bit 27.
game.state.group_item_flag_bit_7 = true
expect_repeat({ "information" }, "information", "more", "more", "more",
              "more")
assert(prompts[2].id == "renraku_map")
assert(prompts[2].text ==
       "<You show Harlequin the map you retrieved from Renraku and the map you took from Vigore and Jarl.>")
assert(prompts[3].id == "both_maps_meaning")
assert(game.state.story_flag_bit_27 == true)
assert(#speeches == 2 and speeches[1].continuation and
       not speeches[2].continuation)

print("Tarislar Garden Apartments tests passed: repeat gates, Ito response, and map branches")
