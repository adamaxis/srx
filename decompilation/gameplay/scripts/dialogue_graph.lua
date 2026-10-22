-- Reusable Lua-owned conversation graph runtime. The host only presents a
-- page/menu and forwards focused-window input; this module owns graph state.
local ConditionEvaluator = require("condition_evaluator")
local DialogueActions = require("dialogue_actions")

local DialogueGraph = {}
local Catalog = {}
Catalog.__index = Catalog
local Session = {}
Session.__index = Session

local MAX_GRAPH_NODES = 256
local MAX_GRAPH_STEPS = 2048
local MAX_CHOICES = 8
-- Runtime content is immutable for the lifetime of one game package. Cache
-- its catalog by table identity and validate each graph on first use, so a
-- single call does not revalidate every unrelated conversation in the game.
local runtime_catalogs = setmetatable({}, { __mode = "k" })

local function is_identifier(value)
    return type(value) == "string" and value ~= "" and
        value:match("^[%w_%-%.]+$") ~= nil
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

local function require_node_reference(nodes, node_id, owner)
    assert(is_identifier(node_id) and nodes[node_id] ~= nil,
        owner .. " references a missing dialogue node: " ..
        tostring(node_id))
end

local function validate_outcome_fields(outcome, required, optional)
    local allowed = { type = true }
    for _, field in ipairs(required) do allowed[field] = true end
    for _, field in ipairs(optional or {}) do allowed[field] = true end
    for field in pairs(outcome) do
        assert(type(field) == "string" and allowed[field],
            "dialogue outcome has an unsupported field: " .. tostring(field))
    end
    for _, field in ipairs(required) do
        assert(outcome[field] ~= nil,
            "dialogue outcome is missing field: " .. field)
    end
end

local function validate_outcome(outcome, cinematics)
    assert(type(outcome) == "table" and is_identifier(outcome.type),
        "dialogue terminal needs a typed outcome")
    if outcome.type == "continue_in_place" or
       outcome.type == "return_to_caller" or
       outcome.type == "resume_source_route" then
        validate_outcome_fields(outcome, {})
        return true
    elseif outcome.type == "return_to_origin" then
        validate_outcome_fields(outcome, { "returnPoint" },
                                { "returnPolicy" })
        assert(is_identifier(outcome.returnPoint),
            "return_to_origin needs returnPoint")
        if outcome.returnPolicy ~= nil then
            assert(is_identifier(outcome.returnPolicy),
                "return_to_origin has an invalid returnPolicy")
        end
    elseif outcome.type == "enter_field_scene" then
        validate_outcome_fields(outcome, { "sceneId", "arrivalId" })
        assert(type(outcome.sceneId) == "number" and
               outcome.sceneId % 1 == 0 and outcome.sceneId >= 0 and
               outcome.sceneId <= 55 and is_identifier(outcome.arrivalId),
            "enter_field_scene needs a sceneId and arrivalId")
    elseif outcome.type == "enter_activity" then
        validate_outcome_fields(outcome, { "activityId" })
        assert(is_identifier(outcome.activityId),
            "enter_activity needs activityId")
    elseif outcome.type == "open_service" then
        validate_outcome_fields(outcome, { "serviceId", "returnPolicy" })
        assert(is_identifier(outcome.serviceId) and
               is_identifier(outcome.returnPolicy),
            "open_service needs serviceId and returnPolicy")
    elseif outcome.type == "play_cinematic" then
        validate_outcome_fields(outcome, { "cinematicId", "returnPolicy" })
        assert(is_identifier(outcome.cinematicId) and
               is_identifier(outcome.returnPolicy),
            "play_cinematic needs cinematicId and returnPolicy")
        if cinematics ~= nil then
            local cinematic = cinematics[outcome.cinematicId]
            assert(type(cinematic) == "table",
                "play_cinematic references an unknown cinematic: " ..
                outcome.cinematicId)
            assert(cinematic.returnPolicy == outcome.returnPolicy,
                "play_cinematic returnPolicy does not match its catalog entry: " ..
                outcome.cinematicId)
        end
    else
        error("unsupported or non-terminal dialogue outcome: " ..
              tostring(outcome.type))
    end
    return true
