-- actor/movement.lua: Lua movement policy, projectile integration, and collision probes.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local Bearing = require("bearing")
local ActorPatrolRoutes = require("actor.patrol_routes")
local EntityMotion = require("entity_motion")
local SoundId, EntryDelayMax, AiTuning =
    E.SoundId, E.EntryDelayMax, E.AiTuning
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
        attack_successes = state.last_action_successes or
            source.attack_successes,
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
                -- ROM 0x14F8E installs selector 0x0014 for the restarted
                -- finite list, clearing the completed-list bit.
                result.animation_finished = false
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
            -- ROM 0x14E32 clears the finite-list selector when it stops a
            -- projectile, so the semantic completion flag must also reset.
            result.animation_finished = false
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
            -- The terminal contact path also clears the selector at 0x14E32.
            result.animation_finished = false
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
            state.last_action_successes = result
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
        state.last_action_successes = result
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
                context,
                { attack_successes = state.last_action_successes },
                hit_target)
        end
    end
    return R.snapshot(state, "hold", audio_event, movement)
end

local function requested_movement_bearing_degrees(context, profile, state)
    if R.action_movement_locked(state) then
        return nil
    end
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

-- ROM 0xBDF2, cardinal step by Quickness clamped to 0..9.
local quickness_cardinal_steps = { 9, 9, 12, 16, 19, 22, 25, 27, 29, 31 }

local function movement_lifecycle(context)
    local facts = context.movement_actor_facts
    return type(facts) == "table" and facts.lifecycle or {}
end

-- Movement is represented as a distance in map units per actor update.
-- ROM C002's doubled direction slots and XY word table stay in the native
-- adapter; Lua uses named compass directions [C].
local direction_names = {
    "north", "north_east", "east", "south_east",
    "south", "south_west", "west", "north_west",
}

local direction_bearings = {
    north = 0, north_east = 45, east = 90, south_east = 135,
    south = 180, south_west = 225, west = 270, north_west = 315,
}

local cardinal_by_octant = {
    [0] = "north", [1] = "north",
    [2] = "east", [3] = "east",
    [4] = "south", [5] = "south",
    [6] = "west", [7] = "west",
}

local next_cardinal_direction = {
    north = "east", east = "south", south = "west", west = "north",
}

local function direction_name(octant)
    if type(octant) ~= "number" or octant < 0 or octant > 7 or
       octant % 1 ~= 0 then
        return nil
    end
    return direction_names[octant + 1]
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
    -- them +0x10=0x05, so THINK alone cannot identify that path. Lua resolves
    -- the actor's route from its stable ID. Target-linked actors use the same
    -- continuation through 0x13D0C -> 0x145A2; their typed ai_runner fact
    -- selects truncation even when THINK is clear. The player BABC path keeps
    -- its existing flag-gated rounded selector.
    if context.dynamic_world_encounter_actor == true or
       actor_lifecycle(context).ai_runner == true or
       ActorPatrolRoutes.has_route(context.actor_id) or
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
    local escort_behavior = context.actor_facts.behavior.escort == true
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
        if not targets_world and not escort_behavior then
            step = 18
        end
        local skip_quickness = false
        if context.actor_facts.roles.pedestrian then
            -- This is the live +0x9E bit-5 movement branch, not actor-kind
            -- classification. ROM 0xB610 selects BE82 (22) while FFDF84 is
            -- active and BE62 (10) while it is clear [C].
            skip_quickness = true
            step = activity and 22 or 10
        end
        if not skip_quickness then
            -- With +0x9E bit 5 clear, ROM 0xB5EA..0xB608 first selects the
            -- ordinary 18-unit BEA2 vector or the 30-unit BEC2 vector from
            -- +0x10 bit 3 / +0xB0 bit 7. Only nonzero FFDF84 enters B63E..B662:
            -- +0xB0 bit 7 selects BE82 (22), otherwise +0x78 indexes BDF2.
            -- Keep the controlled-runner path separate [C].
            if activity and escort_behavior then
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

