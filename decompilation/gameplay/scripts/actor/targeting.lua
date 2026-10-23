-- actor/targeting.lua: logic: profile lookup, target eligibility and selection, motion bounds, initial state.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local ActionKind = R.ActionKind   -- from actor/tables.lua
local ActionMotionDistanceBounds = R.ActionMotionDistanceBounds   -- from actor/tables.lua
local BehaviorFlag = R.BehaviorFlag   -- from actor/tables.lua
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local ActorFlag = R.ActorFlag   -- from actor/tables.lua
local EntityMotionDistanceBounds = R.EntityMotionDistanceBounds   -- from actor/tables.lua
local EntryAudioEventByKind = R.EntryAudioEventByKind   -- from actor/tables.lua
local idle_pose_selectors = R.idle_pose_selectors   -- from actor/tables.lua
local profiles = R.profiles   -- from actor/tables.lua
local ActorProfileCatalog = require("data.actor_profiles")
local source_close_range_threshold = R.source_close_range_threshold   -- from actor/tables.lua
local target_motion_reacquire_updates = R.target_motion_reacquire_updates   -- from actor/tables.lua

local function direction_from_heading(heading)
    -- ROM 0xB36A rotates the heading three bits, rounds signed values upward,
    -- then masks to one of the eight C0B2 directional animation lists.
    local value = u16((heading << 3) | (heading >> 13))
    if (value & 0x8000) ~= 0 then
        value = u16(value + 1)
    end
    return value & 0x7
end

local function approach_heading(current, target)
    -- ROM 0xB530 moves a 16-bit fixed-point heading in 0x0800-unit steps.
    local delta = u16(current - target)
    if delta == 0 then
        return current
    end
    local signed_delta = s16(delta)
    if signed_delta < 0 then
        if delta < 0xf800 then
            return u16(current + 0x0800)
        end
    elseif delta > 0x07ff then
        return u16(current - 0x0800)
    end
    return target
end

local function dynamic_encounter_profile(context)
    if type(context.profile_id) ~= "string" then
        return nil
    end
    local kind_hex, action_name, action_hex = string.match(
        context.profile_id, "^.+_k([0-9a-f][0-9a-f])_([a-z]+)(%x%x)$")
    local action_kind = ActionKind[action_name]
    if kind_hex == nil or action_kind == nil then
        return nil
    end
    local entity_kind = tonumber(kind_hex, 16)
    local action_code = tonumber(action_hex, 16)
    local minimum_entity_kind = context.world_activity_script_actor == true
        and 3 or 4
    if entity_kind == nil or action_code == nil or
       entity_kind < minimum_entity_kind or entity_kind > 14 or
       entity_kind ~= context.entity_kind or
       action_kind ~= context.action_kind or
       action_code ~= context.action_code then
        return nil
    end
    local source_actor_profile = ActorProfileCatalog.dynamic(
        entity_kind, action_kind, action_code)
    return {
        package_id = "world_actor.kind_" .. tostring(entity_kind) ..
                     ".source_target_motion",
        entity_kind = source_actor_profile.sprite_type,
        action_kind = source_actor_profile.action_kind,
        action_code = source_actor_profile.action_code,
        weapon_item_id = source_actor_profile.weapon,
        controls_movement = source_actor_profile.controls_movement,
        dynamic_encounter_only =
            source_actor_profile.dynamic_encounter_only,
        entry_audio_event = source_actor_profile.entry_sound,
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
        (context.source_record_flags & 0x01) == 0) or
       (not profile.projectile_animation and
        context.entity_kind ~= profile.entity_kind) or
       context.action_kind ~= profile.action_kind or
       context.action_code ~= profile.action_code then
        return nil
    end
    return profile
end

local function target_is_eligible(candidate, world_actor_pool)
    if type(candidate) ~= "table" or type(candidate.id) ~= "string" or
       candidate.id == "" or candidate.has_action ~= true or
       (candidate.service_flags & ActorStatus.ACTIVE) == 0 or
       (candidate.service_flags & ActorStatus.DEAD) ~= 0 or
       (candidate.service_flags & ActorStatus.DOWN) ~= 0 then
        return false
    end
    if candidate.world_activity_actor ~= world_actor_pool then
        return false
    end
    if world_actor_pool and
       ((candidate.behavior_flags & BehaviorFlag.ESCORT) ~= 0 or
        (candidate.spawn_state_flags & ActorFlag.PEDESTRIAN) ~= 0 or
        (candidate.spawn_state_flags & ActorFlag.QUEST) ~= 0) then
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
    local world_actor_pool =
        (context.source_record_flags & ActorStatus.AI_RUNNER) ~= 0
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