end

local function validate_provenance(provenance)
    assert(type(provenance) == "table" and
           is_array(provenance.romAddresses) and
           #provenance.romAddresses > 0,
        "dialogue graph needs ROM provenance")
    for _, address in ipairs(provenance.romAddresses) do
        assert(type(address) == "string" and address:match("^0x%x+$"),
            "dialogue graph has invalid ROM provenance")
    end
end

local function validate_graph(graph_id, graph, conditions, cinematics)
    assert(is_identifier(graph_id) and type(graph) == "table" and
           type(graph.entryNode) == "string" and
           type(graph.nodes) == "table",
        "dialogue graph definition is malformed: " .. tostring(graph_id))
    if graph.speakerId ~= nil then
        assert(is_identifier(graph.speakerId),
            "dialogue graph has an invalid default speaker")
    end
    if graph.textProfile ~= nil then
        assert(is_identifier(graph.textProfile),
            "dialogue graph has an invalid default text profile")
    end
    if graph.precondition ~= nil then
        assert(is_identifier(graph.precondition) and
               conditions[graph.precondition] ~= nil,
            "dialogue graph references an unknown precondition")
    end
    validate_provenance(graph.provenance)

    local node_count = 0
    for node_id in pairs(graph.nodes) do
        assert(is_identifier(node_id), "dialogue graph node id is invalid")
        node_count = node_count + 1
    end
    assert(node_count > 0 and node_count <= MAX_GRAPH_NODES,
        "dialogue graph node count is outside its limit")
    require_node_reference(graph.nodes, graph.entryNode, "graph entry")

    for node_id, node in pairs(graph.nodes) do
        assert(type(node) == "table" and is_identifier(node.type),
            "dialogue graph node is malformed: " .. node_id)
        local owner = "dialogue node " .. node_id
        if node.type == "page" then
            assert(is_identifier(node.eventId), owner .. " needs eventId")
            require_node_reference(graph.nodes, node.next, owner)
            if node.presentation ~= nil then
                assert(node.presentation == "field_npc" or
                       node.presentation == "field_mode_text",
                    owner .. " has an unsupported presentation contract")
            end
            if node.inputDelayFrames ~= nil then
                assert(type(node.inputDelayFrames) == "number" and
                       node.inputDelayFrames % 1 == 0 and
                       node.inputDelayFrames >= 0 and
                       node.inputDelayFrames <= 255 and
                       node.presentation == "field_mode_text",
                    owner .. " has an invalid input delay")
            end
            if node.presentation == "field_mode_text" then
                assert((node.inputDelayFrames or 0) > 0,
                    owner .. " mode-text page needs an input delay")
            end
            if node.speakerId ~= nil then
                assert(is_identifier(node.speakerId),
                    owner .. " has an invalid speaker")
            end
            if node.textProfile ~= nil then
                assert(is_identifier(node.textProfile),
                    owner .. " has an invalid text profile")
            end
        elseif node.type == "choice" then
            assert(is_identifier(node.menuId) and
                   (node.inputContract == "buttons_abc" or
                    node.inputContract == "yes_no" or
                    node.inputContract == "cursor_list") and
                   is_array(node.choices) and #node.choices > 0 and
                   #node.choices <= MAX_CHOICES,
                owner .. " has an invalid choice input contract")
            if node.promptTextKey ~= nil then
                assert(is_identifier(node.promptTextKey) and
                       node.inputContract ~= "cursor_list",
                    owner .. " has an invalid inline prompt text key")
            end
            if node.speakerId ~= nil then
                assert(is_identifier(node.speakerId),
                    owner .. " has an invalid speaker")
            end
            if node.textProfile ~= nil then
                assert(is_identifier(node.textProfile),
                    owner .. " has an invalid text profile")
            end
            if node.noSelectionNext ~= nil then
                assert(node.inputContract == "buttons_abc",
                    owner .. " noSelectionNext needs buttons_abc")
                require_node_reference(
                    graph.nodes, node.noSelectionNext, owner)
            end
            if node.inputContract == "buttons_abc" then
                local unconditional_choices = 0
                for _, choice in ipairs(node.choices) do
                    if choice.when == nil then
                        unconditional_choices = unconditional_choices + 1
                    end
                end
                assert(unconditional_choices <= 3,
                    owner .. " has too many unconditional A/B/C choices")
            elseif node.inputContract == "yes_no" then
                assert(#node.choices == 2,
                    owner .. " yes_no contract needs two choices")
            else
                assert(node.cancelNext ~= nil,
                    owner .. " cursor list needs cancelNext")
                require_node_reference(graph.nodes, node.cancelNext, owner)
            end
            if node.initialSelection ~= nil then
                assert(type(node.initialSelection) == "number" and
                       node.initialSelection % 1 == 0 and
                       node.initialSelection >= 1 and
                       node.initialSelection <= #node.choices,
                    owner .. " has an invalid initial selection")
            end
            local seen = {}
            for _, choice in ipairs(node.choices) do
                assert(type(choice) == "table" and
                       is_identifier(choice.id) and not seen[choice.id],
                    owner .. " has an invalid or duplicate choice id")
                seen[choice.id] = true
                require_node_reference(graph.nodes, choice.next, owner)
                if choice.when ~= nil then
                    assert(is_identifier(choice.when) and
                           conditions[choice.when] ~= nil,
                        owner .. " choice references an unknown condition")
                end
            end
        elseif node.type == "branch" then
            assert(is_array(node.cases) and #node.cases > 0 and
                   is_identifier(node.fallback),
                owner .. " needs ordered cases and an explicit fallback")
            require_node_reference(graph.nodes, node.fallback, owner)
            for _, branch in ipairs(node.cases) do
                assert(type(branch) == "table" and
                       is_identifier(branch.when) and
                       conditions[branch.when] ~= nil,
                    owner .. " has a malformed condition case")
                require_node_reference(graph.nodes, branch.next, owner)
            end
        elseif node.type == "action" then
            assert(is_identifier(node.actionId) and
                   (node.arguments == nil or type(node.arguments) == "table"),
                owner .. " has a malformed registered action")
            if node.next ~= nil then
                require_node_reference(graph.nodes, node.next, owner)
            end
            if node.results ~= nil then
                assert(type(node.results) == "table" and
                       next(node.results) ~= nil,
                    owner .. " result edges must be a non-empty object")
                for result_id, target in pairs(node.results) do
                    assert(is_identifier(result_id),
                        owner .. " has an invalid action result id")
                    require_node_reference(graph.nodes, target, owner)
                end
            end
            assert(node.next ~= nil or node.results ~= nil,
                owner .. " needs a next edge or result edges")
        elseif node.type == "terminal" then
            validate_outcome(node.outcome, cinematics)
        elseif node.type == "source_boundary" then
            assert(type(node.reason) == "string" and node.reason ~= "" and
                   #node.reason <= 384,
                owner .. " needs a bounded source-boundary reason")
        else
            error(owner .. " uses an unsupported node type")
        end
    end
    return true
