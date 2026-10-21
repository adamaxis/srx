-- Evaluate JSON-authored conditions against Lua-owned state and typed context.
-- Missing source facts stay unknown; callers must not treat them as false.
local ConditionEvaluator = {}
ConditionEvaluator.__index = ConditionEvaluator

local FactProvider = {}
FactProvider.__index = FactProvider

local unknown_fact = {}

ConditionEvaluator.Status = {
    true_value = "true",
    false_value = "false",
    unknown = "unknown",
}

ConditionEvaluator.unknown_fact = unknown_fact

local function state_value(state, key)
    if type(state) ~= "table" then return unknown_fact end
    local value = state[key]
    if value == nil then return unknown_fact end
    return value
end

function FactProvider.new(options)
    options = options or {}
    return setmetatable({
        state = options.state,
        location = options.location,
        run_registry = options.run_registry,
    }, FactProvider)
end

function FactProvider:read(fact_name, context)
    if type(fact_name) ~= "string" then return unknown_fact end

    if fact_name:sub(1, 6) == "story." then
        local state = self.state
        if state == nil and type(game) == "table" then
            state = game.state
        end
        return state_value(state, fact_name:sub(7))
    end

    if fact_name:sub(1, 6) == "field." then
        local key = fact_name:sub(7)
        local source = type(context) == "table" and context or nil
        if source == nil and type(game) == "table" then
            source = game.field
        end
        if type(source) ~= "table" then return unknown_fact end
        local value = source[key]
        if value == nil then return unknown_fact end
        return value
    end

    if fact_name:sub(1, 9) == "location." then
        local location = self.location
        if location == nil and type(game) == "table" then
            location = game.location
        end
        return state_value(location, fact_name:sub(10))
    end

    local registry = self.run_registry
    if registry == nil and type(game) == "table" and
       type(game.runs) == "table" then
        registry = game.runs.registry
    end
    if fact_name == "run.active_id" then
        if type(registry) ~= "table" or
           type(registry.active_run_id) ~= "function" then
            return unknown_fact
        end
        local value = registry.active_run_id()
        return value == nil and unknown_fact or value
    end
    if fact_name == "run.current_offer_id" then
        if type(registry) ~= "table" or
           type(registry.current_offer_id) ~= "function" then
            return unknown_fact
        end
        local value = registry.current_offer_id()
        return value == nil and unknown_fact or value
    end

    return unknown_fact
end

local function is_array(value)
    if type(value) ~= "table" then return false end
    local count = 0
    for key in pairs(value) do
        if type(key) ~= "number" or key < 1 or key % 1 ~= 0 then
            return false
        end
        count = count + 1
    end
    return count == #value
end

local function same_value(left, right)
    if type(left) ~= type(right) then return false end
    return left == right
end

local function validate_expression(expression, depth)
    assert(type(expression) == "table" and depth <= 32,
        "condition expression must be a bounded JSON object")
    local forms = 0
    for _, key in ipairs({ "all", "any", "not", "fact" }) do
        if expression[key] ~= nil then forms = forms + 1 end
    end
    assert(forms == 1, "condition node must have one expression form")
    if expression.all ~= nil or expression.any ~= nil then
        local children = expression.all or expression.any
        assert(is_array(children), "condition group must be an array")
        for _, child in ipairs(children) do
            validate_expression(child, depth + 1)
        end
        return true
    end
    if expression["not"] ~= nil then
        validate_expression(expression["not"], depth + 1)
        return true
    end
    local operator = expression.op
    assert(type(expression.fact) == "string" and
           type(operator) == "string" and expression.value ~= nil,
        "condition comparison needs fact, op, and value")
    assert(operator == "eq" or operator == "ne" or operator == "lt" or
           operator == "lte" or operator == "gt" or operator == "gte" or
           operator == "in",
        "condition comparison has an unsupported operator")
    if operator == "in" then
        assert(is_array(expression.value), "condition in value must be an array")
    elseif operator == "lt" or operator == "lte" or
           operator == "gt" or operator == "gte" then
        assert(type(expression.value) == "number",
            "ordered condition value must be numeric")
    else
        assert(type(expression.value) == "boolean" or
               type(expression.value) == "number" or
               type(expression.value) == "string",
            "condition value must be a JSON scalar")
    end
    return true
end

