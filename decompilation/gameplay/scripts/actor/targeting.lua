-- actor/targeting.lua: logic: profile lookup, target eligibility and selection, motion bounds, initial state.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local Bearing = require("bearing")
local SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local ActionKind = R.ActionKind   -- from actor/tables.lua
local ActorApproachDistanceBounds = R.ActorApproachDistanceBounds -- from actor/tables.lua
local profiles = R.profiles   -- from actor/tables.lua
local ActorProfileCatalog = require("data.actor_profiles")
local ProjectileAnimationClips =
    require("data.weapon_projectile_animation_clips")
local source_close_range_threshold = R.source_close_range_threshold   -- from actor/tables.lua
local VisualProfile = E.ActorVisualProfile
local DynamicEncounterVisualProfiles = {
    [VisualProfile.ORK_BANDERSNATCH] = true,
    [VisualProfile.HUMAN_GUARD] = true,
    [VisualProfile.ELVEN_MAGE] = true,
    [VisualProfile.GHOUL_EYE_FIVER] = true,
    [VisualProfile.YAKUZA_WEENR] = true,
    [VisualProfile.VAMPIRE_GATOR_SHAMAN] = true,
    [VisualProfile.LONE_STAR_MAGE] = true,
    [VisualProfile.PEDESTRIAN_RAT_SHAMAN] = true,
    [VisualProfile.HELL_HOUND] = true,
    [VisualProfile.GARGOYLE] = true,
    [VisualProfile.WENDIGO] = true,
    [VisualProfile.THON] = true,
}

local function copy_actor_facts(facts)
    facts = type(facts) == "table" and facts or {}
    local lifecycle = type(facts.lifecycle) == "table" and facts.lifecycle or {}
    local interaction = type(facts.interaction) == "table" and facts.interaction or {}
    local behavior = type(facts.behavior) == "table" and facts.behavior or {}
    local roles = type(facts.roles) == "table" and facts.roles or {}
    return {
        lifecycle = {
            active = lifecycle.active == true,
            thinking_enabled = lifecycle.thinking_enabled == true,
            ai_runner = lifecycle.ai_runner == true,
            downed = lifecycle.downed == true,
            group_leader = lifecycle.group_leader == true,
            dead = lifecycle.dead == true,
        },
        interaction = {
            projectile = interaction.projectile == true,
            proximity_interaction_pending =
                interaction.proximity_interaction_pending == true,
            use_collision_heading = interaction.use_collision_heading == true,
            collision_recovery_active =
                interaction.collision_recovery_active == true,
            action_hook_pending = interaction.action_hook_pending == true,
            defense_difficulty_reduction =
                interaction.defense_difficulty_reduction == true,
            breathing_out = interaction.breathing_out == true,
            unseen = interaction.unseen == true,
            no_coverage_difficulty_bonus =
                interaction.no_coverage_difficulty_bonus == true,
            defense_difficulty_reduction =
                interaction.defense_difficulty_reduction == true,
        },
        behavior = {
            route_clear = behavior.route_clear == true,
            path_active = behavior.path_active == true,
            restart_steering = behavior.restart_steering == true,
            wall_following = behavior.wall_following == true,
            in_target_band = behavior.in_target_band == true,
            route_transition = behavior.route_transition == true,
            fleeing = behavior.fleeing == true,
            escort = behavior.escort == true,
        },
        roles = {
            spotted = roles.spotted == true,
            reinforcement = roles.reinforcement == true,
            despawn_offscreen = roles.despawn_offscreen == true,
            quest = roles.quest == true,
            runner = roles.runner == true,
            pedestrian = roles.pedestrian == true,
            hostile = roles.hostile == true,
        },
    }
end
local target_motion_reacquire_updates = R.target_motion_reacquire_updates   -- from actor/tables.lua

-- ROM 0x13874 selects the two pointer-list refreshes from FFDFE8 and
-- FFDF88. Return world-list and runner-list decisions in that order [C].
function R.should_rebuild_target_lists(frame_counter, display_service_word)
    if type(frame_counter) ~= "number" or frame_counter % 1 ~= 0 or
       frame_counter < 0 or frame_counter > AiTuning.WORD_MAX or
       type(display_service_word) ~= "number" or
       display_service_word % 1 ~= 0 or display_service_word < 0 or
       display_service_word > AiTuning.WORD_MAX then
        return nil
    end
    if display_service_word ~= 0 then
        return true, true
    end
    local phase = frame_counter & AiTuning.TARGET_LIST_PHASE_MASK
    return phase == AiTuning.WORLD_TARGET_LIST_PHASE,
           phase == AiTuning.RUNNER_TARGET_LIST_PHASE
