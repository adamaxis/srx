-- actor/heartbeat.lua: logic: ActorAI.heartbeat facade (public entry point, unchanged signature).
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local ActorStatus = E.ActorStatus
local ActorAI = {}
local states = game.actors
local pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot
local spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot
local ActionKind = R.ActionKind   -- from actor/tables.lua
local function advance_dynamic_actor_action_animation(...) return R.advance_dynamic_actor_action_animation(...) end   -- from actor/anim.lua
local function advance_dynamic_actor_item_action_animation(...) return R.advance_dynamic_actor_item_action_animation(...) end   -- from actor/anim.lua
local function advance_dynamic_actor_walk_animation(...) return R.advance_dynamic_actor_walk_animation(...) end   -- from actor/anim.lua
local function advance_entity_movement(...) return R.advance_entity_movement(...) end   -- from actor/movement.lua
local function advance_entry_animation(...) return R.advance_entry_animation(...) end   -- from actor/anim.lua
local function advance_finite_actor_animation(...) return R.advance_finite_actor_animation(...) end   -- from actor/anim.lua
local function advance_player_animation(...) return R.advance_player_animation(...) end   -- from actor/anim.lua
local function advance_received_hit_reaction(...) return R.advance_received_hit_reaction(...) end   -- from actor/anim.lua
local function advance_source_action_delay(...) return R.advance_source_action_delay(...) end   -- from actor/actions.lua
local function advance_spell_effect(...) return R.advance_spell_effect(...) end   -- from actor/actions.lua
local function advance_target_marker(...) return R.advance_target_marker(...) end   -- from actor/actions.lua
local function advance_target_motion_effect(...) return R.advance_target_motion_effect(...) end   -- from actor/actions.lua
local function advance_world_target_selection(...) return R.advance_world_target_selection(...) end   -- from actor/actions.lua
local function approach_heading(...) return R.approach_heading(...) end   -- from actor/targeting.lua
local function begin_dynamic_item_action(...) return R.begin_dynamic_item_action(...) end   -- from actor/actions.lua
local function begin_dynamic_target_action(...) return R.begin_dynamic_target_action(...) end   -- from actor/actions.lua
local function begin_received_hit_reaction(...) return R.begin_received_hit_reaction(...) end   -- from actor/anim.lua
local function begin_spell_cast(...) return R.begin_spell_cast(...) end   -- from actor/actions.lua
local function classify_item_dispatch(...) return R.classify_item_dispatch(...) end   -- from actor/actions.lua
local function classify_spell_dispatch(...) return R.classify_spell_dispatch(...) end   -- from actor/actions.lua
local function direction_from_heading(...) return R.direction_from_heading(...) end   -- from actor/targeting.lua
local function finish_entity_movement(...) return R.finish_entity_movement(...) end   -- from actor/movement.lua
local function first_wounded_party_runner(...) return R.first_wounded_party_runner(...) end   -- from actor/actions.lua
local idle_pose_selectors = R.idle_pose_selectors   -- from actor/tables.lua
local function initial_state(...) return R.initial_state(...) end   -- from actor/targeting.lua
local function projectile_heartbeat(...) return R.projectile_heartbeat(...) end   -- from actor/movement.lua
local function requested_movement_heading(...) return R.requested_movement_heading(...) end   -- from actor/movement.lua
local function resolve_source_target_motion(...) return R.resolve_source_target_motion(...) end   -- from actor/movement.lua
local function retained_linked_target(...) return R.retained_linked_target(...) end   -- from actor/targeting.lua
local function same_actor(...) return R.same_actor(...) end   -- from actor/targeting.lua
local function snapshot(...) return R.snapshot(...) end   -- from actor/anim.lua
local function source_motion_destination_heading(...) return R.source_motion_destination_heading(...) end   -- from actor/targeting.lua
local function source_profile(...) return R.source_profile(...) end   -- from actor/targeting.lua
local function transfer_pending_spell_effect(...) return R.transfer_pending_spell_effect(...) end   -- from actor/actions.lua
local walk_pose_selectors = R.walk_pose_selectors   -- from actor/tables.lua

function ActorAI.heartbeat(context)
    if type(context) ~= "table" then
        return nil
    end
    local profile = source_profile(context)
    if profile == nil or type(context.actor_id) ~= "string" or
       context.actor_id == "" or type(context.initial_heading) ~= "number" or
       type(context.entry_delay) ~= "number" or context.entry_delay < 0 or
       context.entry_delay >
           (context.dynamic_world_encounter_actor and EntryDelayMax.DYNAMIC_ENCOUNTER or EntryDelayMax.STATIC) or
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
        transfer_pending_spell_effect(state, context)
        advance_world_target_selection(state, context)
        advance_spell_effect(state, context)
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
    if not same_actor(state, context, profile) then
        return nil
    end
    state.source_action_dispatch = nil
    state.spell_action_resolution = nil
    state.source_record_flags = context.source_record_flags or
                                state.source_record_flags
    state.source_physical_meter = context.source_physical_meter or
                                  state.source_physical_meter
    state.source_record_slot = context.source_record_slot or
                               state.source_record_slot
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
    advance_source_action_delay(state, context)
    if context.action_kind == ActionKind.item and
       context.source_item_spawn_result == "spawned" then
        local target = retained_linked_target(context, state)
        if not begin_dynamic_item_action(state, context, target) then
            return nil
        end
    end
    transfer_pending_spell_effect(state, context)
    advance_world_target_selection(state, context)
    if profile.projectile_animation then
        return projectile_heartbeat(state, context)
    end
    advance_target_marker(state, context)
    advance_target_motion_effect(state, context)
    advance_spell_effect(state, context)

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
                if state.transition_started and
                   not state.transition_complete then
                    advance_entry_animation(state)
                elseif state.animation_id == "action" then
                    if not advance_dynamic_actor_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "weapon" then
                    if not advance_dynamic_actor_item_action_animation(state) then
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
                    state.source_animation_selector =
                        walk_pose_selectors[state.direction + 1]
                else
                    local facing_heading = state.ai_target_engaged
                        and state.ai_target_heading or
                        state.movement_current_heading or
                        context.initial_heading
                    state.direction = direction_from_heading(facing_heading)
                    state.animation_id = "idle"
                    state.animation_frame = 0
                    state.source_animation_selector =
                        idle_pose_selectors[state.direction + 1]
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
                            local spell_target = target
                            if context.action_code == 12 then
                                spell_target =
                                    first_wounded_party_runner(context)
                            end
                            if context.action_code == 12 and
                               spell_target == nil then
                                -- 0x13BBA returns carry clear when 0x13C20
                                -- finds no eligible runner lane.
                                state.source_action_dispatch = "suppressed"
                            else
                                state.source_action_dispatch =
                                    classify_spell_dispatch(
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
                                begin_spell_cast(state, context, spell_target,
                                                 spell_resolution)
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
                            local route_heading =
                                source_motion_destination_heading(
                                    context, state, target)
                            state.movement_heading = route_heading
                            state.movement_active = route_heading ~= nil
                            if not state.movement_active then
                                state.movement_route_distance = nil
                            end
                            state.source_action_dispatch =
                                classify_item_dispatch(
                                    state, context, target)
                        elseif state.damage_reaction_active then
                            -- 0xB07E owns the service through the B03E cue.
                            -- Starting 0xB1DC on that frame replaces C072
                            -- and publishes the cue under the action pose.
                            if state.hurt_audio_emitted and
                               state.animation_id ~= "down" then
                                state.damage_reaction_active = false
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