end

local function validate_cinematic_document(document)
    assert(type(document) == "table" and document.schemaVersion == 1 and
           type(document.cinematics) == "table",
        "cinematic catalog has an unsupported schema")
    for cinematic_id, cinematic in pairs(document.cinematics) do
        assert(is_identifier(cinematic_id) and type(cinematic) == "table" and
               is_identifier(cinematic.presentation) and
               is_identifier(cinematic.assetProfileId) and
               is_identifier(cinematic.inputContract) and
               is_identifier(cinematic.returnPolicy) and
               is_array(cinematic.pages) and #cinematic.pages > 0,
            "cinematic definition is malformed: " .. tostring(cinematic_id))
        validate_provenance(cinematic.provenance)
        local seen_pages = {}
        for _, page in ipairs(cinematic.pages) do
            assert(type(page) == "table" and is_identifier(page.id) and
                   is_identifier(page.textKey) and
                   type(page.provenance) == "table" and
                   type(page.provenance.romAddress) == "string" and
                   page.provenance.romAddress:match("^0x%x+$") and
                   not seen_pages[page.id],
                "cinematic page is malformed or duplicated: " ..
                tostring(cinematic_id))
            seen_pages[page.id] = true
        end
    end
    return document.cinematics
end

function Catalog.new(document, condition_document, cinematic_document,
                     validation_mode)
    assert(type(document) == "table" and document.schemaVersion == 1 and
           type(document.graphs) == "table",
        "dialogue graph catalog has an unsupported schema")
    assert(validation_mode == nil or validation_mode == "on_use",
        "dialogue graph catalog has an unsupported validation mode")
    assert(type(condition_document) == "table" and
           condition_document.schemaVersion == 1 and
           type(condition_document.conditions) == "table",
        "dialogue condition catalog has an unsupported schema")
    local cinematics = validate_cinematic_document(cinematic_document)
    local conditions = {}
    for condition_id, definition in pairs(condition_document.conditions) do
        assert(is_identifier(condition_id) and type(definition) == "table" and
               type(definition.expression) == "table",
            "dialogue condition definition is malformed")
        ConditionEvaluator.validate(definition.expression)
        conditions[condition_id] = definition
    end
    local catalog = setmetatable({
        graphs = document.graphs,
        conditions = conditions,
        cinematics = cinematics,
        validation_mode = validation_mode or "eager",
        validated_graphs = {},
    }, Catalog)
    if catalog.validation_mode == "eager" then
        for graph_id, graph in pairs(document.graphs) do
            validate_graph(graph_id, graph, conditions, cinematics)
            catalog.validated_graphs[graph_id] = true
        end
    end
    return catalog
