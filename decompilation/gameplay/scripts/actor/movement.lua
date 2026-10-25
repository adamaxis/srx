-- actor/movement.lua: Lua movement policy, projectile integration, and collision probes.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local BehaviorFlag = R.BehaviorFlag   -- from actor/tables.lua
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local ActorFlag = R.ActorFlag   -- from actor/tables.lua
local SpriteType = R.SpriteType   -- from actor/tables.lua
local byte_mask = R.byte_mask   -- from actor/tables.lua
local byte_modulus = R.byte_modulus   -- from actor/tables.lua
local fixed_fraction_scale = R.fixed_fraction_scale   -- from actor/tables.lua
local fixed_integer_shift = R.fixed_integer_shift   -- from actor/tables.lua
local fixed_long_mask = R.fixed_long_mask   -- from actor/tables.lua
local projectile = R.projectile   -- from actor/tables.lua
local signed_byte_threshold = R.signed_byte_threshold   -- from actor/tables.lua

-- 68000 helpers now shared via lib/int68k.lua (lua_review #4); names kept for call-site compatibility.
local signed_byte = I.s8
local signed_word_from_long = I.s32
local integer_world_word = I.fixed_int_word

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
       type(source.heading) ~= "number" or source.heading < 0 or
       source.heading > AiTuning.WORD_MAX or
       type(source.saved_position_x) ~= "number" or
       source.saved_position_x < 0 or source.saved_position_x > AiTuning.WORD_MAX or
       type(source.saved_position_y) ~= "number" or
       source.saved_position_y < 0 or source.saved_position_y > AiTuning.WORD_MAX or
       type(source.animation_selector) ~= "number" or
       source.animation_selector < 0 or
       source.animation_selector > AiTuning.WORD_MAX then
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
        heading = source.heading,
        saved_position_x = source.saved_position_x,
        saved_position_y = source.saved_position_y,
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
                -- ROM 0x14F86 starts the next member of the three-round
                -- weapon burst at the saved spawn coordinate. It rotates the
                -- heading by -0x400 for the second shot and +0x800 for the
                -- third, then derives the 0x150E8 movement offsets.
                result.heading = u16(result.heading +
                    (result.linked_action_timer ~= 0 and -0x0400 or 0x0800))
                result.world_x = signed_word_from_long(
                    (result.saved_position_x << fixed_integer_shift) |
                    (result.world_x & AiTuning.WORD_MAX))
                result.world_y = signed_word_from_long(
                    (result.saved_position_y << fixed_integer_shift) |
                    (result.world_y & AiTuning.WORD_MAX))
                local entity_motion = game.modules.entity_motion
                local offsets = type(entity_motion) == "table" and
                    type(entity_motion.heading_offsets) == "function" and
                    entity_motion.heading_offsets(result.heading) or nil
                if type(offsets) ~= "table" or
                   type(offsets.horizontal) ~= "number" or
                   type(offsets.vertical) ~= "number" then
                    return nil
                end
                result.tile_offset_x = offsets.horizontal
                result.tile_offset_y = offsets.vertical
                result.retry_counter = 0x20
                result.animation_selector = 0x14
                result.record_flags = result.record_flags &
                                      ~projectile.flags.stopped_record
                result.outcome = projectile.outcome.burst_followup_restarted
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
                u8(result.retry_counter - 1)
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
                    (probe_x & AiTuning.WORD_MAX))
                result.world_y = signed_word_from_long(
                    (source_y << fixed_integer_shift) |
                    (probe_y & AiTuning.WORD_MAX))
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
    if movement.outcome == projectile.outcome.burst_followup_restarted then
        local result = game.modules.combat.resolve_weapon_attack_seed(context)
        if type(result) ~= "number" then
            return nil
        end
        state.attack_seed_resolved = true
        state.action_result = result
        movement.action_result = result
        -- ROM 0x14F86 clears +0x18 and +0x1E before the next render pass.
        state.animation_frame = 0
        state.animation_accumulator = 0
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
    return R.snapshot(state, "hold", audio_event, movement)
end

