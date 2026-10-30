package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local map_information_calls = 0
package.loaded["harlequin_dialogue"] = {
    present_map_information = function()
        map_information_calls = map_information_calls + 1
        return "left"
    end,
}
local Apartments = require("tarislar_apartments")

local location_id = "puyallup_site_02_tarislar_garden_apartments"
local available_choices = {}
local queued_choices = {}
local prompts = {}
local speeches = {}

local function eq(fact, value)
    return { fact = fact, op = "eq", value = value }
end

local function condition(expression)
    return { expression = expression, provenance = { romAddresses = { "0x544E8" } } }
end

local graph = {
    entryNode = "ito_victory_gate",
    precondition = "repeat_service_available",
    speakerId = "tarislar_apartments_narrator",
    textProfile = "tarislar.garden.apartments.entry",
    provenance = { romAddresses = { "0x5447E", "0x544E8", "0xFC111" } },
    nodes = {
        ito_victory_gate = {
            type = "branch",
            cases = { { when = "ito_fight_won", next = "briefing_gate" } },
            fallback = "questions_choice",
        },
        briefing_gate = {
            type = "branch",
            cases = { { when = "briefing_pending", next = "mark_briefed" } },
            fallback = "map_information_gate",
        },
        mark_briefed = {
            type = "action",
            actionId = "tarislar_apartments.mark_post_ito_briefed",
            results = { applied = "post_ito_choice" },
        },
        map_information_gate = {
            type = "branch",
            cases = { { when = "map_information_available", next = "information_choice" } },
            fallback = "questions_choice",
        },
        questions_choice = {
            type = "choice",
            menuId = "repeat_service",
            promptTextKey = "tarislar.repeat_greeting",
            inputContract = "buttons_abc",
            choices = { { id = "questions", next = "questions_response" } },
        },
        post_ito_choice = {
            type = "choice",
            menuId = "repeat_service",
            promptTextKey = "tarislar.repeat_greeting",
            inputContract = "buttons_abc",
            choices = {
                { id = "ito_killed", next = "ito_response" },
                { id = "information", next = "map_information" },
            },
        },
        information_choice = {
            type = "choice",
            menuId = "repeat_service",
            promptTextKey = "tarislar.repeat_greeting",
            inputContract = "buttons_abc",
            choices = { { id = "information", next = "map_information" } },
        },
        questions_response = {
            type = "page", eventId = "repeat_questions",
            next = "continue_in_place",
        },
        ito_response = {
            type = "choice",
            menuId = "ito_response",
            promptTextKey = "tarislar.ito_response",
            inputContract = "buttons_abc",
            choices = { { id = "more", next = "map_information" } },
        },
        map_information = {
            type = "action",
            actionId = "tarislar_apartments.present_map_information",
            results = { left = "continue_in_place" },
        },
        continue_in_place = {
            type = "terminal", outcome = { type = "continue_in_place" },
        },
    },
}

local map_information_available = {
    any = {
        { all = {
            eq("story.renraku_map_shown", false),
            eq("story.group_item_flag_bit_8", true),
        } },
        { all = {
            eq("story.renraku_map_shown", true),
            eq("story.both_maps_shown", false),
            eq("story.group_item_flag_bit_7", true),
        } },
    },
}

