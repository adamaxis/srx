-- actor/movement.lua: Lua movement policy, projectile integration, and collision probes.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local Bearing = require("bearing")
local EntityMotion = require("entity_motion")
local u16 = I.u16
local SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local MovementBearingDegrees = E.MovementBearingDegrees
local VisualProfile = R.VisualProfile -- from actor/tables.lua
local projectile = R.projectile   -- from actor/tables.lua
local PROJECTILE_PROBE_DISTANCE = 96
local BURST_FOLLOWUP_RANGE = 33 * PROJECTILE_PROBE_DISTANCE

local function wrap_map_coordinate(value)
    -- [C] The native boundary stores map positions on a 16-bit world axis.
    return value % 65536
end

local function preserve_fractional_position(integer_position, position)
    -- [C] ROM 0x15ABA replaces the integer coordinate and keeps its fraction.
    return wrap_map_coordinate(integer_position + position % 1)
end

local function projectile_movement_snapshot(context, state)
    local source = context.projectile_movement
    local position = type(source) == "table" and source.position or nil
    local velocity = type(source) == "table" and source.velocity or nil
    local origin = type(source) == "table" and source.burst_origin or nil
    if type(source) ~= "table" or
       (source.role ~= projectile.role.primary and
        source.role ~= projectile.role.secondary) or
       (source.phase ~= "inactive" and source.phase ~= "flying" and
        source.phase ~= "awaiting_animation") or
       type(source.impact_occurred) ~= "boolean" or
       type(source.animation_finished) ~= "boolean" or
       type(position) ~= "table" or
       type(position.x) ~= "number" or position.x < 0 or position.x >= 65536 or
       position.x ~= position.x or
       type(position.y) ~= "number" or position.y < 0 or position.y >= 65536 or
       position.y ~= position.y or
       type(velocity) ~= "table" or
       type(velocity.x) ~= "number" or velocity.x ~= velocity.x or
       velocity.x == math.huge or velocity.x == -math.huge or
       type(velocity.y) ~= "number" or velocity.y ~= velocity.y or
       velocity.y == math.huge or velocity.y == -math.huge or
       type(source.remaining_flight_range) ~= "number" or
       source.remaining_flight_range < PROJECTILE_PROBE_DISTANCE or
       source.remaining_flight_range > 128 * PROJECTILE_PROBE_DISTANCE or
       source.remaining_flight_range ~= source.remaining_flight_range or
       source.remaining_flight_range == math.huge or
       source.remaining_flight_range == -math.huge or
       type(source.shots_remaining) ~= "number" or
       source.shots_remaining < 0 or source.shots_remaining > 2 or
       source.shots_remaining ~= math.floor(source.shots_remaining) or
       type(source.bearing_degrees) ~= "number" or
       source.bearing_degrees < 0 or source.bearing_degrees >= 360 or
       type(source.attack_successes) ~= "number" or
       source.attack_successes < 0 or source.attack_successes > 65535 or
       source.attack_successes ~= math.floor(source.attack_successes) or
       type(origin) ~= "table" or
       type(origin.x) ~= "number" or origin.x < 0 or
       origin.x > AiTuning.WORD_MAX or origin.x ~= math.floor(origin.x) or
       type(origin.y) ~= "number" or origin.y < 0 or
       origin.y > AiTuning.WORD_MAX or origin.y ~= math.floor(origin.y) or
       type(source.animation_action) ~= "string" or
       source.animation_action ~= "unchanged" then
        return nil
    end

    local result = {
        role = source.role,
        phase = source.phase,
        impact_occurred = source.impact_occurred,
        animation_finished = source.animation_finished,
        position = { x = position.x, y = position.y },
        velocity = { x = velocity.x, y = velocity.y },
        remaining_flight_range = source.remaining_flight_range,
        shots_remaining = source.shots_remaining,
        bearing_degrees = source.bearing_degrees,
        burst_origin = { x = origin.x, y = origin.y },
        attack_successes = state.action_result or source.attack_successes,
        animation_action = "unchanged",
        outcome = projectile.outcome.none,
    }
    local audio_event

    if result.phase == "inactive" then
        return result, nil
    end
    if result.role == projectile.role.secondary then
        -- ROM 0xAC4E retires the secondary record after the preceding render
        -- heartbeat marks its finite animation list complete.
        if result.animation_finished then
            result.phase = "inactive"
            result.outcome = projectile.outcome.retired
        end
        return result, nil
    end

    if result.phase == "awaiting_animation" then
        -- ROM 0x14E0A/0x14F1C wait for the stopped record's finite list.
        -- Weapon action-pair resolution in ROM 0x14F86 remains an explicit boundary.
        if result.animation_finished then
            if result.shots_remaining == 0 then
                result.phase = "inactive"
                result.outcome = projectile.outcome.retired
            else
                result.shots_remaining = result.shots_remaining - 1
                -- ROM 0x14F86 starts the next member of the three-round
                -- weapon burst at the saved spawn coordinate. It rotates the
                -- heading by -0x400 for the second shot and +0x800 for the
                -- third, then recalculates its semantic vector step.
                result.bearing_degrees = Bearing.normalize(
                    result.bearing_degrees +
                    (result.shots_remaining > 0 and -5.625 or 11.25))
                result.position.x = preserve_fractional_position(
                    result.burst_origin.x, result.position.x)
                result.position.y = preserve_fractional_position(
                    result.burst_origin.y, result.position.y)
                local step = EntityMotion.projectile_step(
                    result.bearing_degrees)
                if type(step) ~= "table" or
                   type(step.x) ~= "number" or
                   type(step.y) ~= "number" then
                    return nil
                end
                result.velocity = { x = step.x, y = step.y }
                result.remaining_flight_range = BURST_FOLLOWUP_RANGE
                result.animation_action = "restart"
                result.phase = "flying"
                result.outcome = projectile.outcome.burst_followup_restarted
            end
        end
        return result, nil
    end

    -- The host sends position, phase, and bearing. Rebuild motion from the
    -- game-level bearing instead of carrying the source record's offset pair.
    local step = EntityMotion.projectile_step(result.bearing_degrees)
    if type(step) ~= "table" or type(step.x) ~= "number" or
       type(step.y) ~= "number" then
        return nil
    end
    result.velocity = { x = step.x, y = step.y }

    local outside_bounds_marker = false
    for _ = 1, projectile.movement_probes_per_update do
        local probe_x = wrap_map_coordinate(
            result.position.x + result.velocity.x)
        local probe_y = wrap_map_coordinate(
            result.position.y + result.velocity.y)
        local terrain = game.movement.projectile_terrain(probe_x, probe_y)
        if type(terrain) ~= "table" or
           type(terrain.blocked) ~= "boolean" or
           type(terrain.boundary_active) ~= "boolean" or
           type(terrain.outside_map) ~= "boolean" then
            return nil
        end
        outside_bounds_marker = terrain.outside_map

        local terminal = terrain.blocked and not terrain.boundary_active
        if not terminal then
            if result.remaining_flight_range <= PROJECTILE_PROBE_DISTANCE then
                terminal = true
            else
                result.remaining_flight_range =
                    result.remaining_flight_range - PROJECTILE_PROBE_DISTANCE
            end
        end
        if terminal then
            result.impact_occurred = true
            result.phase = "awaiting_animation"
            result.animation_action = "stop"
            result.velocity = { x = 0, y = 0 }
            if outside_bounds_marker then
                result.phase = "inactive"
                result.outcome = projectile.outcome.retired
            else
                result.outcome = projectile.outcome.stopped
            end
            -- ROM 0x14E32 requests event 0x3C when D1 is zero, including
            -- the outside-bounds branch.
            audio_event = projectile.impact_sound_event
            break
        end

        result.position.x = probe_x
        result.position.y = probe_y
        local contact = game.movement.projectile_collision(probe_x, probe_y)
        if type(contact) ~= "table" or
           (contact.kind ~= "none" and contact.kind ~= "target_hit" and
            contact.kind ~= "linked_followup") then
            return nil
        end
        local target_position = contact.target_position
        if contact.kind == "target_hit" then
            if type(target_position) ~= "table" or
               type(target_position.x) ~= "number" or
               target_position.x < 0 or target_position.x > AiTuning.WORD_MAX or
               target_position.x ~= math.floor(target_position.x) or
               type(target_position.y) ~= "number" or
               target_position.y < 0 or target_position.y > AiTuning.WORD_MAX or
               target_position.y ~= math.floor(target_position.y) then
                return nil
            end
        elseif target_position ~= nil then
            return nil
        end
        local hit_target_id = contact.target_id
        if hit_target_id ~= nil and
           (type(hit_target_id) ~= "string" or hit_target_id == "") then
            return nil
        end
        if contact.kind ~= "none" then
            result.impact_occurred = true
            result.phase = "awaiting_animation"
            result.animation_action = "stop"
            result.velocity = { x = 0, y = 0 }
            if outside_bounds_marker then
                result.phase = "inactive"
                result.outcome = projectile.outcome.retired
            end
            if contact.kind == "target_hit" then
                result.position.x = preserve_fractional_position(
                    target_position.x, probe_x)
                result.position.y = preserve_fractional_position(
                    target_position.y, probe_y)
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
        if not R.advance_projectile_animation(state, context) then
            return nil
        end
        return R.snapshot(state, "hold", nil)
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
    if movement.animation_action == "stop" then
        -- Stopping clears the finite visual's completion signal while
        -- preserving the visible frame phase; the host chooses the asset.
        state.animation_complete = false
    end
    if movement.outcome == projectile.outcome.burst_followup_restarted then
        local result = game.modules.combat.resolve_weapon_attack_seed(context)
        if type(result) ~= "number" then
            return nil
        end
        state.attack_seed_resolved = true
        state.action_result = result
        movement.attack_successes = result
        -- ROM 0x14F86 clears +0x18 and +0x1E before the next render pass.
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        state.animation_complete = false
        state.projectile_animation_started = true
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
                context, { attack_successes = state.action_result }, hit_target)
        end
    end
    return R.snapshot(state, "hold", audio_event, movement)