end

local function direction_from_bearing_degrees(bearing_degrees)
    -- The eight way animation selector rounds the measured bearing to the
    -- nearest authored facing. Bearings use degrees clockwise from north.
    return Bearing.octant(bearing_degrees, true)
end

local function approach_bearing_degrees(current_degrees, target_degrees)
    -- The source slew is an 11.25 degree step per actor service pass. Keep
    -- that behavior while storing an ordinary bearing in Lua state.
    return Bearing.approach(current_degrees, target_degrees, 11.25)
end

local function dynamic_encounter_profile(context)
    if context.dynamic_world_encounter_actor ~= true or
       type(context.visual_profile_id) ~= "string" then
        return nil
    end
    local visual_profile_id = context.visual_profile_id
    local action_kind = context.action_kind
    local action_id
    if action_kind == ActionKind.item then
        action_id = context.action_item_id
    elseif action_kind == ActionKind.spell then
        action_id = context.action_spell_id
    elseif action_kind ~= ActionKind.none then
        return nil
    end
    local world_activity_runner =
        context.world_activity_script_actor == true and
        visual_profile_id == VisualProfile.FIELD_RUNNER
    if (not DynamicEncounterVisualProfiles[visual_profile_id] and
        not world_activity_runner) or
       (visual_profile_id == VisualProfile.THON and
        (action_kind ~= ActionKind.spell or
         action_id ~= "flame_dart")) or
       action_kind ~= context.action_kind or
       ((action_kind == ActionKind.item or
         action_kind == ActionKind.spell) and
        type(action_id) ~= "string") then
        return nil
    end
    local source_actor_profile = ActorProfileCatalog.dynamic(
        visual_profile_id, action_kind, action_id)
    return {
        package_id = "world_actor." .. visual_profile_id ..
                     ".source_target_motion",
        visual_profile_id = source_actor_profile.visual_profile_id,
        action_kind = source_actor_profile.action_kind,
        action_id = source_actor_profile.action_id,
        weapon_item_id = source_actor_profile.weapon,
        controls_movement = source_actor_profile.controls_movement,
        dynamic_encounter_only =
            source_actor_profile.dynamic_encounter_only,
        source_profile = source_actor_profile,
    }
end

local function source_profile(context)
    local profile = context.dynamic_world_encounter_actor == true
        and dynamic_encounter_profile(context)
        or profiles[context.profile_id]
    if profile == nil or
       (profile.dynamic_encounter_only and
        context.dynamic_world_encounter_actor ~= true) or
       (profile.dynamic_encounter_only and
        not context.actor_facts.lifecycle.active) or
       (not profile.projectile_animation and
        not profile.hit_spark_animation and
        context.visual_profile_id ~= profile.visual_profile_id) or
       context.action_kind ~= profile.action_kind or
       (profile.action_kind == ActionKind.item and
        context.action_item_id ~= profile.weapon_item_id) or
       (profile.action_id ~= nil and
        ((context.action_kind == ActionKind.item and
          context.action_item_id ~= profile.action_id) or
         (context.action_kind == ActionKind.spell and
          context.action_spell_id ~= profile.action_id))) then
        return nil
    end
    -- ROM 0x142AE takes the default-list waypoint branch for records carrying
    -- its route behavior flag. 0x44BC initializes that route for list actors,
    -- independent of sprite kind or the +0x9E pedestrian flag.
    local source_profile_patrol =
        context.source_actor_has_waypoint_data == true
    local resolved_profile = {}
    for key, value in pairs(profile) do
        resolved_profile[key] = value
    end
    resolved_profile.source_profile_patrol = source_profile_patrol
    return resolved_profile
end