local function requested_movement_heading(context, profile, state)
    if profile.dynamic_encounter_only then
        local target = R.linked_target(context, state)
        local heading = target and target.heading or nil
        local distance = target and target.distance or nil
        -- Keep the selected source actor relation explicit in Lua state.
        -- The host may render and inspect this R.snapshot but does not choose
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
    if type(heading) ~= "number" or heading < 0 or
       heading >= AiTuning.WORD_MAX then
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

    local heading = target.heading & AiTuning.WORD_MAX
    local distance = target.distance & AiTuning.WORD_MAX
    local accepted = false

    if context.entity_kind == SpriteType.HELL_HOUND then
        -- ROM 0x14D04: inside 0x300, nonnegative +0xDC accepts the target
        -- heading; negative +0xDC first needs a free effect slot and then
        -- replaces both +0x2E and +0x34. From 0x300 through 0x5FF both
        -- headings change immediately. At 0x600 the actor keeps pursuing.
        if distance < AiTuning.HELL_HOUND_CLOSE_RANGE then
            if state.action_delay < 0 then
                accepted = R.request_target_motion_effect(
                    state, context, heading, target)
                if accepted then
                    state.movement_current_heading = heading
                end
            else
                accepted = true
            end
        elseif distance < AiTuning.HELL_HOUND_PURSUIT_RANGE then
            state.movement_current_heading = heading
            accepted = true
        end
    elseif context.entity_kind == SpriteType.GARGOYLE or
           context.entity_kind == SpriteType.WENDIGO then
        -- 0x14C7C calls 0x15C84, whose 0x15C58 range test measures the
        -- linked record's actual coordinates and accepts only distance <
        -- 0x200. The preceding 0x14C56 projection result is not consumed by
        -- that test. Kind 0xD at frame +0x1E == 2 forces +0x1A to 0x1000,
        -- but clears carry, so the actor continues its route on that pass.
        if distance < 0x200 then
            if context.entity_kind == SpriteType.GARGOYLE and
               context.ai_animation_frame == 2 then
                state.animation_accumulator = 0x1000
            else
                state.movement_current_heading = heading
                accepted = true
            end
        end
    elseif distance < AiTuning.CLOSE_STRIKE_RANGE then
        -- Other kinds use 0x15C58 directly. That branch writes +0x34 but
        -- leaves +0x2E to the actor's own action continuation.
        accepted = true
    end

    if accepted then
        state.movement_heading = heading
        state.movement_active = false
        state.movement_route_distance = nil
        state.ai_target_engaged = true
        state.direction = R.direction_from_heading(heading)
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

local function movement_record_flags(context)
    -- +0x10. The runner publishes it as movement_entity_flags. Dynamic
    -- actors publish the same byte as source_record_flags.
    return context.dynamic_world_encounter_actor == true
        and context.source_record_flags
        or context.movement_entity_flags
end

local function walk_direction_from_heading(heading)
    -- ROM 0xB574 walk handler (0xB57E, 0xB5BE) and its walk animation pick
    -- (0xB6CC, C062): rol.w #3 then andi.w #7 with no round-up, so the
    -- octant is truncated. 0xB36A-family animation picks and the 0xBABC
    -- step round up instead (R.direction_from_heading) [C: disasm; emulator
    -- run5 FF012E 0x7800..0x6000 step diagonally, 0x5800 steps right].
    return (heading >> 13) & 0x7
end

local function movement_octant_selector(context)
    -- 0xACAE installs 0xB574 when +0x10 bit 1 (THINK) is set.
    if (movement_record_flags(context) & ActorStatus.THINK) ~= 0 then
        return walk_direction_from_heading
    end
    return R.direction_from_heading
end

local function creature_movement_offsets(context, state)
    local record_flags = movement_record_flags(context)
    local controlled_action = ActorStatus.THINK
    local walk_fast_selected = context.walk_fast_enabled == true and
        (record_flags & controlled_action) ~= 0
    -- Contexts without a live Quickness byte keep the host vector list.
    -- Walk Fast is an earlier ROM branch and does not require Quickness.
    local attribute_speed =
        context.dynamic_world_encounter_actor == true or
        (type(context.combatant) == "table" and
         type(context.combatant.command_attribute_78) == "number")
    if not attribute_speed and not walk_fast_selected then
        return context.movement_offsets
    end

    -- 0xACAE installs 0xB574 when +0x10 bit 1 is set. Bit 1 clear is the
    -- distance-band path at 0xB6E4.
    local targets_world =
        (record_flags & ActorStatus.AI_RUNNER) ~= 0
    local high_effect =
        (context.source_behavior_flags & BehaviorFlag.ESCORT) ~= 0
    local activity = context.world_activity_active == true
    local step

    if context.entity_kind == SpriteType.GARGOYLE then
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
        if (context.source_spawn_state_flags &
                ActorFlag.PEDESTRIAN) ~= 0 then
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
                local quickness = type(context.combatant) == "table" and
                    context.combatant.command_attribute_78 or nil
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
        if (record_flags & controlled_action) == 0 then
            local distance = state.movement_route_distance
            if type(distance) ~= "number" then
                return context.movement_offsets
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

    -- ROM 0xBADE copies the selected vector table's +4 word to actor +0xCA.
    state.animation_timing_word = step
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
    return offsets[pair][component] & AiTuning.WORD_MAX
end

local function probe_movement(context, movement_state, direction_slot,
                              movement_offsets)
    local direction = (direction_slot >> 1) & 0x7
    local offset = movement_offsets[direction + 1]
    local candidate_x = u16(movement_state.position_x + offset[1])
    local candidate_y = u16(movement_state.position_y + offset[2])
    local collision_code, terrain_collision_code = game.movement.collision(
        candidate_x, candidate_y)
    movement_state.terrain_collision_code = terrain_collision_code
    -- ROM 0xB77E maps detailed negative terrain results to 0x7F unless the
    -- actor's +0x10 bit 1 requests that negative values be preserved.
    if collision_code < 0 and (context.movement_entity_flags & 0x02) == 0 then
        collision_code = Collision.BLOCKED
    end
    return {
        collision_code = collision_code,
        candidate_x = candidate_x,
        candidate_y = candidate_y,
        direction_slot = direction_slot,
    }