end

function Catalog:get(graph_id)
    local graph = self.graphs[graph_id]
    if graph ~= nil and not self.validated_graphs[graph_id] then
        validate_graph(graph_id, graph, self.conditions, self.cinematics)
        self.validated_graphs[graph_id] = true
    end
    return graph
end

function Session.new(catalog, condition_evaluator, action_registry, host)
    assert(type(catalog) == "table" and type(catalog.get) == "function" and
           type(condition_evaluator) == "table" and
           type(condition_evaluator.evaluate) == "function" and
           type(action_registry) == "table" and
           type(action_registry.execute) == "function" and
           type(host) == "table" and
           type(host.present_page) == "function" and
           type(host.present_choice) == "function",
        "dialogue graph session is missing a required adapter")
    return setmetatable({
        catalog = catalog,
        condition_evaluator = condition_evaluator,
        action_registry = action_registry,
        host = host,
    }, Session)
end

function Session:_condition(condition_id, context)
    local definition = self.catalog.conditions[condition_id]
    if definition == nil then
        return { status = ConditionEvaluator.Status.unknown,
                 unknown_facts = { "condition." .. tostring(condition_id) } }
    end
    return self.condition_evaluator:evaluate(definition.expression, context)
end

function Session:_boundary(graph_id, node_id, reason)
    return {
        type = "source_boundary",
        reason = "dialogue_graph:" .. graph_id .. ":" .. node_id .. ":" ..
            reason,
    }
end

local function presentation_for(graph, node, context)
    local invocation = context and context.presentation or {}
    return node.speakerId or graph.speakerId or invocation.speakerId or "",
           node.textProfile or graph.textProfile or
               invocation.textProfile or ""
end

function Session:_choose(graph_id, node_id, graph, node, choices, context)
    local ids = {}
    for index, choice in ipairs(choices) do ids[index] = choice.id end
    local speaker_id, text_profile = presentation_for(graph, node, context)
    if node.inputContract ~= "cursor_list" then
        local selected = self.host.present_choice(
            node, ids, speaker_id, text_profile)
        return selected, false
    end

    local selected_index = node.initialSelection or 1
    while true do
        local input = self.host.present_choice(
            node, ids, speaker_id, text_profile, selected_index)
        if input == "up" then
            selected_index = selected_index == 1
                and #choices or selected_index - 1
        elseif input == "down" then
            selected_index = selected_index == #choices
                and 1 or selected_index + 1
        elseif input == "confirm" then
            return choices[selected_index].id, false
        elseif input == "cancel" or input == "back" then
            return nil, true
        elseif input == "page" then
            -- ROM Vid-Phone C input is ignored by the contact-list loop.
        else
            return nil, false, self:_boundary(
                graph_id, node_id, "cursor_input_unresolved")
        end
    end