local function probe_movement(movement_state, direction,
                              step_distance)
    if direction_bearings[direction] == nil then
        return nil
    end
    local collision = game.movement.probe(
        movement_state.position_x, movement_state.position_y,
        direction, step_distance)
    if type(collision) ~= "table" or
       (collision.kind ~= "clear" and
        collision.kind ~= "requires_recovery" and
        collision.kind ~= "blocked") or
       type(collision.blocks_route_fallback) ~= "boolean" or
       type(collision.terrain_is_clear) ~= "boolean" or
       type(collision.candidate_x) ~= "number" or
       type(collision.candidate_y) ~= "number" then
        return nil
    end
    movement_state.terrain_is_clear = collision.terrain_is_clear
    return {
        collision = collision,
        candidate_x = collision.candidate_x,
        candidate_y = collision.candidate_y,
        direction = direction,
    }
end

local function route_direction_for_bearing_degrees(bearing_degrees)
    local octant = Bearing.octant(bearing_degrees, false)
    if octant == nil then
        return nil
    end
    return cardinal_by_octant[octant]
end

local function movement_probe_is_blocked(probe)
    return probe.collision.blocks_route_fallback
end

-- ROM 0x14902 groups world coordinates in 0x200-unit cells. Lua retains
-- ordinary world positions and a short typed waypoint list, not FF1700 data.
local NAVIGATION_CELL_SIZE = 512
-- The direct 0x148AA/0x14854 probes have no descriptor-bound check in ROM.
-- 0x148AA and 0x14854 have no source scan-depth counter. Stop on the typed
-- collision query or the host's 16-bit world-coordinate boundary; do not add
-- a fixed grid-depth cap. The separate 0x14800 strip uses descriptor bounds.
local navigation_cardinal_offsets = {
    north = { x = 0, y = -1 },
    east = { x = 1, y = 0 },
    south = { x = 0, y = 1 },
    west = { x = -1, y = 0 },
}

local function navigation_cell_center(coordinate)
    return math.floor(coordinate / NAVIGATION_CELL_SIZE) *
        NAVIGATION_CELL_SIZE + NAVIGATION_CELL_SIZE // 2
end

