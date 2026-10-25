-- actor/anim.lua: presentation: snapshots, pose selectors, entry/hit/walk/action animations.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local PlayerHurt = E.PlayerHurt
local HitSpark = E.HitSpark
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local SpriteType = R.SpriteType   -- from actor/tables.lua
local function direction_from_heading(...) return R.direction_from_heading(...) end   -- from actor/targeting.lua
local idle_pose_selectors = R.idle_pose_selectors   -- from actor/tables.lua
local walk_pose_selectors = R.walk_pose_selectors   -- from actor/tables.lua
local weapon_projectile_descriptor_increment = R.weapon_projectile_descriptor_increment   -- from actor/tables.lua

local function snapshot(state, action, audio_event, projectile_movement,
                        target_heading_audio_event)
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
        animation_selector = state.animation_selector,
        direction = state.direction,
        frame = state.animation_frame,
        animation_accumulator = state.animation_accumulator,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        source_behavior_flags = state.source_behavior_flags,
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

    -- ROM 0x5FDC adds +0x18 to +0x1A as a signed 16-bit word, compares it
    -- with +0x1C, and advances +0x1E once when the period is reached.
    local accumulated_word =
        u16(state.animation_accumulator + state.animation_increment)
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if state.animation_period <= accumulated_signed then
        state.animation_frame = u16(state.animation_frame + 1)
        state.animation_accumulator =
            u16(accumulated_word - state.animation_period)
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

local function advance_hit_spark_animation(state, context)
    local animation = context.hit_spark_animation
    if context.phase ~= "render" or type(animation) ~= "table" or
       type(animation.selector) ~= "number" or
       type(animation.frame_count) ~= "number" or
       animation.frame_count ~= HitSpark.FRAME_COUNT or
       type(animation.period) ~= "number" or
       animation.period ~= HitSpark.FRAME_PERIOD then
        return false
    end

    -- 0x15118 initializes +0x1C to 0x1000; the kind-1 list at 0x7330 sets
    -- +0x18 to 0x0400. 0x5FDC advances +0x1E, then 0x600E marks the
    -- selector negative after the fifth list entry [C].
    state.animation_selector = animation.selector
    state.animation_period = animation.period
    state.animation_frame_count = animation.frame_count
    local accumulated_word =
        u16(state.animation_accumulator + HitSpark.FRAME_INCREMENT)
    local accumulated_signed = accumulated_word
    if accumulated_word >= HitSpark.FINISHED_SELECTOR_BIT then
        accumulated_signed = accumulated_word - HitSpark.WORD_MODULUS
    end
    if state.animation_period <= accumulated_signed then
        state.animation_frame = u16(state.animation_frame + 1)
        state.animation_accumulator =
            u16(accumulated_word - state.animation_period)
        if state.animation_accumulator >= HitSpark.FINISHED_SELECTOR_BIT then
            state.animation_accumulator = state.animation_accumulator -
                HitSpark.WORD_MODULUS
        end
    else
        state.animation_accumulator = accumulated_signed
    end
    if state.animation_frame_count <= state.animation_frame then
        state.animation_selector = state.animation_selector |
            HitSpark.FINISHED_SELECTOR_BIT
        state.animation_frame = state.animation_frame_count - 1
    end
    state.animation_increment = HitSpark.FRAME_INCREMENT
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
        u16(state.animation_accumulator + increment)
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
    if (state.source_record_flags & ActorStatus.DOWN) ~= 0 then
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
            ActorStatus.DOWN
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
    if state.entity_kind == SpriteType.GARGOYLE then
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
        u16(state.animation_accumulator + increment)
    local accumulated_signed = accumulated_word
    if accumulated_word >= 0x8000 then
        accumulated_signed = accumulated_word - 0x10000
    end
    if state.animation_period <= accumulated_signed then
        state.animation_frame = u16(state.animation_frame + 1)
        state.animation_accumulator =
            u16(accumulated_word - state.animation_period)
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
        u16(state.animation_accumulator + increment)
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

local function advance_dynamic_actor_item_action_animation(state)
    if state.animation_id ~= "weapon" or state.item_action_complete then
        return true
    end
    local frame_count = state.item_action_frame_count
    local increment = state.item_action_increment
    local threshold = state.item_action_threshold
    if type(frame_count) ~= "number" or frame_count < 1 or frame_count > 8 or
       type(increment) ~= "number" or increment < 1 or increment > 0xffff or
       type(threshold) ~= "number" or threshold ~= 0x1000 then
        return false
    end

    -- ROM 0x600E advances the selected C042 list with its +2 increment and
    -- the actor's +0x1C threshold, then marks a finite list complete.
    local accumulated_word =
        u16(state.animation_accumulator + increment)
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
            state.animation_frame = frame_count - 1
            state.source_animation_selector =
                state.source_animation_selector | 0x8000
            state.item_action_complete = true
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
        state.direction = direction_from_heading(context.initial_heading)
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
        state.animation_selector = 0xC0 + state.direction * 4
        local accumulated = state.animation_accumulator + 0x0200
        if accumulated >= 0x1000 then
            accumulated = accumulated - 0x1000
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
        state.animation_selector = 0xE8 + state.direction * 4
        local accumulated = state.animation_accumulator + 0x0200
        if accumulated >= 0x1000 then
            accumulated = accumulated - 0x1000
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


-- register this module's definitions
R.snapshot = snapshot
R.advance_projectile_animation = advance_projectile_animation
R.advance_hit_spark_animation = advance_hit_spark_animation
R.hit_spark_heartbeat = hit_spark_heartbeat
R.advance_entry_animation = advance_entry_animation
R.advance_finite_actor_animation = advance_finite_actor_animation
R.begin_received_hit_reaction = begin_received_hit_reaction
R.advance_received_hit_reaction = advance_received_hit_reaction
R.begin_player_hurt_reaction = begin_player_hurt_reaction
R.finish_player_hurt_reaction = finish_player_hurt_reaction
R.advance_dynamic_actor_walk_animation = advance_dynamic_actor_walk_animation
R.advance_dynamic_actor_action_animation = advance_dynamic_actor_action_animation
R.advance_dynamic_actor_item_action_animation = advance_dynamic_actor_item_action_animation
R.advance_player_animation = advance_player_animation
R.spell_pose_selectors = spell_pose_selectors
return R