end

local function requested_movement_bearing_degrees(context, profile, state)
    if profile.dynamic_encounter_only then
        local target = R.linked_target(context, state)
        local heading = target and target.bearing_degrees or nil
        local distance = target and target.distance or nil
        -- Keep the selected source actor relation explicit in Lua state.
        -- The host may render and inspect this R.snapshot but does not choose
        -- the target or apply the creature's movement policy.
        state.ai_target_id = target and target.id or nil
        state.ai_target_bearing_degrees = heading
        state.ai_target_distance = distance
        -- ROM 0x13D0C selects a destination on the +0xB6 timer. Its route is
        -- then followed by the actor's +0x3A continuation each service pass.
        return nil
    end
    if not profile.controls_movement or not context.movement_allowed then
        return nil
    end
    local direction = context.movement_direction_override or
        context.movement_input_direction
    if type(direction) ~= "string" then
        return nil
    end
    return MovementBearingDegrees[direction]
end

local function resolve_source_target_motion(context, state, target)
    state.ai_target_engaged = false
    if not context.movement_allowed or type(target) ~= "table" or
       type(target.bearing_degrees) ~= "number" or
       type(target.distance) ~= "number" then
        return false
    end

    local bearing_degrees = Bearing.normalize(target.bearing_degrees)
    if bearing_degrees == nil then
        return false
    end
    local distance = target.distance & AiTuning.WORD_MAX
    local accepted = false

    if context.visual_profile_id == VisualProfile.HELL_HOUND then
        -- ROM 0x14D04: inside 0x300, nonnegative +0xDC accepts the target
        -- heading; negative +0xDC first needs a free effect slot and then
        -- replaces both +0x2E and +0x34. From 0x300 through 0x5FF both
        -- headings change immediately. At 0x600 the actor keeps pursuing.
        if distance < AiTuning.HELL_HOUND_CLOSE_RANGE then
            if state.action_delay < 0 then
                accepted = R.request_target_motion_effect(
                    state, context, bearing_degrees, target)
                if accepted then
                    state.movement_current_bearing_degrees = bearing_degrees
                end
            else
                accepted = true
            end
        elseif distance < AiTuning.HELL_HOUND_PURSUIT_RANGE then
            state.movement_current_bearing_degrees = bearing_degrees
            accepted = true
        end
    elseif context.visual_profile_id == VisualProfile.GARGOYLE or
           context.visual_profile_id == VisualProfile.WENDIGO then
        -- 0x14C7C calls 0x15C84, whose 0x15C58 range test measures the
        -- linked record's actual coordinates and accepts only distance <
        -- 0x200. The preceding 0x14C56 projection result is not consumed by
        -- that test. A Gargoyle at frame 2 reaches the range branch that
        -- writes the source animation phase to its threshold; represent that
        -- as complete normalized progress for the next render update. The
        -- actor continues its route on this behavior pass. [C: 0x14C7C,
        -- 0x15C58, 0x5FDC]
        if distance < 0x200 then
            if context.visual_profile_id == VisualProfile.GARGOYLE and
               context.ai_animation_frame == 2 then
                state.animation_progress = 1.0
            else
                state.movement_current_bearing_degrees = bearing_degrees
                accepted = true
            end
        end
    elseif distance < AiTuning.CLOSE_STRIKE_RANGE then
        -- Other kinds use 0x15C58 directly. That branch writes +0x34 but
        -- leaves +0x2E to the actor's own action continuation.
        accepted = true
    end

    if accepted then
        state.movement_bearing_degrees = bearing_degrees
        state.movement_active = false
        state.movement_route_distance = nil
        state.ai_target_engaged = true
        state.direction = R.direction_from_bearing_degrees(bearing_degrees)
    end
    return accepted
