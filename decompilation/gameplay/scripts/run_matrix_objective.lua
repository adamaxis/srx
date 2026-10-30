-- Matrix contracts use three named difficulty bands and a shared
-- objective/system selector. Runtime data is kept in the typed catalog under
-- data/matrix_run_catalog.lua rather than as ROM addresses or byte records.
local E = require("enums")
local Catalog = require("data.matrix_run_catalog")
local MatrixObjective = {}

-- F3EBC selects one of three Matrix bands through the F3DC6 dispatch table;
-- the gameplay policy keeps those bands as named variants in Lua.
local intro_by_band_id = {
    simple =
        "Looks like I have a simple matrix run lined up next.",
    moderate =
        "My comp shows a moderate matrix run to be contracted.",
    expert =
        "I have a matrix run to contract out, red-flagged for expert runners only!",
}

local band_by_id = {
    simple = { systems = Catalog.bands.simple },
    moderate = { systems = Catalog.bands.moderate },
    expert = { systems = Catalog.bands.expert },
}

local system_ordinal = {}
for index, system_id in ipairs(Catalog.system_order) do
    system_ordinal[system_id] = index - 1
end

local function includes(values, wanted)
    for _, value in ipairs(values) do
        if value == wanted then return true end
    end
    return false
end

local function select_objective(band)
    -- ROM 0x1ED2C rolls objective index, then a system from that objective's
    -- candidate list; it repeats both rolls until the system matches the
    -- selected simple/moderate/expert band.
    while true do
        local objective_index = game.modules.run_support.bounded_random(
            #Catalog.objectives)
        local objective = Catalog.objectives[objective_index + 1]
        local system_id = objective.systems[
            game.modules.run_support.bounded_random(#objective.systems) + 1]
        if includes(band.systems, system_id) then
            return objective_index, objective, system_id
        end
    end
end

local function existing_completion_marker()
    -- [C] Objective handlers clear FFFC28 bit 0 on completion. Carry only the
    -- confirmed fact, not the surrounding source flags word.
    if type(game.matrix_world_state) == "function" then
        local world = game.matrix_world_state()
        if type(world) == "table" and type(world.run) == "table" and
           type(world.run.completion_marker_present) == "boolean" then
            return world.run.completion_marker_present
        end
    end
    return false
end

function MatrixObjective.prepare_offer(contact_id, source_record)
    local band_id = source_record.matrix_band_id
    local band = band_by_id[band_id]
    assert(band ~= nil,
        "Matrix offer requires a named difficulty band")

    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local objective_index, objective, target_system_id =
        select_objective(band)
    local size = (support.bounded_random(3) + 1) * 10
    local state = {
        status = E.MatrixRunStatus.PENDING,
        objective = {
            node = objective.node,
            action = objective.action,
            file_class = objective.file_class,
            file_id = objective.file_id,
        },
        file_size = size,
        completion_marker_present = existing_completion_marker(),
        completion_recorded = false,
        target_system_id = target_system_id,
    }
    local offer = support.new_offer(
        "matrix_objective",
        "matrix",
        "complete_the_selected_matrix_objective",
        "complete_the_source_matrix_objective",
        nil,
        payment,
        source_record.runner_progress_delta)
    offer.contact_id = contact_id
    offer.objective_index = objective_index
    offer.target_system_id = target_system_id
    offer.target_system_name = Catalog.systems[target_system_id]
    offer.objective_text = objective_index < 17 and
        (offer.target_system_name .. " " .. objective.description) or
        objective.description
    offer.matrix_run_state = state
    game.runs.registry.current_offer = offer
    return offer.id
end

function MatrixObjective.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "matrix_objective")
    game.ui.say_text(intro_by_band_id[offer.matrix_band_id], true)
    game.ui.say_text(offer.objective_text, true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for completing this Matrix run.")
end

function MatrixObjective.resolve_field_event()
    -- Matrix completion is resolved by the Matrix objective handlers. Return
    -- settlement to the Johnson still needs the field-exit result bridge.
    return { action = "no_action" }
end

return MatrixObjective
