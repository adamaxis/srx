-- actor/anim.lua: presentation: snapshots, pose selectors, entry/hit/walk/action animations.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local Animator = R.Animator   -- from actor/animator.lua
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local PlayerHurt = E.PlayerHurt
local HitSpark = E.HitSpark
local ActorAnimation = E.ActorAnimation
local ActorPose = E.ActorPose
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local SpriteType = R.SpriteType   -- from actor/tables.lua
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
    local strike_heading = state.strike_heading
    state.strike_heading = nil
    local hit_spark_spawn_request = state.hit_spark_spawn_request
    state.hit_spark_spawn_request = nil
    local result = {
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
        strike_heading = strike_heading,
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
        hit_spark_spawn_request = hit_spark_spawn_request,
    }
    if state.profile_id == "field_hit_spark" then
        result.hit_spark_retired = state.hit_spark_retired == true
    end
    return result
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
        state.spell_death_pose or Animator.Pose.DEATH, state.direction)
    return list
end

-- Player hit spark (kind 1): 0x15118 sets +0x1C to 0x1000 and 0xB2BC
-- installs selector 4, whose ROM list 0x7330 is finite (5 frames, +0x400).
-- The shared 0x5FDC/0x600E clock steps it; past the last entry 0x600E marks
-- +0x16 negative [C].
local function hit_spark_list()
    return Animator.list(SpriteType.HIT_SPARK, HitSpark.SELECTOR)
end

local function advance_hit_spark_animation(state, context)
    local animation = context.hit_spark_animation
    local list = hit_spark_list()
    if context.phase ~= "render" or type(animation) ~= "table" or
       list == nil or not list.finite or
       type(animation.selector) ~= "number" or
       animation.frame_count ~= list.count or
       animation.period ~= Animator.PERIOD then
        return false
    end
    state.animation_selector = animation.selector
    state.animation_period = animation.period
    state.animation_frame_count = list.count
    if Animator.step_finite(state, list.count, list.rate,
                            state.animation_period) then
        state.animation_selector = state.animation_selector |
            HitSpark.FINISHED_SELECTOR_BIT
    end
    state.animation_increment = list.rate
    state.animation_id = "hit_spark"
    return true
end

local function hit_spark_heartbeat(state, context)
    if context.phase == "render" then
        if not advance_hit_spark_animation(state, context) then
            return nil
        end
    elseif context.phase == "behavior" then
        -- ROM 0xAC4E clears +0x10 bit 0 after 0x600E marks +0x16 negative [C].
        state.hit_spark_retired =
            (state.animation_selector & HitSpark.FINISHED_SELECTOR_BIT) ~= 0
    else
        return nil
    end
    return snapshot(state, "hold", nil)
end

local function advance_entry_animation(state)
    if state.animation_id ~= "entry" or
       state.transition_frame_list_finished then
        return
    end
    local list = Animator.list(state.entity_kind, Animator.Pose.DEATH,
                               state.direction)
    if list == nil or not list.finite then
        return
    end
    if Animator.step_finite(state, list.count, list.rate,
                            Animator.PERIOD) then
        state.transition_frame_list_finished = true
    end
end

local function advance_finite_actor_animation(state, animation)
    if state.damage_reaction_animation_complete then
        return true
    end
    if type(animation) ~= "table" or animation.finite ~= true or
       type(animation.count) ~= "number" or animation.count < 1 or
       type(animation.rate) ~= "number" or animation.rate < 1 then
        return false
    end
    -- 0x600E sets the list-finished selector and holds its last frame.
    if Animator.step_finite(state, animation.count, animation.rate,
                            Animator.PERIOD) then
        state.damage_reaction_animation_complete = true
    end
    return true
end

