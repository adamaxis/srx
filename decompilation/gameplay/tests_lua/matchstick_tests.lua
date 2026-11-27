package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local DialogueGraph = require("dialogue_graph")
local ConditionEvaluator = require("condition_evaluator")

local function read_json(path)
    local file = assert(io.open(path, "rb"))
    local content = Json.decode(file:read("*a"))
    file:close()
    return content
end

local source_graph = read_json(
    "decompilation/gameplay/dialogue/conversations/matchstick.json")
local source_conditions = read_json("decompilation/gameplay/dialogue/conditions.json")
local graph_document = {
    schemaVersion = 1,
    graphs = {
        ["matchstick.scene0_doorman_entry"] =
            source_graph.graphs["matchstick.scene0_doorman_entry"],
    },
}
local catalog = DialogueGraph.Catalog.new(graph_document, source_conditions, {
    schemaVersion = 1,
    cinematics = {},
})

local function play(reputation, ogami_left, ito_fight_won,
                    charge_result, choice_id)
    local state = {
        ogami_left = ogami_left,
        ito_fight_won = ito_fight_won,
    }
    game = {
        state = state,
        party = function()
            return { [1] = { skills = { reputation = reputation } } }
        end,
        active_slot = function() return 1 end,
    }
    local evaluator = ConditionEvaluator.new(
        ConditionEvaluator.FactProvider.new({ state = state }))
    local pages, presented_choices, actions_seen = {}, {}, {}
    local actions = {
        execute = function(_, action_id, arguments)
            actions_seen[#actions_seen + 1] = action_id
            if action_id == "building.charge_amount" then
                assert(arguments.amount == 150)
                return true, charge_result
            end
            if action_id == "field.queue_ito_activity" then
                return true, "queued"
            end
            error("unexpected Matchstick action " .. action_id)
        end,
    }
    local host = {
        present_page = function(node)
            pages[#pages + 1] = node.eventId
            return true
        end,
        present_choice = function(node, choices)
            presented_choices[#presented_choices + 1] = {
                menu_id = node.menuId,
                choices = choices,
            }
            return choice_id
        end,
    }
    local outcome = DialogueGraph.Session.new(
        catalog, evaluator, actions, host):run(
            "matchstick.scene0_doorman_entry")
    return pages, presented_choices, actions_seen, outcome
end

-- ROM 0x22452: Reputation below 6 reaches the 150-nuyen choice.
local pages, choices, actions, outcome = play(5, true, false, "paid", "later")
assert(choices[1].menu_id == "entry_fee" and
       choices[1].choices[1] == "pay" and choices[1].choices[2] == "later")
assert(#actions == 0 and outcome.type == "continue_in_place")

-- ROM 0x2246C/0x22474: missing Ogami's lead selects descriptor 0x111176.
pages, choices, actions, outcome = play(6, false, false, "paid", "pay")
assert(pages[2] == "high_reputation_notice" and #actions == 0 and
       outcome.type == "resume_source_route")

-- ROM 0x224B4/0x224EC: payment plus both story gates queues 0x1E5C4.
pages, choices, actions, outcome = play(5, true, false, "paid", "pay")
assert(pages[2] == "entry_fee_paid" and pages[3] == "ito_intro" and
       actions[1] == "building.charge_amount" and
       actions[2] == "field.queue_ito_activity" and
       outcome.type == "resume_source_route")

-- ROM 0x21A4E returns the insufficient-funds descriptor without queuing.
pages, choices, actions, outcome = play(5, true, false, "insufficient", "pay")
assert(pages[2] == "entry_fee_insufficient" and #actions == 1 and
       actions[1] == "building.charge_amount" and
       outcome.type == "continue_in_place")

-- ROM 0x22474/0x22478: a completed Ito fight suppresses the activity too.
pages, choices, actions, outcome = play(6, true, true, "paid", "pay")
assert(pages[2] == "high_reputation_notice" and #actions == 0)

print("Matchstick Ito producer tests passed: reputation, bit gates, payment, and queue")

-- ROM callback 0x54286: the Ito result also sets FFFC28 bit 11 at
-- 0x542BA..0x542C4, the Enter Passcode entry for Ito's system.
do
    package.loaded["matchstick"] = nil
    local nuyen, heat, stage = 500, 3, 2
    local state = { matrix_passcode_flags_resolved = true }
    game = {
        state = state,
        nuyen = function() return nuyen end,
        set_nuyen = function(value) nuyen = value end,
        services = {
            call = function(domain, operation)
                assert(domain == "story", operation)
                if operation == "get_heat_level" then
                    return "level_" .. heat
                elseif operation == "get_strike_team_stage" then
                    return "stage_" .. stage
                end
                local value = operation:match("^set_heat_level_(%d+)$")
                if value then heat = tonumber(value); return "applied" end
                value = operation:match("^set_strike_team_stage_(%d+)$")
                if value then stage = tonumber(value); return "applied" end
                error("unexpected story service " .. operation)
            end,
        },
    }
    local Matchstick = require("matchstick")
    assert(Matchstick.apply_ito_callback() == "applied")
    assert(nuyen == 10500 and heat == 5 and stage == 4)
    assert(state.matrix_passcode_bit_11 == true,
           "Ito callback must set the Ito passcode bit (FFFC28 bit 11)")
    print("Matchstick Ito callback tests passed: passcode bit 11")
end
