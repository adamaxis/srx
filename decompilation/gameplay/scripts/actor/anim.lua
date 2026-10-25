-- actor/anim.lua: presentation: snapshots, pose selectors, entry/hit/walk/action animations.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local Animator = R.Animator   -- from actor/animator.lua
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local SpriteType = R.SpriteType   -- from actor/tables.lua
local function direction_from_heading(...) return R.direction_from_heading(...) end   -- from actor/targeting.lua
local idle_pose_selectors = R.idle_pose_selectors   -- from actor/tables.lua
local walk_pose_selectors = R.walk_pose_selectors   -- from actor/tables.lua
local weapon_projectile_descriptor_increment = R.weapon_projectile_descriptor_increment   -- from actor/tables.lua

local function snapshot(state, action, audio_event, projectile_movement,
                        target_heading_audio_event)
    local world_activity_requested = state.world_activity_requested == true
    local field_event_request = state.field_event_request
    local think_token_claimed = state.think_token_claimed == true
    local source_thon_period_counter = state.source_thon_period_counter
    state.world_activity_requested = false
    state.field_event_request = nil
    state.think_token_claimed = false
    state.source_thon_period_counter = nil
    if state.animation_id == "idle" then
        state.animation_frame = 0
        state.source_animation_selector =
            idle_pose_selectors[state.direction + 1]
    elseif state.animation_id == "walk" then
        state.source_animation_selector =
            walk_pose_selectors[state.direction + 1]
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
        source_spawn_state_flags = state.source_spawn_state_flags or 0,
        animation_selector = state.animation_selector,
        direction = state.direction,
        frame = state.animation_frame,
        animation_accumulator = state.animation_accumulator,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        source_behavior_flags = state.source_behavior_flags,
        world_activity_requested = world_activity_requested,
        field_event_request = field_event_request,
        think_token_claimed = think_token_claimed,
        source_thon_period_counter = source_thon_period_counter,
        source_animation_selector = state.source_animation_selector,
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
        spell_effect_slot_released = state.spell_effect_slot_released,
        spell_effect_completed = state.spell_effect_completed,
        spell_effect_visible = state.spell_effect_visible,
        spell_effect_phase = state.spell_effect_phase,
        spell_effect_action_code = state.spell_effect_action_code,
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
        spell_action_resolution = state.spell_action_resolution,
        spell_effect_resolution = state.spell_effect_resolution,
        source_action_dispatch = state.source_action_dispatch,
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

    -- 0x5FDC with the previous +0x18, then 0x600E's finite clamp. The sixteen
    -- weapon-effect lists use 0x8003 or 0x800B markers and 0x270F cadence.
    if Animator.step_finite(state, animation.frame_count,
                            state.animation_increment, state.animation_period) then
        state.animation_selector = state.animation_selector | 0x8000
    end
    state.animation_increment = weapon_projectile_descriptor_increment
    state.animation_id = "projectile"
    return true
end

-- ROM 0xC0B2 death list (pose 0xE8) or a 0xB0AC/0xB0E4 spell-death list
-- (pose 0xE0/0xE4): finite, timed by that list's own +2 rate for this kind
-- (0x200 for kinds 3..14, 0x80 for THON) [C 0x62AC]. Returns rate, count.
local function death_list(state)
    local list = Animator.list(state.entity_kind,
        state.spell_death_pose or Animator.Pose.DEATH)
    if list == nil then
        return 0x0200, nil
    end
    return list.rate, list.count
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
    if Animator.step_finite(state, frame_count, (death_list(state))) then
        state.transition_frame_list_finished = true
    end
end

local function advance_finite_actor_animation(state, frame_count,
                                              increment, threshold)
    if state.damage_reaction_animation_complete then
        return true
    end
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 16 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold < 1 or threshold > 0xffff then
        return false
    end
    -- 0x600E sets the list-finished selector and holds its last frame.
    if Animator.step_finite(state, frame_count, increment, threshold) then
        state.damage_reaction_animation_complete = true
    end
    return true
end