end

local function movement_collision_code(code, context)
    if code >= 0 then
        return code
    end
    local low_bits = code & Collision.LOW_BITS
    if context.movement_transition_active or low_bits >= Collision.IMPASSABLE_FROM or
       ((context.movement_scene_id == SceneId.ALARM_COMBAT or
         context.movement_collision_mode == 2) and
        low_bits >= 2 and low_bits <= 5) then
        return Collision.BLOCKED
    end
    return code
end

-- ROM 0xBDF2, cardinal step by Quickness clamped to 0..9.
local quickness_cardinal_steps = { 9, 9, 12, 16, 19, 22, 25, 27, 29, 31 }

local function movement_lifecycle(context)
    local facts = context.movement_actor_facts
    return type(facts) == "table" and facts.lifecycle or {}
end

-- Movement is represented as a distance in map units per actor update.
-- Direction supplies signed X/Y components; ROM C002's eight-entry XY word
-- table is source evidence, not a runtime Lua data structure [C].
local direction_vectors = {
    { x = 0, y = -1 }, { x = 1, y = -1 },
    { x = 1, y = 0 },  { x = 1, y = 1 },
    { x = 0, y = 1 },  { x = -1, y = 1 },
    { x = -1, y = 0 }, { x = -1, y = -1 },
}

