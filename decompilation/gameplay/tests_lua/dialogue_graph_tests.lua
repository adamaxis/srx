package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local ConditionEvaluator = require("condition_evaluator")
local DialogueGraph = require("dialogue_graph")
local Status = ConditionEvaluator.Status

local function eq(fact, value)
    return { fact = fact, op = "eq", value = value }
end
local condition_document = {
    schemaVersion = 1,
    conditions = {
        ["test.enabled"] = {
            expression = eq("story.enabled", true),
            provenance = { romAddresses = { "0x1000" } },
        },
        ["test.disabled"] = {
            expression = eq("story.enabled", false),
            provenance = { romAddresses = { "0x1002" } },
        },
        ["test.unknown"] = {
            expression = eq("story.not_loaded", true),
            provenance = { romAddresses = { "0x1004" } },
        },
    },
}
local cinematic_document = {
    schemaVersion = 1,
    cinematics = {
        source_confirmed_sequence = {
            presentation = "full_screen_story_pages",
            assetProfileId = "test.story_sequence",
            inputContract = "controller_event",
            returnPolicy = "resume_origin",
            provenance = { romAddresses = { "0x1000" } },
            pages = {
                { id = "page_one", textKey = "test.sequence.page_one",
                  provenance = { romAddress = "0x1002" } },
            },
        },
    },
}

local function graph(entry_node, nodes)
    return {
        entryNode = entry_node,
        speakerId = "test_speaker",
        textProfile = "test.dialogue",
        provenance = { romAddresses = { "0x1000" } },
        nodes = nodes,
    }
end