local function target_is_eligible(candidate, world_actor_pool)
    if type(candidate) ~= "table" or type(candidate.id) ~= "string" or
       candidate.id == "" or candidate.projected_y_nonzero ~= true or
       candidate.actor_facts.lifecycle.active ~= true or
       candidate.actor_facts.lifecycle.dead == true or
       candidate.actor_facts.lifecycle.downed == true then
        return false
    end
    if candidate.world_activity_actor ~= world_actor_pool then
        return false
    end
    if world_actor_pool and
       (candidate.actor_facts.behavior.escort == true or
        candidate.actor_facts.roles.pedestrian == true or
        candidate.actor_facts.roles.quest == true) then
        return false
    end
    return true
end

local function linked_target(context, state)
    local source_candidates = context.ai_target_candidates
    if type(source_candidates) ~= "table" then
        state.linked_target_id = nil
        return nil
    end
    local world_actor_pool = context.actor_facts.lifecycle.ai_runner == true
    local previous_target_id = state.linked_target_id
    local candidates = {}
    local retained
    for _, candidate in ipairs(source_candidates) do
        if target_is_eligible(candidate, world_actor_pool) then
            candidates[#candidates + 1] = candidate
            if candidate.id == state.linked_target_id then
                retained = candidate
            end
        end
    end
    if retained ~= nil then
        return retained
    end
    state.linked_target_id = nil
    if #candidates == 0 or type(context.ai_random_bound) ~= "function" then
        return nil
    end
    local choice = context.ai_random_bound(#candidates)
    if type(choice) ~= "number" or choice % 1 ~= 0 or
       choice < 0 or choice >= #candidates then
        return nil
    end
    retained = candidates[choice + 1]
    state.linked_target_id = retained.id
    if not world_actor_pool and retained.id ~= previous_target_id then
        -- ROM 0x144E0 sets +0xB5 to six only when a new target is linked
        -- from the three-runner pool. 0x13D0C consumes one on this same
        -- 21-pass AI refresh, then one on each later route update.
        state.target_motion_delay = target_motion_reacquire_updates
    end
    return retained
end

local function action_target_motion_range(context)
    if context.action_kind == ActionKind.item then
        local item_module = game.modules.items
        local item = type(item_module) == "table" and
            type(item_module.by_id) == "table" and
            item_module.by_id[context.action_item_id] or nil
        if type(item) == "table" and item.id == context.action_item_id and
           type(item.target_motion_range) == "table" then
            return item.target_motion_range
        end
    elseif context.action_kind == ActionKind.spell then
        local magic_module = game.modules.magic
        if type(magic_module) == "table" and
           type(magic_module.target_motion_range) == "function" then
            return magic_module.target_motion_range(context.action_spell_id)
        end
    end
    return nil
end

local function source_motion_bounds(context, state)
    if context.actor_facts.lifecycle.ai_runner == true then
        -- ROM 0x13D84 selects a distance interval from actor +0x4D; the host
        -- has already converted that source value to a named approach band.
        local profile = context.actor_behavior_profile
        local band = type(profile) == "table" and profile.approach_band
        local bounds = type(band) == "string" and
            ActorApproachDistanceBounds[band] or nil
        if type(bounds) == "table" then
            return bounds, true
        end
        -- Do not guess a destination when the host has no source-derived band.
        return nil, false
    end

    if state.target_motion_delay == 0 then
        return nil, true
    end
    state.target_motion_delay = state.target_motion_delay - 1
    local range = action_target_motion_range(context)
    if type(range) ~= "table" or
       type(range.minimum_distance) ~= "number" or
       type(range.maximum_distance) ~= "number" or
       range.minimum_distance % 1 ~= 0 or
       range.maximum_distance % 1 ~= 0 or
       range.minimum_distance < 0 or
       range.maximum_distance < range.minimum_distance then
        -- Leave the route pending when the selected action has no recovered
        -- target-distance interval.
        return nil, false
    end
    return range, true
end

local function source_motion_destination_bearing_degrees(context, state, target)
    -- ROM 0x13D0C clears +0xB0.4 before selecting the action distance row.
    state.actor_facts.behavior.in_target_band = false
    if type(target) ~= "table" or not context.movement_allowed or
       type(target.bearing_degrees) ~= "number" or
       type(target.distance) ~= "number" or
       type(target.direct_route_distance) ~= "number" or
       type(target.perpendicular_route_distance) ~= "number" or
       type(target.direct_route_step_cost) ~= "number" or
       type(target.perpendicular_route_step_cost) ~= "number" then
        return nil
    end

    local bounds, bounds_resolved = source_motion_bounds(context, state)
    if not bounds_resolved then
        return nil
    end
    local bearing_degrees = target.bearing_degrees
    local distance = target.distance & 0xffff
    if bounds == nil or distance < bounds.maximum_distance then
        -- A zero +0xB5 counter reaches 0x13DCA directly. The selected action
        -- range reaches the same state below its upper distance.
        state.actor_facts.behavior.in_target_band = true
    end
    context.movement_state.route_facts = state.actor_facts.behavior
    local destination_bearing_degrees =
        Bearing.normalize(bearing_degrees + AiTuning.QUARTER_TURN_DEGREES)
    local destination_step_cost = target.perpendicular_route_step_cost
    local destination_distance = target.perpendicular_route_distance & 0xffff

    if bounds ~= nil then
        if distance >= bounds.minimum_distance and
           distance < bounds.maximum_distance then
            -- In-band actors drop +0xB5 and choose the lateral destination.
            state.target_motion_delay = 0
        elseif distance >= bounds.maximum_distance then
            -- At or beyond the upper bound the ROM copies the linked
            -- position and routes directly toward it.
            destination_bearing_degrees = bearing_degrees
            destination_step_cost = target.direct_route_step_cost
            destination_distance = target.direct_route_distance & 0xffff
        end
    end

    -- 0x144E8/0x1460E finish the route when the source diagonal step metric
    -- from the acting record to the chosen destination is below 0x1E0.
    if destination_step_cost < source_close_range_threshold then
        state.movement_active = false
        state.movement_route_distance = nil
        return nil
    end
    state.movement_route_distance = destination_distance
    return destination_bearing_degrees
end

local function retained_linked_target(context, state)
    if type(state.linked_target_id) ~= "string" or
       type(context.ai_target_candidates) ~= "table" then
        return nil
    end
    local world_actor_pool = context.actor_facts.lifecycle.ai_runner == true
    for _, candidate in ipairs(context.ai_target_candidates) do
        if candidate.id == state.linked_target_id and
           target_is_eligible(candidate, world_actor_pool) then
            return candidate
        end
    end
    return nil
end

local function initial_state(context, profile)
    local action_id = context.action_kind == ActionKind.item
        and context.action_item_id or
        context.action_kind == ActionKind.spell
        and context.action_spell_id or nil
    local state = {
        actor_id = context.actor_id,
        profile_id = context.profile_id,
        visual_profile_id = context.visual_profile_id,
        action_kind = context.action_kind,
        action_id = action_id,
        action_item_id = context.action_item_id,
        actor_facts = copy_actor_facts(context.actor_facts),
        live_field_actor = context.live_field_actor == true,
        movement_scene_id = context.movement_scene_id,
        physical_condition = context.physical_condition,
        -- Lua owns the mutable combat meters used by world-activity policy.
        -- The host supplies this record only at spawn and later combat results
        -- replace it through combat.lua.
        combatant = type(context.combatant) == "table"
            and context.combatant or nil,
        ammunition_remaining = context.ammunition_remaining or 0,
        package_id = profile.package_id,
        animation_cadence_profile =
            profile.animation_cadence_profile or "moving_actor_walk",
        weapon_item_id = profile.weapon_item_id,
        direction = direction_from_bearing_degrees(
            context.initial_bearing_degrees),
        entry_delay = context.entry_delay,
        behavior_timer = 0,
        movement_distance_per_update = 0,
        animation_id = "idle",
        animation_frame = 0,
        animation_elapsed_updates = 0,
        animation_progress = 0,
        action_animation_complete = false,
        item_action_complete = false,
        damage_reaction_active = false,
        damage_reaction_animation_complete = false,
        hurt_behavior_timer = 0,
        hurt_audio_event = nil,
        hurt_audio_emitted = false,
        movement_current_bearing_degrees =
            (profile.controls_movement or
             (profile.source_profile_patrol and
              context.live_field_actor == true)) and
            context.initial_bearing_degrees or nil,
        movement_route_distance = nil,
        movement_active = false,
        transition_started = false,
        transition_frame_list_finished = false,
        transition_complete = false,
        combat_action_requested = false,
        ai_target_id = nil,
        ai_target_bearing_degrees = nil,
        ai_target_distance = nil,
        ai_target_engaged = false,
        target_motion_delay = 0,
        target_selection_latched = false,
        target_marker_visible = false,
        target_marker_animating = false,
        target_marker_style = 0,
        target_marker_radius = 0,
        target_marker_countdown = 0,
        action_delay = context.initial_action_delay or 0,
        last_action_successes = type(context.combatant) == "table"
            and context.combatant.last_action_successes or 0,
        spell_action_resolution = nil,
        spell_effect_pending = nil,
        spell_effect_scan_marker = nil,
        spell_effect_completed = false,
        spell_effect_slot_released = nil,
        spell_effect_resolution = nil,
        spell_effect_visible = false,
        spell_effect_phase = 0,
        spell_effect_id = nil,
        pending_source_action = nil,
        action_timing = type(context.action_timing) == "table" and {
            quickness = context.action_timing.quickness or 0,
            intelligence = context.action_timing.intelligence or 0,
            action_delay_modifier =
                context.action_timing.action_delay_modifier or 0,
        } or nil,
        target_motion_effect_active = false,
        target_motion_effect_slot = nil,
        target_motion_effect_slot_request = nil,
        target_motion_effect_slot_released = false,
        target_motion_effect_countdown = 0,
        target_motion_effect_phase = 0,
        target_motion_effect_direction = 0,
        target_motion_effect_visible = false,
        selected_world_actor_id = nil,
        target_selection_processed = false,
    }
    if profile.projectile_animation then
        local animation = context.projectile_animation
        if type(animation) ~= "table" or
           type(animation.frame_count) ~= "number" or
           type(ProjectileAnimationClips[animation.frame_count]) ~= "table" then
            return nil
        end
        state.animation_id = "projectile"
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        state.projectile_animation_started = false
        state.animation_complete = false
        local projectile = context.projectile_movement
        state.last_action_successes = type(projectile) == "table"
            and projectile.attack_successes or 0
        state.attack_seed_resolved =
            not context.projectile_attack_seed_pending
        state.combat_meter_update = nil
    end
    if profile.hit_spark_animation then
        state.animation_id = "hit_spark"
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        state.hit_spark_retired = false
        state.hit_spark_animation_complete = false
    end
    return state
end

local function same_actor(state, context, profile)
    local action_id = context.action_kind == ActionKind.item
        and context.action_item_id or
        context.action_kind == ActionKind.spell
        and context.action_spell_id or nil
    local same_action = state.action_kind == context.action_kind and
        state.action_id == action_id
    if state.profile_id == context.profile_id and
       state.visual_profile_id == context.visual_profile_id and
       same_action then
        return true
    end
    local pending = state.pending_source_action
    local pending_action_matches = type(pending) == "table" and
        pending.action_kind == context.action_kind and
        (context.action_kind == ActionKind.item
            and pending.action_item_id == context.action_item_id
            or context.action_kind == ActionKind.none and
                pending.action_item_id == nil)
    if type(pending) ~= "table" or
       pending.profile_prefix ~= context.profile_prefix or
       not pending_action_matches or
       state.visual_profile_id ~= context.visual_profile_id then
        return false
    end
    state.profile_id = context.profile_id
    state.action_kind = context.action_kind
    state.action_id = action_id
    state.action_item_id = context.action_item_id
    state.package_id = profile.package_id
    state.weapon_item_id = profile.weapon_item_id
    state.pending_source_action = nil
    state.animation_id = "idle"
    state.animation_frame = 0
    state.animation_elapsed_updates = 0
    state.animation_progress = 0
    return true
end


-- register this module's definitions
R.direction_from_bearing_degrees = direction_from_bearing_degrees
R.approach_bearing_degrees = approach_bearing_degrees
R.dynamic_encounter_profile = dynamic_encounter_profile
R.source_profile = source_profile
R.target_is_eligible = target_is_eligible
R.linked_target = linked_target
R.action_target_motion_range = action_target_motion_range
R.source_motion_bounds = source_motion_bounds
R.source_motion_destination_bearing_degrees = source_motion_destination_bearing_degrees
R.retained_linked_target = retained_linked_target
R.initial_state = initial_state
R.same_actor = same_actor
return R