local function choose_obstruction_waypoint(
    from_x, from_y, target_x, target_y, source_cell_is_clear, source_heading,
    source_strip_primary_cell_is_in_bounds)
    local delta_x = target_x - from_x
    local delta_y = target_y - from_y
    local absolute_x = math.abs(delta_x)
    local absolute_y = math.abs(delta_y)
    if absolute_x == 0 and absolute_y == 0 then
        return nil
    end
    if type(source_cell_is_clear) ~= "function" then
        return nil
    end

    -- ROM 0x148D2 selects the greater target axis, with X on ties. D7 is the
    -- dominant direction; D6 is the minor direction (0x148CA defaults it to
    -- -Y for horizontal routes and -X for vertical routes). 0x148AA scans
    -- along D7 to the first occupied cell and returns the preceding open
    -- cell; 0x146C8..0x146E2 queues that result [C]. 0x148D2 compares the
    -- absolute axis deltas in D5, restores the actor XY pair at 0x148FE, then
    -- falls through to 0x14902, which overwrites D5 with the actor's grid
    -- index at 0x14910. 0x146B8 rejects a blocked origin before any scan [C].
    local horizontal = absolute_x >= absolute_y
    local major_direction
    local minor_direction
    if horizontal then
        major_direction = {
            x = delta_x > 0 and 1 or -1,
            y = 0,
        }
        minor_direction = {
            x = 0,
            y = delta_y > 0 and 1 or -1,
        }
    else
        major_direction = {
            x = 0,
            y = delta_y > 0 and 1 or -1,
        }
        minor_direction = {
            x = delta_x > 0 and 1 or -1,
            y = 0,
        }
    end
    local origin_cell_x = navigation_cell_center(from_x)
    local origin_cell_y = navigation_cell_center(from_y)
    if source_cell_is_clear(origin_cell_x, origin_cell_y) ~= true then
        return nil
    end
    local target_cell_x = navigation_cell_center(target_x)
    local target_cell_y = navigation_cell_center(target_y)

    local function inside_world(x, y)
        return x >= 0 and x <= 0xFFFF and y >= 0 and y <= 0xFFFF
    end
    local function new_route(waypoints)
        return {
            waypoints = waypoints,
            target_cell_x = target_cell_x,
            target_cell_y = target_cell_y,
            strategy = "dominant_axis_bypass",
        }
    end

    -- The caller reaches 0x14682 after a blocked movement pass. The selector
    -- scans map cells directly; 0x14914 is only used later by 0x149C0 to
    -- validate an already-active route. Keep those as separate host facts.
    -- 0x148AA tests individual cells until nonzero, then returns the previous
    -- cell (0x148B6..0x148C8).
    local last_open_major_x
    local last_open_major_y
    local cell_offset = 1
    while true do
        local major_x = origin_cell_x +
            major_direction.x * cell_offset * NAVIGATION_CELL_SIZE
        local major_y = origin_cell_y +
            major_direction.y * cell_offset * NAVIGATION_CELL_SIZE
        if not inside_world(major_x, major_y) or
           source_cell_is_clear(major_x, major_y) ~= true then
            break
        end
        last_open_major_x, last_open_major_y = major_x, major_y
        cell_offset = cell_offset + 1
    end
    if last_open_major_x ~= nil then
        return new_route({ {
            x = last_open_major_x,
            y = last_open_major_y,
        } })
    end

    -- With the first D7 step blocked, ROM 0x14854 scans D6, first in its
    -- signed direction and then in the opposite direction. It selects the
    -- first open D6 cell whose D7 neighbor is also open (0x14864..0x1487C),
    -- and the caller queues that pair at 0x146FE..0x14702. The typed cell
    -- query keeps FF1700 and its source-grid storage out of Lua. Bounds remain
    -- a host policy for these unchecked source probes [H].
    local function find_minor_bypass(direction)
        local cell_offset = 1
        while true do
            local candidate_x = origin_cell_x +
                direction.x * cell_offset * NAVIGATION_CELL_SIZE
            local candidate_y = origin_cell_y +
                direction.y * cell_offset * NAVIGATION_CELL_SIZE
            if not inside_world(candidate_x, candidate_y) or
               source_cell_is_clear(candidate_x, candidate_y) ~= true then
                return nil
            end

            local side_x = candidate_x +
                major_direction.x * NAVIGATION_CELL_SIZE
            local side_y = candidate_y +
                major_direction.y * NAVIGATION_CELL_SIZE
            if inside_world(side_x, side_y) and
               source_cell_is_clear(side_x, side_y) == true then
                return new_route({
                    { x = candidate_x, y = candidate_y },
                    { x = side_x, y = side_y },
                })
            end
            cell_offset = cell_offset + 1
        end
    end

    local bypass = find_minor_bypass(minor_direction)
    if bypass ~= nil then
        return bypass
    end
    bypass = find_minor_bypass({
        x = -minor_direction.x,
        y = -minor_direction.y,
    })
    if bypass ~= nil then
        return bypass
    end

    -- ROM 0x14AF4 retries five heading-rotated strips. Each 0x14800 scan
    -- advances one primary cell until a primary collision or descriptor edge;
    -- five is the number of direction pairs, not a scan-depth limit. The
    -- descriptor-bound query is separate from the direct FF1700 map facts.
    -- Direction order, descriptor comparisons, and returning the previous
    -- primary cell are [C: 0x14812..0x14852, 0x14AF4].
    local function route_to_last_open_strip_cell(x, y)
        if navigation_cell_center(x) == origin_cell_x and
           navigation_cell_center(y) == origin_cell_y then
            return nil
        end
        return {
            waypoints = { { x = x, y = y } },
            target_cell_x = target_cell_x,
            target_cell_y = target_cell_y,
            strategy = "heading_oriented_strip",
        }
    end
    local heading_octant = type(source_heading) == "number" and
        Bearing.octant(source_heading, false) or nil
    local facing_octant = heading_octant ~= nil and
        (heading_octant & 0x6) or nil
    if facing_octant ~= nil and
       type(source_strip_primary_cell_is_in_bounds) == "function" then
        for rotation = 0, 4 do
            local primary_octant = (facing_octant - rotation * 2) & 0x7
            local secondary_octant = (primary_octant + 2) & 0x7
            local primary_direction =
                navigation_cardinal_offsets[
                    cardinal_by_octant[primary_octant]]
            local secondary_direction =
                navigation_cardinal_offsets[
                    cardinal_by_octant[secondary_octant]]
            local scan_from_x, scan_from_y = from_x, from_y
            local cell_depth = 1
            while true do
                local waypoint_x = origin_cell_x +
                    primary_direction.x * cell_depth * NAVIGATION_CELL_SIZE
                local waypoint_y = origin_cell_y +
                    primary_direction.y * cell_depth * NAVIGATION_CELL_SIZE
                if waypoint_x < 0 or waypoint_x > 0xFFFF or
                   waypoint_y < 0 or waypoint_y > 0xFFFF then
                    -- ROM 0x14800 backs D1 up by one primary step when the
                    -- next grid coordinate leaves the active map. 0x14AF4
                    -- accepts that last in-map cell unless it is still the
                    -- starting cell (0x14B22..0x14B48) [C].
                    local route = route_to_last_open_strip_cell(
                        scan_from_x, scan_from_y)
                    if route ~= nil then return route end
                    break
                end
                if source_strip_primary_cell_is_in_bounds(
                       waypoint_x, waypoint_y) ~= true then
                    -- ROM 0x14816..0x14836 returns the previous D1 when the
                    -- next primary cell exceeds the selected descriptor's
                    -- width or Y upper bound; 0x14B22..0x14B48 accepts that
                    -- previous cell when it differs from the origin [C].
                    local route = route_to_last_open_strip_cell(
                        scan_from_x, scan_from_y)
                    if route ~= nil then return route end
                    break
                end
                if source_cell_is_clear(waypoint_x, waypoint_y) ~= true then
                    -- 0x14800 likewise returns the preceding primary cell
                    -- when that cell is occupied (0x1483A..0x14852) [C].
                    -- Retain the last traversed cell rather than discarding
                    -- this source fallback.
                    local route = route_to_last_open_strip_cell(
                        scan_from_x, scan_from_y)
                    if route ~= nil then return route end
                    break
                end
                scan_from_x, scan_from_y = waypoint_x, waypoint_y
                local adjacent_x = waypoint_x +
                    secondary_direction.x * NAVIGATION_CELL_SIZE
                local adjacent_y = waypoint_y +
                    secondary_direction.y * NAVIGATION_CELL_SIZE
                if adjacent_x >= 0 and adjacent_x <= 0xFFFF and
                   adjacent_y >= 0 and adjacent_y <= 0xFFFF and
                   source_cell_is_clear(adjacent_x, adjacent_y) == true then
                    return {
                        waypoints = { { x = waypoint_x, y = waypoint_y } },
                        target_cell_x = target_cell_x,
                        target_cell_y = target_cell_y,
                        strategy = "heading_oriented_strip",
                    }
                end
                cell_depth = cell_depth + 1
            end
        end
    end
    return nil