local document = {
    schemaVersion = 1,
    graphs = {
        ["test.branch_action"] = graph("branch", {
            branch = {
                type = "branch",
                cases = { { when = "test.enabled", next = "effect" } },
                fallback = "declined",
            },
            effect = {
                type = "action", actionId = "test.effect",
                arguments = { value = 7 },
                results = { complete = "service" },
            },
            service = {
                type = "terminal",
                outcome = {
                    type = "open_service", serviceId = "hotel.options",
                    returnPolicy = "origin",
                },
            },
            declined = {
                type = "terminal",
                outcome = { type = "continue_in_place" },
            },
        }),
        ["test.filtered_choices"] = graph("menu", {
            menu = {
                type = "choice", menuId = "test_menu",
                inputContract = "buttons_abc",
                choices = {
                    { id = "hidden", when = "test.disabled",
                      next = "hidden_result" },
                    { id = "first", when = "test.enabled",
                      next = "first_result" },
                    { id = "second", next = "second_result" },
                },
            },
            hidden_result = { type = "terminal",
                              outcome = { type = "continue_in_place" } },
            first_result = { type = "terminal",
                             outcome = { type = "continue_in_place" } },
            second_result = { type = "terminal",
                              outcome = { type = "resume_source_route" } },
        }),
        ["test.filtered_button_overflow"] = graph("menu", {
            menu = {
                type = "choice", menuId = "test_menu",
                inputContract = "buttons_abc",
                choices = {
                    { id = "first", next = "done" },
                    { id = "second", next = "done" },
                    { id = "third", next = "done" },
                    { id = "conditional", when = "test.enabled",
                      next = "done" },
                },
            },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
        ["test.inline_prompt"] = graph("prompt", {
            prompt = {
                type = "choice", menuId = "test_inline_menu",
                promptTextKey = "test.inline_prompt.text",
                speakerId = "inline_prompt_speaker",
                textProfile = "test.inline_prompt_profile",
                inputContract = "buttons_abc",
                choices = { { id = "take_room", next = "done" } },
            },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
        ["test.cursor"] = graph("menu", {
            menu = {
                type = "choice", menuId = "test_cursor",
                inputContract = "cursor_list", initialSelection = 1,
                cancelNext = "cancelled",
                choices = {
                    { id = "one", next = "one_result" },
                    { id = "two", next = "two_result" },
                },
            },
            one_result = { type = "terminal",
                           outcome = { type = "continue_in_place" } },
            two_result = { type = "terminal",
                           outcome = { type = "enter_activity",
                                       activityId = "resource_scene_35" } },
            cancelled = { type = "terminal",
                          outcome = { type = "return_to_origin",
                                      returnPoint = "saved_door_entry" } },
        }),
        ["test.yes_no"] = graph("prompt", {
            prompt = {
                type = "choice", menuId = "test_yes_no",
                inputContract = "yes_no",
                choices = {
                    { id = "yes", next = "accepted" },
                    { id = "no", next = "declined" },
                },
            },
            accepted = { type = "terminal",
                         outcome = { type = "return_to_caller" } },
            declined = { type = "terminal",
                         outcome = { type = "continue_in_place" } },
        }),
        ["test.page"] = graph("page", {
            page = { type = "page", eventId = "page_one", next = "done" },
            done = { type = "terminal",
                     outcome = { type = "play_cinematic",
                                 cinematicId = "source_confirmed_sequence",
                                 returnPolicy = "resume_origin" } },
        }),
        ["test.mode_text"] = graph("page", {
            page = {
                type = "page", eventId = "mode_text_page",
                presentation = "field_mode_text", inputDelayFrames = 30,
                next = "done",
            },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
        ["test.unknown"] = graph("branch", {
            branch = {
                type = "branch",
                cases = { { when = "test.unknown", next = "done" } },
                fallback = "done",
            },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
        ["test.source_boundary"] = graph("blocked", {
            blocked = { type = "source_boundary",
                        reason = "ROM command 0x1234 is not converted." },
        }),
        ["test.return_to_caller"] = graph("done", {
            done = { type = "terminal",
                     outcome = { type = "return_to_caller" } },
        }),
    },
}

local catalog = DialogueGraph.Catalog.new(
    document, condition_document, cinematic_document)
local evaluator = ConditionEvaluator.new(
    ConditionEvaluator.FactProvider.new({ state = { enabled = true } }))
local invocation_evaluator = ConditionEvaluator.new(
    ConditionEvaluator.FactProvider.new({
        state = { story_flag_bit_3 = true },
        location = {
            type = "building",
            id = "salish_owlfeather_ranger_lodge",
        },
    }))
local invocation_context = {
    field = { mode = 6, event_code = 0 },
    dialogue = { entry_id = "return_followup" },
}
assert(invocation_evaluator:evaluate(
    eq("dialogue.entry_id", "return_followup"),
    invocation_context).status == Status.true_value)
assert(invocation_evaluator:evaluate(
    eq("field.mode", 6), invocation_context).status == Status.true_value)
assert(invocation_evaluator:evaluate(
    eq("location.id", "salish_owlfeather_ranger_lodge"),
    invocation_context).status == Status.true_value)
assert(invocation_evaluator:evaluate(
    eq("story.story_flag_bit_3", true), invocation_context).status ==
        Status.true_value)
local action_calls = 0
local actions = {
    execute = function(_, action_id, arguments)
        assert(action_id == "test.effect" and arguments.value == 7)
        action_calls = action_calls + 1
        return true, "complete"
    end,
}
local presented_pages = {}
local presented_choices = {}
local cursor_inputs = { "down", "confirm" }
local yes_no_inputs = { "yes", "no" }
local host = {
    present_page = function(node, speaker_id, text_profile)
        presented_pages[#presented_pages + 1] = {
            event_id = node.eventId,
            speaker_id = speaker_id,
            text_profile = text_profile,
            presentation = node.presentation or "field_npc",
            input_delay_frames = node.inputDelayFrames or 0,
        }
        return true
    end,
    present_choice = function(node, choices, speaker_id, text_profile,
                              selected_index)
        presented_choices[#presented_choices + 1] = {
            menu_id = node.menuId,
            choices = choices,
            speaker_id = speaker_id,
            text_profile = text_profile,
            selected_index = selected_index,
        }
        if node.inputContract == "cursor_list" then
            local input = table.remove(cursor_inputs, 1)
            if input == "down" then
                assert(selected_index == 1)
            elseif input == "confirm" then
                assert(selected_index == 2)
            end
            return input
        elseif node.inputContract == "yes_no" then
            return table.remove(yes_no_inputs, 1)
        end
        return "second"
    end,
}
local session = DialogueGraph.Session.new(
    catalog, evaluator, actions, host)

local outcome = session:run("test.branch_action")
assert(outcome.type == "open_service" and action_calls == 1)

outcome = session:run("test.filtered_choices")
assert(outcome.type == "resume_source_route")
local visible_choices = presented_choices[#presented_choices].choices
assert(#visible_choices == 2 and visible_choices[1] == "first" and
       visible_choices[2] == "second")

outcome = session:run("test.filtered_button_overflow")
assert(outcome.type == "source_boundary" and
       outcome.reason:find("visible_button_choice_limit_exceeded", 1, true) ~= nil)

outcome = session:run("test.cursor")
assert(outcome.type == "enter_activity" and
       outcome.activityId == "resource_scene_35")

outcome = session:run("test.yes_no")
assert(outcome.type == "return_to_caller")
outcome = session:run("test.yes_no")
assert(outcome.type == "continue_in_place")

outcome = session:run("test.page")
assert(outcome.type == "play_cinematic" and
       presented_pages[#presented_pages].event_id == "page_one" and
       presented_pages[#presented_pages].speaker_id == "test_speaker")

outcome = session:run("test.mode_text")
assert(outcome.type == "continue_in_place" and
       presented_pages[#presented_pages].presentation == "field_mode_text" and
       presented_pages[#presented_pages].input_delay_frames == 30)

local previous_game = game
local inline_prompt_call
local presented_script_page
game = {
    content = {
        conversations = { graph_tests = document },
        conditions = condition_document,
        cinematics = cinematic_document,
    },
    field = {},
    state = {},
    ui = {
        prompt_text_key = function(menu_id, text_key, choice_ids,
                                   speaker_id, text_profile)
            inline_prompt_call = {
                menu_id = menu_id,
                text_key = text_key,
                choices = choice_ids,
                speaker_id = speaker_id,
                text_profile = text_profile,
            }
            return "take_room"
        end,
        say = function(event_id, speaker_id, text_profile, presentation,
                       input_delay_frames)
            presented_script_page = {
                event_id = event_id,
                speaker_id = speaker_id,
                text_profile = text_profile,
                presentation = presentation,
                input_delay_frames = input_delay_frames,
            }
            return true
        end,
        menu = function()
            error("inline prompt incorrectly used the catalog-title menu")
        end,
    },
}
outcome = DialogueGraph.run("test.inline_prompt")
assert(outcome.type == "continue_in_place" and
       inline_prompt_call.menu_id == "test_inline_menu" and
       inline_prompt_call.text_key == "test.inline_prompt.text" and
       inline_prompt_call.choices[1] == "take_room" and
       inline_prompt_call.speaker_id == "inline_prompt_speaker" and
       inline_prompt_call.text_profile == "test.inline_prompt_profile")
outcome = DialogueGraph.run("test.mode_text")
assert(outcome.type == "continue_in_place" and
       presented_script_page.event_id == "mode_text_page" and
       presented_script_page.speaker_id == "test_speaker" and
       presented_script_page.text_profile == "test.dialogue" and
       presented_script_page.presentation == "field_mode_text" and
       presented_script_page.input_delay_frames == 30)
game = previous_game

outcome = session:run("test.unknown")
assert(outcome.type == "source_boundary" and
       outcome.reason:match("story.not_loaded") ~= nil)
outcome = session:run("test.source_boundary")
assert(outcome.type == "source_boundary" and
       outcome.reason:match("ROM command 0x1234 is not converted") ~= nil)
outcome = session:run("test.return_to_caller")
assert(outcome.type == "return_to_caller")
assert(Status.unknown == "unknown")

local invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("missing", {
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("page", {
            page = { type = "page", eventId = "mode_text_page",
                     presentation = "field_mode_text", next = "done" },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("page", {
            page = { type = "page", eventId = "page_one",
                     presentation = "field_npc", inputDelayFrames = 30,
                     next = "done" },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("done", {
            done = { type = "terminal",
                     outcome = { type = "continue_in_place",
                                 resultId = "legacy-result" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("prompt", {
            prompt = {
                type = "choice", menuId = "inline_cursor",
                promptTextKey = "test.inline_prompt.text",
                inputContract = "cursor_list", cancelNext = "done",
                choices = { { id = "take_room", next = "done" } },
            },
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("done", {
            done = { type = "terminal",
                     outcome = { type = "play_cinematic",
                                 cinematicId = "missing.sequence",
                                 returnPolicy = "resume_origin" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

invalid = pcall(function()
    DialogueGraph.Catalog.new({ schemaVersion = 1, graphs = {
        broken = graph("done", {
            done = { type = "terminal",
                     outcome = { type = "play_cinematic",
                                 cinematicId = "source_confirmed_sequence",
                                 returnPolicy = "wrong_return" } },
        }),
    } }, condition_document, cinematic_document)
end)
assert(not invalid)

-- Runtime catalogs defer only graph validation until a graph is selected;
-- the content test still proves that malformed data fails at that boundary.
local runtime_catalog = DialogueGraph.Catalog.new(
    document, condition_document, cinematic_document, "on_use")
assert(runtime_catalog:get("test.return_to_caller") ~= nil)
local runtime_invalid_catalog = DialogueGraph.Catalog.new({
    schemaVersion = 1,
    graphs = {
        broken = graph("missing", {
            done = { type = "terminal",
                     outcome = { type = "continue_in_place" } },
        }),
    },
}, condition_document, cinematic_document, "on_use")
invalid = pcall(function() runtime_invalid_catalog:get("broken") end)
assert(not invalid)

print("dialogue graph tests passed: branches, pages, menus, actions, outcomes, boundaries")
