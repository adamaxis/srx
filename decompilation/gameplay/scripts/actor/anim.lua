-- actor/anim.lua: semantic clip selection, snapshots, and animation timing.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local Animator = R.Animator   -- from actor/animator.lua
local SoundId, SceneId, EntryDelayMax, AiTuning =
    E.SoundId, E.SceneId, E.EntryDelayMax, E.AiTuning
local PlayerHurt = E.PlayerHurt
local VisualProfile = R.VisualProfile -- from actor/tables.lua
local ProjectileAnimationClips =
    require("data.weapon_projectile_animation_clips")

local function snapshot(state, action, audio_event, projectile_movement,
                        target_heading_audio_event)
    local world_activity_requested = state.world_activity_requested == true
    local field_event_request = state.field_event_request
    local think_token_claimed = state.think_token_claimed == true
    local thon_action_cooldown_remaining =
        state.thon_action_cooldown_remaining
    state.world_activity_requested = false
    state.field_event_request = nil
    state.think_token_claimed = false
    state.thon_action_cooldown_remaining = nil
    if state.animation_id == "idle" then
        state.animation_frame = 0
    end
    local combat_meter_update = state.combat_meter_update
    state.combat_meter_update = nil
    local strike_bearing_degrees = state.strike_bearing_degrees
    state.strike_bearing_degrees = nil
    local hit_spark_spawn_request = state.hit_spark_spawn_request
    state.hit_spark_spawn_request = nil
    local result = {
        actor_id = state.actor_id,
        profile_id = state.profile_id,
        package_id = state.package_id,
        action_kind = state.action_kind,
        action_id = state.action_id,
        action = action,
        animation = state.animation_id,
        animation_complete =
            (state.animation_id == "weapon" and
             state.item_action_complete == true) or
            (state.animation_id == "action" and
             state.action_animation_complete == true) or
            ((state.animation_id == "hurt" or
              state.animation_id == "down") and
             state.damage_reaction_animation_complete == true) or
            (state.profile_id == "weapon_projectile" and
             state.animation_complete == true) or
            (state.profile_id == "field_hit_spark" and
             state.hit_spark_animation_complete == true),
        animation_variant = state.animation_id == "down" and
            state.spell_death_variant or "",
        actor_facts = state.actor_facts,
        direction = state.direction,
        frame = state.animation_frame,
        transition_started = state.transition_started,
        transition_complete = state.transition_complete,
        world_activity_requested = world_activity_requested,
        field_event_request = field_event_request,
        think_token_claimed = think_token_claimed,
        thon_action_cooldown_remaining = thon_action_cooldown_remaining,
        combat_action_requested = state.combat_action_requested,
        audio_event = audio_event,
        target_heading_audio_event = target_heading_audio_event,
        strike_bearing_degrees = strike_bearing_degrees,
        weapon_item_id = state.weapon_item_id,
        movement_bearing_degrees = state.movement_bearing_degrees,
        movement_current_bearing_degrees = state.movement_current_bearing_degrees,
        movement_route_distance = state.movement_route_distance,
        ai_target_id = state.ai_target_id,
        ai_target_bearing_degrees = state.ai_target_bearing_degrees,
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
        spell_effect_id = state.spell_effect_id,
        field_barrier_remaining = state.field_barrier_remaining,
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
    local clip = type(animation) == "table" and
        ProjectileAnimationClips[animation.frame_count] or nil
    if context.phase ~= "render" or type(animation) ~= "table" or
       type(animation.frame_count) ~= "number" or
       animation.frame_count < 1 or
       type(clip) ~= "table" or
       #clip.frame_durations ~= animation.frame_count then
        return false
    end

    -- The asset list remains host presentation data; Lua only owns its frame
    -- timeline and whether the temporary effect has completed.
    state.animation_frame_count = animation.frame_count
    if state.projectile_animation_started ~= true then
        state.projectile_animation_started = true
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        state.animation_complete = false
    else
        Animator.advance_finite_schedule(state, clip.frame_durations)
    end
    state.animation_id = "projectile"
    return true
end

-- ROM 0xC0B2 death list (pose 0xE8) or a 0xB0AC/0xB0E4 spell-death list
-- (pose 0xE0/0xE4): finite, timed by that list's own +2 rate for this kind
-- The imported catalog supplies its frame count and normalized update duration.
local function death_list(state)
    local clip = state.spell_death_variant == "spell_death_e0"
        and Animator.Clip.SPELL_DEATH_E0 or
        (state.spell_death_variant == "spell_death_e4"
            and Animator.Clip.SPELL_DEATH_E4 or Animator.Clip.DEATH)
    local list = Animator.list(state.visual_profile_id, clip, state.direction)
    return list
end

-- Player hit spark (kind 1): ROM 0x7330 supplies the five-frame clip and its
-- duration. Lua stores its frame and completion as gameplay state; the host
-- maps that completion to the auxiliary record's terminal marker [C].
local function hit_spark_list()
    return Animator.list(VisualProfile.HIT_SPARK, Animator.Clip.HIT_SPARK)
end

local function advance_hit_spark_animation(state, context)
    local list = hit_spark_list()
    if context.phase ~= "render" or list == nil or list.looping then
        return false
    end
    state.animation_frame_count = list.frame_count
    if Animator.advance_finite_clip(state, list) then
        state.hit_spark_animation_complete = true
    end
    state.animation_id = "hit_spark"
    return true
end

local function hit_spark_heartbeat(state, context)
    if context.phase == "render" then
        if not advance_hit_spark_animation(state, context) then
            return nil
        end
    elseif context.phase == "behavior" then
        state.hit_spark_retired =
            state.hit_spark_animation_complete == true
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
    local list = Animator.list(state.visual_profile_id, Animator.Clip.DEATH,
                               state.direction)
    if list == nil or list.looping then
        return
    end
    if Animator.advance_finite_clip(state, list) then
        state.transition_frame_list_finished = true
    end
end

local function advance_finite_actor_animation(state, animation)
    if state.damage_reaction_animation_complete then
        return true
    end
    if type(animation) ~= "table" or animation.looping ~= false or
       type(animation.frame_count) ~= "number" or animation.frame_count < 1 or
       type(animation.frame_duration_updates) ~= "number" or
       animation.frame_duration_updates <= 0 then
        return false
    end
    -- Completion is a semantic flag; the last visible frame remains held.
    if Animator.advance_finite_clip(state, animation) then
        state.damage_reaction_animation_complete = true
    end
    return true
end

-- ROM 0xB330 (death/down): B05E[kind] cue, +0x10 bit 4, +0x3A = 0xB38A,
-- pose C0B2[dir8] (0xE8 + 4*dir), 0xAF6C resets +0x1A/+0x1E. Callers:
-- 0xAFF2 hit with +0x4C == 0, 0xAA4C service with +0x4C or +0x9C == 0,
-- 0xE4BA kill_leader. It is never selected at spawn.
local function begin_down_reaction(state)
    local down_audio_event = Animator.down_audio(state.visual_profile_id)
    if type(down_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_progress = 0
    state.animation_elapsed_updates = 0
    state.animation_id = "down"
    state.spell_death_variant = nil
    state.actor_facts.lifecycle.downed = true
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
    if state.actor_facts.lifecycle.downed then
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
    if state.actor_facts.lifecycle.downed then
        return nil
    end
    if hit.physical_meter == 0 and hit.spell_death_variant ~= nil and
       not state.actor_facts.lifecycle.ai_runner and
       context.visual_profile_id ~= VisualProfile.HELL_HOUND then
        -- 0xB0AC/0xB0E4 (spell death): finite spell-death clip,
        -- continuation 0xB0D4/0xB10C, no +0x10 b4 and no cue. 0xA9A6 then
        -- runs 0xAB02 and 0x1E514 when the list finishes. The selected kind's
        -- single undirected list supplies its own art and cadence [C: 0x62AC].
        -- Player spells on world actors are not connected to the live route [H].
        state.damage_reaction_active = true
        state.damage_reaction_animation_complete = false
        state.animation_frame = 0
        state.animation_progress = 0
        state.animation_elapsed_updates = 0
        state.animation_id = "down"
        state.spell_death_variant = hit.spell_death_variant
        state.spell_death = true
        state.hurt_behavior_timer = 0
        state.movement_active = false
        return nil
    end
    if hit.physical_meter == 0 then
        -- 0xAFF2: zero +0x4C enters 0xB330 directly, whatever the pose.
        -- (0xB0AC/0xB0E4 send +0x10 b3 records and kind 0xC here too.)
        return begin_down_reaction(state)
    end
    local list = Animator.list(state.visual_profile_id, Animator.Clip.HURT,
                               state.direction)
    local hurt_audio_event = Animator.hurt_audio(state.visual_profile_id)
    if list == nil or list.looping or
       type(hurt_audio_event) ~= "number" then
        return nil
    end
    state.damage_reaction_active = true
    state.damage_reaction_animation_complete = false
    state.animation_frame = 0
    state.animation_progress = 0
    state.animation_elapsed_updates = 0
    -- 0xAFF2 selects C072 and starts B07E's eight-service countdown.
    state.animation_id = "hurt"
    state.spell_death_variant = nil
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
    local actor_id = context.controlled_party_member_id
    if context.phase ~= "behavior" or
       type(actor_id) ~= "string" or actor_id == "" or
       context.is_controlled_party_actor ~= true then
        return false
    end
    -- ROM 0x55C76 updates the selected target record. The pending host event
    -- is keyed by that actor's stable id; a world heartbeat may know which
    -- party member it is engaging but cannot consume that member's reaction.
    -- [C]
    local pending = game.player_hurt_pending_by_actor_id
    if type(pending) ~= "table" or pending[actor_id] ~= true then
        return false
    end
    pending[actor_id] = nil
    state.player_hurt_pose_active = true
    state.player_hurt_animation_complete_pending = nil
    state.player_hurt_selector_reset_after_render = nil
    state.player_hurt_sound_active = true
    state.animation_id = "runner_hurt"
    state.animation_frame = 0
    state.animation_progress = 0
    state.animation_elapsed_updates = 0
    state.hurt_behavior_timer = PlayerHurt.SOUND_DELAY
    state.hurt_audio_event = SoundId.DAC_SAMPLE_40
    state.hurt_audio_emitted = false
    return true
end

local function finish_player_hurt_reaction(state, context)
    if type(context.controlled_party_member_id) ~= "string" or
       context.controlled_party_member_id == "" or
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
        state.animation_frame = 0
        state.animation_progress = 0
        state.animation_elapsed_updates = 0
        state.player_hurt_selector_reset_after_render = true
    end
    return true
end

local function advance_dynamic_actor_walk_animation(state)
    local list = Animator.list(state.visual_profile_id, Animator.Clip.WALK,
                               state.direction)
    if list == nil or not list.looping then
        return false
    end

    local progress_per_update = Animator.progress_per_update(
        state.animation_cadence_profile, state.animation_frame,
        state.movement_distance_per_update)
    if progress_per_update == nil then
        return false
    end
    Animator.advance_loop_clip(state, list.frame_count,
                               progress_per_update)
    return true
end

local function advance_dynamic_actor_idle_animation(state)
    local list = Animator.list(state.visual_profile_id, Animator.Clip.IDLE,
                               state.direction)
    if list == nil or not list.looping then
        return true
    end
    local progress_per_update = Animator.progress_per_update(
        state.animation_cadence_profile, state.animation_frame,
        state.movement_distance_per_update)
    if progress_per_update == nil then
        return false
    end
    Animator.advance_loop_clip(state, list.frame_count,
                               progress_per_update)
    return true
end

local function advance_dynamic_actor_action_animation(state)
    if state.animation_id ~= "action" or state.action_animation_complete then
        return true
    end
    local list = Animator.list(state.visual_profile_id, Animator.Clip.ACTION,
                               state.direction)
    if list == nil or list.looping then
        return false
    end
    if Animator.advance_finite_clip(state, list) then
        state.action_animation_complete = true
    end
    return true
end

local function advance_dynamic_actor_item_action_animation(state)
    if state.animation_id ~= "weapon" or state.item_action_complete then
        return true
    end
    local list = Animator.list(state.visual_profile_id, Animator.Clip.ITEM,
                               state.direction)
    if list == nil or list.looping then
        return false
    end
    if Animator.advance_finite_clip(state, list) then
        state.item_action_complete = true
    end
    return true
end

local function advance_player_finite_clip(state, clip)
    local list = Animator.list(VisualProfile.FIELD_RUNNER, clip, state.direction)
    if list == nil or list.looping then
        return false
    end
    return Animator.advance_finite_clip(state, list)
end

-- ROM 0xC082. AF7C/AFAE index this by the same 0xB36A direction as C0A2.
local function advance_player_animation(state, context)
    if state.player_hurt_selector_reset_after_render then
        state.player_hurt_selector_reset_after_render = nil
    end
    local animation_mode = context.animation_mode
    if context.animation_clock_reset_requested == true then
        state.animation_progress = 0
        state.animation_elapsed_updates = 0
    end
    if type(context.animation_direction_override) == "number" then
        state.direction = context.animation_direction_override & 0x7
    else
        state.direction = R.direction_from_bearing_degrees(context.initial_bearing_degrees)
    end

    local runner_hurt = context.animation_mode == "runner_hurt" or
        (state.player_hurt_pose_active and
         context.animation_mode == "idle")
    if runner_hurt then
        if state.animation_id ~= "runner_hurt" then
            state.animation_frame = 0
            state.animation_elapsed_updates = 0
        end
        state.player_hurt_pose_active = true
        state.animation_id = "runner_hurt"
        if advance_player_finite_clip(state, Animator.Clip.HURT) then
            state.player_hurt_animation_complete_pending = true
        end
        return
    end
    if state.player_hurt_pose_active then
        state.player_hurt_pose_active = nil
        state.player_hurt_animation_complete_pending = nil
    end

    if context.animation_mode == "attack" then
        -- C042 (pose 0x40) for kind 3 is one frame; the extracted art is
        -- attack-<dir>.bmp. Its 0x80 cadence only drives pose bit 15, which
        -- the C++ attack-pose timer owns [H: not ported here].
        state.animation_id = "attack"
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        return
    end
    if context.animation_mode == "strike" then
        -- C0A2 supplies the runner strike clip. The importer records its
        -- finite frame duration; gameplay advances the clip by elapsed updates.
        if state.animation_id ~= "strike" then
            state.animation_frame = 0
            state.animation_elapsed_updates = 0
        end
        state.animation_id = "strike"
        advance_player_finite_clip(state, Animator.Clip.ACTION)
        return
    end
    if context.animation_mode == "runner_down" then
        -- C0B2 supplies the runner down clip. Gameplay uses the imported
        -- duration and holds the final frame.
        if state.animation_id ~= "runner_down" then
            state.animation_frame = 0
            state.animation_elapsed_updates = 0
        end
        state.animation_id = "runner_down"
        advance_player_finite_clip(state, Animator.Clip.DEATH)
        return
    end
    if context.animation_mode == "cast" then
        -- ROM 0xAF7C/0xAFAE select C082[dir] (0x80..0x9C), one frame.
        state.animation_id = "cast"
        state.animation_frame = 0
        state.animation_elapsed_updates = 0
        return
    end

    -- Kind 3 idle/walk lists loop, so 0x6092 times them from +0xCA = 0x28
    -- (C002+4 via 0xBABC/0xBADE): 0x28 * 0x10 = 0x280 [C].
    local visual_profile_id = VisualProfile.FIELD_RUNNER
    if state.animation_id ~= nil and state.animation_id ~= "idle" and
       state.animation_id ~= "walk" then
        -- A one-shot clip has its own duration and cannot leave fractional
        -- locomotion time behind when movement resumes.
        state.animation_progress = 0
    end
    if context.animation_mode == "walking" then
        state.animation_id = "walk"
        Animator.advance_loop_clip(state,
            Animator.list(visual_profile_id, Animator.Clip.WALK, state.direction).frame_count,
            Animator.PLAYER_WALK_PROGRESS_PER_UPDATE)
    else
        state.animation_id = "idle"
        Animator.advance_loop_clip(state,
            Animator.list(visual_profile_id, Animator.Clip.IDLE, state.direction).frame_count,
            Animator.PLAYER_WALK_PROGRESS_PER_UPDATE)
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
R.advance_dynamic_actor_idle_animation = advance_dynamic_actor_idle_animation
R.advance_dynamic_actor_walk_animation = advance_dynamic_actor_walk_animation
R.begin_received_hit_reaction = begin_received_hit_reaction
R.begin_down_reaction = begin_down_reaction
R.service_zero_meter_down = service_zero_meter_down
R.advance_received_hit_reaction = advance_received_hit_reaction
R.begin_player_hurt_reaction = begin_player_hurt_reaction
R.finish_player_hurt_reaction = finish_player_hurt_reaction
R.advance_dynamic_actor_action_animation = advance_dynamic_actor_action_animation
R.advance_dynamic_actor_item_action_animation = advance_dynamic_actor_item_action_animation
R.advance_player_animation = advance_player_animation
return R