-- ROM 0xB330 (death/down): B05E[kind] cue, +0x10 bit 4, +0x3A = 0xB38A,
-- pose C0B2[dir8] (0xE8 + 4*dir), 0xAF6C resets +0x1A/+0x1E. Callers:
-- 0xAFF2 hit with +0x4C == 0, 0xAA4C service with +0x4C or +0x9C == 0,
-- 0xE4BA kill_leader. It is never selected at spawn.
local function begin_down_reaction(state, context)
    if type(context.entry_animation_frame_count) ~= "number" or
       context.entry_animation_frame_count < 1 or
       context.entry_animation_frame_count > 8 or
       type(context.down_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_id = "down"
    state.animation_selector = 0xe8 + state.direction * 4
    state.source_record_flags = state.source_record_flags | ActorStatus.DOWN
    state.hurt_behavior_timer = 0
    state.movement_active = false
    return context.down_audio_event
end

-- ROM 0xA93A..0xAA4C, run on every 0xA8EA service pass: when +0x9C (stun)
-- or +0x4C (physical) is zero and the pose is idle/walk (+0x16 <= 0x3C) or
-- the current finite list has finished (+0x16 bit 15), the first pass sets
-- +0x10 bit 4 and enters 0xB330. This is how stun kills (mental meter 0) and
-- deferred zero-meter hits die; the hit path alone only checks +0x4C.
local function service_zero_meter_down(state, context)
    if (state.source_record_flags & ActorStatus.DOWN) ~= 0 then
        return nil
    end
    -- The host's per-pass record (context.combatant) holds the live +0x4C /
    -- +0x9C meters; state.combatant is only the spawn copy.
    local c = type(context.combatant) == "table" and context.combatant or
              state.combatant
    if type(c) ~= "table" or type(c.physical_meter) ~= "number" or
       type(c.mental_meter) ~= "number" then
        return nil
    end
    if c.physical_meter ~= 0 and c.mental_meter ~= 0 then
        return nil
    end
    local id = state.animation_id
    local pose_open = id == "idle" or id == "walk" or
        (id == "hurt" and state.damage_reaction_animation_complete) or
        (id == "action" and state.action_animation_complete) or
        (id == "weapon" and state.item_action_complete) or
        id == "spell"
    if not pose_open then
        return nil
    end
    return begin_down_reaction(state, context)
end

local function begin_received_hit_reaction(state, context)
    local hit = state.incoming_hit
    if type(hit) ~= "table" or
       type(hit.physical_meter) ~= "number" or
       type(hit.mental_meter) ~= "number" then
        return nil
    end
    state.incoming_hit = nil
    if (state.source_record_flags & ActorStatus.DOWN) ~= 0 then
        return nil
    end
    if hit.physical_meter == 0 and hit.spell_death_pose ~= nil and
       (state.source_record_flags & ActorStatus.AI_RUNNER) == 0 and
       context.entity_kind ~= SpriteType.HELL_HOUND then
        -- 0xB0AC/0xB0E4 (spell death): pose 0xE0/0xE4, continuation
        -- 0xB0D4/0xB10C, no +0x10 b4 and no cue. 0xA9A6 then runs 0xAB02 and
        -- 0x1E514 when the list finishes. Kinds 12..15 point 0xE0/0xE4 at
        -- their C0B2 list; kinds 3..11 use 10 (0xE0) / 9 (0xE4) frames at
        -- 0x280 whose art is not extracted, so the C0B2 art and cadence
        -- stand in [H: humanoid spell-death art gap]. Unreachable live:
        -- player spells on world actors are not ported.
        if type(context.entry_animation_frame_count) ~= "number" or
           context.entry_animation_frame_count < 1 or
           context.entry_animation_frame_count > 8 then
            return nil
        end
        state.damage_reaction_active = true
        state.damage_reaction_animation_complete = false
        state.animation_frame = 0
        state.animation_accumulator = 0
        state.animation_id = "down"
        state.animation_selector = hit.spell_death_pose
        state.spell_death = true
        state.spell_death_pose = hit.spell_death_pose
        state.hurt_behavior_timer = 0
        state.movement_active = false
        return nil
    end
    if hit.physical_meter == 0 then
        -- 0xAFF2: zero +0x4C enters 0xB330 directly, whatever the pose.
        -- (0xB0AC/0xB0E4 send +0x10 b3 records and kind 0xC here too.)
        return begin_down_reaction(state, context)
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

    -- ROM 0x6092 looping-list rate from +0xCA (the movement vector's timing
    -- word); the positive C062 list wraps at its source marker.
    local increment = Animator.loop_rate(state.entity_kind,
        state.animation_frame, state.animation_timing_word)
    if increment == nil then
        return false
    end
    state.animation_increment = increment
    state.animation_period = Animator.PERIOD
    state.animation_selector = Animator.Pose.WALK + state.direction * 4
    Animator.step_loop(state, frame_count, increment, state.animation_period)
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
    -- C0A2 list: 0x600E holds the last frame until a new list is selected.
    if Animator.step_finite(state, frame_count, increment, threshold) then
        state.action_animation_complete = true
    end
    return true
end

local function advance_dynamic_actor_item_action_animation(state)
    if state.animation_id ~= "weapon" or state.item_action_complete then
        return true
    end
    local frame_count = state.item_action_frame_count
    local increment = state.item_action_increment
    local threshold = state.item_action_threshold
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 8 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold ~= Animator.PERIOD then
        return false
    end
    -- C042 list: finite, marks pose bit 15 when done.
    if Animator.step_finite(state, frame_count, increment, threshold) then
        state.source_animation_selector =
            state.source_animation_selector | 0x8000
        state.item_action_complete = true
    end
    return true
end

-- ROM 0xC082. AF7C/AFAE index this by the same 0xB36A direction as C0A2.
local spell_pose_selectors = {
    0x80, 0x84, 0x88, 0x8C, 0x90, 0x94, 0x98, 0x9C,
}

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
        -- C042 (pose 0x40) for kind 3 is one frame; the extracted art is
        -- attack-<dir>.bmp. Its 0x80 cadence only drives pose bit 15, which
        -- the C++ attack-pose timer owns [H: not ported here].
        state.animation_id = "attack"
        state.animation_selector = nil
        state.animation_frame = 0
        state.animation_accumulator = 0
        return
    end

    -- Kind 3 idle/walk lists loop, so 0x6092 times them from +0xCA = 0x28
    -- (C002+4 via 0xBABC/0xBADE): 0x28 * 0x10 = 0x280 [C].
    local kind = SpriteType.FIELD_RUNNER -- 3
    local rate = Animator.loop_rate(kind, state.animation_frame,
                                    Animator.PLAYER_SPEED)
    if context.animation_mode == "walking" then
        state.animation_id = "walk"
        state.animation_selector = Animator.Pose.WALK + state.direction * 4
        Animator.step_loop(state,
            Animator.list(kind, Animator.Pose.WALK).count, rate)
    else
        state.animation_id = "idle"
        state.animation_selector = Animator.Pose.IDLE + state.direction * 4
        Animator.step_loop(state,
            Animator.list(kind, Animator.Pose.IDLE).count, rate)
    end
end


-- register this module's definitions
R.snapshot = snapshot
R.advance_projectile_animation = advance_projectile_animation
R.advance_entry_animation = advance_entry_animation
R.death_list = death_list
R.advance_finite_actor_animation = advance_finite_actor_animation
R.begin_received_hit_reaction = begin_received_hit_reaction
R.begin_down_reaction = begin_down_reaction
R.service_zero_meter_down = service_zero_meter_down
R.advance_received_hit_reaction = advance_received_hit_reaction
R.advance_dynamic_actor_walk_animation = advance_dynamic_actor_walk_animation
R.advance_dynamic_actor_action_animation = advance_dynamic_actor_action_animation
R.advance_dynamic_actor_item_action_animation = advance_dynamic_actor_item_action_animation
R.advance_player_animation = advance_player_animation
R.spell_pose_selectors = spell_pose_selectors
return R
