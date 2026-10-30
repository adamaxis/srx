-- Generic access to editor-authored map entities. The native Lua package
-- loader reads map structure from gameplay/world TSV files into
-- game.modules.world_content_data and versioned route JSON into
-- game.content.entities; this module joins those records for map scripts.
local World = {}
local selected_recruitment_pedestrian_variant = nil
local mode_one_activities =
    require("data.mode_one_activities")
local field_actor_contacts = require("data.field_actor_contacts")
local valid_field_scene_modes = {
    none = true,
    street = true,
    building = true,
    cave = true,
    unknown = true,
}

local function content()
    local data = game.modules.world_content_data
    if type(data) ~= "table" then
        return {
            entities = {}, areas = {}, map_triggers = {}, map_events = {},
            actor_spawns = {}, encounter_regions = {},
            world_activity_outcomes = {},
            dynamic_world_activity_actors = {},
            world_activity_actors = {},
        }
    end
    return data
end

function World.entity(entity_id)
    if type(entity_id) ~= "string" then
        return nil
    end
    return content().entities[entity_id]
end

function World.instances_in_scene(scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    return content().areas[scene_id] or {}
end

local function records_in_scene(field, scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    local records = content()[field]
    if type(records) ~= "table" then
        return {}
    end
    return records[scene_id] or {}
end

function World.triggers_in_scene(scene_id)
    return records_in_scene("map_triggers", scene_id)
end

-- Runtime map-event rules are also authored with the area's other map data.
-- Their exact coordinate scope, trigger type, state gates, and repeat policy
-- are read directly from the viewer TSV records into the runtime data table.
function World.events_in_scene(scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    local events = content().map_events or {}
    local result = {}
    for _, event in ipairs(events) do
        if event.scene == scene_id then
            result[#result + 1] = event
        end
    end
    return result
end

local interaction_task_fields = {
    routePreparation = "route_preparation",
    indexedTitleService = "indexed_title_service",
    fieldExitPrompt = "field_exit_prompt",
}

local function interaction_location_fields(link, context)
    if type(context) ~= "table" then return end
    link.location_type = context.type
    link.location_id = context.id
    link.location_text_profile = context.textProfile
    link.location_speaker_id = context.speakerId
end

local function append_interaction_task(link, task_id, task, context)
    local field = interaction_task_fields[task_id]
    assert(field ~= nil,
        "entity interaction has an unsupported task: " .. tostring(task_id))
    assert(type(task) == "table" and type(task.module) == "string" and
           task.module ~= "" and type(task["function"]) == "string" and
           task["function"] ~= "",
        "entity interaction task needs a module and function")
    link[field .. "_module"] = task.module
    link[field .. "_function"] = task["function"]
    interaction_location_fields(link, context)
end

local function normalize_entity_interaction(entity, interaction)
    assert(type(interaction) == "table" and type(interaction.id) == "string" and
           interaction.id ~= "" and type(interaction.source) == "table" and
           type(interaction.source.sceneId) == "number" and
           interaction.source.sceneId % 1 == 0 and
           type(interaction.source.eventCode) == "string" and
           type(interaction.route) == "table" and
           type(interaction.route.kind) == "string" and
           type(interaction.route.script) == "string",
        "entity interaction has an invalid id, source, or route")
    local route = interaction.route
    local link = {
        id = interaction.id,
        link_id = interaction.id,
        entity_id = entity.id,
        source_scene = interaction.source.sceneId,
        source_event = interaction.source.eventCode,
        route_kind = route.kind,
        script = route.script,
        route_priority = route.priority or 0,
        route_condition_id = route.conditionId,
        source_route_policy = route.sourceRoutePolicy,
        expected_destination_scene = route.sourceTableDestination,
        destination_scene = route.destinationScene,
        destination_mode = route.destinationMode,
        field_service_id = route.fieldServiceId,
        return_anchor = route.returnAnchor or "",
        return_facing = route.returnFacing or "",
    }
    assert(type(link.route_priority) == "number" and
           link.route_priority % 1 == 0,
        "entity interaction route priority must be an integer")

    local location_context = interaction.locationContext
    for task_id, task in pairs(interaction.tasks or {}) do
        append_interaction_task(link, task_id, task, location_context)
    end
    if type(interaction.entryDialogue) == "table" then
        local entry = interaction.entryDialogue
        local task = entry.task
        assert(type(task) == "table" and type(task.module) == "string" and
               type(task["function"]) == "string",
            "entity entry dialogue needs a module and function")
        link.entry_dialogue_module = task.module
        link.entry_dialogue_function = task["function"]
        link.entry_condition_id = entry.conditionId
        link.entry_event_id = entry.eventId or ""
        link.entry_continue_result_id = entry.continueResultId or ""
        link.entry_continue_outcome_type = entry.continueOutcomeType or ""
        link.entry_resource_override = entry.resourceOverride or ""
        interaction_location_fields(
            link, entry.locationContext or location_context)
        link.entry_location_type = link.location_type
        link.entry_location_id = link.location_id
        link.entry_text_profile = link.location_text_profile
        link.entry_speaker_id = link.location_speaker_id
    end
    return link
end

local function entity_interactions(entity)
    local catalog = type(game) == "table" and game.content or nil
    local definitions = type(catalog) == "table" and catalog.entities or nil
    local entity_content = type(definitions) == "table" and
        definitions[entity.id] or nil
    local document = type(entity_content) == "table" and
        entity_content.interactions or nil
    if document == nil then return {} end
    assert(type(document) == "table" and document.schemaVersion == 1 and
           document.entityId == entity.id and
           type(document.interactions) == "table",
        "entity interaction document has an invalid schema or entity id: " ..
            tostring(entity.id))
    return document.interactions
end

-- A location link binds a ROM collision event to the entity it belongs to.
-- More than one row may describe the same entrance when route conditions
-- select different source-verified outcomes. The Lua route policy resolves
-- those variants; the world index only returns their authored data.
function World.location_links_for_event(scene_id, event_code)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       type(event_code) ~= "string" then
        return {}
    end
    local result = {}
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        for _, link in ipairs(entity and entity.links or {}) do
            if link.source_scene == scene_id and
               link.source_event == event_code then
                local resolved_link = {}
                for key, value in pairs(link) do
                    resolved_link[key] = value
                end
                resolved_link.entity_id = entity.id
                resolved_link.link_id = link.link_id or link.id
                resolved_link.instance_id = instance.instance_id
                result[#result + 1] = resolved_link
            end
        end
        if entity ~= nil then
            for _, interaction in ipairs(entity_interactions(entity)) do
                local resolved_link =
                    normalize_entity_interaction(entity, interaction)
                if resolved_link.source_scene == scene_id and
                   resolved_link.source_event == event_code then
                    resolved_link.instance_id = instance.instance_id
                    result[#result + 1] = resolved_link
                end
            end
        end
    end
    return result
end

-- Keep the unique-link convenience API for callers that do not need
-- conditional variants. Ambiguous bindings remain unresolved here.
function World.location_link_for_event(scene_id, event_code)
    local links = World.location_links_for_event(scene_id, event_code)
    if #links == 1 then return links[1] end
    return nil
end

function World.actor_spawns_in_scene(scene_id)
    return records_in_scene("actor_spawns", scene_id)
end

-- Return actor definitions for one named activity. The import boundary
-- resolves source selectors and gate encodings before Lua sees these rows.
local function world_activity_actor_descriptors(activity_id, scene_id)
    local rows = content().world_activity_actors or {}
    local result = {}
    for _, row in ipairs(rows) do
        if row.activity_id == activity_id and
           (scene_id == nil or row.scene == scene_id) then
            result[#result + 1] = {
                id = row.id,
                scene = row.scene,
                activity_id = row.activity_id,
                activity_kind = row.activity_kind,
                activity_probe_enabled = row.activity_probe_enabled,
                name = row.actor_name,
                profile_id = row.profile_id,
                visual_profile_id = row.visual_profile_id,
                palette_line = row.palette_line,
                sprite = row.sprite_stem,
                x = row.world_x,
                y = row.world_y,
                heading = row.initial_heading,
                action_kind = row.action_kind,
                action_id = row.action_id,
                actor_facts = row.actor_facts,
                physical_meter = row.physical_meter,
                action_delay_modifier = row.action_delay_modifier,
                combatant = {
                    actor_facts = {
                        interaction = row.actor_facts.interaction,
                    },
                    physical_meter = row.physical_meter,
                    mental_meter = row.combat_mental_meter,
                    cyberware = row.combat_cyberware,
                    posture_reduction_percent = row.posture_reduction_percent,
                    attributes = {
                        body = row.combat_body,
                        quickness = row.combat_quickness,
                        strength = row.combat_strength,
                        intelligence = row.combat_intelligence,
                        willpower = row.combat_willpower,
                    },
                    skills = {
                        sorcery = row.combat_sorcery,
                        firearms = row.combat_firearms,
                        pistols = row.combat_pistols,
                        smgs = row.combat_smgs,
                        shotguns = row.combat_shotguns,
                        melee_combat = row.combat_melee_combat,
                    },
                    equipped_armor_id = row.equipped_armor_id,
                    combat_traits = row.combat_traits,
                    action_profile_options = {
                        primary = row.primary_action_profile_enabled,
                        secondary = row.secondary_action_profile_enabled,
                    },
                    force = row.force,
                    spell_trial_cap = row.spell_trial_cap,
                    weapon_action_policies = row.weapon_action_policies,
                    inventory = row.inventory,
                },
            }
        end
    end
    return result
end

function World.world_activity_actors_for_activity_id(activity_id, scene_id)
    if type(activity_id) ~= "string" or activity_id == "" or
       (scene_id ~= nil and
        (type(scene_id) ~= "number" or scene_id % 1 ~= 0)) then
        return {}
    end
    return world_activity_actor_descriptors(activity_id, scene_id)
end

-- Expose only the semantic choices the A800 policy needs. Spawn placement,
-- artwork, source flags, and source-table annotations stay in the host catalog.
function World.conditional_actor_candidates_for_scene(scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       scene_id < 0 or scene_id > 55 then
        return {}
    end
    local result = {}
    for _, row in ipairs(content().conditional_actor_candidates or {}) do
        if row.scene == scene_id then
            result[#result + 1] = {
                candidate_id = row.candidate_id,
                variant = row.variant,
            }
        end
    end
    local variant_order = { first = 1, second = 2, unreachable = 3 }
    table.sort(result, function(left, right)
        return variant_order[left.variant] < variant_order[right.variant]
    end)
    return result
end

function World.reset_conditional_actor_selection()
    selected_recruitment_pedestrian_variant = nil
end

-- The request path only needs to know whether the chosen option is pending.
-- The selected first/second variant stays private to this Lua policy.
function World.consume_conditional_actor_selection()
    local variant = selected_recruitment_pedestrian_variant or "none"
    selected_recruitment_pedestrian_variant = nil
    return variant
end

local function mode_one_activity(scene_id, facts)
    if facts.scene_mode ~= "street" then
        return ""
    end
    local rows = mode_one_activities[scene_id]
    if type(rows) ~= "table" then
        return ""
    end
    for _, row in ipairs(rows) do
        local variant_matches = row.variant == nil or
            facts.scene_ten_activity_variant == row.variant
        local required_fact = row.required_story_fact
        local gate_matches = required_fact == nil or
            facts.completed_story_facts[required_fact] == true
        local party_does_not_include_actor =
            facts.party_present_activity_ids[row.activity_id] ~= true
        if variant_matches and gate_matches and party_does_not_include_actor then
            return row.world_activity_id
        end
    end
    return ""
end

-- Resolve scene activities from named gameplay facts. The host translates
-- persisted source state at the boundary; this policy keeps a chosen
-- recruitment encounter stable while scene setup is deferred.
function World.resolve_conditional_actor_selection(
    scene_id, facts, random_below)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       scene_id < 0 or scene_id > 55 then
        error("conditional actor selection scene is invalid")
    end
    if type(facts) ~= "table" then
        error("conditional actor selection context is invalid")
    end
    local valid_activity_states = {
        idle = true,
        actors_being_installed = true,
        story_interaction_active = true,
        other_activity_active = true,
        queued_activity = true,
    }
    if valid_activity_states[facts.activity_state] ~= true then
        error("conditional actor activity state is invalid: " ..
            tostring(facts.activity_state))
    end
    if valid_field_scene_modes[facts.scene_mode] ~= true then
        error("conditional actor scene mode is invalid: " ..
            tostring(facts.scene_mode))
    end
    if (facts.scene_ten_activity_variant ~= "standard" and
        facts.scene_ten_activity_variant ~= "alternate") or
       type(facts.completed_story_facts) ~= "table" or
       type(facts.party_present_activity_ids) ~= "table" or
       type(facts.recruitment_encounter_ready) ~= "boolean" or
       type(random_below) ~= "function" then
        error("conditional actor selection facts are invalid")
    end
    if (facts.active_offer_id ~= nil and
        (type(facts.active_offer_id) ~= "string" or
         facts.active_offer_id == "")) then
        error("conditional actor selection has invalid gameplay facts")
    end

    for activity_id, present in pairs(facts.party_present_activity_ids) do
        if type(activity_id) ~= "string" or activity_id == "" or
           type(present) ~= "boolean" then
            error("conditional actor party facts are invalid")
        end
    end
    for fact_id, completed in pairs(facts.completed_story_facts) do
        if type(fact_id) ~= "string" or fact_id == "" or
           type(completed) ~= "boolean" then
            error("conditional actor story facts are invalid")
        end
    end

    local variant = selected_recruitment_pedestrian_variant
    local choice_changed = false
    local result = function(kind, selection_id)
        return {
            kind = kind,
            selection_variant = variant or "none",
            choice_changed = choice_changed,
            selection_id = selection_id,
        }
    end

    -- Finish the current scene transition before choosing another activity.
    if facts.activity_state == "actors_being_installed" then
        return result("none", "")
    end

    -- Active interactions keep priority over ambient scene activities.
    if facts.activity_state == "story_interaction_active" then
        return result("story_contact", "scene_34_story_contact")
    elseif facts.activity_state == "other_activity_active" then
        return result("none", "")
    elseif facts.activity_state == "queued_activity" then
        return result("pending_world_activity", "scene_20_ilene_activity")
    end

    -- Street locations can start their authored activities before an
    -- incidental recruitment encounter is considered.
    if facts.scene_mode == "street" then
        local activity_id = mode_one_activity(scene_id, facts)
        if activity_id ~= "" then
            return result("mode_one_world_activity", activity_id)
        end
        return result("none", "")
    end

    -- The encounter belongs to the accepted employee-recruitment run.
    if facts.active_offer_id ~= "employee_recruitment" or
       facts.scene_mode ~= "building" then
        return result("none", "")
    end
    if variant == nil then
        local roll = random_below(2)
        if type(roll) ~= "number" or roll % 1 ~= 0 or
           roll < 0 or roll >= 2 then
            error("conditional actor random service returned an invalid roll")
        end
        variant = roll == 0 and "first" or "second"
        selected_recruitment_pedestrian_variant = variant
        choice_changed = true
    end

    -- Keep this target while the arrival is temporarily unable to place it.
    if not facts.recruitment_encounter_ready then
        return result("none", "")
    end

    local candidates = World.conditional_actor_candidates_for_scene(scene_id)
    for _, candidate in ipairs(candidates) do
        if candidate.variant == variant then
            return result("random_pedestrian", candidate.candidate_id)
        end
    end
    return result("none", "")
end

-- The host identifies the interaction category and Lua selects the gameplay
-- response for that actor. Contact behavior is keyed by stable content IDs.
function World.field_actor_contact_task(
    scene_id, actor_instance_id, activity_id, handler)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       scene_id < 0 or scene_id > 55 or
       type(actor_instance_id) ~= "string" or actor_instance_id == "" or
       type(activity_id) ~= "string" or
       (handler ~= "ordinary" and handler ~= "world_effect" and
        handler ~= "pending_world_effect") then
        error("field actor contact needs semantic actor and handler facts")
    end

    local activity_route = field_actor_contacts[activity_id]
    if activity_route ~= nil then
        if activity_route.handler ~= handler then
            return "start_activity"
        end
        if activity_route.mark_escort and
           not World.mark_contact_actor_for_escort_cleanup(
               scene_id, actor_instance_id) then
            return "start_activity"
        end
        return activity_route.action
    end
    if handler ~= "ordinary" then
        return "start_activity"
    end

    -- The A800 selector gives reachable random pedestrians a stable
    -- candidate identity. This handles its ordinary contact response through
    -- the same stable ID used by the encounter catalog.
    for _, candidate in ipairs(
        World.conditional_actor_candidates_for_scene(scene_id)) do
        if candidate.candidate_id == activity_id then
            return "random_pedestrian_contact"
        end
    end
    return "start_activity"
end

function World.mark_contact_actor_for_escort_cleanup(
    scene_id, actor_instance_id)
    if type(game.actors) ~= "table" then
        error("contact actor cleanup needs the Lua actor registry")
    end
    local state = game.actors[actor_instance_id]
    if type(state) ~= "table" or
       state.scene_id ~= scene_id or
       type(state.actor_facts) ~= "table" or
       type(state.actor_facts.lifecycle) ~= "table" or
       state.actor_facts.lifecycle.active ~= true or
       type(state.actor_facts.behavior) ~= "table" then
        return false
    end
    state.actor_facts.behavior.escort = true
    return true
end

function World.pending_world_activity_id_for_scene(
    scene_id, world_effect_active, pending_world_effect)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       type(world_effect_active) ~= "boolean" or
       type(pending_world_effect) ~= "boolean" then
        return nil
    end
    local selected_activity_id = nil
    local function requirement_matches(requirement, actual, active_word,
                                       inactive_word)
        if requirement == active_word then
            return actual
        end
        if requirement == inactive_word then
            return not actual
        end
        return false
    end
    for _, row in ipairs(content().world_activity_actors or {}) do
        if row.scene == scene_id and row.activity_kind == "event_driven" and
           requirement_matches(row.world_effect_active_requirement,
                               world_effect_active, "active", "inactive") and
           requirement_matches(row.pending_world_effect_requirement,
                               pending_world_effect, "pending", "clear") then
            if type(row.activity_id) ~= "string" or row.activity_id == "" then
                error("pending world activity is missing its activity id")
            end
            if selected_activity_id ~= nil and
               selected_activity_id ~= row.activity_id then
                error("ambiguous pending world activities")
            end
            selected_activity_id = row.activity_id
        end
    end
    return selected_activity_id
end

function World.encounter_regions_in_scene(scene_id)
    return records_in_scene("encounter_regions", scene_id)
end

-- Collision rectangles belong to reusable entity definitions and are
-- translated by the area's entity origin. Unreviewed records are inspectable
-- in the viewer but are not exposed as runtime collision results.
function World.collision_at(scene_id, x, y)
    if type(scene_id) ~= "number" or type(x) ~= "number" or
       type(y) ~= "number" then
        return nil
    end
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        if entity then
            for _, region in ipairs(entity.collision_regions or {}) do
                local x0 = instance.origin_x + region.local_x
                local y0 = instance.origin_y + region.local_y
                local reviewed = region.review_status == "source_verified" or
                    region.review_status == "new_content"
                if reviewed and region.mode ~= "unreviewed" and
                   x >= x0 and y >= y0 and
                   x < x0 + region.width and
                   y < y0 + region.height then
                    return {
                        instance_id = instance.instance_id,
                        entity_id = entity.id,
                        collision_id = region.id,
                        mode = region.mode,
                        evidence = region.evidence,
                        review_status = region.review_status,
                    }
                end
            end
        end
    end
    return nil
end

-- Resolve a manually authored interaction at map-pixel coordinates. ROM
-- event records remain the authority for existing interactions; authored
-- rectangles provide the reusable extension point for new world content.
function World.interaction_at(scene_id, x, y, source_event)
    if type(scene_id) ~= "number" or type(x) ~= "number" or
       type(y) ~= "number" then
        return nil
    end
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        if entity then
            for _, interaction in ipairs(entity.interactables or {}) do
                local event_matches = source_event == nil or
                    interaction.source_event == "" or
                    interaction.source_event == source_event
                local x0 = instance.origin_x + interaction.local_x
                local y0 = instance.origin_y + interaction.local_y
                if event_matches and x >= x0 and y >= y0 and
                   x < x0 + interaction.width and
                   y < y0 + interaction.height then
                    return {
                        instance_id = instance.instance_id,
                        entity_id = entity.id,
                        lua_handler = interaction.lua_handler,
                        source_event = interaction.source_event,
                        interaction_id = interaction.id,
                    }
                end
            end
        end
    end
    return nil
end

return World