end

local function current_world_target(context, state)
    if type(state.ai_target_id) ~= "string" or
       type(context.ai_target_candidates) ~= "table" then
        return nil
    end
    for _, candidate in ipairs(context.ai_target_candidates) do
        if candidate.id == state.ai_target_id and
           type(candidate.world_x) == "number" and
           type(candidate.world_y) == "number" then
            return candidate
        end
    end
    return nil
end

local function current_flee_boundary_target(state)
    local actor_facts = state.actor_facts
    local behavior = type(actor_facts) == "table" and actor_facts.behavior
    -- ROM 0x1397A dispatches an already-fleeing actor to 0x13E8C without a
    -- projection check. 0x13E9E leaves +0x96/+0x98 unchanged once the flee
    -- state is set, so that stored destination remains the movement target
    -- after the actor leaves the visible projection [C].
    if type(behavior) == "table" and behavior.fleeing == true and
       type(state.flee_target_x) == "number" and
       type(state.flee_target_y) == "number" then
        return {
            id = "flee-boundary",
            world_x = state.flee_target_x,
            world_y = state.flee_target_y,
        }
    end
    return nil
end

local function current_obstruction_target(context, state)
    -- The flee branch writes its own boundary destination to +0x96/+0x98 at
    -- 0x13E9E, replacing any earlier combat target before shared movement;
    -- later offscreen passes retain that same target while +0xB0 bit 6 is set.
    local flee_target = current_flee_boundary_target(state)
    if flee_target ~= nil then
        return flee_target
    end

    local target = current_world_target(context, state)
    if target ~= nil then
        return target
    end

    -- ROM 0x142AE places ordinary source patrol waypoints in the same
    -- +0x96/+0x98 movement target used by 0x145A2. Their stable actor ID maps
    -- to extracted route data; keep that target in Lua state rather than
    -- requiring the dynamic-combat target-candidate interface.
    if not ActorPatrolRoutes.has_route(context.actor_id) then
        return nil
    end
    local route_target = state.ai_route_target
    if type(route_target) ~= "table" or
       type(route_target.x) ~= "number" or
       type(route_target.y) ~= "number" then
        return nil
    end
    return {
        id = "patrol-route:" .. tostring(state.ai_waypoint_index or 0),
        world_x = route_target.x,
        world_y = route_target.y,
    }
