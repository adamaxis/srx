-- Lua owns per-actor behavior and presentation state. The host calls a
-- behavior heartbeat during 0xA8EA and a render heartbeat during 0x4618;
-- drawing consumes each returned snapshot without advancing it again.
local ActorAI = {}
local states = game.actors

-- Source C002 movement context words. The table is static game data; collision
-- sampling itself remains a narrow native service.
local movement_context_words = {
    0, -40, 40, -40, 40, 0, 40, 40,
    0, 40, -40, 40, -40, 0, -40, -40,
}
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
}
local ActionKind = {
    none = "none",
    item = "item",
    spell = "spell",
}
local ItemAction = {
    max_power_heavy_pistol = 6,
}
local ActorEntryCue = {
    corp_guard = 0x2e,
    pedestrian = 0x2f,
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

local function source_profile(context)
    local profile = profiles[context.profile_id]
    if profile == nil or
       (not profile.projectile_animation and
        context.entity_kind ~= profile.entity_kind) or
       context.action_kind ~= profile.action_kind or
       context.action_code ~= profile.action_code then
        return nil
    end
    return profile
end

local function initial_state(context, profile)
    local state = {
        actor_id = context.actor_id,
        profile_id = context.profile_id,
        entity_kind = context.entity_kind,
        action_kind = context.action_kind,
        action_code = context.action_code,
        package_id = profile.package_id,
        weapon_item_id = profile.weapon_item_id,
        direction = direction_from_heading(context.initial_heading),
        entry_delay = context.entry_delay,
        behavior_timer = 0,
        animation_id = "idle",
        animation_frame = 0,
        animation_accumulator = 0,
        animation_increment = 0,
        movement_current_heading = profile.controls_movement
            and context.initial_heading or nil,
        transition_started = false,
        transition_frame_list_finished = false,
        transition_complete = false,
        combat_action_requested = false,
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
    end
    return state
end

local function same_actor(state, context)
    return state.profile_id == context.profile_id and
           state.entity_kind == context.entity_kind and
           state.action_kind == context.action_kind and
           state.action_code == context.action_code
end

local function snapshot(state, action, audio_event, projectile_movement)
    if state.animation_id == "idle" then
        state.animation_frame = 0
    end
    return {
        actor_id = state.actor_id,
        profile_id = state.profile_id,
        package_id = state.package_id,
        action_kind = state.action_kind,
        action_code = state.action_code,
        action = action,
        animation = state.animation_id,
        animation_selector = state.animation_selector,
        direction = state.direction,
        frame = state.animation_frame,
        animation_accumulator = state.animation_accumulator,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        combat_action_requested = state.combat_action_requested,
        audio_event = audio_event,
        weapon_item_id = state.weapon_item_id,
        animation_increment = state.animation_increment,
        movement_heading = state.movement_heading,
        movement_current_heading = state.movement_current_heading,
        movement_step = state.movement_step,
        projectile_movement = projectile_movement,
    }
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

local function projectile_movement_snapshot(context)
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
        local source_carry, source_position_updated, source_x, source_y =
            game.movement.projectile_collision(
                integer_world_word(probe_x), integer_world_word(probe_y),
                result.flags)
        if type(source_carry) ~= "boolean" or
           type(source_position_updated) ~= "boolean" or
           type(source_x) ~= "number" or type(source_y) ~= "number" then
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
    local movement, audio_event = projectile_movement_snapshot(context)
    if movement == nil then
        return nil
    end
    state.animation_selector = movement.animation_selector
    return snapshot(state, "hold", audio_event, movement)
end

local function advance_entry_animation(state)
    if state.animation_id ~= "entry" or
       state.transition_frame_list_finished then
        return
    end
    local accumulated = state.animation_accumulator + 0x0200
    if accumulated >= 0x1000 then
        state.animation_accumulator = accumulated - 0x1000
        if state.animation_frame == 0 then
            state.animation_frame = 1
        else
            state.animation_frame = 1
            state.transition_frame_list_finished = true
        end
    else
        state.animation_accumulator = accumulated
    end
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

local function requested_movement_heading(context, profile)
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

local function probe_movement(context, movement_state, direction_slot)
    local direction = (direction_slot >> 1) & 0x7
    local offset = context.movement_offsets[direction + 1]
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

local function advance_player_movement(context, state)
    if not context.movement_services_available or
       state.movement_heading == nil or
       state.movement_current_heading == nil then
        return nil
    end

    local movement_state = {}
    for key, value in pairs(context.movement_state) do
        movement_state[key] = value
    end
    movement_state.heading = state.movement_current_heading
    movement_state.animation_heading = movement_state.heading
    movement_state.collision_extent = -12

    local initial_direction = direction_from_heading(movement_state.heading)
    local initial_slot = initial_direction * 2
    movement_state.collision_context =
        movement_context_words[3 + initial_direction] & 0xffff
    local initial = probe_movement(context, movement_state, initial_slot)

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
            fallback = probe_movement(context, movement_state, fallback_slot)
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
        context, movement_state, movement_direction << 1)
    if movement_probe_is_blocked(selected, context) then
        movement_state.route_status = movement_direction
        local alternate_direction = (movement_direction + 2) & 0x07
        selected = probe_movement(
            context, movement_state, alternate_direction << 1)
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

local function finish_player_movement(context, movement_step)
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
       context.entry_delay > 15 or
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
        if profile.projectile_animation then
            return projectile_heartbeat(state, context)
        end
        if profile.controls_movement then
            state.combat_action_requested =
                context.phase == "behavior" and
                context.combat_action_available == true and
                context.combat_confirm_held == true and
                context.combat_action_ready == true and
                context.combat_input_blocked ~= true
            state.movement_step = nil
            if context.phase == "behavior" then
                state.movement_heading =
                    requested_movement_heading(context, profile)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                    state.movement_step = finish_player_movement(
                        context, advance_player_movement(context, state))
                end
            else
                advance_player_animation(state, context)
            end
        end
        return snapshot(state, "hold", nil)
    end
    if not same_actor(state, context) then
        return nil
    end
    if profile.projectile_animation then
        return projectile_heartbeat(state, context)
    end

    local action = "hold"
    local audio_event
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
            state.movement_heading = nil
            advance_player_animation(state, context)
        else
            state.movement_step = nil
            state.movement_heading =
                requested_movement_heading(context, profile)
            if state.movement_heading ~= nil then
                state.movement_current_heading = approach_heading(
                    state.movement_current_heading, state.movement_heading)
                state.movement_step = finish_player_movement(
                    context, advance_player_movement(context, state))
            end
        end
        return snapshot(state, action, nil)
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
