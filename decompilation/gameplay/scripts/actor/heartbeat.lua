-- actor/heartbeat.lua: logic: ActorAI.heartbeat facade (public entry point, unchanged signature).
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local ActorStatus = E.ActorStatus
local ActorPool = E.ActorPool
local ActorFlag, SpriteType = E.ActorFlag, E.SpriteType

-- ROM 0x1E514: bclr #7,+0xB0 (escort; when it was set the ROM also clears FFFC3C,
-- which the port does not model [H: FFFC3C meaning unknown]) and bclr #0,+0x10
-- (ACTIVE). The C++ host frees the record slot (0x1A304) unless +0x10 b3 is set
-- and stops drawing/servicing the record (0x4618/0xA8EA skip inactive records).
local function deactivate_record(state)
    state.source_behavior_flags =
        u8((state.source_behavior_flags or 0) & ~BehaviorFlag.ESCORT)
    state.source_record_flags =
        state.source_record_flags & ~ActorStatus.ACTIVE
end
local ActorAI = {}
local states = game.actors
local pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot
local spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot
local ActionKind = R.ActionKind   -- from actor/tables.lua
local idle_pose_selectors = R.idle_pose_selectors   -- from actor/tables.lua
local walk_pose_selectors = R.walk_pose_selectors   -- from actor/tables.lua

function ActorAI.should_rebuild_target_lists(frame_counter, encounter_flag)
    return R.should_rebuild_target_lists(frame_counter, encounter_flag)
end