local function action_motion_distance_profile(context)
    if context.action_kind == ActionKind.item then
        local item_module = game.modules.items
        local item = type(item_module) == "table" and
            type(item_module.catalog) == "table" and
            item_module.catalog[context.action_code] or nil
        if type(item) == "table" and item.code == context.action_code and
           type(item.action_motion_distance_profile) == "number" then
            return item.action_motion_distance_profile
        end
    elseif context.action_kind == ActionKind.spell then
        local magic_module = game.modules.magic
        if type(magic_module) == "table" and
           type(magic_module.action_motion_distance_profile) == "function" then
            return magic_module.action_motion_distance_profile(
                context.action_code)
        end
    end
    return nil
end

local function source_motion_bounds(context, state)
    if (context.source_record_flags &
        ActorStatus.AI_RUNNER) ~= 0 then
        -- ROM 0x13D84 indexes the nine-pair table at 0x13E58 with the live
        -- actor's +0x4D posture. The host supplies that named typed fact.
        local distance_type = context.source_actor_posture
        if type(distance_type) == "number" and distance_type % 1 == 0 and
           distance_type >= 0 and
           distance_type < #EntityMotionDistanceBounds then
            return EntityMotionDistanceBounds[distance_type + 1], true
        end
        -- Do not guess a destination when the source posture is unavailable.
        return nil, false
    end

    if state.target_motion_delay == 0 then
        return nil, true
    end
    state.target_motion_delay = state.target_motion_delay - 1
    local profile = action_motion_distance_profile(context)
    if type(profile) ~= "number" or profile % 1 ~= 0 or
       profile < 0 or profile >= #ActionMotionDistanceBounds then
        -- The source indexes the selected row while +0xB5 is active. An
        -- unknown or 0xFF row remains unresolved for this route update.
        return nil, false
    end
    return ActionMotionDistanceBounds[profile + 1], true
end

local function source_motion_destination_heading(context, state, target)
    -- ROM 0x13D0C clears +0xB0.4 before selecting the action distance row.
    state.source_behavior_flags = u8(state.source_behavior_flags & ~BehaviorFlag.IN_BAND)
    if type(target) ~= "table" or not context.movement_allowed or
       type(target.heading) ~= "number" or
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
    local heading = target.heading
    local distance = target.distance & 0xffff
    if bounds == nil or distance < bounds.upper then
        -- A zero +0xB5 counter reaches 0x13DCA directly. A selected action
        -- row reaches the same flag when distance is below its upper bound.
        state.source_behavior_flags = state.source_behavior_flags |
            BehaviorFlag.IN_BAND
    end
    context.movement_state.route_flags = state.source_behavior_flags
    local destination_heading = u16(heading + AiTuning.QUARTER_TURN)
    local destination_step_cost = target.perpendicular_route_step_cost
    local destination_distance = target.perpendicular_route_distance & 0xffff

    if bounds ~= nil then
        if distance >= bounds.lower and distance < bounds.upper then
            -- In-band actors drop +0xB5 and choose the lateral destination.
            state.target_motion_delay = 0
        elseif distance >= bounds.upper then
            -- At or beyond the upper bound the ROM copies the linked
            -- position and routes directly toward it.
            destination_heading = heading
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
    return destination_heading
end

local function retained_linked_target(context, state)
    if type(state.linked_target_id) ~= "string" or
       type(context.ai_target_candidates) ~= "table" then
        return nil
    end
    local world_actor_pool =
        (context.source_record_flags &
         ActorStatus.AI_RUNNER) ~= 0
    for _, candidate in ipairs(context.ai_target_candidates) do
        if candidate.id == state.linked_target_id and
           target_is_eligible(candidate, world_actor_pool) then
            return candidate
        end
    end
    return nil
end

