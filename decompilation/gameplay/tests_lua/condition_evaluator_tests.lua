package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local ConditionEvaluator = require("condition_evaluator")
local Status = ConditionEvaluator.Status
local facts = ConditionEvaluator.FactProvider.new({
    state = { story_flag_bit_0 = false, story_flag_bit_1 = true },
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
    { expression = compare("story.story_flag_bit_0", "eq", false),
      expected = Status.true_value },
    { expression = compare("story.story_flag_bit_0", "ne", true),
      expected = Status.true_value },
    { expression = compare("story.story_flag_bit_1", "in", { false, true }),
      expected = Status.true_value },
    { expression = compare("field.alternate_result", "gt", 0),
      context = { alternate_result = 2 }, expected = Status.true_value },
    { expression = compare("field.alternate_result", "lt", 0),
      context = { alternate_result = -1 }, expected = Status.true_value },
    { expression = compare("field.alternate_result", "lte", 0),
      context = { alternate_result = 0 }, expected = Status.true_value },
    { expression = compare("field.alternate_result", "gte", 0),
      context = { alternate_result = 0 }, expected = Status.true_value },
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
    compare("story.story_flag_bit_0", "eq", true),
} }).status == Status.false_value)
assert(evaluator:evaluate({ all = {
    compare("story.not_loaded", "eq", true),
} }).status == Status.unknown)
assert(evaluator:evaluate({ any = {
    compare("story.not_loaded", "eq", true),
    compare("story.story_flag_bit_1", "eq", true),
} }).status == Status.true_value)
assert(evaluator:evaluate({ ["not"] =
    compare("story.not_loaded", "eq", true) }).status == Status.unknown)
assert(evaluator:evaluate(nil).status == Status.true_value)

local valid, err = pcall(ConditionEvaluator.validate,
    compare("story.story_flag_bit_0", "approximately", false))
assert(not valid and tostring(err):match("unsupported operator") ~= nil)

print("condition evaluator tests passed: typed operators and tri-state logic")
