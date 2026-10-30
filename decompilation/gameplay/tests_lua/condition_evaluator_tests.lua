package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local ConditionEvaluator = require("condition_evaluator")
local Status = ConditionEvaluator.Status
local facts = ConditionEvaluator.FactProvider.new({
    state = { motel_bill_paid = false, tarislar_asked_shale = true },
    run_registry = {
        active_run_id = function() return "package_recovery" end,
        current_offer_id = function() return "employee_recruitment" end,
    },
})
local evaluator = ConditionEvaluator.new(facts)
local function compare(fact, op, value)
    return { fact = fact, op = op, value = value }
end

local cases = {
    { expression = compare("story.motel_bill_paid", "eq", false),
      expected = Status.true_value },
    { expression = compare("story.motel_bill_paid", "ne", true),
      expected = Status.true_value },
    { expression = compare("story.tarislar_asked_shale", "in", { false, true }),
      expected = Status.true_value },
    { expression = compare("field.package_run_status", "eq", "payment_due"),
      context = { package_run_status = "payment_due" },
      expected = Status.true_value },
    { expression = compare("field.package_run_status", "eq", "handoff_due"),
      context = { package_run_status = "handoff_due" },
      expected = Status.true_value },
    { expression = compare("field.package_run_status", "eq", "none"),
      context = { package_run_status = "none" },
      expected = Status.true_value },
    { expression = compare("field.package_run_status", "ne", "none"),
      context = { package_run_status = "payment_due" },
      expected = Status.true_value },
    { expression = compare("run.active_id", "eq", "package_recovery"),
      expected = Status.true_value },
    { expression = compare("run.current_offer_id", "eq", "employee_recruitment"),
      expected = Status.true_value },
    { expression = compare("story.not_loaded", "eq", true),
      expected = Status.unknown },
}
for _, case in ipairs(cases) do
    ConditionEvaluator.validate(case.expression)
    assert(evaluator:evaluate(case.expression, case.context).status ==
           case.expected)
end

assert(evaluator:evaluate({ all = {
    compare("story.not_loaded", "eq", true),
    compare("story.motel_bill_paid", "eq", true),
} }).status == Status.false_value)
assert(evaluator:evaluate({ all = {
    compare("story.not_loaded", "eq", true),
} }).status == Status.unknown)
assert(evaluator:evaluate({ any = {
    compare("story.not_loaded", "eq", true),
    compare("story.tarislar_asked_shale", "eq", true),
} }).status == Status.true_value)
assert(evaluator:evaluate({ ["not"] =
    compare("story.not_loaded", "eq", true) }).status == Status.unknown)
assert(evaluator:evaluate(nil).status == Status.true_value)

local previous_game = game
local active_reputation = 5
game = {
    party = function()
        return { [0] = { skills = { reputation = active_reputation } } }
    end,
    active_slot = function() return 0 end,
}
local runner_evaluator = ConditionEvaluator.new(
    ConditionEvaluator.FactProvider.new())
assert(runner_evaluator:evaluate(
    compare("runner.skill.reputation", "gte", 5)).status ==
    Status.true_value)
active_reputation = 4
assert(runner_evaluator:evaluate(
    compare("runner.skill.reputation", "lt", 5)).status ==
    Status.true_value)
game.party = function()
    return { [0] = { skills = { reputation = nil } } }
end
assert(runner_evaluator:evaluate(
    compare("runner.skill.reputation", "gte", 5)).status ==
    Status.unknown)
game = previous_game

local valid, err = pcall(ConditionEvaluator.validate,
    compare("story.motel_bill_paid", "approximately", false))
assert(not valid and tostring(err):match("unsupported operator") ~= nil)

print("condition evaluator tests passed: typed operators and tri-state logic")