end

function Session:run(graph_id, context)
    local graph = self.catalog:get(graph_id)
    if graph == nil then
        return self:_boundary(graph_id, "catalog", "graph_missing")
    end
    if graph.precondition ~= nil then
        local result = self:_condition(graph.precondition, context)
        if result.status == ConditionEvaluator.Status.unknown then
            return self:_boundary(
                graph_id, "precondition",
                "condition_unknown:" ..
                    table.concat(result.unknown_facts or {}, ","))
        elseif result.status ~= ConditionEvaluator.Status.true_value then
            return self:_boundary(graph_id, "precondition", "not_satisfied")
        end
    end

    local node_id = graph.entryNode
    for _ = 1, MAX_GRAPH_STEPS do
        local node = graph.nodes[node_id]
        if node.type == "page" then
            local speaker_id, text_profile = presentation_for(
                graph, node, context)
            if self.host.present_page(node, speaker_id, text_profile) ~= true then
                return self:_boundary(graph_id, node_id,
                                      "page_presentation_failed")
            end
            node_id = node.next
        elseif node.type == "choice" then
            local visible = {}
            for _, choice in ipairs(node.choices) do
                if choice.when == nil then
                    visible[#visible + 1] = choice
                else
                    local result = self:_condition(choice.when, context)
                    if result.status == ConditionEvaluator.Status.unknown then
                        return self:_boundary(
                            graph_id, node_id,
                            "choice_condition_unknown:" ..
                                table.concat(result.unknown_facts or {}, ","))
                    elseif result.status ==
                           ConditionEvaluator.Status.true_value then
                        visible[#visible + 1] = choice
                    end
                end
            end
            if #visible == 0 then
                return self:_boundary(graph_id, node_id,
                                      "no_available_choices")
            end
            if node.inputContract == "buttons_abc" and #visible > 3 then
                return self:_boundary(graph_id, node_id,
                                      "visible_button_choice_limit_exceeded")
            end
            local choice_id, cancelled, boundary = self:_choose(
                graph_id, node_id, graph, node, visible, context)
            if boundary ~= nil then
                return boundary
            elseif cancelled then
                node_id = node.cancelNext
            elseif choice_id == nil then
                -- Some source button prompts return to their caller when no
                -- listed result is selected. Content must declare that edge.
                if node.noSelectionNext ~= nil then
                    node_id = node.noSelectionNext
                else
                    return self:_boundary(graph_id, node_id,
                                          "choice_input_unresolved")
                end
            else
                local selected
                for _, choice in ipairs(visible) do
                    if choice.id == choice_id then selected = choice break end
                end
                if selected == nil then
                    if node.noSelectionNext ~= nil then
                        node_id = node.noSelectionNext
                    else
                        return self:_boundary(
                            graph_id, node_id,
                            "choice_not_in_visible_options")
                    end
                else
                    node_id = selected.next
                end
            end
        elseif node.type == "branch" then
            local next_node
            for _, branch in ipairs(node.cases) do
                local result = self:_condition(branch.when, context)
                if result.status == ConditionEvaluator.Status.unknown then
                    return self:_boundary(
                        graph_id, node_id,
                        "branch_condition_unknown:" ..
                            table.concat(result.unknown_facts or {}, ","))
                elseif result.status == ConditionEvaluator.Status.true_value then
                    next_node = branch.next
                    break
                end
            end
            node_id = next_node or node.fallback
        elseif node.type == "action" then
            local succeeded, result_id, reason =
                self.action_registry:execute(
                    node.actionId, node.arguments, context)
            if not succeeded then
                return self:_boundary(
                    graph_id, node_id, "action_failed:" .. tostring(reason))
            end
            if result_id ~= nil then
                local target_node = node.results and
                    node.results[result_id] or nil
                if target_node == nil then
                    return self:_boundary(
                        graph_id, node_id,
                        "action_result_unmapped:" .. result_id)
                end
                node_id = target_node
            else
                node_id = node.next
            end
        elseif node.type == "terminal" then
            local outcome = {}
            for key, value in pairs(node.outcome) do
                outcome[key] = value
            end
            return outcome
        elseif node.type == "source_boundary" then
            return {
                type = "source_boundary",
                reason = "dialogue_graph:" .. graph_id .. ":" .. node_id ..
                    ":" .. node.reason,
            }
        else
            return self:_boundary(graph_id, node_id, "node_type_unresolved")
        end
    end
    return self:_boundary(graph_id, node_id, "step_limit_exceeded")
end

function DialogueGraph.run(graph_id, registered_actions, invocation_context)
    local content = type(game) == "table" and game.content or nil
    local graph_documents = type(content) == "table" and
        type(content.conversations) == "table" and
        content.conversations or nil
    local conditions = type(content) == "table" and content.conditions or nil
    local cinematic_document = type(content) == "table" and
        content.cinematics or nil
    if type(graph_documents) ~= "table" or type(conditions) ~= "table" or
       type(cinematic_document) ~= "table" then
        return { type = "source_boundary",
                 reason = "dialogue_graph:content_catalog_missing" }
    end
    local catalog = runtime_catalogs[content]
    if catalog == nil then
        local graphs = {}
        for _, document in pairs(graph_documents) do
            if type(document) == "table" and document.schemaVersion == 1 and
               type(document.graphs) == "table" then
                for id, graph in pairs(document.graphs) do
                    if graphs[id] ~= nil then
                        return { type = "source_boundary",
                                 reason = "dialogue_graph:duplicate_graph:" .. id }
                    end
                    graphs[id] = graph
                end
            end
        end
        local graph_document = { schemaVersion = 1, graphs = graphs }
        catalog = Catalog.new(
            graph_document, conditions, cinematic_document, "on_use")
        runtime_catalogs[content] = catalog
    end
    local evaluator = ConditionEvaluator.new(
        ConditionEvaluator.FactProvider.new())
    local actions = DialogueActions.new_default_registry(registered_actions)
    local context = { field = game.field }
    if invocation_context ~= nil then
        assert(type(invocation_context) == "table",
            "dialogue graph invocation context must be an object")
        for key, value in pairs(invocation_context) do
            context[key] = value
        end
    end
    if context.presentation ~= nil then
        assert(type(context.presentation) == "table",
            "dialogue graph presentation context must be an object")
        for key, value in pairs(context.presentation) do
            assert(key == "speakerId" or key == "textProfile",
                "dialogue graph presentation has an unsupported field: " ..
                tostring(key))
            assert(is_identifier(value),
                "dialogue graph presentation has an invalid " .. key)
        end
    end
    local host = {
        present_page = function(node, speaker_id, text_profile)
            return game.ui.say(node.eventId, speaker_id, text_profile,
                               node.presentation or "field_npc",
                               node.inputDelayFrames or 0)
        end,
        present_choice = function(node, choice_ids, speaker_id,
                                  text_profile, selected_index)
            if node.inputContract == "cursor_list" then
                return game.ui.cursor_menu(
                    node.menuId, choice_ids, selected_index,
                    speaker_id, text_profile)
            end
            if node.promptTextKey ~= nil then
                return game.ui.prompt_text_key(
                    node.menuId, node.promptTextKey, choice_ids,
                    speaker_id, text_profile)
            end
            return game.ui.menu(
                node.menuId, choice_ids, speaker_id, text_profile)
        end,
    }
    local session = Session.new(catalog, evaluator, actions, host)
    return session:run(graph_id, context)
end

DialogueGraph.Catalog = Catalog
DialogueGraph.Session = Session
DialogueGraph.validate_outcome = validate_outcome
return DialogueGraph