-- ROM 0xB330 (death/down): B05E[kind] cue, +0x10 bit 4, +0x3A = 0xB38A,
-- pose C0B2[dir8] (0xE8 + 4*dir), 0xAF6C resets +0x1A/+0x1E. Callers:
-- 0xAFF2 hit with +0x4C == 0, 0xAA4C service with +0x4C or +0x9C == 0,
-- 0xE4BA kill_leader. It is never selected at spawn.
local function begin_down_reaction(state)
    local down_audio_event = Animator.down_audio(state.entity_kind)
    if type(down_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_id = "down"
    state.animation_selector = Animator.Pose.DEATH + state.direction * 4
    state.source_animation_selector = state.animation_selector
    state.source_record_flags = state.source_record_flags | ActorStatus.DOWN
    state.hurt_behavior_timer = 0
    state.movement_active = false
    return down_audio_event
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
    return begin_down_reaction(state)
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
        -- 0xB0AC/0xB0E4 (spell death): pose E.ActorPose.SPELL_DEATH_E0/E4,
        -- continuation 0xB0D4/0xB10C, no +0x10 b4 and no cue. 0xA9A6 then
        -- runs 0xAB02 and 0x1E514 when the list finishes. The selected kind's
        -- single undirected list supplies its own art and cadence [C: 0x62AC].
        -- Player spells on world actors are not connected to the live route [H].
        state.damage_reaction_active = true
        state.damage_reaction_animation_complete = false
        state.animation_frame = 0
        state.animation_accumulator = 0
        state.animation_id = "down"
        state.animation_selector = hit.spell_death_pose
        state.source_animation_selector = hit.spell_death_pose
        state.spell_death = true
        state.spell_death_pose = hit.spell_death_pose
        state.hurt_behavior_timer = 0
        state.movement_active = false
        return nil
    end
    if hit.physical_meter == 0 then
        -- 0xAFF2: zero +0x4C enters 0xB330 directly, whatever the pose.
        -- (0xB0AC/0xB0E4 send +0x10 b3 records and kind 0xC here too.)
        return begin_down_reaction(state)
    end
    local list = Animator.list(state.entity_kind, Animator.Pose.HURT,
                               state.direction)
    local hurt_audio_event = Animator.hurt_audio(state.entity_kind)
    if list == nil or not list.finite or
       type(hurt_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_accumulator = 0
    -- 0xAFF2 selects C072 and starts B07E's eight-service countdown.
    state.animation_id = "hurt"
    state.animation_selector = Animator.Pose.HURT + state.direction * 4
    state.source_animation_selector = state.animation_selector
    state.animation_increment = list.rate
    state.animation_period = Animator.PERIOD
    state.hurt_behavior_timer = 8
    state.hurt_audio_event = hurt_audio_event
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

local function begin_player_hurt_reaction(state, context)
    if context.actor_id ~= "field_player_runner" or
       context.phase ~= "behavior" or
       type(context.source_record_slot) ~= "number" or
       context.source_record_slot % 1 ~= 0 or
       context.source_record_slot < 0 or
       context.source_record_slot >= E.ActorPool.PARTY then
        return false
    end
    local pending = game.player_hurt_pending_by_slot
    if type(pending) ~= "table" or
       pending[context.source_record_slot] ~= true then
        return false
    end
    pending[context.source_record_slot] = nil
    state.player_hurt_pose_active = true
    state.player_hurt_animation_complete_pending = nil
    state.player_hurt_selector_reset_after_render = nil
    state.player_hurt_sound_active = true
    state.animation_id = "runner_hurt"
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_selector = PlayerHurt.SELECTOR_BASE +
        state.direction * PlayerHurt.SELECTOR_STEP
    state.hurt_behavior_timer = PlayerHurt.SOUND_DELAY
    state.hurt_audio_event = SoundId.DAC_SAMPLE_40
    state.hurt_audio_emitted = false
    return true
end

local function finish_player_hurt_reaction(state, context)
    if context.actor_id ~= "field_player_runner" or
       context.phase ~= "behavior" or
       state.player_hurt_animation_complete_pending ~= true then
        return false
    end
    state.player_hurt_animation_complete_pending = nil
    state.player_hurt_pose_active = nil
    if state.animation_id == "runner_hurt" then
        -- ROM 0x600E completes the kind-3 C072 0x8002 list after its second
        -- 0x0300/0x1000 step. On the next 0xA8EA pass, the living party
        -- record re-enters 0xACAE; its no-direction path installs 0xB45C,
        -- which selects the matching C052 idle pose. 0xB07E only keeps the
        -- independent eight-service cue; its 0xB48C call is collision work.
        state.animation_id = "idle"
        state.animation_selector = idle_pose_selectors[state.direction + 1]
        state.animation_frame = 0
        state.animation_accumulator = 0
        state.player_hurt_selector_reset_after_render = true
    end
    return true
end

local function advance_dynamic_actor_walk_animation(state, context)
    local list = Animator.list(state.entity_kind, Animator.Pose.WALK,
                               state.direction)
    if list == nil or list.finite then
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
    Animator.step_loop(state, list.count, increment, state.animation_period)
    return true
end

local function advance_dynamic_actor_action_animation(state)
    if state.animation_id ~= "action" or state.action_animation_complete then
        return true
    end
    local list = Animator.list(state.entity_kind, Animator.Pose.ACTION,
                               state.direction)
    if list == nil or not list.finite then
        return false
    end
    -- C0A2 list: 0x600E holds the last frame until a new list is selected.
    if Animator.step_finite(state, list.count, list.rate,
                            Animator.PERIOD) then
        state.action_animation_complete = true
    end
    return true
end

local function advance_dynamic_actor_item_action_animation(state)
    if state.animation_id ~= "weapon" or state.item_action_complete then
        return true
    end
    local list = Animator.list(state.entity_kind, Animator.Pose.ITEM,
                               state.direction)
    if list == nil or not list.finite then
        return false
    end
    -- C042 list: finite, marks pose bit 15 when done.
    if Animator.step_finite(state, list.count, list.rate,
                            Animator.PERIOD) then
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
    if state.player_hurt_selector_reset_after_render then
        -- The behavior pass above published C052 to the host. Do not carry
        -- that finite-reset selector into a later C062 movement decision.
        state.animation_selector = nil
        state.player_hurt_selector_reset_after_render = nil
    end
    if type(context.animation_accumulator_override) == "number" then
        state.animation_accumulator =
            context.animation_accumulator_override
    end
    if type(context.animation_direction_override) == "number" then
        state.direction = context.animation_direction_override & 0x7
    else
        state.direction = R.direction_from_heading(context.initial_heading)
    end

    local runner_hurt = context.animation_mode == "runner_hurt" or
        (state.player_hurt_pose_active and
         context.animation_mode == "idle")
    if runner_hurt then
        if state.animation_id ~= "runner_hurt" then
            state.animation_frame = 0
            state.animation_accumulator = 0
        end
        state.player_hurt_pose_active = true
        state.animation_id = "runner_hurt"
        state.animation_selector = PlayerHurt.SELECTOR_BASE +
            state.direction * PlayerHurt.SELECTOR_STEP
        local accumulated = state.animation_accumulator +
            PlayerHurt.FRAME_INCREMENT
        if accumulated >= PlayerHurt.FRAME_PERIOD then
            accumulated = accumulated - PlayerHurt.FRAME_PERIOD
            if state.animation_frame + 1 < PlayerHurt.FRAME_COUNT then
                state.animation_frame = state.animation_frame + 1
            else
                -- The 11th render pass reaches +0x1E=2; 0x600E marks the
                -- C072 selector complete but still displays its last frame.
                state.player_hurt_animation_complete_pending = true
            end
        end
        state.animation_accumulator = accumulated
        return
    end
    if state.player_hurt_pose_active then
        state.player_hurt_pose_active = nil
        state.player_hurt_animation_complete_pending = nil
        state.animation_selector = nil
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
    if context.animation_mode == "strike" then
        -- ROM 0xB1DC selects C0A2[dir] (0xC0..0xDC). Its finite 0x8002 list
        -- holds two frames advanced by +0x200 per 0x5FDC pass, then holds.
        if state.animation_id ~= "strike" then
            state.animation_frame = 0
            state.animation_accumulator = 0
        end
        state.animation_id = "strike"
        state.animation_selector = ActorPose.ACTION +
            state.direction * ActorAnimation.DIRECTION_SELECTOR_STEP
        local accumulated = state.animation_accumulator +
            ActorAnimation.FRAME_INCREMENT
        if accumulated >= ActorAnimation.FRAME_PERIOD then
            accumulated = accumulated - ActorAnimation.FRAME_PERIOD
            if state.animation_frame < 1 then
                state.animation_frame = state.animation_frame + 1
            end
        end
        state.animation_accumulator = accumulated
        return
    end
    if context.animation_mode == "runner_down" then
        -- ROM 0xB330 selects C0B2[dir] (0xE8..0x104). Its finite 0x8008
        -- list advances +0x200 per pass and holds on entry 7; entries 1..7
        -- share one descriptor.
        if state.animation_id ~= "runner_down" then
            state.animation_frame = 0
            state.animation_accumulator = 0
        end
        state.animation_id = "runner_down"
        state.animation_selector = ActorPose.DEATH +
            state.direction * ActorAnimation.DIRECTION_SELECTOR_STEP
        local accumulated = state.animation_accumulator +
            ActorAnimation.FRAME_INCREMENT
        if accumulated >= ActorAnimation.FRAME_PERIOD then
            accumulated = accumulated - ActorAnimation.FRAME_PERIOD
            if state.animation_frame < 7 then
                state.animation_frame = state.animation_frame + 1
            end
        end
        state.animation_accumulator = accumulated
        return
    end
    if context.animation_mode == "cast" then
        -- ROM 0xAF7C/0xAFAE select C082[dir] (0x80..0x9C), one frame.
        state.animation_id = "cast"
        state.animation_selector = 0x80 + state.direction * 4
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
            Animator.list(kind, Animator.Pose.WALK, state.direction).count, rate)
    else
        state.animation_id = "idle"
        state.animation_selector = Animator.Pose.IDLE + state.direction * 4
        Animator.step_loop(state,
            Animator.list(kind, Animator.Pose.IDLE, state.direction).count, rate)
    end
end


-- register this module's definitions
R.snapshot = snapshot
R.advance_projectile_animation = advance_projectile_animation
R.advance_hit_spark_animation = advance_hit_spark_animation
R.hit_spark_heartbeat = hit_spark_heartbeat
R.advance_entry_animation = advance_entry_animation
R.death_list = death_list
R.advance_finite_actor_animation = advance_finite_actor_animation
R.begin_received_hit_reaction = begin_received_hit_reaction
R.begin_down_reaction = begin_down_reaction
R.service_zero_meter_down = service_zero_meter_down
R.advance_received_hit_reaction = advance_received_hit_reaction
R.begin_player_hurt_reaction = begin_player_hurt_reaction
R.finish_player_hurt_reaction = finish_player_hurt_reaction
R.advance_dynamic_actor_walk_animation = advance_dynamic_actor_walk_animation
R.advance_dynamic_actor_action_animation = advance_dynamic_actor_action_animation
R.advance_dynamic_actor_item_action_animation = advance_dynamic_actor_item_action_animation
R.advance_player_animation = advance_player_animation
R.spell_pose_selectors = spell_pose_selectors
return R