function ActorAI.heartbeat(context)
    if type(context) ~= "table" then
        return nil
    end
    local profile = R.source_profile(context)
    if profile == nil or type(context.actor_id) ~= "string" or
       context.actor_id == "" or type(context.initial_heading) ~= "number" or
       type(context.entry_delay) ~= "number" or context.entry_delay < 0 or
       context.entry_delay >
           (context.dynamic_world_encounter_actor and EntryDelayMax.DYNAMIC_ENCOUNTER or EntryDelayMax.STATIC) or
       (context.phase ~= "behavior" and context.phase ~= "render") then
        return nil
    end

    local state = states[context.actor_id]
    if context.spawn or state == nil then
        state = R.initial_state(context, profile)
        if state == nil then
            return nil
        end
        states[context.actor_id] = state
        R.transfer_pending_spell_effect(state, context)
        R.advance_world_target_selection(state, context)
        R.advance_spell_effect(state, context)
        if profile.projectile_animation then
            return R.projectile_heartbeat(state, context)
        end
        if profile.hit_spark_animation then
            return R.hit_spark_heartbeat(state, context)
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
                    R.requested_movement_heading(context, profile, state)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = R.approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                    state.movement_step = R.finish_entity_movement(
                        context, R.advance_entity_movement(context, state))
                end
            else
                R.advance_player_animation(state, context)
            end
        end
        R.advance_target_marker(state, context)
        return R.snapshot(state, "hold", nil)
    end
    if not R.same_actor(state, context, profile) then
        return nil
    end
    state.source_action_dispatch = nil
    state.strike_heading = nil
    state.spell_action_resolution = nil
    state.source_record_flags = context.source_record_flags or
                                state.source_record_flags
    if state.damage_reaction_active and state.animation_id == "down" and
       not state.spell_death then
        -- 0xB330 sets +0x10 bit 4 and leaves it set while the ordinary down
        -- reaction is active. Preserve that Lua-owned transition if a later
        -- host pass still carries the pre-transition record byte [C].
        state.source_record_flags = state.source_record_flags |
                                    ActorStatus.DOWN
    end
    state.source_physical_meter = context.source_physical_meter or
                                  state.source_physical_meter
    state.source_record_slot = context.source_record_slot or
                               state.source_record_slot
    state.source_spawn_state_flags = context.source_spawn_state_flags or
                                     state.source_spawn_state_flags or 0
    state.source_behavior_flags = context.source_behavior_flags or
                                  state.source_behavior_flags
    state.source_action_slot = context.source_action_slot or
                               state.source_action_slot
    state.source_action_ammo = context.source_action_ammo or
                               state.source_action_ammo
    state.source_action_counter = context.source_action_counter or
                                  state.source_action_counter
    state.source_animation_selector = context.source_animation_selector or
                                      state.source_animation_selector
    R.advance_source_action_delay(state, context)
    if context.action_kind == ActionKind.item and
       context.source_item_spawn_result == "spawned" then
        local target = R.retained_linked_target(context, state)
        if not R.begin_dynamic_item_action(state, context, target) then
            return nil
        end
    end
    R.transfer_pending_spell_effect(state, context)
    R.advance_world_target_selection(state, context)
    if profile.projectile_animation then
        return R.projectile_heartbeat(state, context)
    end
    if profile.hit_spark_animation then
        return R.hit_spark_heartbeat(state, context)
    end
    R.advance_target_marker(state, context)
    R.advance_target_motion_effect(state, context)
    R.advance_spell_effect(state, context)

    local reaction_audio_event
    local player_hurt_started = false
    if context.actor_id == "field_player_runner" and
       context.phase == "behavior" then
        player_hurt_started = R.begin_player_hurt_reaction(state, context)
        if not player_hurt_started and state.player_hurt_sound_active then
            reaction_audio_event =
                R.advance_received_hit_reaction(state, context)
            if state.hurt_behavior_timer == 0 then
                state.player_hurt_sound_active = false
            end
        end
        if not player_hurt_started then
            R.finish_player_hurt_reaction(state, context)
        end
    end
    -- ROM 0xAFF2 -> 0xB330 and 0xA93A..0xAA4C service every live world
    -- record in slots 3..9, including the default actor list. Dynamic
    -- encounter membership is not the combat-record boundary.
    if context.dynamic_world_encounter_actor or
       (type(context.combatant) == "table" and
        type(context.combatant.record_type) == "number" and
        context.combatant.record_type >= ActorPool.PARTY) then
        local reaction_started = false
        if context.phase == "behavior" and state.incoming_hit ~= nil then
            reaction_audio_event =
                R.begin_received_hit_reaction(state, context)
            reaction_started = state.damage_reaction_active
        end
        if context.phase == "behavior" and reaction_audio_event == nil then
            local down_cue = R.service_zero_meter_down(state, context)
            if down_cue ~= nil then
                reaction_audio_event = down_cue
                reaction_started = true
            end
        end
        if state.damage_reaction_active and
           state.animation_id == "down" then
            if context.phase == "render" then
                state.movement_step = nil
                -- The death list's ROM rate/count for this kind; the art
                -- count stands in when the list has no extracted frames.
                local animation = R.death_list(state)
                if not R.advance_finite_actor_animation(state, animation) then
                    return nil
                end
            elseif state.damage_reaction_animation_complete and
                   (state.source_record_flags & ActorStatus.DEAD) == 0 then
                -- ROM 0xA93E: the finished C0B2 list sets pose bit 15, so the
                -- next 0xA8EA service enters 0xAB02 once: drops, FFE177--,
                -- and bset #6,+0x10 (DEAD, 0x40). The body stays drawn.
                state.source_record_flags =
                    state.source_record_flags | ActorStatus.DEAD
                -- ROM 0xA9DE/0xAA14: a Wendigo (kind 0xE, 14) whose list is
                -- finished takes 0xA9BE instead: 0xAB02 then 0x1E514, so the
                -- record leaves the field after its death list.
                -- 0xA9A6: a finished 0xB0D4/0xB10C spell-death list does the
                -- same for every kind.
                if context.entity_kind == SpriteType.WENDIGO or
                   state.spell_death then
                    deactivate_record(state)
                end
            end
            return R.snapshot(state, "hold", reaction_audio_event)
        end
        if state.damage_reaction_active then
            if context.phase == "render" and
               state.animation_id == "hurt" then
                state.movement_step = nil
                local animation = R.Animator.list(
                    state.entity_kind, R.Animator.Pose.HURT, state.direction)
                if not R.advance_finite_actor_animation(state, animation) then
                    return nil
                end
                return R.snapshot(state, "hold", nil)
            elseif context.phase == "behavior" and not reaction_started then
                -- B07E remains the actor continuation while the ordinary AI
                -- coordinator can replace C072 with C062 after a movement
                -- step. Its eight-service cue countdown survives that pose
                -- change and movement remains live on every service pass.
                reaction_audio_event =
                    R.advance_received_hit_reaction(state, context)
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
        if not profile.dynamic_encounter_only then
            local cast_audio = R.begin_player_spell_cast(state, context)
            if cast_audio ~= nil then
                audio_event = cast_audio
            end
            local swing_audio, strike_heading_audio =
                R.begin_player_melee(state, context)
            if swing_audio ~= nil then
                audio_event = swing_audio
                target_heading_audio_event = strike_heading_audio
            end
        end
        if context.phase == "render" then
            state.movement_step = nil
            if not profile.dynamic_encounter_only then
                state.movement_heading = nil
            end
            if profile.dynamic_encounter_only then
                if state.animation_id == "action" then
                    if not R.advance_dynamic_actor_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "weapon" then
                    if not R.advance_dynamic_actor_item_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "spell" then
                    -- C082 lists are one frame. Hold it; do not invent a count.
                    state.animation_frame = 0
                    state.animation_accumulator = 0
                elseif state.animation_id == "walk" then
                    if not R.advance_dynamic_actor_walk_animation(state, context) then
                        return nil
                    end
                    state.source_animation_selector =
                        walk_pose_selectors[state.direction + 1]
                else
                    local facing_heading = state.ai_target_engaged
                        and state.ai_target_heading or
                        state.movement_current_heading or
                        context.initial_heading
                    state.direction = R.direction_from_heading(facing_heading)
                    state.animation_id = "idle"
                    state.animation_frame = 0
                    state.source_animation_selector =
                        idle_pose_selectors[state.direction + 1]
                end
            else
                R.advance_player_animation(state, context)
            end
        else
            state.movement_step = nil
            if profile.dynamic_encounter_only then
                if not state.transition_complete then
                    -- ROM 0x1E25A spawns the encounter record live: posture
                    -- +0x4D=4, +0x9E.6 hostile, then 0x1DBAA (combat on).
                    -- No death/"entry" list is selected at spawn; C0B2 is
                    -- read only by 0xB330 (callers 0xAA4C zero meter,
                    -- 0xB002..0xB0F4 hit path, 0xE4BA). The seeded delay is
                    -- the +0xB6 think timer that 0x13904 counts down while
                    -- the actor idles (it stays hittable meanwhile).
                    state.transition_started = true
                    if state.entry_delay == 0 then
                        state.transition_complete = true
                        state.behavior_timer = 0
                    else
                        state.entry_delay = state.entry_delay - 1
                    end
                else
                    if R.think(state, context) then
                        if state.ai_deactivate_requested then
                            deactivate_record(state)
                            return R.snapshot(state, "hold", nil)
                        end
                    else
                        -- 0x13904 refreshes target/routing every 21 actor
                        -- service passes; +0x3A continues movement each pass.
                        -- Lua's typed patrol/follow destination remains active
                        -- here while the source continuation advances it [C].
                        R.requested_movement_heading(context, profile, state)
                        local target = R.retained_linked_target(context, state)
                        if context.action_kind == ActionKind.spell then
                            -- Negative +0x56 never enters 0x14C7C / 0xB1DC.
                            state.ai_target_engaged = false
                            local spell_target = target
                            if context.action_code == 12 then
                                spell_target =
                                    R.first_wounded_party_runner(context)
                            end
                            if context.action_code == 12 and
                               spell_target == nil then
                                -- 0x13BBA returns carry clear when 0x13C20
                                -- finds no eligible runner lane.
                                state.source_action_dispatch = "suppressed"
                            else
                                state.source_action_dispatch =
                                    R.classify_spell_dispatch(
                                        state, context, target)
                            end
                            if state.source_action_dispatch == "spell" then
                                local spell_resolution
                                local combat = game.modules.combat
                                if type(combat) == "table" and
                                   type(combat.resolve_spell_action) ==
                                       "function" then
                                    spell_resolution =
                                        combat.resolve_spell_action(context,
                                                                    spell_target)
                                    if spell_resolution ~= nil then
                                        state.spell_action_resolution =
                                            spell_resolution
                                        state.action_result =
                                            spell_resolution.action_result
                                        if type(spell_resolution.next_action_kind) ==
                                               "string" and
                                           type(spell_resolution.next_action_code) ==
                                               "number" and
                                           type(spell_resolution.next_profile_id) ==
                                               "string" then
                                            state.pending_source_action = {
                                                profile_id =
                                                    spell_resolution.next_profile_id,
                                                action_kind =
                                                    spell_resolution.next_action_kind,
                                                action_code =
                                                    spell_resolution.next_action_code,
                                            }
                                        end
                                        if type(spell_resolution.combatant) ==
                                           "table" then
                                            state.combatant =
                                                spell_resolution.combatant
                                        end
                                        if type(spell_resolution.audio_event) ==
                                           "number" then
                                            audio_event =
                                                spell_resolution.audio_event
                                        end
                                    end
                                end
                                R.begin_spell_cast(state, context, spell_target,
                                                 spell_resolution)
                            elseif state.animation_id ~= "spell" then
                                local route_heading =
                                    R.source_motion_destination_heading(
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
                            local route_heading =
                                R.source_motion_destination_heading(
                                    context, state, target)
                            state.movement_heading = route_heading
                            state.movement_active = route_heading ~= nil
                            if not state.movement_active then
                                state.movement_route_distance = nil
                            end
                            state.source_action_dispatch =
                                R.classify_item_dispatch(
                                    state, context, target)
                        elseif state.damage_reaction_active then
                            -- 0xB07E owns the service through the B03E cue.
                            -- Starting 0xB1DC on that frame replaces C072
                            -- and publishes the cue under the action pose.
                            if state.hurt_audio_emitted and
                               state.animation_id ~= "down" then
                                state.damage_reaction_active = false
                            end
                        elseif not R.resolve_source_target_motion(
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
                                R.begin_dynamic_target_action(
                                    state, context, target)
                        end
                    end

                    -- 0xA8EA invokes the actor's +0x3A continuation every
                    -- service pass. 0xB574 slews +0x2E toward the stored
                    -- +0x34 heading by 0x0800 and advances using +0xC8, which
                    -- 0x145A2 measured to the selected route destination.
                    if state.transition_complete and state.movement_active then
                        state.movement_current_heading = R.approach_heading(
                            state.movement_current_heading,
                            state.movement_heading)
                        state.movement_step = R.finish_entity_movement(
                            context,
                            R.advance_entity_movement(context, state))
                        if state.movement_step ~= nil and
                           state.movement_step.state ~= nil then
                            state.direction = R.movement_direction_from_heading(
                                context,
                                state.movement_step.state.animation_heading)
                        end
                    end
                end
            else
                state.movement_heading =
                    R.requested_movement_heading(context, profile, state)
                if state.movement_heading ~= nil then
                    state.movement_current_heading = R.approach_heading(
                        state.movement_current_heading,
                        state.movement_heading)
                    state.movement_step = R.finish_entity_movement(
                        context, R.advance_entity_movement(context, state))
                    if state.movement_step ~= nil and
                       state.movement_step.state ~= nil then
                        state.direction = R.direction_from_heading(
                            state.movement_step.state.animation_heading)
                    end
                end
            end
        end
        return R.snapshot(state, action, audio_event, nil,
                        target_heading_audio_event)
    end

    if context.phase == "render" then
        -- ROM 0x4618 draws the record's current pose. A default-list actor is
        -- placed live by 0x43B6/0x4466 with a nonzero +0x4C meter, so it
        -- shows its idle (C052) pose. C0B2 is selected only by 0xB330 after
        -- the meter reaches zero; it is never a spawn sequence.
        return R.snapshot(state, action, nil)
    end
    state.movement_heading = nil

    -- The seeded delay is the +0xB6 think timer 0x13904 counts down; the
    -- actor stays live (idle, hittable) meanwhile. No death list, no cue.
    state.transition_started = true
    if not state.transition_complete then
        if state.entry_delay == 0 then
            state.transition_complete = true
            state.behavior_timer = 0
        else
            state.entry_delay = state.entry_delay - 1
        end
    end

    return R.snapshot(state, action, audio_event)
end

-- ROM 0x5D29C scans records FF0500..FF0A00 when the effect animation reaches
-- frame 3. Those addresses map to typed host slots 4..9 (FF0400 is slot 3).
-- Lua evaluates the supplied actors' authoritative service flags and meters.
function ActorAI.has_world_scene_effect_target(actor_ids)
    if type(actor_ids) ~= "table" then
        return nil
    end
    for index = 1, 6 do
        local actor_id = actor_ids[index]
        if actor_id ~= nil and actor_id ~= "" then
            if type(actor_id) ~= "string" then
                return nil
            end
            local state = states[actor_id]
            local physical_meter = type(state) == "table" and
                state.source_physical_meter or nil
            if physical_meter == nil and type(state) == "table" and
               type(state.combatant) == "table" then
                physical_meter = state.combatant.physical_meter
            end
            if type(state) ~= "table" or
               state.source_record_slot ~= index + 3 or
               type(state.source_record_flags) ~= "number" or
               state.source_record_flags < 0 or
               state.source_record_flags > 0xff or
               type(physical_meter) ~= "number" or
               physical_meter < 0 or physical_meter > 0xff then
                return nil
            end
            local flags = state.source_record_flags
            if (flags & ActorStatus.ACTIVE) ~= 0 and
               (flags & ActorStatus.DOWN) == 0 and
               (flags & ActorStatus.DEAD) == 0 and
               physical_meter ~= 0 then
                return true
            end
        end
    end
    return false
end


-- register this module's definitions
return ActorAI
