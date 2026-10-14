-- Lua owns per-actor behavior and presentation state. The host calls a
-- behavior heartbeat during 0xA8EA and a render heartbeat during 0x4618;
-- drawing consumes each returned snapshot without advancing it again.
local ActorAI = {}
local states = game.actors

-- The directional weapon-effect records at ROM 0x630C use this descriptor
-- cadence in their finite 0x8003/0x800B lists.
local weapon_projectile_descriptor_increment = 0x270f
local projectile = {
    role = { primary = "primary", secondary = "secondary" },
    flags = {
        active_record = 0x01,
        stopped_record = 0x08,
        terminal_record = 0x09,
        animation_list_finished = 0x8000,
    },
    outcome = {
        none = "none",
        stopped = "stopped",
        retired = "retired",
        hit_resolution_pending = "hit_resolution_pending",
        linked_followup_pending = "linked_followup_pending",
    },
    movement_probes_per_update = 4,
    impact_sound_event = 0x003c,
}
local byte_mask = 0xff
local signed_byte_threshold = 0x80
local byte_modulus = 0x100
local fixed_long_mask = 0xffffffff
local fixed_integer_shift = 16
local fixed_fraction_scale = 0x100

-- Semantic names for the source-template facts consumed by actor packages.
-- Numeric values remain here as data; package definitions use game concepts.
local EntityKind = {
    field_runner = 3,
    corp_guard = 5,
    pedestrian = 11,
    hell_hound = 12,
    gargoyle = 13,
    wendigo = 14,
}
local ActionKind = {
    none = "none",
    item = "item",
    spell = "spell",
}

local ActorServiceFlag = {
    active = 0x01,
    targets_world_actors = 0x08,
    action_candidate_reserved = 0x10,
    effect_locked = 0x40,
}
local ActorSpawnFlag = {
    transitioning = 0x20,
    effect_route = 0x08,
}
local ActorBehaviorFlag = {
    high_effect = 0x80,
}
-- ROM 0x13E7C has four finite bounds pairs selected by item/spell template
-- byte +0x27. Coordinates are typed 16-bit scene units.
local ActionMotionDistanceBounds = {
    { lower = 0x0500, upper = 0x0640 },
    { lower = 0x05a0, upper = 0x0780 },
    { lower = 0x0640, upper = 0x08c0 },
    { lower = 0x08c0, upper = 0x0c80 },
}
local EntityMotionDistanceBounds = {
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0640, upper = 0x0780 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x0460, upper = 0x05a0 },
    { lower = 0x01e0, upper = 0x03c0 },
    { lower = 0x01e0, upper = 0x03c0 },
    { lower = 0x01e0, upper = 0x03c0 },
}
local target_motion_reacquire_updates = 6
local source_close_range_threshold = 0x01e0
local ItemAction = {
    max_power_heavy_pistol = 6,
}
-- ROM 0xB330 indexes the word table at 0xB05E by entity kind. These are
-- source audio requests; some table-2 slots are confirmed silent/no-op.
local EntryAudioEventByKind = {
    [0] = 0x40,
    [1] = 0x40,
    [2] = 0x40,
    [3] = 0x2e,
    [4] = 0x2f,
    [5] = 0x2e,
    [6] = 0x58,
    [7] = 0x2e,
    [8] = 0x2e,
    [9] = 0x2e,
    [10] = 0x58,
    [11] = 0x2f,
    [12] = 0x56,
    [13] = 0x5a,
    [14] = 0x5a,
    [15] = 0x5a,
}
local ActorEntryCue = {
    corp_guard = EntryAudioEventByKind[EntityKind.corp_guard],
    pedestrian = EntryAudioEventByKind[EntityKind.pedestrian],
}