end

local function obstruction_routing_available(context, state)
    local actor_facts = context.actor_facts
    local lifecycle = type(actor_facts) == "table" and actor_facts.lifecycle
    local linked_world_target = type(lifecycle) == "table" and
        lifecycle.ai_runner == true and
        current_world_target(context, state) ~= nil
    return context.dynamic_world_encounter_actor == true or
        linked_world_target or
        current_flee_boundary_target(state) ~= nil or
        ActorPatrolRoutes.has_route(context.actor_id)
end

local function measure_and_store_destination(context, state, x, y)
    if type(context.measure_destination) ~= "function" then
        return false
    end
    local measured = context.measure_destination(x, y)
    if type(measured) ~= "table" or
       type(measured.bearing_degrees) ~= "number" or
       type(measured.distance) ~= "number" then
        return false
    end
    state.movement_bearing_degrees = measured.bearing_degrees
    state.movement_route_distance = measured.distance
    state.movement_current_bearing_degrees =
        R.approach_bearing_degrees(
            state.movement_current_bearing_degrees,
            state.movement_bearing_degrees)
    return true
end

local function apply_obstruction_waypoint(context, state, movement_state)
    local route = state.obstruction_route
    if not obstruction_routing_available(context, state) or
       type(route) ~= "table" or type(route.waypoints) ~= "table" then
        return false
    end
    local target = current_obstruction_target(context, state)
    if target == nil or target.id ~= route.target_id or
       navigation_cell_center(target.world_x) ~= route.target_cell_x or
       navigation_cell_center(target.world_y) ~= route.target_cell_y then
        state.obstruction_route = nil
        if target ~= nil then
            measure_and_store_destination(
                context, state, target.world_x, target.world_y)
        end
        return false
    end
    local source_path_is_clear = game.movement.source_path_is_clear
    if type(source_path_is_clear) == "function" and
       source_path_is_clear(
           movement_state.position_x, movement_state.position_y,
           target.world_x, target.world_y) == true then
        -- ROM 0x149C0 -> 0x149CE clears its route flags when 0x14914 finds
        -- either source corridor unobstructed. Rejoin the live target vector.
        state.obstruction_route = nil
        measure_and_store_destination(
            context, state, target.world_x, target.world_y)
        return false
    end
    local waypoint = route.waypoints[route.index]
    if type(route.index) ~= "number" or route.index < 1 or
       route.index % 1 ~= 0 or waypoint == nil then
        state.obstruction_route = nil
        measure_and_store_destination(
            context, state, target.world_x, target.world_y)
        return false
    end
    while waypoint ~= nil and
          navigation_cell_center(movement_state.position_x) ==
              navigation_cell_center(waypoint.x) and
          navigation_cell_center(movement_state.position_y) ==
              navigation_cell_center(waypoint.y) do
        route.index = route.index + 1
        waypoint = route.waypoints[route.index]
    end
    if waypoint == nil then
        state.obstruction_route = nil
        -- 0x149C0 returns to the live target heading when its route queue
        -- reaches the terminator. Re-measure the current target so movement
        -- does not keep the bearing of the last detour point [C].
        measure_and_store_destination(
            context, state, target.world_x, target.world_y)
        return false
    end
    if not measure_and_store_destination(
           context, state, waypoint.x, waypoint.y) then
        state.obstruction_route = nil
        measure_and_store_destination(
            context, state, target.world_x, target.world_y)
        return false
    end
    return true