game = {
    state = {
        ito_fight_won = false,
        garden_apts_entered = true,
        renraku_map_shown = false,
        both_maps_shown = false,
        post_ito_briefed = false,
        frosty_met = true,
        group_item_flag_bit_7 = false,
        group_item_flag_bit_8 = false,
    },
    location = { type = "building", id = location_id },
    field = { package_run_status = "none" },
    content = {
        conversations = {
            { schemaVersion = 1, graphs = {
                ["tarislar_garden_apartments.repeat_service"] = graph,
            } },
        },
        conditions = {
            schemaVersion = 1,
            conditions = {
                repeat_service_available = condition({ all = {
                    eq("location.type", "building"),
                    eq("location.id", location_id),
                    eq("field.package_run_status", "none"),
                    eq("story.garden_apts_entered", true),
                    eq("story.frosty_met", true),
                } }),
                ito_fight_won = condition(eq("story.ito_fight_won", true)),
                briefing_pending = condition(eq("story.post_ito_briefed", false)),
                map_information_available = condition(map_information_available),
            },
        },
        cinematics = { schemaVersion = 1, cinematics = {} },
        texts = {
            ["tarislar.repeat_greeting"] =
                "Hello again, Joshua.||You have bought my time with your pain.||Spend it as you wish.",
            ["tarislar.ito_response"] =
                "You managed to have him killed?||It seems I am not the only person to underestimate you, although it appears I'll be the only one around to correct my mistake.||Well, with Ogami out of the way Renraku should not bother us any longer.",
            ["tarislar.repeat_questions"] =
                "I have no new information at this time.||My resources are spread too thin, but that shouldn't concern you.||For now, you are on your own.",
        },
    },
    ui = {
        prompt_text_key = function(id, text_key, choices)
            local text = game.content.texts[text_key]
            assert(text ~= nil, "missing prompt text key: " .. text_key)
            local prompt = { id = id, text = text, choices = {} }
            for index, choice in ipairs(choices) do
                prompt.choices[index] = choice
            end
            prompts[#prompts + 1] = prompt
            local selected = table.remove(queued_choices, 1)
            assert(selected ~= nil, "unexpected prompt: " .. id)
            local allowed = false
            for _, choice in ipairs(choices) do
                if choice == selected then allowed = true end
            end
            assert(allowed,
                "choice " .. selected .. " is not available at " .. id)
            return selected
        end,
        prompt_text = function(id, text, choices)
            local prompt = { id = id, text = text, choices = {} }
            for index, choice in ipairs(choices) do
                prompt.choices[index] = choice
            end
            prompts[#prompts + 1] = prompt
            local selected = table.remove(queued_choices, 1)
            assert(selected ~= nil, "unexpected prompt: " .. id)
            local allowed = false
            for _, choice in ipairs(choices) do
                if choice == selected then allowed = true end
            end
            assert(allowed,
                "choice " .. selected .. " is not available at " .. id)
            return selected
        end,
        say = function(event_id)
            assert(event_id == "repeat_questions")
            speeches[#speeches + 1] = {
                text = game.content.texts["tarislar.repeat_questions"],
                continuation = false,
            }
            return true
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
    queued_choices = { ... }
    prompts = {}
    speeches = {}
end

local function set_story(fact_id, value)
    game.state[fact_id] = value
end

local function expect_repeat(expected_choices, ...)
    queue(...)
    local outcome = Apartments.present_repeat_service()
    assert(outcome == "left")
    assert(#queued_choices == 0, "unused prompt choice")
    assert(prompts[1].id == "repeat_service")
    assert(prompts[1].text == game.content.texts["tarislar.repeat_greeting"])
    assert(#prompts[1].choices == #expected_choices)
    for index, expected in ipairs(expected_choices) do
        assert(prompts[1].choices[index] == expected)
    end
end

local function expect_rejected()
    queue("questions")
    local outcome = Apartments.present_repeat_service()
    assert(type(outcome) == "table" and outcome.type == "source_boundary")
    assert(#prompts == 0)
end

-- The graph precondition preserves the service location, event, story, and
-- alternate-result gates before presenting any dialogue.
game.field.package_run_status = "payment_due"
expect_rejected()
game.field.package_run_status = "none"
game.location.id = "another_building"
expect_rejected()
game.location.id = location_id
set_story("frosty_met", false)
expect_rejected()
set_story("frosty_met", true)
set_story("garden_apts_entered", false)
expect_rejected()
set_story("garden_apts_entered", true)

-- Before Ito is defeated, the questions-only root presents the extracted
-- greeting and ROM response page without a More continuation.
expect_repeat({ "questions" }, "questions")
assert(#speeches == 1 and speeches[1].text ==
    game.content.texts["tarislar.repeat_questions"])
assert(speeches[1].continuation == false)

-- Ito's win with bit 28 clear marks the fixed post-Ito trigger before the
-- two-choice page. Choosing Ito enters the shared map-information action.
set_story("ito_fight_won", true)
queue("ito_killed", "more")
assert(Apartments.present_repeat_service() == "left")
assert(game.state.post_ito_briefed == true)
assert(#prompts == 2 and prompts[1].choices[1] == "ito_killed" and
       prompts[1].choices[2] == "information")
assert(prompts[2].id == "ito_response")
assert(prompts[2].text == game.content.texts["tarislar.ito_response"])
assert(#speeches == 0 and map_information_calls == 1)

-- A held Renraku map exposes the outer graph's map-information route. The
-- injected helper stands in for the separately tested shared map graph.
game.state.group_item_flag_bit_8 = true
expect_repeat({ "information" }, "information")
assert(#prompts == 1 and map_information_calls == 2)
assert(game.state.renraku_map_shown ~= true)
assert(game.state.field_mode_effect_flag_bit_12 ~= true)
assert(game.state.both_maps_shown ~= true)

-- After story bit 26 is set, item bit 7 exposes the paired-map route.
game.state.group_item_flag_bit_7 = true
expect_repeat({ "information" }, "information")
assert(#prompts == 1 and map_information_calls == 3)
assert(game.state.both_maps_shown ~= true)

-- Once both map routes are complete, or before any map item is available,
-- the service falls back to questions.
set_story("renraku_map_shown", true)
set_story("both_maps_shown", true)
game.state.group_item_flag_bit_8 = false
game.state.group_item_flag_bit_7 = false
expect_repeat({ "questions" }, "questions")

print("Tarislar Apartments dialogue graph and map-information tests passed")