local profiles = {
    ["corp_guard_k05_item06"] = {
        package_id = "corp_security.max_power_heavy_pistol",
        entity_kind = EntityKind.corp_guard,
        action_kind = ActionKind.item,
        action_code = ItemAction.max_power_heavy_pistol,
        weapon_item_id = ItemAction.max_power_heavy_pistol,
        entry_audio_event = ActorEntryCue.corp_guard,
    },
    ["pedestrian_k0b_none00"] = {
        package_id = "pedestrian.no_active_action",
        entity_kind = EntityKind.pedestrian,
        action_kind = ActionKind.none,
        action_code = 0,
        entry_audio_event = ActorEntryCue.pedestrian,
    },
    ["field_runner_k03_none00"] = {
        package_id = "runner.field_controlled",
        entity_kind = EntityKind.field_runner,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
    },
    ["hell_hound_k0c_none00"] = {
        package_id = "creature.hell_hound.source_target_motion",
        entity_kind = EntityKind.hell_hound,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["gargoyle_k0d_none00"] = {
        package_id = "creature.gargoyle.source_target_motion",
        entity_kind = EntityKind.gargoyle,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["wendigo_k0e_none00"] = {
        package_id = "creature.wendigo.source_target_motion",
        entity_kind = EntityKind.wendigo,
        action_kind = ActionKind.none,
        action_code = 0,
        controls_movement = true,
        dynamic_encounter_only = true,
    },
    ["weapon_projectile"] = {
        package_id = "combat.weapon_projectile",
        action_kind = ActionKind.none,
        action_code = 0,
        projectile_animation = true,
    },
}

local function direction_from_heading(heading)
    -- ROM 0xB36A rotates the heading three bits, rounds signed values upward,
    -- then masks to one of the eight C0B2 directional animation lists.
    local value = ((heading << 3) | (heading >> 13)) & 0xffff
    if (value & 0x8000) ~= 0 then
        value = (value + 1) & 0xffff
    end
    return value & 0x7
end

local function approach_heading(current, target)
    -- ROM 0xB530 moves a 16-bit fixed-point heading in 0x0800-unit steps.
    local delta = (current - target) & 0xffff
    if delta == 0 then
        return current
    end
    local signed_delta = delta
    if (delta & 0x8000) ~= 0 then
        signed_delta = delta - 0x10000
    end
    if signed_delta < 0 then
        if delta < 0xf800 then
            return (current + 0x0800) & 0xffff
        end
    elseif delta > 0x07ff then
        return (current - 0x0800) & 0xffff
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
    if entity_kind == nil or action_code == nil or
       entity_kind < 4 or entity_kind > 14 or
       entity_kind ~= context.entity_kind or
       action_kind ~= context.action_kind or
       action_code ~= context.action_code then
        return nil
    end
    return {
        package_id = "world_actor.kind_" .. tostring(entity_kind) ..
                     ".source_target_motion",
        entity_kind = entity_kind,
        action_kind = action_kind,
        action_code = action_code,
        weapon_item_id = action_kind == ActionKind.item
            and action_code or nil,
        controls_movement = true,
        dynamic_encounter_only = true,
        entry_audio_event = EntryAudioEventByKind[entity_kind],
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
       (candidate.service_flags & ActorServiceFlag.active) == 0 or
       (candidate.service_flags & ActorServiceFlag.effect_locked) ~= 0 or
       (candidate.service_flags & ActorServiceFlag.action_candidate_reserved) ~= 0 then
        return false
    end
    if candidate.world_activity_actor ~= world_actor_pool then
        return false
    end
    if world_actor_pool and
       ((candidate.behavior_flags & ActorBehaviorFlag.high_effect) ~= 0 or
        (candidate.spawn_state_flags & ActorSpawnFlag.transitioning) ~= 0 or
        (candidate.spawn_state_flags & ActorSpawnFlag.effect_route) ~= 0) then
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
        (context.source_record_flags & ActorServiceFlag.targets_world_actors) ~= 0
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
        ActorServiceFlag.targets_world_actors) ~= 0 then
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
    local destination_heading = (heading + 0x4000) & 0xffff
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
         ActorServiceFlag.targets_world_actors) ~= 0
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

local function same_actor(state, context)
    return state.profile_id == context.profile_id and
           state.entity_kind == context.entity_kind and
           state.action_kind == context.action_kind and
           state.action_code == context.action_code
end

local function snapshot(state, action, audio_event, projectile_movement,
                        target_heading_audio_event)
    if state.animation_id == "idle" then
        state.animation_frame = 0
    end
    local combat_meter_update = state.combat_meter_update
    state.combat_meter_update = nil
    return {
        actor_id = state.actor_id,
        profile_id = state.profile_id,
        package_id = state.package_id,
        action_kind = state.action_kind,
        action_code = state.action_code,
        action = action,
        animation = state.animation_id,
        source_record_flags = state.source_record_flags,
        animation_selector = state.animation_selector,
        direction = state.direction,
        frame = state.animation_frame,
        animation_accumulator = state.animation_accumulator,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        combat_action_requested = state.combat_action_requested,
        audio_event = audio_event,
        target_heading_audio_event = target_heading_audio_event,
        weapon_item_id = state.weapon_item_id,
        animation_increment = state.animation_increment,
        movement_heading = state.movement_heading,
        movement_current_heading = state.movement_current_heading,
        movement_route_distance = state.movement_route_distance,
        ai_target_id = state.ai_target_id,
        ai_target_heading = state.ai_target_heading,
        ai_target_distance = state.ai_target_distance,
        ai_target_engaged = state.ai_target_engaged,
        target_motion_delay = state.target_motion_delay,
        action_delay = state.action_delay,
        target_motion_effect_slot_request =
            state.target_motion_effect_slot_request,
        target_motion_effect_visible =
            state.target_motion_effect_visible,
        target_motion_effect_direction =
            state.target_motion_effect_direction,
        target_motion_effect_phase = state.target_motion_effect_phase,
        target_motion_effect_slot_released =
            state.target_motion_effect_slot_released,
        target_marker_visible = state.target_marker_visible,
        target_marker_animating = state.target_marker_animating,
        target_marker_style = state.target_marker_style,
        target_marker_radius = state.target_marker_radius,
        target_selection_processed = state.target_selection_processed,
        selected_world_actor_id = state.target_selection_processed
            and state.selected_world_actor_id or nil,
        combat_meter_update = combat_meter_update,
        source_action_dispatch = state.source_action_dispatch,
        movement_step = state.movement_step,
        projectile_movement = projectile_movement,
    }
end

local function source_action_delay(context, state, mode)
    -- ROM 0xB11C clears D0 before 0xABFC. 0xD7EA adds the first enabled
    -- +0x90 capability modifier to the wrapped average of +0x78/+0x7B.
    local capability_flags = context.delay_capability_flags or
        state.delay_capability_flags or 0
    local modifier = 0
    for bit = 12, 14 do
        if (capability_flags & (1 << bit)) ~= 0 then
            modifier = (bit - 11) * 4
            break
        end
    end
    local primary = context.delay_primary_attribute or
        state.delay_primary_attribute or 0
    local secondary = context.delay_secondary_attribute or
        state.delay_secondary_attribute or 0
    local wrapped_sum = (primary + secondary) & 0xff
    local factor = (wrapped_sum >> 1) + modifier - 1
    if factor < 0 then
        factor = 0
    end
    local delay = 0x6a - factor * 4
    -- ROM 0xABFC halves D2 with ASR when D0 is 1 (the B1DC target-action
    -- transition). Keep the ordinary full reset used by B11C as mode 0.
    if mode == 1 then
        delay = math.floor(delay / 2)
    elseif mode == 2 then
        delay = delay * 2
    end
    if delay < 0x0f then
        delay = 0x0f
    end
    return delay
end

local function advance_source_action_delay(state, context)
    if context.phase ~= "behavior" or
       not context.dynamic_world_encounter_actor then
        return
    end
    -- ROM 0xA8EA decrements active actor +0xDC once before the actor-service
    -- continuation, while the value is nonnegative.
    if state.action_delay >= 0 then
        state.action_delay = state.action_delay - 1
    end
end

local function advance_target_motion_effect(state, context)
    state.target_motion_effect_slot_request = nil
    state.target_motion_effect_slot_released = false
    state.target_motion_effect_visible = false
    if context.phase ~= "render" or
       not state.target_motion_effect_active then
        return
    end

    -- ROM 0x4E86 decrements the signed byte at +0x02, reloads 6 on
    -- underflow, and advances +0x03. Phases 1..3 draw; phase 4 clears the
    -- shared slot and +0x11.6 without emitting a sprite.
    state.target_motion_effect_countdown =
        state.target_motion_effect_countdown - 1
    if state.target_motion_effect_countdown < 0 then
        state.target_motion_effect_countdown = 6
        state.target_motion_effect_phase =
            state.target_motion_effect_phase + 1
        if state.target_motion_effect_phase >= 4 then
            state.target_motion_effect_active = false
            state.target_motion_effect_slot = nil
            state.target_motion_effect_slot_released = true
            return
        end
    end
    state.target_motion_effect_visible = true
end

local function request_target_motion_effect(state, context, heading)
    for slot = 0, 17 do
        if context.world_effect_slot_occupancy[slot + 1] ~= true then
            state.target_motion_effect_active = true
            state.target_motion_effect_slot = slot
            state.target_motion_effect_slot_request = slot
            state.target_motion_effect_countdown = 0
            state.target_motion_effect_phase = 0
            state.target_motion_effect_direction =
                direction_from_heading(heading)
            -- 0x14C7C sets +0x11.6; 0x13FFE then enters 0xB11C, which
            -- resets +0xDC with the actor's source-derived delay attributes.
            state.action_delay = source_action_delay(context, state)
            return true
        end
    end
    return false
end

local function advance_target_marker(state, context)
    if context.phase ~= "render" then
        return
    end
    if context.target_selection_latched ~= true then
        state.target_selection_latched = false
        state.target_marker_visible = false
        state.target_marker_animating = false
        state.target_marker_style = 0
        state.target_marker_radius = 0
        state.target_marker_countdown = 0
        return
    end

    -- ROM 0x5C7E chooses a tile/palette group from the actor's +0x4C action
    -- byte. The alternate +0x9C path depends on mode/runner flags which are
    -- not present on these world-encounter records yet.
    local action_code = context.action_code or 0
    if action_code >= 9 then
        state.target_marker_style = 3
    elseif action_code >= 6 then
        state.target_marker_style = 2
    elseif action_code >= 3 then
        state.target_marker_style = 1
    else
        state.target_marker_style = 0
    end

    if not state.target_selection_latched then
        -- ROM 0x1DC4C seeds +0xA6 with -0x7FF7. The render pass then calls
        -- 0x5C7E, which decrements that signed word before placing its four
        -- corner sprites at an 8-pixel-per-step shrinking radius.
        state.target_marker_countdown = -0x7ff7
    end
    state.target_selection_latched = true
    state.target_marker_visible = true

    local countdown = state.target_marker_countdown or 0
    if countdown < 0 then
        local next_word = (countdown - 1) & 0xffff
        local next_countdown = next_word >= 0x8000
            and next_word - 0x10000 or next_word
        if next_countdown < 0 then
            state.target_marker_countdown = next_countdown
            state.target_marker_animating = true
            state.target_marker_radius = (next_word & 0x7f) * 8
        else
            state.target_marker_countdown = 0
            state.target_marker_animating = false
            state.target_marker_radius = 0
        end
    else
        state.target_marker_animating = false
        state.target_marker_radius = 0
    end
end

local function advance_world_target_selection(state, context)
    state.target_selection_processed = false
    if context.phase ~= "behavior" or
       context.target_selection_requested ~= true then
        return
    end

    local candidates_by_slot = {}
    for _, candidate in ipairs(context.world_target_selection_candidates or {}) do
        local slot = candidate.source_record_slot
        if type(slot) == "number" and slot % 1 == 0 and
           slot >= 0 and slot < 16 then
            candidates_by_slot[slot] = candidate
        end
    end

    local special_path = context.target_selection_special_marker == true and
        (context.target_selection_mode == 0x0c or
         context.target_selection_mode == 0x04 or
         context.target_selection_mode == 0x0b)
    state.selected_world_actor_id = nil
    local start
    if special_path then
        start = context.target_selection_current_slot
        if start == nil then
            start = context.target_selection_owner_slot
        end
        if start ~= nil and context.target_selection_advances_cursor then
            start = start + 1
        end
        if start ~= nil and
           (start >= 16 or
            ((candidates_by_slot[start] or {}).descriptor_class or 0) >= 3) then
            start = 0
        end
    else
        start = 3
        local previous = context.target_selection_current_slot
        if previous ~= nil then
            if previous < 9 then
                start = previous
                if context.target_selection_advances_cursor then
                    start = start + 1
                end
            end
        elseif context.target_selection_linked_slot ~= nil then
            start = context.target_selection_linked_slot
        end
        if start >= 16 then
            start = 3
        end
        if ((candidates_by_slot[start] or {}).descriptor_class or 0) < 3 then
            start = 3
        end
    end

    if start ~= nil then
        for offset = 0, 7 do
            local candidate = candidates_by_slot[start + offset]
            if candidate ~= nil then
                local available =
                    (candidate.service_flags & ActorServiceFlag.active) ~= 0
                if special_path then
                    if available and type(candidate.id) == "string" and
                       candidate.id ~= "" then
                        state.selected_world_actor_id = candidate.id
                        break
                    end
                else
                    available = available and
                        (candidate.service_flags &
                         ActorServiceFlag.effect_locked) == 0 and
                        (candidate.service_flags &
                         ActorServiceFlag.action_candidate_reserved) == 0 and
                        (candidate.behavior_flags &
                         ActorBehaviorFlag.high_effect) == 0 and
                        candidate.has_action == true
                    if available and type(candidate.id) == "string" and
                       candidate.id ~= "" then
                        state.selected_world_actor_id = candidate.id
                        break
                    end
                end
            end
        end
    end

    state.target_selection_processed = true
end

local function advance_projectile_animation(state, context)
    local animation = context.projectile_animation
    if context.phase ~= "render" or type(animation) ~= "table" or
       type(animation.selector) ~= "number" or
       type(animation.frame_count) ~= "number" or
       animation.frame_count < 1 or animation.frame_count > 16 or
       type(animation.finite) ~= "boolean" or not animation.finite or
       type(animation.period) ~= "number" then
        return false
    end

    -- The behavior pass may select a new list (for example, the stopped
    -- projectile tail). Preserve +0x1A/+0x1E as 0x600E does: only the source
    -- selector changes before this render heartbeat.
    state.animation_selector = animation.selector
    state.animation_period = animation.period
    state.animation_frame_count = animation.frame_count

    -- ROM 0x5FDC adds +0x18 to +0x1A as a signed 16-bit word, compares it
    -- with +0x1C, and advances +0x1E once when the period is reached.
    local accumulated_word =
        (state.animation_accumulator + state.animation_increment) & 0xffff
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if state.animation_period <= accumulated_signed then
        state.animation_frame = (state.animation_frame + 1) & 0xffff
        state.animation_accumulator =
            (accumulated_word - state.animation_period) & 0xffff
        if state.animation_accumulator >= 0x8000 then
            state.animation_accumulator = state.animation_accumulator - 0x10000
        end
    else
        state.animation_accumulator = accumulated_signed
    end

    -- ROM 0x600E resolves the finite source list after 0x5FDC. The sixteen
    -- weapon-effect lists use 0x8003 or 0x800B markers and 0x270F cadence.
    if animation.frame_count <= state.animation_frame then
        state.animation_selector = state.animation_selector | 0x8000
        state.animation_frame = animation.frame_count - 1
    end
    state.animation_increment = weapon_projectile_descriptor_increment
    state.animation_id = "projectile"
    return true
end

local function signed_byte(value)
    local byte = value & byte_mask
    if byte >= signed_byte_threshold then
        return byte - byte_modulus
    end
    return byte
end

local function signed_word_from_long(value)
    local word = value & fixed_long_mask
    if word >= 0x80000000 then
        return word - 0x100000000
    end
    return word
end

local function integer_world_word(value)
    return ((value & fixed_long_mask) >> fixed_integer_shift) & 0xffff
end

local function projectile_movement_snapshot(context, state)
    local source = context.projectile_movement
    if type(source) ~= "table" or
       (source.role ~= projectile.role.primary and
        source.role ~= projectile.role.secondary) or
       type(source.active) ~= "boolean" or
       type(source.flags) ~= "number" or source.flags < 0 or
       source.flags > 0xff or type(source.record_flags) ~= "number" or
       source.record_flags < 0 or source.record_flags > 0xff or
       type(source.world_x) ~= "number" or
       type(source.world_y) ~= "number" or
       type(source.tile_offset_x) ~= "number" or
       type(source.tile_offset_y) ~= "number" or
       type(source.retry_counter) ~= "number" or
       source.retry_counter < 0 or source.retry_counter > 0xff or
       type(source.linked_action_timer) ~= "number" or
       source.linked_action_timer < -128 or
       source.linked_action_timer > 127 or
       type(source.animation_selector) ~= "number" or
       source.animation_selector < 0 or source.animation_selector > 0xffff then
        return nil
    end

    local result = {
        role = source.role,
        active = source.active,
        flags = source.flags,
        record_flags = source.record_flags,
        world_x = source.world_x,
        world_y = source.world_y,
        tile_offset_x = source.tile_offset_x,
        tile_offset_y = source.tile_offset_y,
        retry_counter = source.retry_counter,
        linked_action_timer = source.linked_action_timer,
        action_result = state.action_result or source.action_result,
        animation_selector = source.animation_selector,
        outcome = projectile.outcome.none,
    }
    local audio_event

    if not result.active then
        return result, nil
    end
    if result.role == projectile.role.secondary then
        -- ROM 0xAC4E retires the secondary record after the preceding render
        -- heartbeat marks its finite animation list complete.
        if (result.animation_selector &
            projectile.flags.animation_list_finished) ~= 0 then
            result.flags = result.flags & ~projectile.flags.active_record
            result.active = false
            result.outcome = projectile.outcome.retired
        end
        return result, nil
    end

    if (result.record_flags & projectile.flags.stopped_record) ~= 0 then
        -- ROM 0x14E0A/0x14F1C wait for the stopped record's finite list.
        -- The linked action pair in 0x14F86 is still an explicit boundary.
        if (result.animation_selector &
            projectile.flags.animation_list_finished) ~= 0 then
            result.linked_action_timer =
                signed_byte(result.linked_action_timer - 1)
            if result.linked_action_timer < 0 then
                result.flags = result.flags & ~projectile.flags.active_record
                result.active = false
                result.outcome = projectile.outcome.retired
            else
                result.outcome = projectile.outcome.linked_followup_pending
            end
        end
        return result, nil
    end

    local outside_bounds_marker = false
    for _ = 1, projectile.movement_probes_per_update do
        -- ROM 0x14F26 adds the signed +0x4C/+0x4E tile offsets as 8.8
        -- fixed-point deltas before sampling terrain.
        local probe_x = signed_word_from_long(
            result.world_x + result.tile_offset_x * fixed_fraction_scale)
        local probe_y = signed_word_from_long(
            result.world_y + result.tile_offset_y * fixed_fraction_scale)
        local terrain_result, boundary_active, outside_marker =
            game.movement.projectile_terrain(
                integer_world_word(probe_x), integer_world_word(probe_y))
        if type(terrain_result) ~= "number" or
           type(boundary_active) ~= "boolean" or
           type(outside_marker) ~= "boolean" then
            return nil
        end
        outside_bounds_marker = outside_marker

        local terminal = terrain_result ~= 0 and not boundary_active
        if not terminal then
            result.retry_counter =
                (result.retry_counter - 1) & byte_mask
            terminal = signed_byte(result.retry_counter) < 0
        end
        if terminal then
            result.record_flags = result.record_flags |
                                  projectile.flags.terminal_record
            result.animation_selector = 0
            result.tile_offset_x = 0
            result.tile_offset_y = 0
            if outside_bounds_marker then
                result.flags = result.flags & ~projectile.flags.active_record
                result.active = false
                result.outcome = projectile.outcome.retired
            else
                result.outcome = projectile.outcome.stopped
            end
            -- ROM 0x14E32 requests event 0x3C when D1 is zero, including
            -- the outside-bounds branch.
            audio_event = projectile.impact_sound_event
            break
        end

        result.world_x = probe_x
        result.world_y = probe_y
        local source_carry, source_position_updated, source_x, source_y,
              hit_target_id =
            game.movement.projectile_collision(
                integer_world_word(probe_x), integer_world_word(probe_y),
                result.flags)
        if type(source_carry) ~= "boolean" or
           type(source_position_updated) ~= "boolean" or
           type(source_x) ~= "number" or type(source_y) ~= "number" then
            return nil
        end
        if hit_target_id ~= nil and
           (type(hit_target_id) ~= "string" or hit_target_id == "") then
            return nil
        end
        if source_carry then
            result.record_flags = result.record_flags |
                                  projectile.flags.terminal_record
            result.animation_selector = 0
            result.tile_offset_x = 0
            result.tile_offset_y = 0
            if outside_bounds_marker then
                result.flags = result.flags & ~projectile.flags.active_record
                result.active = false
                result.outcome = projectile.outcome.retired
            end
            if source_position_updated then
                -- 0x15ABA copies the target's integer coordinates while
                -- preserving the projectile record's fractional words.
                result.world_x = signed_word_from_long(
                    (source_x << fixed_integer_shift) |
                    (probe_x & 0xffff))
                result.world_y = signed_word_from_long(
                    (source_y << fixed_integer_shift) |
                    (probe_y & 0xffff))
                result.outcome =
                    projectile.outcome.hit_resolution_pending
                result.hit_target_id = hit_target_id
                audio_event = projectile.impact_sound_event
            else
                result.outcome =
                    projectile.outcome.linked_followup_pending
            end
            break
        end
    end
    return result, audio_event
end

local function projectile_heartbeat(state, context)
    if context.phase == "render" then
        if not advance_projectile_animation(state, context) then
            return nil
        end
        return snapshot(state, "hold", nil)
    end
    if context.phase ~= "behavior" then
        return nil
    end
    if context.projectile_attack_seed_pending then
        if type(context.projectile_movement) ~= "table" or
           context.projectile_movement.role ~= projectile.role.primary then
            return nil
        end
        local result = game.modules.combat.resolve_weapon_attack_seed(context)
        state.attack_seed_resolved = type(result) == "number"
        if state.attack_seed_resolved then
            state.action_result = result
        end
    end
    local movement, audio_event = projectile_movement_snapshot(context, state)
    if movement == nil then
        return nil
    end
    if movement.outcome == projectile.outcome.hit_resolution_pending then
        local hit_target
        for _, candidate in ipairs(context.projectile_combat_targets or {}) do
            if candidate.id == movement.hit_target_id then
                hit_target = candidate
                break
            end
        end
        if hit_target ~= nil and state.attack_seed_resolved then
            state.combat_meter_update = game.modules.combat.resolve_weapon_hit(
                context, { action_result = state.action_result }, hit_target)
        end
    end
    state.animation_selector = movement.animation_selector
    return snapshot(state, "hold", audio_event, movement)
end

local function advance_entry_animation(state)
    if state.animation_id ~= "entry" or
       state.transition_frame_list_finished then
        return
    end
    local frame_count = state.entry_frame_count
    if type(frame_count) ~= "number" or frame_count < 1 or
       frame_count > 8 then
        return
    end
    local accumulated = state.animation_accumulator + 0x0200
    if accumulated >= 0x1000 then
        state.animation_accumulator = accumulated - 0x1000
        if state.animation_frame + 1 < frame_count then
            state.animation_frame = state.animation_frame + 1
        else
            state.transition_frame_list_finished = true
        end
    else
        state.animation_accumulator = accumulated
    end
end

local function advance_finite_actor_animation(state, frame_count,
                                              increment, threshold)
    if state.damage_reaction_animation_complete then
        return true
    end
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 8 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold < 1 or threshold > 0xffff then
        return false
    end
    local accumulated_word =
        (state.animation_accumulator + increment) & 0xffff
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if threshold <= accumulated_signed then
        state.animation_frame = state.animation_frame + 1
        state.animation_accumulator = accumulated_word - threshold
        if state.animation_accumulator >= 0x8000 then
            state.animation_accumulator = state.animation_accumulator - 0x10000
        end
        if state.animation_frame >= frame_count then
            -- 0x600E sets the list-finished selector and holds its last frame.
            state.animation_frame = frame_count - 1
            state.damage_reaction_animation_complete = true
        end
    else
        state.animation_accumulator = accumulated_signed
    end
    return true
end

local function begin_received_hit_reaction(state, context)
    local hit = state.incoming_hit
    if type(hit) ~= "table" or
       type(hit.physical_meter) ~= "number" or
       type(hit.mental_meter) ~= "number" then
        return nil
    end
    state.incoming_hit = nil
    if (state.source_record_flags & ActorServiceFlag.action_candidate_reserved) ~= 0 then
        return nil
    end
    if hit.physical_meter == 0 then
        if type(context.entry_animation_frame_count) ~= "number" or
           context.entry_animation_frame_count < 1 or
           context.entry_animation_frame_count > 8 or
           type(context.down_audio_event) ~= "number" then
            return nil
        end
        -- 0xAFF2 enters 0xB330 at zero physical meter, requests B05E[kind],
        -- reserves +0x10.4, and selects the directional C0B2 transition.
        state.damage_reaction_active = true
        state.damage_reaction_animation_complete = false
        state.animation_frame = 0
        state.animation_accumulator = 0
        state.animation_id = "down"
        state.animation_selector = 0xe8 + state.direction * 4
        state.source_record_flags =
            state.source_record_flags |
            ActorServiceFlag.action_candidate_reserved
        state.hurt_behavior_timer = 0
        return context.down_audio_event
    end
    local frame_count = context.hurt_animation_frame_count
    local increment = context.hurt_animation_increment
    local threshold = context.hurt_animation_threshold
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 8 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold < 1 or threshold > 0xffff or
       type(context.hurt_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_accumulator = 0
    -- 0xAFF2 selects C072 and starts B07E's eight-service countdown.
    state.animation_id = "hurt"
    state.animation_selector = 0x60 + state.direction * 4
    state.animation_increment = increment
    state.animation_period = threshold
    state.hurt_frame_count = frame_count
    state.hurt_behavior_timer = 8
    state.hurt_audio_event = context.hurt_audio_event
    state.hurt_audio_emitted = false
    return nil
end

local function advance_received_hit_reaction(state, context)
    local audio_event
    if context.phase == "behavior" then
        if state.hurt_behavior_timer > 0 then
            state.hurt_behavior_timer = state.hurt_behavior_timer - 1
            if state.hurt_behavior_timer == 0 and
               not state.hurt_audio_emitted then
                state.hurt_audio_emitted = true
                audio_event = state.hurt_audio_event
            end
        end
    end
    return audio_event
end

local function advance_dynamic_actor_walk_animation(state, context)
    local frame_count = context.walk_animation_frame_count
    if type(frame_count) ~= "number" or frame_count == 0 then
        frame_count = state.walk_animation_frame_count
    end
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 16 then
        return false
    end

    -- ROM 0x600E uses a special kind-0xD cadence. Other kinds derive +0x18
    -- from the selected movement vector's +0xCA timing word; 0x5FDC then advances +0x1E on
    -- the signed 0x1000-period comparison. The positive C062 list wraps at
    -- its source marker after the render heartbeat.
    local increment
    if state.entity_kind == EntityKind.gargoyle then
        increment = state.animation_frame == 2 and 0x0090 or 0x0180
    else
        local timing_word = state.animation_timing_word
        if type(timing_word) ~= "number" or timing_word < 0 or
           timing_word > 0xffff then
            return false
        end
        increment = timing_word < 0x16 and timing_word * 0x1c
            or timing_word * 0x10
    end
    increment = increment & 0xffff
    state.animation_increment = increment
    state.animation_period = 0x1000
    state.animation_selector = 0x20 + state.direction * 4

    local accumulated_word =
        (state.animation_accumulator + increment) & 0xffff
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if state.animation_period <= accumulated_signed then
        state.animation_frame = (state.animation_frame + 1) & 0xffff
        state.animation_accumulator =
            (accumulated_word - state.animation_period) & 0xffff
        if state.animation_accumulator >= 0x8000 then
            state.animation_accumulator = state.animation_accumulator - 0x10000
        end
    else
        state.animation_accumulator = accumulated_signed
    end
    if frame_count <= state.animation_frame then
        state.animation_frame = 0
    end
    return true
end

local function advance_dynamic_actor_action_animation(state)
    if state.animation_id ~= "action" or state.action_animation_complete then
        return true
    end
    local frame_count = state.action_animation_frame_count
    local increment = state.action_animation_increment
    local threshold = state.action_animation_threshold
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 8 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold < 1 or threshold > 0xffff then
        return false
    end

    -- ROM 0x5FDC adds the C0A2 list's +2 increment to +0x1A and advances
    -- +0x1E when the signed accumulator reaches the actor's +0x1C threshold.
    local accumulated_word =
        (state.animation_accumulator + increment) & 0xffff
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if threshold <= accumulated_signed then
        state.animation_frame = state.animation_frame + 1
        state.animation_accumulator = accumulated_word - threshold
        if state.animation_accumulator >= 0x8000 then
            state.animation_accumulator = state.animation_accumulator - 0x10000
        end
        if state.animation_frame >= frame_count then
            -- 0x600E sets the finite-list completion marker and holds the
            -- last C0A2 frame until another behavior selects a new list.
            state.animation_frame = frame_count - 1
            state.action_animation_complete = true
        end
    else
        state.animation_accumulator = accumulated_signed
    end
    return true
end

-- ROM 0xC082. AF7C/AFAE index this by the same 0xB36A direction as C0A2.
local spell_pose_selectors = {
    0x80, 0x84, 0x88, 0x8C, 0x90, 0x94, 0x98, 0x9C,
}

local function action_line_clear(context, target)
    local line_clear = context.action_line_clear
    return type(line_clear) == "function" and
        line_clear(target.world_x, target.world_y) == true
end

local function classify_spell_dispatch(state, context, target)
    -- 0x13FFE reaches the spell branch only with a linked target. Delay and
    -- the projected +0x0C word are tested before the code-4/11 RTS.
    if type(target) ~= "table" then
        return nil
    end
    if state.action_delay >= 0 then
        return "waiting"
    end
    if context.source_projection_active ~= true then
        return "offscreen"
    end
    if context.action_code == 4 or context.action_code == 11 then
        return "suppressed"
    end
    if not action_line_clear(context, target) then
        return "blocked"
    end
    return "spell"
end

local function classify_item_dispatch(state, context, target)
    -- 0x13FFE tests delay and +0x0C before the nonnegative +0x56 branch.
    -- Codes 13..15 reach 0x15646. Other nonzero codes reach the 0x1411E
    -- gate, which needs FFE186 or +0xB0 bit 4. Those facts are not hosted,
    -- so a clear probe is not a spawn and not a C042/C092 pose.
    if type(target) ~= "table" then
        return nil
    end
    if state.action_delay >= 0 then
        return "waiting"
    end
    if context.source_projection_active ~= true then
        return "offscreen"
    end
    if not action_line_clear(context, target) then
        return "blocked"
    end
    local code = context.action_code
    if code >= 13 and code <= 15 then
        return "item_effect"
    end
    return "item_gated"
end

local function begin_spell_cast(state, context, target)
    -- 0xAE6E aims +0x2E at the link, then AF7C/AFAE install C082 and reload
    -- +0xDC with mode 0. 0x1033E runs before that pose. This slice records
    -- the entry; it does not roll dice or spawn the 0x15830 effect.
    local heading = target.heading & 0xffff
    local direction = direction_from_heading(heading)
    state.direction = direction
    state.animation_id = "spell"
    state.animation_selector = spell_pose_selectors[direction + 1]
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_increment = 0
    state.action_animation_complete = false
    state.action_delay = source_action_delay(context, state, 0)
    state.ai_target_engaged = false
    state.movement_heading = nil
    state.movement_active = false
    state.movement_route_distance = nil
    state.movement_step = nil
    state.combat_meter_update = nil
    state.combat_action_requested = false
end

local function begin_dynamic_target_action(state, context, target)
    if not context.dynamic_world_encounter_actor or
       state.action_delay >= 0 or state.target_motion_effect_active or
       type(context.action_animation_frame_count) ~= "number" or
       context.action_animation_frame_count < 1 or
       type(context.action_animation_increment) ~= "number" or
       context.action_animation_increment < 1 or
       type(context.action_animation_threshold) ~= "number" or
       context.action_animation_threshold < 1 or
       type(context.action_animation_audio_event) ~= "number" then
        return nil
    end

    -- ROM 0xB1DC selects C0A2 for the accepted target heading, requests the
    -- kind-specific action cue, resets +0x1A/+0x1E, and applies delay mode 1.
    state.animation_id = "action"
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_increment = context.action_animation_increment
    state.animation_period = context.action_animation_threshold
    state.action_animation_complete = false
    state.action_delay = source_action_delay(context, state, 1)
    -- ROM 0xB284 invokes the shared 0x55C76 reaction only after the linked
    -- heading is accepted. Lua resolves the source-derived action now and
    -- returns a one-heartbeat meter update for the host to apply.
    state.combat_meter_update = game.modules.combat.resolve_reaction(
        context, target)
    local heading_event = context.target_heading_audio_alternate
        and 0x4E or 0x4D
    return context.action_animation_audio_event, heading_event
end

local function advance_player_animation(state, context)
    if type(context.animation_accumulator_override) == "number" then
        state.animation_accumulator =
            context.animation_accumulator_override
    end
    if type(context.animation_direction_override) == "number" then
        state.direction = context.animation_direction_override & 0x7
    else
        state.direction = direction_from_heading(context.initial_heading)
    end

    if context.animation_mode == "attack" then
        state.animation_id = "attack"
        state.animation_selector = nil
        state.animation_frame = 0
        state.animation_accumulator = 0
        return
    end

    local accumulated = state.animation_accumulator + 0x0280
    local frame_advanced = accumulated >= 0x1000
    if frame_advanced then
        accumulated = accumulated - 0x1000
    end
    state.animation_accumulator = accumulated
    if context.animation_mode == "walking" then
        state.animation_id = "walk"
        state.animation_selector = 0x20 + state.direction * 4
        if frame_advanced then
            state.animation_frame = (state.animation_frame + 1) % 4
        end
    else
        state.animation_id = "idle"
        state.animation_selector = state.direction * 4
        state.animation_frame = 0
    end
end

local function requested_movement_heading(context, profile, state)
    if profile.dynamic_encounter_only then
        local target = linked_target(context, state)
        local heading = target and target.heading or nil
        local distance = target and target.distance or nil
        -- Keep the selected source actor relation explicit in Lua state.
        -- The host may render and inspect this snapshot but does not choose
        -- the target or apply the creature's movement policy.
        state.ai_target_id = target and target.id or nil
        state.ai_target_heading = heading
        state.ai_target_distance = distance
        -- ROM 0x13D0C selects a destination on the +0xB6 timer. Its route is
        -- then followed by the actor's +0x3A continuation each service pass.
        return nil
    end
    if not profile.controls_movement or not context.movement_allowed then
        return nil
    end
    if context.movement_heading_override ~= nil then
        return context.movement_heading_override
    end
    if type(context.controller_nibble) ~= "number" or
       context.controller_nibble < 0 or context.controller_nibble > 15 or
       type(context.controller_heading_table) ~= "table" then
        return nil
    end
    local heading = context.controller_heading_table[
        context.controller_nibble + 1]
    if type(heading) ~= "number" or heading < 0 or heading >= 0xffff then
        return nil
    end
    return heading
end

local function resolve_source_target_motion(context, state, target)
    state.ai_target_engaged = false
    if not context.movement_allowed or type(target) ~= "table" or
       type(target.heading) ~= "number" or
       type(target.distance) ~= "number" then
        return false
    end

    local heading = target.heading & 0xffff
    local distance = target.distance & 0xffff
    local accepted = false

    if context.entity_kind == EntityKind.hell_hound then
        -- ROM 0x14D04: inside 0x300, nonnegative +0xDC accepts the target
        -- heading; negative +0xDC first needs a free effect slot and then
        -- replaces both +0x2E and +0x34. From 0x300 through 0x5FF both
        -- headings change immediately. At 0x600 the actor keeps pursuing.
        if distance < 0x300 then
            if state.action_delay < 0 then
                accepted = request_target_motion_effect(
                    state, context, heading)
                if accepted then
                    state.movement_current_heading = heading
                end
            else
                accepted = true
            end
        elseif distance < 0x600 then
            state.movement_current_heading = heading
            accepted = true
        end
    elseif context.entity_kind == EntityKind.gargoyle or
           context.entity_kind == EntityKind.wendigo then
        -- 0x14C7C calls 0x15C84, whose 0x15C58 range test measures the
        -- linked record's actual coordinates and accepts only distance <
        -- 0x200. The preceding 0x14C56 projection result is not consumed by
        -- that test. Kind 0xD at frame +0x1E == 2 forces +0x1A to 0x1000,
        -- but clears carry, so the actor continues its route on that pass.
        if distance < 0x200 then
            if context.entity_kind == EntityKind.gargoyle and
               context.ai_animation_frame == 2 then
                state.animation_accumulator = 0x1000
            else
                state.movement_current_heading = heading
                accepted = true
            end
        end
    elseif distance < 0x200 then
        -- Other kinds use 0x15C58 directly. That branch writes +0x34 but
        -- leaves +0x2E to the actor's own action continuation.
        accepted = true
    end

    if accepted then
        state.movement_heading = heading
        state.movement_active = false
        state.movement_route_distance = nil
        state.ai_target_engaged = true
        state.direction = direction_from_heading(heading)
    end
    return accepted
end

local function movement_collision_code(code, context)
    if code >= 0 then
        return code
    end
    local low_bits = code & 0x1f
    if context.movement_transition_active or low_bits >= 0x18 or
       ((context.movement_scene_id == 0x14 or
         context.movement_collision_mode == 2) and
        low_bits >= 2 and low_bits <= 5) then
        return 0x7f
    end
    return code
end

local function creature_movement_offsets(context, state)
    if context.dynamic_world_encounter_actor ~= true then
        return context.movement_offsets
    end

    local step
    if context.entity_kind == EntityKind.gargoyle then
        -- ROM 0xB574 selects the zero-vector list until frame +0x1E reaches 2;
        -- that frame selects the 36-unit creature step list.
        step = context.ai_animation_frame == 2 and 36 or 0
    else
        local distance = state.movement_route_distance
        if type(distance) ~= "number" then
            return context.movement_offsets
        end
        if distance < 0x100 then
            step = 8
        elseif distance < 0x200 then
            step = 10
        elseif distance < 0x360 then
            step = 15
        elseif distance < 0x460 then
            step = 20
        elseif (context.source_record_flags &
                ActorServiceFlag.targets_world_actors) ~= 0 then
            step = 31
        elseif (context.source_spawn_state_flags &
                ActorSpawnFlag.transitioning) ~= 0 then
            -- ROM 0xB610 selects 22 or 10 from the shared FFE84 world-activity
            -- gate. Here the typed world-activity state supplies that fact.
            step = context.world_activity_active and 22 or 10
        elseif (context.source_behavior_flags &
                ActorBehaviorFlag.high_effect) ~= 0 then
            step = 30
        else
            step = 18
        end
    end

    -- ROM 0xBADE copies the selected vector table's +4 word to actor +0xCA.
    state.animation_timing_word = step

    -- The ROM tables at 0xBD72..0xBFE2 contain these eight signed X/Y
    -- displacements. Expand their selected speed into ordinary Lua vectors;
    -- the host still owns only the collision query.
    return {
        {0, -step}, {step, -step}, {step, 0}, {step, step},
        {0, step}, {-step, step}, {-step, 0}, {-step, -step},
    }
end

local function movement_context_word(offsets, direction)
    -- ROM 0xBADE reads the selected table's word at byte offset 4 + D0,
    -- where D0 is the doubled direction ordinal.
    local word_index = 2 + direction
    local pair = math.floor(word_index / 2) + 1
    local component = (word_index % 2) + 1
    return offsets[pair][component] & 0xffff
end

local function probe_movement(context, movement_state, direction_slot,
                              movement_offsets)
    local direction = (direction_slot >> 1) & 0x7
    local offset = movement_offsets[direction + 1]
    local candidate_x = (movement_state.position_x + offset[1]) & 0xffff
    local candidate_y = (movement_state.position_y + offset[2]) & 0xffff
    local collision_code, terrain_collision_code = game.movement.collision(
        candidate_x, candidate_y)
    movement_state.terrain_collision_code = terrain_collision_code
    -- ROM 0xB77E maps detailed negative terrain results to 0x7F unless the
    -- actor's +0x10 bit 1 requests that negative values be preserved.
    if collision_code < 0 and (context.movement_entity_flags & 0x02) == 0 then
        collision_code = 0x7f
    end
    return {
        collision_code = collision_code,
        candidate_x = candidate_x,
        candidate_y = candidate_y,
        direction_slot = direction_slot,
    }
end

local function route_key_for_heading(heading)
    local rotated = ((heading << 3) | (heading >> 13)) & 0xffff
    rotated = rotated & 0x07
    return (rotated & 0x06) << 1
end

local function movement_probe_is_blocked(probe, context)
    return movement_collision_code(probe.collision_code, context) > 0
end

local function commit_movement_candidate(movement_state, probe)
    movement_state.position_x = probe.candidate_x
    movement_state.position_y = probe.candidate_y
    movement_state.event_flags = movement_state.event_flags & 0xfe
end

local function reset_blocked_movement(context, state, movement_state)
    movement_state.route_flags = movement_state.route_flags & 0xfd
    movement_state.heading = state.movement_heading
    movement_state.movement_flags = movement_state.movement_flags | 0x01
    movement_state.route_status = -1
    state.movement_current_heading = state.movement_heading
    local direction = direction_from_heading(movement_state.heading)
    state.direction = direction
    state.animation_id = "idle"
    -- B442 replaces +0x16 and clears movement +0x38, but leaves the sprite
    -- frame (+0x1E) and cadence accumulator (+0x1A) to the render pass.
    local _, terrain_collision_code = game.movement.collision(
        movement_state.position_x, movement_state.position_y)
    movement_state.terrain_collision_code = terrain_collision_code
    return {
        outcome = "reset",
        state = movement_state,
        direction_animation = context.movement_idle_selectors[direction + 1],
    }
end

local function advance_entity_movement(context, state)
    if not context.movement_services_available or
       state.movement_heading == nil or
       state.movement_current_heading == nil then
        return nil
    end

    local movement_offsets = creature_movement_offsets(context, state)
    if type(movement_offsets) ~= "table" or #movement_offsets ~= 8 then
        return nil
    end

    local movement_state = {}
    for key, value in pairs(context.movement_state) do
        movement_state[key] = value
    end
    movement_state.heading = state.movement_current_heading
    movement_state.animation_heading = movement_state.heading
    if context.dynamic_world_encounter_actor == true then
        movement_state.collision_extent =
            context.entity_kind == EntityKind.gargoyle and
            context.ai_animation_frame == 2 and -20 or 0
    else
        movement_state.collision_extent = -12
    end

    local initial_direction = direction_from_heading(movement_state.heading)
    local initial_slot = initial_direction * 2
    movement_state.collision_context = movement_context_word(
        movement_offsets, initial_direction)
    local initial = probe_movement(
        context, movement_state, initial_slot, movement_offsets)

    if initial.collision_code == 0 then
        movement_state.movement_substate = 0x1000
        commit_movement_candidate(movement_state, initial)
        state.animation_id = "walk"
        state.direction = direction_from_heading(movement_state.heading)
        return {
            outcome = "accepted",
            state = movement_state,
            direction_animation = 0x20 + context.movement_idle_selectors[
                direction_from_heading(movement_state.heading) + 1],
        }
    end

    if initial.collision_code < 0 then
        local recovered, route_flags = game.movement.recover_collision(
            initial.candidate_x, initial.candidate_y,
            initial.collision_code, initial.direction_slot)
        if type(recovered) ~= "boolean" or
           type(route_flags) ~= "number" or route_flags < 0 or
           route_flags > 0xff then
            return nil
        end
        -- ROM 0xB7B8 sets the actor +0xB0 retry bit before dispatching the
        -- map event. Preserve that native collision side effect across the
        -- Lua movement snapshot; 0xAC5E clears it after clear terrain. The
        -- recovery result already carries the transition's separate movement
        -- outcome, so keep the other route-state bits in Lua's snapshot.
        movement_state.route_flags =
            (movement_state.route_flags & 0xdf) | (route_flags & 0x20)
        if not recovered then
            movement_state.movement_substate = 0x1000
            commit_movement_candidate(movement_state, initial)
            state.animation_id = "walk"
            state.direction = direction_from_heading(movement_state.heading)
            return {
                outcome = "accepted",
                state = movement_state,
                direction_animation = 0x20 + context.movement_idle_selectors[
                    direction_from_heading(movement_state.heading) + 1],
            }
        end
    end

    if (movement_state.heading & 0x3fff) == 0 then
        local key = route_key_for_heading(movement_state.heading)
        local route = game.movement.route(key)
        local fallback
        local carry_set = false
        if not (route.primary_marker > 0 and route.secondary_marker > 0) then
            local fallback_slot = route.primary_marker <= 0
                and route.primary_fallback_slot
                or route.secondary_fallback_slot
            fallback = probe_movement(
                context, movement_state, fallback_slot, movement_offsets)
            carry_set = movement_probe_is_blocked(fallback, context)
        else
            carry_set = true
        end
        if not carry_set and fallback ~= nil then
            movement_state.collision_flags = movement_state.collision_flags & 0xfe
            commit_movement_candidate(movement_state, fallback)
            state.animation_id = "walk"
            state.direction = direction_from_heading(movement_state.heading)
            return {
                outcome = "accepted",
                state = movement_state,
                direction_animation = 0x20 + context.movement_idle_selectors[
                    direction_from_heading(movement_state.heading) + 1],
            }
        end
        local reset = reset_blocked_movement(
            context, state, movement_state)
        state.animation_id = "idle"
        return reset
    end

    local movement_direction = (movement_state.heading >> 13) & 0x06
    movement_state.movement_substate = movement_direction
    local selected = probe_movement(
        context, movement_state, movement_direction << 1,
        movement_offsets)
    if movement_probe_is_blocked(selected, context) then
        movement_state.route_status = movement_direction
        local alternate_direction = (movement_direction + 2) & 0x07
        selected = probe_movement(
            context, movement_state, alternate_direction << 1,
            movement_offsets)
        if movement_probe_is_blocked(selected, context) then
            local reset = reset_blocked_movement(
                context, state, movement_state)
            state.animation_id = "idle"
            return reset
        end
    end

    movement_state.animation_heading =
        ((selected.direction_slot >> 4) |
         (selected.direction_slot << 12)) & 0xffff
    movement_state.movement_flags = movement_state.movement_flags | 0x04
    commit_movement_candidate(movement_state, selected)
    state.animation_id = "walk"
    state.direction = direction_from_heading(movement_state.animation_heading)
    return {
        outcome = "accepted",
        state = movement_state,
        direction_animation = 0x20 + context.movement_idle_selectors[
            direction_from_heading(movement_state.animation_heading) + 1],
    }
end

local function finish_entity_movement(context, movement_step)
    if movement_step ~= nil and
       (context.movement_entity_flags & 0x08) ~= 0 and
       movement_step.state.terrain_collision_code == 0 then
        -- ROM 0xAC5E releases B7B8's retry bit after FFDF64 reports clear
        -- terrain, allowing a later doorway or terminal event to dispatch.
        movement_step.state.route_flags =
            movement_step.state.route_flags & 0xdf
    end
    return movement_step
end

function ActorAI.heartbeat(context)
    if type(context) ~= "table" then
        return nil
    end
    local profile = source_profile(context)
    if profile == nil or type(context.actor_id) ~= "string" or
       context.actor_id == "" or type(context.initial_heading) ~= "number" or
       type(context.entry_delay) ~= "number" or context.entry_delay < 0 or
       context.entry_delay >
           (context.dynamic_world_encounter_actor and 0x28 or 15) or
       (context.dynamic_world_encounter_actor and
        (type(context.entry_animation_frame_count) ~= "number" or
         context.entry_animation_frame_count % 1 ~= 0 or
         context.entry_animation_frame_count < 1 or
         context.entry_animation_frame_count > 8)) or
       (context.phase ~= "behavior" and context.phase ~= "render") then
        return nil
    end

    local state = states[context.actor_id]
    if context.spawn or state == nil then
        state = initial_state(context, profile)
        if state == nil then
            return nil
        end
        states[context.actor_id] = state
        advance_world_target_selection(state, context)
        if profile.projectile_animation then
            return projectile_heartbeat(state, context)
        end
        if profile.controls_movement and not profile.dynamic_encounter_only then
            state.combat_action_requested =
                context.phase == "behavior" and
                context.combat_action_available == true and
                context.combat_confirm_held == true and
                context.combat_action_ready == true and
                context.combat_input_blocked ~= true
            state.movement_step = nil
            if context.phase == "behavior" then
                state.movement_heading =
                    requested_movement_heading(context, profile, state)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                    state.movement_step = finish_entity_movement(
                        context, advance_entity_movement(context, state))
                end
            else
                advance_player_animation(state, context)
            end
        end
        advance_target_marker(state, context)
        return snapshot(state, "hold", nil)
    end
    if not same_actor(state, context) then
        return nil
    end
    state.source_action_dispatch = nil
    state.source_record_flags = context.source_record_flags or
                                state.source_record_flags
    advance_source_action_delay(state, context)
    advance_world_target_selection(state, context)
    if profile.projectile_animation then
        return projectile_heartbeat(state, context)
    end
    advance_target_marker(state, context)
    advance_target_motion_effect(state, context)

    local reaction_audio_event
    if context.dynamic_world_encounter_actor then
        local reaction_started = false
        if context.phase == "behavior" and state.incoming_hit ~= nil then
            reaction_audio_event =
                begin_received_hit_reaction(state, context)
            reaction_started = state.damage_reaction_active
        end
        if state.damage_reaction_active and
           state.animation_id == "down" then
            if context.phase == "render" then
                state.movement_step = nil
                if not advance_finite_actor_animation(
                        state, context.entry_animation_frame_count,
                        0x0200, 0x1000) then
                    return nil
                end
            end
            return snapshot(state, "hold", reaction_audio_event)
        end
        if state.damage_reaction_active then
            if context.phase == "render" and
               state.animation_id == "hurt" then
                state.movement_step = nil
                if not advance_finite_actor_animation(
                        state, context.hurt_animation_frame_count,
                        context.hurt_animation_increment,
                        context.hurt_animation_threshold) then
                    return nil
                end
                return snapshot(state, "hold", nil)
            elseif context.phase == "behavior" and not reaction_started then
                -- B07E remains the actor continuation while the ordinary AI
                -- coordinator can replace C072 with C062 after a movement
                -- step. Its eight-service cue countdown survives that pose
                -- change and movement remains live on every service pass.
                reaction_audio_event =
                    advance_received_hit_reaction(state, context)
            end
        end
    end

    local action = "hold"
    local audio_event = reaction_audio_event
    local target_heading_audio_event
    if profile.controls_movement then
        -- ROM 0xACAE observes the held A bit and only dispatches a player
        -- action after the source +0xDC delay gate expires. The host supplies
        -- those typed source facts; this Lua heartbeat owns the input policy.
        state.combat_action_requested =
            context.phase == "behavior" and
            context.combat_action_available == true and
            context.combat_confirm_held == true and
            context.combat_action_ready == true and
            context.combat_input_blocked ~= true
        if context.phase == "render" then
            state.movement_step = nil
            if not profile.dynamic_encounter_only then
                state.movement_heading = nil
            end
            if profile.dynamic_encounter_only then
                if state.transition_started and
                   not state.transition_complete then
                    advance_entry_animation(state)
                elseif state.animation_id == "action" then
                    if not advance_dynamic_actor_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "spell" then
                    -- C082 lists are one frame. Hold it; do not invent a count.
                    state.animation_frame = 0
                    state.animation_accumulator = 0
                elseif state.animation_id == "walk" then
                    if not advance_dynamic_actor_walk_animation(state, context) then
                        return nil
                    end
                else
                    local facing_heading = state.ai_target_engaged
                        and state.ai_target_heading or
                        state.movement_current_heading or
                        context.initial_heading
                    state.direction = direction_from_heading(facing_heading)
                    state.animation_id = "idle"
                    state.animation_frame = 0
                end
            else
                advance_player_animation(state, context)
            end
        else
            state.movement_step = nil
            if profile.dynamic_encounter_only then
                if not state.transition_started then
                    -- ROM 0x13904 services +0xB6 before it enters 0xB330.
                    if state.entry_delay == 0 then
                        state.transition_started = true
                        state.animation_id = "entry"
                        state.animation_frame = 0
                        state.animation_accumulator = -16
                        state.behavior_timer = 20
                        action = "play_entry"
                        audio_event = profile.entry_audio_event
                    else
                        state.entry_delay = state.entry_delay - 1
                    end
                elseif not state.transition_complete then
                    if state.behavior_timer == 0 then
                        state.behavior_timer = 20
                        if state.transition_frame_list_finished then
                            state.transition_complete = true
                        end
                    else
                        state.behavior_timer = state.behavior_timer - 1
                    end
                else
                    if state.behavior_timer > 0 then
                        state.behavior_timer = state.behavior_timer - 1
                    else
                        -- ROM 0x13904 decrements +0xB6 on every actor-service
                        -- pass, then refreshes target and route policy every
                        -- 21 passes. The +0x3A continuation moves each pass.
                        state.behavior_timer = 0x14
                        requested_movement_heading(context, profile, state)
                        local target = retained_linked_target(context, state)
                        if context.action_kind == ActionKind.spell then
                            -- Negative +0x56 never enters 0x14C7C / 0xB1DC.
                            state.ai_target_engaged = false
                            state.source_action_dispatch =
                                classify_spell_dispatch(
                                    state, context, target)
                            if state.source_action_dispatch == "spell" then
                                begin_spell_cast(state, context, target)
                            elseif state.animation_id ~= "spell" then
                                local route_heading =
                                    source_motion_destination_heading(
                                        context, state, target)
                                state.movement_heading = route_heading
                                state.movement_active = route_heading ~= nil
                                if not state.movement_active then
                                    state.movement_route_distance = nil
                                end
                            end
                        elseif context.action_kind == ActionKind.item then
                            -- Nonnegative +0x56 never enters 0x14C7C / 0xB1DC.
                            state.ai_target_engaged = false
                            state.source_action_dispatch =
                                classify_item_dispatch(
                                    state, context, target)
                            local route_heading =
                                source_motion_destination_heading(
                                    context, state, target)
                            state.movement_heading = route_heading
                            state.movement_active = route_heading ~= nil
                            if not state.movement_active then
                                state.movement_route_distance = nil
                            end
                        elseif not resolve_source_target_motion(
                            context, state, target) then
                            -- 0x141DE stores mode 2 when 0x14C7C does not
                            -- accept. 0x13D42 copies the linked position and
                            -- 0x145A2 routes straight at it. The +0xB5 flank
                            -- stays on the spell and item callers.
                            local route_heading = nil
                            if context.movement_allowed and
                               type(target) == "table" and
                               type(target.heading) == "number" and
                               type(target.distance) == "number" then
                                route_heading = target.heading & 0xffff
                                state.movement_route_distance =
                                    target.distance & 0xffff
                            else
                                state.movement_route_distance = nil
                            end
                            state.movement_heading = route_heading
                            state.movement_active = route_heading ~= nil
                        else
                            audio_event, target_heading_audio_event =
                                begin_dynamic_target_action(
                                    state, context, target)
                        end
                    end

                    -- 0xA8EA invokes the actor's +0x3A continuation every
                    -- service pass. 0xB574 slews +0x2E toward the stored
                    -- +0x34 heading by 0x0800 and advances using +0xC8, which
                    -- 0x145A2 measured to the selected route destination.
                    if state.transition_complete and state.movement_active then
                        state.movement_current_heading = approach_heading(
                            state.movement_current_heading,
                            state.movement_heading)
                        state.movement_step = finish_entity_movement(
                            context,
                            advance_entity_movement(context, state))
                        if state.movement_step ~= nil and
                           state.movement_step.state ~= nil then
                            state.direction = direction_from_heading(
                                state.movement_step.state.animation_heading)
                        end
                    end
                end
            else
                state.movement_heading =
                    requested_movement_heading(context, profile, state)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                    state.movement_step = finish_entity_movement(
                        context, advance_entity_movement(context, state))
                    if state.movement_step ~= nil and
                       state.movement_step.state ~= nil then
                        state.direction = direction_from_heading(
                            state.movement_step.state.animation_heading)
                    end
                end
            end
        end
        return snapshot(state, action, audio_event, nil,
                        target_heading_audio_event)
    end

    if context.phase == "render" then
        advance_entry_animation(state)
        return snapshot(state, action, nil)
    end
    state.movement_heading = nil

    -- ROM 0x13904 services the actor's signed initial delay before B330.
    if not state.transition_started then
        if state.entry_delay == 0 then
            state.transition_started = true
            state.animation_id = "entry"
            state.animation_frame = 0
            state.animation_accumulator = -16
            state.behavior_timer = 20
            action = "play_entry"
            audio_event = profile.entry_audio_event
        else
            state.entry_delay = state.entry_delay - 1
        end
    elseif not state.transition_complete then
        if state.behavior_timer == 0 then
            state.behavior_timer = 20
            if state.transition_frame_list_finished then
                state.transition_complete = true
            end
        else
            state.behavior_timer = state.behavior_timer - 1
        end
    end

    return snapshot(state, action, audio_event)
end

return ActorAI