function ConditionEvaluator.new(fact_provider)
    assert(type(fact_provider) == "table" and
           type(fact_provider.read) == "function",
        "condition evaluator requires a fact provider")
    return setmetatable({ fact_provider = fact_provider }, ConditionEvaluator)
end

function ConditionEvaluator:_evaluate(expression, context)
    assert(type(expression) == "table",
        "condition expression must be a JSON object")

    if expression.all ~= nil then
        assert(is_array(expression.all), "condition all must be an array")
        local saw_unknown = false
        local unknown_facts = {}
        for _, child in ipairs(expression.all) do
            local result = self:_evaluate(child, context)
            if result.status == ConditionEvaluator.Status.false_value then
                return result
            end
            if result.status == ConditionEvaluator.Status.unknown then
                saw_unknown = true
                for _, fact in ipairs(result.unknown_facts) do
                    unknown_facts[fact] = true
                end
            end
        end
        if saw_unknown then
            local facts = {}
            for fact in pairs(unknown_facts) do facts[#facts + 1] = fact end
            table.sort(facts)
            return { status = ConditionEvaluator.Status.unknown,
                     unknown_facts = facts }
        end
        return { status = ConditionEvaluator.Status.true_value,
                 unknown_facts = {} }
    end

    if expression.any ~= nil then
        assert(is_array(expression.any), "condition any must be an array")
        local saw_unknown = false
        local unknown_facts = {}
        for _, child in ipairs(expression.any) do
            local result = self:_evaluate(child, context)
            if result.status == ConditionEvaluator.Status.true_value then
                return result
            end
            if result.status == ConditionEvaluator.Status.unknown then
                saw_unknown = true
                for _, fact in ipairs(result.unknown_facts) do
                    unknown_facts[fact] = true
                end
            end
        end
        if saw_unknown then
            local facts = {}
            for fact in pairs(unknown_facts) do facts[#facts + 1] = fact end
            table.sort(facts)
            return { status = ConditionEvaluator.Status.unknown,
                     unknown_facts = facts }
        end
        return { status = ConditionEvaluator.Status.false_value,
                 unknown_facts = {} }
    end

    if expression["not"] ~= nil then
        local result = self:_evaluate(expression["not"], context)
        if result.status == ConditionEvaluator.Status.true_value then
            result.status = ConditionEvaluator.Status.false_value
        elseif result.status == ConditionEvaluator.Status.false_value then
            result.status = ConditionEvaluator.Status.true_value
        end
        return result
    end

    local fact_name = expression.fact
    local operator = expression.op
    assert(type(fact_name) == "string" and
           type(operator) == "string" and expression.value ~= nil,
        "condition comparison needs fact, op, and value")
    local actual = self.fact_provider:read(fact_name, context)
    if actual == unknown_fact then
        return { status = ConditionEvaluator.Status.unknown,
                 unknown_facts = { fact_name } }
    end

    local expected = expression.value
    local matched
    if operator == "eq" then
        matched = same_value(actual, expected)
    elseif operator == "ne" then
        matched = not same_value(actual, expected)
    elseif operator == "lt" or operator == "lte" or
           operator == "gt" or operator == "gte" then
        if type(actual) ~= "number" or type(expected) ~= "number" then
            return { status = ConditionEvaluator.Status.unknown,
                     unknown_facts = { fact_name } }
        end
        if operator == "lt" then matched = actual < expected
        elseif operator == "lte" then matched = actual <= expected
        elseif operator == "gt" then matched = actual > expected
        else matched = actual >= expected end
    elseif operator == "in" then
        assert(is_array(expected), "condition in value must be an array")
        matched = false
        for _, candidate in ipairs(expected) do
            if same_value(actual, candidate) then
                matched = true
                break
            end
        end
    else
        error("unsupported condition operator: " .. operator)
    end

    return {
        status = matched and ConditionEvaluator.Status.true_value or
            ConditionEvaluator.Status.false_value,
        unknown_facts = {},
    }
end

function ConditionEvaluator:evaluate(expression, context)
    if expression == nil then
        return { status = ConditionEvaluator.Status.true_value,
                 unknown_facts = {} }
    end
    return self:_evaluate(expression, context)
end

function ConditionEvaluator.validate(expression)
    return validate_expression(expression, 1)
end

ConditionEvaluator.FactProvider = FactProvider
return ConditionEvaluator