local function initial_state(context, profile)
    local state = {
        actor_id = context.actor_id,
        profile_id = context.profile_id,
        entity_kind = context.entity_kind,
        action_kind = context.action_kind,
        action_code = context.action_code,
        source_record_flags = context.source_record_flags or 0,
        source_physical_meter = context.source_physical_meter,
        source_record_slot = context.source_record_slot,
        source_behavior_flags = context.source_behavior_flags or 0,
        -- Lua owns the mutable combat meters used by world-activity policy.
        -- The host supplies this record only at spawn and later combat results
        -- replace it through combat.lua.
        combatant = type(context.combatant) == "table"
            and context.combatant or nil,
        source_action_slot = context.source_action_slot or 0,
        source_action_ammo = context.source_action_ammo or 0,
        source_action_counter = context.source_action_counter or 0,
        source_animation_selector = context.source_animation_selector or
            idle_pose_selectors[
                direction_from_heading(context.initial_heading) + 1],
        package_id = profile.package_id,
        weapon_item_id = profile.weapon_item_id,
        direction = direction_from_heading(context.initial_heading),
        entry_delay = context.entry_delay,
        entry_frame_count = context.entry_animation_frame_count,
        source_interaction_flags = context.source_interaction_flags or 0,
        behavior_timer = 0,
        animation_id = "idle",
        animation_frame = 0,
        animation_accumulator = 0,
        animation_increment = 0,
        animation_timing_word = 0,
        walk_animation_frame_count = context.walk_animation_frame_count or 0,
        action_animation_frame_count =
            context.action_animation_frame_count or 0,
        action_animation_increment =
            context.action_animation_increment or 0,
        action_animation_threshold =
            context.action_animation_threshold or 0,
        action_animation_audio_event =
            context.action_animation_audio_event,
        action_animation_complete = false,
        item_action_frame_count = context.item_action_frame_count or 0,
        item_action_increment = context.item_action_increment or 0,
        item_action_threshold = context.item_action_threshold or 0,
        item_action_complete = false,
        damage_reaction_active = false,
        damage_reaction_animation_complete = false,
        hurt_behavior_timer = 0,
        hurt_audio_event = nil,
        hurt_audio_emitted = false,
        movement_current_heading = profile.controls_movement
            and context.initial_heading or nil,
        movement_route_distance = nil,
        movement_active = false,
        transition_started = false,
        transition_frame_list_finished = false,
        transition_complete = false,
        combat_action_requested = false,
        ai_target_id = nil,
        ai_target_heading = nil,
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
        action_result = type(context.combatant) == "table"
            and context.combatant.action_result or 0,
        spell_action_resolution = nil,
        spell_effect_pending = nil,
        spell_effect_scan_marker = nil,
        spell_effect_completed = false,
        spell_effect_slot_released = nil,
        spell_effect_resolution = nil,
        spell_effect_visible = false,
        spell_effect_phase = 0,
        spell_effect_action_code = 0,
        pending_source_action = nil,
        delay_primary_attribute = context.delay_primary_attribute or 0,
        delay_secondary_attribute = context.delay_secondary_attribute or 0,
        delay_capability_flags = context.delay_capability_flags or 0,
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
        if type(animation) ~= "table" then
            return nil
        end
        state.animation_id = "projectile"
        state.animation_selector = animation.selector
        state.animation_frame = animation.frame
        state.animation_accumulator = animation.accumulator
        state.animation_increment = animation.increment
        state.animation_period = animation.period
        local projectile = context.projectile_movement
        state.action_result = type(projectile) == "table"
            and projectile.action_result or 0
        state.attack_seed_resolved =
            not context.projectile_attack_seed_pending
        state.combat_meter_update = nil
    end
    return state
end

local function same_actor(state, context, profile)
    if state.profile_id == context.profile_id and
       state.entity_kind == context.entity_kind and
       state.action_kind == context.action_kind and
       state.action_code == context.action_code then
        return true
    end
    local pending = state.pending_source_action
    if type(pending) ~= "table" or
       pending.profile_id ~= context.profile_id or
       pending.action_kind ~= context.action_kind or
       pending.action_code ~= context.action_code or
       state.entity_kind ~= context.entity_kind then
        return false
    end
    state.profile_id = context.profile_id
    state.action_kind = context.action_kind
    state.action_code = context.action_code
    state.package_id = profile.package_id
    state.weapon_item_id = profile.weapon_item_id
    state.pending_source_action = nil
    state.animation_id = "idle"
    state.animation_selector = nil
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_increment = 0
    return true
end


-- register this module's definitions
R.direction_from_heading = direction_from_heading
R.approach_heading = approach_heading
R.dynamic_encounter_profile = dynamic_encounter_profile
R.source_profile = source_profile
R.target_is_eligible = target_is_eligible
R.linked_target = linked_target
R.action_motion_distance_profile = action_motion_distance_profile
R.source_motion_bounds = source_motion_bounds
R.source_motion_destination_heading = source_motion_destination_heading
R.retained_linked_target = retained_linked_target
R.initial_state = initial_state
R.same_actor = same_actor
return R