end

local function plan_obstruction_waypoint(context, state, movement_state)
    if not obstruction_routing_available(context, state) then
        return false
    end
    local target = current_obstruction_target(context, state)
    if target == nil then
        return false
    end
    local waypoint = choose_obstruction_waypoint(
        movement_state.position_x, movement_state.position_y,
        target.world_x, target.world_y,
        game.movement.source_cell_is_clear,
        -- ROM 0x14AF4 rotates from actor +0x34, the measured destination
        -- heading written at 0x145FC. +0x2E is the actor's current facing and
        -- may still be slewing toward that destination.
        state.movement_bearing_degrees,
        game.movement.source_strip_primary_cell_is_in_bounds)
    if waypoint == nil then
        -- ROM 0x14ADE reaches 0x14756 after the rotating-strip candidates
        -- fail; that path clears both obstruction-route flags. Drop the
        -- previous Lua queue and resume measuring the live target so a stale
        -- waypoint cannot keep pulling the actor toward a blocked cell. [C]
        state.obstruction_route = nil
        measure_and_store_destination(
            context, state, target.world_x, target.world_y)
        return false
    end
    waypoint.target_id = target.id
    waypoint.index = 1
    state.obstruction_route = waypoint
    local first_waypoint = waypoint.waypoints[1]
    if first_waypoint == nil then
        state.obstruction_route = nil
        return false
    end
    if not measure_and_store_destination(
           context, state, first_waypoint.x, first_waypoint.y) then
        state.obstruction_route = nil
        return false
    end
    state.movement_active = true
    return true
end

local function commit_movement_candidate(movement_state, probe)
    movement_state.position_x = probe.candidate_x
    movement_state.position_y = probe.candidate_y
    movement_state.clear_pending_event = true
end