local function direction_vector(direction)
    return direction_vectors[(direction & 0x7) + 1]
end

local function actor_lifecycle(context)
    if context.dynamic_world_encounter_actor == true then
        local facts = context.actor_facts
        return type(facts) == "table" and facts.lifecycle or {}
    end
    return movement_lifecycle(context)
end

local function walk_direction_from_bearing_degrees(bearing_degrees)
    -- ROM 0xB574 walk handler (0xB57E, 0xB5BE) and its walk animation pick
    -- (0xB6CC, C062): rol.w #3 then andi.w #7 with no round-up, so the
    -- octant is truncated. 0xB36A-family animation picks and the 0xBABC
    -- step round up instead (R.direction_from_bearing_degrees) [C: disasm; emulator
    -- run5 FF012E 0x7800..0x6000 step diagonally, 0x5800 steps right].
    return Bearing.octant(bearing_degrees, false)
end

local function movement_octant_selector(context)
    -- 0x142AE selects a tagged actor's next waypoint and 0x145A2 installs the
    -- B574 continuation. Its generic B5BE movement selector and B6BA-B6DE
    -- C062 walk selector truncate for routed default actors too: 0x44BC gives
    -- them +0x10=0x05, so THINK alone cannot identify that path. The player
    -- BABC path keeps its existing flag-gated rounded selector.
    if context.dynamic_world_encounter_actor == true or
       context.source_actor_has_waypoint_data == true or
       actor_lifecycle(context).thinking_enabled then
        return walk_direction_from_bearing_degrees
    end
    return R.direction_from_bearing_degrees