end

local function route_key_for_heading(heading)
    local rotated = u16((heading << 3) | (heading >> 13))
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
    movement_state.route_flags = u8(movement_state.route_flags & ~BehaviorFlag.PATH)
    movement_state.heading = state.movement_heading
    movement_state.movement_flags = movement_state.movement_flags | 0x01
    movement_state.route_status = -1
    state.movement_current_heading = state.movement_heading
    local direction = R.direction_from_heading(movement_state.heading)
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
            context.entity_kind == SpriteType.GARGOYLE and
            context.ai_animation_frame == 2 and -20 or 0
    else
        movement_state.collision_extent = -12
    end

    local movement_octant = movement_octant_selector(context)
    local initial_direction = movement_octant(movement_state.heading)
    local initial_slot = initial_direction * 2
    movement_state.collision_context = movement_context_word(
        movement_offsets, initial_direction)
    local initial = probe_movement(
        context, movement_state, initial_slot, movement_offsets)

    if initial.collision_code == 0 then
        movement_state.movement_substate = 0x1000
        commit_movement_candidate(movement_state, initial)
        state.animation_id = "walk"
        state.direction = initial_direction
        return {
            outcome = "accepted",
            state = movement_state,
            direction_animation = 0x20 + context.movement_idle_selectors[
                initial_direction + 1],
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
        -- Lua movement R.snapshot; 0xAC5E clears it after clear terrain. The
        -- recovery result already carries the transition's separate movement
        -- outcome, so keep the other route-state bits in Lua's R.snapshot.
        movement_state.route_flags =
            u8(movement_state.route_flags & ~BehaviorFlag.ROUTE_TRANSITION) | (route_flags & BehaviorFlag.ROUTE_TRANSITION)
        if not recovered then
            movement_state.movement_substate = 0x1000
            commit_movement_candidate(movement_state, initial)
            state.animation_id = "walk"
            state.direction = R.direction_from_heading(movement_state.heading)
            return {
                outcome = "accepted",
                state = movement_state,
                direction_animation = 0x20 + context.movement_idle_selectors[
                    R.direction_from_heading(movement_state.heading) + 1],
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
            -- ROM 0xBBC8: the descriptor word is a direction ordinal;
            -- 0xBBC8 doubles it into the probe slot (0xB77E doubles again
            -- for the vector table) [C].
            fallback = probe_movement(
                context, movement_state, fallback_slot << 1, movement_offsets)
            carry_set = movement_probe_is_blocked(fallback, context)
        else
            carry_set = true
        end
        if not carry_set and fallback ~= nil then
            movement_state.collision_flags = movement_state.collision_flags & 0xfe
            commit_movement_candidate(movement_state, fallback)
            state.animation_id = "walk"
            state.direction = R.direction_from_heading(movement_state.heading)
            return {
                outcome = "accepted",
                state = movement_state,
                direction_animation = 0x20 + context.movement_idle_selectors[
                    R.direction_from_heading(movement_state.heading) + 1],
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
        u16((selected.direction_slot >> 4) |
         (selected.direction_slot << 12))
    movement_state.movement_flags = movement_state.movement_flags | 0x04
    commit_movement_candidate(movement_state, selected)
    state.animation_id = "walk"
    state.direction = R.direction_from_heading(movement_state.animation_heading)
    return {
        outcome = "accepted",
        state = movement_state,
        direction_animation = 0x20 + context.movement_idle_selectors[
            R.direction_from_heading(movement_state.animation_heading) + 1],
    }
end

local function finish_entity_movement(context, movement_step)
    if movement_step ~= nil and
       (context.movement_entity_flags & 0x08) ~= 0 and
       movement_step.state.terrain_collision_code == 0 then
        -- ROM 0xAC5E releases B7B8's retry bit after FFDF64 reports clear
        -- terrain, allowing a later doorway or terminal event to dispatch.
        movement_step.state.route_flags =
            u8(movement_step.state.route_flags & ~BehaviorFlag.ROUTE_TRANSITION)
    end
    return movement_step
end


-- register this module's definitions
R.signed_byte = signed_byte
R.signed_word_from_long = signed_word_from_long
R.integer_world_word = integer_world_word
R.projectile_movement_snapshot = projectile_movement_snapshot
R.projectile_heartbeat = projectile_heartbeat
R.requested_movement_heading = requested_movement_heading
R.resolve_source_target_motion = resolve_source_target_motion
R.movement_collision_code = movement_collision_code
R.movement_record_flags = movement_record_flags
R.walk_direction_from_heading = walk_direction_from_heading
R.movement_octant_selector = movement_octant_selector
R.creature_movement_offsets = creature_movement_offsets
R.movement_context_word = movement_context_word
R.probe_movement = probe_movement
R.route_key_for_heading = route_key_for_heading
R.movement_probe_is_blocked = movement_probe_is_blocked
R.commit_movement_candidate = commit_movement_candidate
R.reset_blocked_movement = reset_blocked_movement
R.advance_entity_movement = advance_entity_movement
R.finish_entity_movement = finish_entity_movement
R.quickness_cardinal_steps = quickness_cardinal_steps
return R