local function reset_blocked_movement(context, state, movement_state)
    plan_obstruction_waypoint(context, state, movement_state)
    movement_state.route_facts.path_active = false
    movement_state.bearing_degrees = state.movement_bearing_degrees
    movement_state.mark_reset_after_block = true
    movement_state.route_update = { kind = "reset" }
    state.movement_current_bearing_degrees = state.movement_bearing_degrees
    local direction = R.direction_from_bearing_degrees(movement_state.bearing_degrees)
    state.direction = direction
    state.animation_id = "idle"
    -- B442 restores the facing and clears pending movement while preserving
    -- the current frame and Lua-owned animation progress for the render pass.
    local collision = game.movement.probe(
        movement_state.position_x, movement_state.position_y)
    movement_state.terrain_is_clear = type(collision) == "table" and
        collision.terrain_is_clear == true
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
    local movement_state = {}
    for key, value in pairs(context.movement_state) do
        movement_state[key] = value
    end
    movement_state.bearing_degrees = state.movement_current_bearing_degrees
    movement_state.animation_bearing_degrees = movement_state.bearing_degrees
    movement_state.substate_update = "unchanged"
    movement_state.cardinal_fallback_direction = nil
    movement_state.route_update = { kind = "unchanged" }
    movement_state.mark_reset_after_block = false
    movement_state.mark_directional_fallback = false
    movement_state.clear_collision_recovery = false
    movement_state.clear_pending_event = false
    apply_obstruction_waypoint(context, state, movement_state)
    movement_state.bearing_degrees = state.movement_current_bearing_degrees
    movement_state.animation_bearing_degrees = movement_state.bearing_degrees

    local movement_step_distance =
        creature_movement_step_distance(context, state)
    if type(movement_step_distance) ~= "number" or
       movement_step_distance <= 0 then
        return nil
    end

    if context.dynamic_world_encounter_actor == true then
        movement_state.collision_extent =
            context.visual_profile_id == VisualProfile.GARGOYLE and
            context.ai_animation_frame == 2 and -20 or 0
    else
        movement_state.collision_extent = -12
    end

    local octant_for_heading = movement_octant_selector(context)
    local initial_direction = octant_for_heading(movement_state.bearing_degrees)
    if initial_direction == nil then
        return nil
    end
    local initial_direction_name = direction_name(initial_direction)
    local initial = probe_movement(
        movement_state, initial_direction_name, movement_step_distance)
    if initial == nil then
        return nil
    end

    if initial.collision.kind == "clear" then
        movement_state.substate_update = "direct_translation"
        commit_movement_candidate(movement_state, initial)
        state.animation_id = "walk"
        state.direction = initial_direction
        return {
            outcome = "accepted",
            state = movement_state,
        }
    end

    if initial.collision.kind == "requires_recovery" then
        local recovered, route_transition = game.movement.recover_collision(
            initial.candidate_x, initial.candidate_y,
            initial.direction)
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
        local route_direction = route_direction_for_bearing_degrees(
            movement_state.bearing_degrees)
        if route_direction == nil then
            return nil
        end
        local route = game.movement.route(route_direction)
        local fallback
        local carry_set = false
        if not (route.primary_route_marker_present and
                route.secondary_route_marker_present) then
            local fallback_direction =
                not route.primary_route_marker_present and
                route.primary_fallback_direction or
                route.secondary_fallback_direction
            -- ROM 0xBBC8's descriptor becomes a compass direction at the
            -- native route boundary, so Lua can make the choice by name.
            fallback = probe_movement(
                movement_state, fallback_direction, movement_step_distance)
            if fallback == nil then
                return nil
            end
            carry_set = movement_probe_is_blocked(fallback)
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
    local cardinal_id = cardinal_by_octant[octant]
    if cardinal_id == nil then
        return nil
    end
    movement_state.substate_update = "cardinal_fallback"
    movement_state.cardinal_fallback_direction = cardinal_id
    local selected = probe_movement(
        movement_state, cardinal_id, movement_step_distance)
    if selected == nil then
        return nil
    end
    if movement_probe_is_blocked(selected) then
        movement_state.route_update = {
            kind = "blocked_by_direction",
            direction = cardinal_id,
        }
        local alternate_direction = next_cardinal_direction[cardinal_id]
        selected = probe_movement(
            movement_state, alternate_direction, movement_step_distance)
        if selected == nil then
            return nil
        end
        if movement_probe_is_blocked(selected) then
            local reset = reset_blocked_movement(
                context, state, movement_state)
            state.animation_id = "idle"
            return reset
        end
    end

    movement_state.animation_bearing_degrees =
        direction_bearings[selected.direction]
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
       movement_step.state.terrain_is_clear == true then
        -- ROM 0xAC5E clears +0xB0 bit 5 only after FFDF64 becomes zero.
        -- The native collision service exposes that as a named terrain fact.
        movement_step.state.route_facts.route_transition = false
    end
    return movement_step
end


-- register this module's definitions
R.projectile_movement_snapshot = projectile_movement_snapshot
R.projectile_heartbeat = projectile_heartbeat
R.requested_movement_bearing_degrees = requested_movement_bearing_degrees
R.resolve_source_target_motion = resolve_source_target_motion
R.walk_direction_from_bearing_degrees = walk_direction_from_bearing_degrees
R.movement_octant_selector = movement_octant_selector
R.movement_direction_from_bearing_degrees = movement_direction_from_bearing_degrees
R.creature_movement_step_distance = creature_movement_step_distance
R.probe_movement = probe_movement
R.route_direction_for_bearing_degrees = route_direction_for_bearing_degrees
R.movement_probe_is_blocked = movement_probe_is_blocked
R.choose_obstruction_waypoint = choose_obstruction_waypoint
R.commit_movement_candidate = commit_movement_candidate
R.reset_blocked_movement = reset_blocked_movement
R.advance_entity_movement = advance_entity_movement
R.finish_entity_movement = finish_entity_movement
R.quickness_cardinal_steps = quickness_cardinal_steps
return R