end

local function movement_direction_from_bearing_degrees(context, bearing_degrees)
    return movement_octant_selector(context)(bearing_degrees)
end

local function creature_movement_step_distance(context, state)
    local lifecycle = actor_lifecycle(context)
    local walk_fast_selected = context.walk_fast_enabled == true and
        lifecycle.thinking_enabled == true
    local combat_attributes = type(context.combatant) == "table" and
        context.combatant.attributes or nil
    local combat_quickness = type(combat_attributes) == "table" and
        combat_attributes.quickness or nil
    -- Contexts without a live Quickness byte keep the host vector list.
    -- Walk Fast is an earlier ROM branch and does not require Quickness.
    local attribute_speed =
        context.dynamic_world_encounter_actor == true or
        type(combat_quickness) == "number"
    if not attribute_speed and not walk_fast_selected then
        return context.movement_default_step_distance
    end

    -- 0xACAE installs 0xB574 when +0x10 bit 1 is set. Bit 1 clear is the
    -- distance-band path at 0xB6E4.
    local targets_world = lifecycle.ai_runner == true
    local high_effect = context.actor_facts.behavior.escort == true
    local activity = context.world_activity_active == true
    local step

    if context.visual_profile_id == VisualProfile.GARGOYLE then
        -- 0xB574: frame +0x1E == 2 uses 0xBE42. Other frames use 0xBE22.
        -- Both branches join 0xB678.
        step = context.ai_animation_frame == 2 and 36 or 0
    elseif walk_fast_selected then
        -- ROM 0xB5CA: FFE168 bit 0 plus actor +0x10 bit 1 selects 0xC022
        -- directly, bypassing the normal distance/Quickness table selection.
        step = 40
    else
        step = 30
        if not targets_world and not high_effect then
            step = 18
        end
        local skip_quickness = false
        if context.actor_facts.roles.pedestrian then
            step = 22
            if activity then
                skip_quickness = true
            else
                step = 10
            end
        end
        if not skip_quickness then
            if activity and high_effect then
                step = 22
            elseif activity then
                local quickness = combat_quickness
                if type(quickness) == "number" then
                    local index = quickness
                    if index >= 128 then
                        index = index - 256
                    end
                    if index >= 10 then
                        index = 9
                    end
                    if index >= 0 then
                        step = quickness_cardinal_steps[index + 1]
                    end
                end
            end
        end
        if not lifecycle.thinking_enabled then
            local distance = state.movement_route_distance
            if type(distance) ~= "number" then
                return context.movement_default_step_distance
            end
            if targets_world then
                step = 31
            end
            if distance < 0x100 then
                step = 8
            elseif distance < 0x200 then
                step = 10
            elseif distance < 0x360 then
                step = 15
            elseif distance < 0x460 then
                step = 20
            end
        end
    end

    -- 0xB678: bit 3 and projected Y == 0 replace the table with 0xC022.
    if targets_world and context.source_projection_active ~= true then
        step = 40
    end

    -- This is the actor's map-coordinate displacement per movement update.
    state.movement_distance_per_update = step
    return step
end

local function probe_movement(context, movement_state, direction_slot,
                              step_distance)
    local direction = (direction_slot >> 1) & 0x7
    local vector = direction_vector(direction)
    local candidate_x = u16(
        movement_state.position_x + vector.x * step_distance)
    local candidate_y = u16(
        movement_state.position_y + vector.y * step_distance)
    local collision_code, terrain_collision_code = game.movement.collision(
        candidate_x, candidate_y)
    movement_state.terrain_collision_code = terrain_collision_code
    -- ROM 0xB77E maps detailed negative terrain results to 0x7F unless the
    -- actor's +0x10 bit 1 requests that negative values be preserved.
    if collision_code < 0 and
       not actor_lifecycle(context).thinking_enabled then
        collision_code = Collision.BLOCKED
    end
    return {
        collision_code = collision_code,
        candidate_x = candidate_x,
        candidate_y = candidate_y,
        direction_slot = direction_slot,
    }
end

local function route_key_for_bearing_degrees(bearing_degrees)
    local octant = Bearing.octant(bearing_degrees, false)
    if octant == nil then
        return nil
    end
    return (octant & 0x06) << 1
end

local function movement_probe_is_blocked(probe, context)
    return movement_collision_code(probe.collision_code, context) > 0
end

local function commit_movement_candidate(movement_state, probe)
    movement_state.position_x = probe.candidate_x
    movement_state.position_y = probe.candidate_y
    movement_state.clear_pending_event = true
end

local function reset_blocked_movement(context, state, movement_state)
    movement_state.route_facts.path_active = false
    movement_state.bearing_degrees = state.movement_bearing_degrees
    movement_state.mark_reset_after_block = true
    movement_state.route_status = -1
    state.movement_current_bearing_degrees = state.movement_bearing_degrees
    local direction = R.direction_from_bearing_degrees(movement_state.bearing_degrees)
    state.direction = direction
    state.animation_id = "idle"
    -- B442 restores the facing and clears pending movement while preserving
    -- the current frame and Lua-owned animation progress for the render pass.
    local _, terrain_collision_code = game.movement.collision(
        movement_state.position_x, movement_state.position_y)
    movement_state.terrain_collision_code = terrain_collision_code
    return {
        outcome = "reset",
        state = movement_state,
    }
end

local function advance_entity_movement(context, state)
    if not context.movement_services_available or
       state.movement_bearing_degrees == nil or
       state.movement_current_bearing_degrees == nil then
        return nil
    end

    local movement_step_distance =
        creature_movement_step_distance(context, state)
    if type(movement_step_distance) ~= "number" or
       movement_step_distance <= 0 then
        return nil
    end

    local movement_state = {}
    for key, value in pairs(context.movement_state) do
        movement_state[key] = value
    end
    movement_state.bearing_degrees = state.movement_current_bearing_degrees
    movement_state.animation_bearing_degrees = movement_state.bearing_degrees
    movement_state.substate_update = "unchanged"
    movement_state.cardinal_direction = 0
    movement_state.mark_reset_after_block = false
    movement_state.mark_directional_fallback = false
    movement_state.clear_collision_recovery = false
    movement_state.clear_pending_event = false
    if context.dynamic_world_encounter_actor == true then
        movement_state.collision_extent =
            context.visual_profile_id == VisualProfile.GARGOYLE and
            context.ai_animation_frame == 2 and -20 or 0
    else
        movement_state.collision_extent = -12
    end

    local octant_for_heading = movement_octant_selector(context)
    local initial_direction = octant_for_heading(movement_state.bearing_degrees)
    local initial_slot = initial_direction * 2
    local initial = probe_movement(
        context, movement_state, initial_slot, movement_step_distance)

    if initial.collision_code == 0 then
        movement_state.substate_update = "direct_translation"
        commit_movement_candidate(movement_state, initial)
        state.animation_id = "walk"
        state.direction = initial_direction
        return {
            outcome = "accepted",
            state = movement_state,
        }
    end

    if initial.collision_code < 0 then
        local recovered, route_transition = game.movement.recover_collision(
            initial.candidate_x, initial.candidate_y,
            initial.collision_code, initial.direction_slot)
        if type(recovered) ~= "boolean" or
           type(route_transition) ~= "boolean" then
            return nil
        end
        -- Collision recovery reports a typed doorway transition outcome.
        movement_state.route_facts.route_transition = route_transition
        if not recovered then
            movement_state.substate_update = "direct_translation"
            commit_movement_candidate(movement_state, initial)
            state.animation_id = "walk"
            local fallback_direction = octant_for_heading(
                movement_state.bearing_degrees)
            state.direction = fallback_direction
            return {
                outcome = "accepted",
                state = movement_state,
            }
        end
    end

    if movement_state.bearing_degrees % 90 == 0 then
        local key = route_key_for_bearing_degrees(
            movement_state.bearing_degrees)
        local route = game.movement.route(key)
        local fallback
        local carry_set = false
        if not (route.primary_marker > 0 and route.secondary_marker > 0) then
            local fallback_slot = route.primary_marker <= 0
                and route.primary_fallback_slot
                or route.secondary_fallback_slot
            -- ROM 0xBBC8: the descriptor word is a direction ordinal;
            -- 0xBBC8 doubles it into the probe slot (0xB77E doubles again
            -- for the vector table) [C].
            fallback = probe_movement(
                context, movement_state, fallback_slot << 1,
                movement_step_distance)
            carry_set = movement_probe_is_blocked(fallback, context)
        else
            carry_set = true
        end
        if not carry_set and fallback ~= nil then
            movement_state.clear_collision_recovery = true
            commit_movement_candidate(movement_state, fallback)
            state.animation_id = "walk"
            local fallback_direction = octant_for_heading(
                movement_state.bearing_degrees)
            state.direction = fallback_direction
            return {
                outcome = "accepted",
                state = movement_state,
            }
        end
        local reset = reset_blocked_movement(
            context, state, movement_state)
        state.animation_id = "idle"
        return reset
    end

    local octant = Bearing.octant(movement_state.bearing_degrees, false)
    if octant == nil then
        return nil
    end
    local movement_direction = octant & 0x06
    movement_state.substate_update = "cardinal_fallback"
    movement_state.cardinal_direction = movement_direction >> 1
    local selected = probe_movement(
        context, movement_state, movement_direction << 1,
        movement_step_distance)
    if movement_probe_is_blocked(selected, context) then
        movement_state.route_status = movement_direction
        local alternate_direction = (movement_direction + 2) & 0x07
        selected = probe_movement(
            context, movement_state, alternate_direction << 1,
            movement_step_distance)
        if movement_probe_is_blocked(selected, context) then
            local reset = reset_blocked_movement(
                context, state, movement_state)
            state.animation_id = "idle"
            return reset
        end
    end

    movement_state.animation_bearing_degrees =
        (selected.direction_slot >> 1) * Bearing.EIGHTH_TURN_DEGREES
    movement_state.mark_directional_fallback = true
    commit_movement_candidate(movement_state, selected)
    state.animation_id = "walk"
    local selected_direction = octant_for_heading(
        movement_state.animation_bearing_degrees)
    state.direction = selected_direction
    return {
        outcome = "accepted",
        state = movement_state,
    }
end

local function finish_entity_movement(context, movement_step)
    if movement_step ~= nil and
       actor_lifecycle(context).ai_runner and
       movement_step.state.terrain_collision_code == 0 then
        -- Clear the transition outcome after the accepted terrain probe.
        movement_step.state.route_facts.route_transition = false
    end
    return movement_step
end


-- register this module's definitions
R.projectile_movement_snapshot = projectile_movement_snapshot
R.projectile_heartbeat = projectile_heartbeat
R.requested_movement_bearing_degrees = requested_movement_bearing_degrees
R.resolve_source_target_motion = resolve_source_target_motion
R.movement_collision_code = movement_collision_code
R.walk_direction_from_bearing_degrees = walk_direction_from_bearing_degrees
R.movement_octant_selector = movement_octant_selector
R.movement_direction_from_bearing_degrees = movement_direction_from_bearing_degrees
R.creature_movement_step_distance = creature_movement_step_distance
R.probe_movement = probe_movement
R.route_key_for_bearing_degrees = route_key_for_bearing_degrees
R.movement_probe_is_blocked = movement_probe_is_blocked
R.commit_movement_candidate = commit_movement_candidate
R.reset_blocked_movement = reset_blocked_movement
R.advance_entity_movement = advance_entity_movement
R.finish_entity_movement = finish_entity_movement
R.quickness_cardinal_steps = quickness_cardinal_steps
return R
