-- actor/heartbeat.lua: logic: ActorAI.heartbeat facade (public entry point, unchanged signature).
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local u16, s16 = I.u16, I.s16
local SoundId, SceneId, EntryDelayMax, AiTuning =
    E.SoundId, E.SceneId, E.EntryDelayMax, E.AiTuning
local VisualProfile = E.ActorVisualProfile
local ActorLifecycleEvent = E.ActorLifecycleEvent
local ActionMovementLockoutKind = E.ActionMovementLockoutKind

-- ROM 0x1E514 clears the escort relationship and deactivates the actor. The
-- host clears its typed template marker and retires the live record slot.
local function deactivate_record(state)
    state.actor_facts.behavior.escort = false
    state.actor_facts.lifecycle.active = false
end
local ActorAI = {}
local states = game.actors
local ActionKind = R.ActionKind   -- from actor/tables.lua

function ActorAI.should_rebuild_target_lists(frame_counter, encounter_flag)
    return R.should_rebuild_target_lists(frame_counter, encounter_flag)
end

function ActorAI.select_world_service_target(policy, candidates)
    local selected_actor_id = R.select_world_service_target(policy, candidates)
    return { selected_actor_id = selected_actor_id or "" }
end

function ActorAI.heartbeat(context)
    if type(context) ~= "table" then
        return nil
    end
    local profile = R.source_profile(context)
    if profile == nil or type(context.actor_id) ~= "string" or
       context.actor_id == "" or type(context.initial_bearing_degrees) ~= "number" or
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
            if state.combat_action_requested then
                -- Latch the successful-action start for the following render
                -- heartbeat; the finite C042 pose has its own lifetime.
                state.player_weapon_animation_start_pending = true
                if context.player_weapon_action_will_fire == true then
                    R.begin_action_movement_lockout(
                        state, ActionMovementLockoutKind.WEAPON)
                end
            end
            state.movement_step = nil
            if context.phase == "behavior" then
                state.movement_bearing_degrees =
                    R.requested_movement_bearing_degrees(context, profile, state)
                if state.movement_bearing_degrees ~= nil then
                    state.movement_current_bearing_degrees = R.approach_bearing_degrees(
                        state.movement_current_bearing_degrees,
                        state.movement_bearing_degrees)
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
    state.strike_bearing_degrees = nil
    state.spell_action_resolution = nil
    local incoming_facts = context.actor_facts
    if type(incoming_facts) ~= "table" then
        return nil
    end
    state.actor_facts = incoming_facts
    if state.damage_reaction_active and state.animation_id == "down" and
       not state.spell_death then
        -- 0xB330 leaves the downed state set throughout its ordinary reaction.
        state.actor_facts.lifecycle.downed = true
    end
    state.physical_condition = context.physical_condition or
                               state.physical_condition
    state.live_field_actor = context.live_field_actor == true or
                             state.live_field_actor
    state.scene_id = context.scene_id or state.scene_id
    state.ammunition_remaining = context.ammunition_remaining or
                                state.ammunition_remaining
    if context.phase == "behavior" and
       state.pending_lifecycle_event ==
           ActorLifecycleEvent.ESCORT_CONCLUDED then
        state.pending_lifecycle_event = nil
        if state.actor_facts.lifecycle.active and
           state.actor_facts.behavior.escort then
            -- ROM 0x1E514 clears +0xB0 bit 7 and +0x10 bit 0. The field host
            -- observes the inactive typed record and retires its live slot.
            deactivate_record(state)
            return R.snapshot(state, "hold", nil)
        end
    end
    R.advance_source_action_delay(state, context)
    R.advance_action_movement_lockout(state, context)
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
    if type(context.controlled_party_member_id) == "string" and
       context.controlled_party_member_id ~= "" and
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
        context.combatant.is_party_member == false) then
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
            local death_transition_completed = false
            if context.phase == "behavior" then
                state.think_token_claimed = false
                state.ai_deactivate_requested = false
            end
            if context.phase == "render" then
                state.movement_step = nil
                -- The death list's ROM rate/count for this kind; the art
                -- count stands in when the list has no extracted frames.
                local animation = R.death_list(state)
                if not R.advance_finite_actor_animation(state, animation) then
                    return nil
                end
            elseif state.damage_reaction_animation_complete and
                   not state.actor_facts.lifecycle.dead then
                death_transition_completed = true
                -- ROM 0xA93E: the finished C0B2 list sets pose bit 15, so the
                -- next 0xA8EA service enters 0xAB02 once: drops, FFE177--,
                -- and bset #6,+0x10 (DEAD, 0x40). The body stays drawn.
                state.actor_facts.lifecycle.dead = true
                -- ROM 0xA9DE/0xAA14: a Wendigo (kind 0xE, 14) whose list is
                -- finished takes 0xA9BE instead: 0xAB02 then 0x1E514, so the
                -- record leaves the field after its death list.
                -- 0xA9A6: a finished 0xB0D4/0xB10C spell-death list does the
                -- same for every kind.
                if context.visual_profile_id == VisualProfile.WENDIGO or
                   state.spell_death then
                    deactivate_record(state)
                end
            end
            if context.phase == "behavior" and
               state.actor_facts.lifecycle.active and
               not state.actor_facts.lifecycle.dead and
               not reaction_started and not death_transition_completed then
                -- A down reaction still reaches the 0x13904 coordinator;
                -- its 0x13962 retirement check precedes the remaining think
                -- paths. Completed ordinary corpses skip 0xAC84 at 0xA91C.
                local _, think_audio_event = R.think(state, context)
                if think_audio_event ~= nil then
                    reaction_audio_event = think_audio_event
                end
                if state.ai_deactivate_requested then
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
                    state.visual_profile_id, R.Animator.Clip.HURT, state.direction)
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
    if profile.controls_movement or profile.dynamic_encounter_only or
       profile.source_profile_patrol then
        if profile.controls_movement then
            -- ROM 0xACAE observes the held A bit and only dispatches a player
            -- action after the source +0xDC delay gate expires. The host
            -- supplies those typed facts; Lua owns the input policy.
            state.combat_action_requested =
                context.phase == "behavior" and
                context.combat_action_available == true and
                context.combat_confirm_held == true and
                context.combat_action_ready == true and
                context.combat_input_blocked ~= true
            if state.combat_action_requested then
                -- A held-fire repeat is a new C042 action and resets its
                -- animation in the next render pass, after the host action.
                state.player_weapon_animation_start_pending = true
                if context.player_weapon_action_will_fire == true then
                    R.begin_action_movement_lockout(
                        state, ActionMovementLockoutKind.WEAPON)
                end
            end
        end
        if profile.controls_movement and not profile.dynamic_encounter_only then
            local cast_audio = R.begin_player_spell_cast(state, context)
            if cast_audio ~= nil then
                audio_event = cast_audio
            end
            if state.source_action_dispatch == "player_spell" or
               state.source_action_dispatch == "player_spell_blocked" or
               state.source_action_dispatch == "player_spell_suppressed" or
               state.source_action_dispatch == "player_spell_no_target" then
                R.begin_action_movement_lockout(
                    state, ActionMovementLockoutKind.SPELL)
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
            if not profile.dynamic_encounter_only and
               not profile.source_profile_patrol then
                state.movement_bearing_degrees = nil
            end
            if profile.dynamic_encounter_only or profile.source_profile_patrol then
                if state.animation_id == "action" then
                    if not R.advance_dynamic_actor_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "weapon" then
                    if not R.advance_dynamic_actor_item_action_animation(state) then
                        return nil
                    end
                elseif state.animation_id == "spell" then
                    if state.action_animation_complete then
                        local facing_bearing_degrees =
                            state.movement_current_bearing_degrees or
                            context.initial_bearing_degrees
                        state.direction = R.direction_from_bearing_degrees(
                            facing_bearing_degrees)
                        state.animation_id = "idle"
                        state.animation_frame = 0
                        if not R.advance_dynamic_actor_idle_animation(
                               state) then
                            return nil
                        end
                    else
                        -- C082 lists are one frame; retain the source cast pose
                        -- through its measured 58-pass movement lockout.
                        state.animation_frame = 0
                        state.animation_progress = 0
                    end
                elseif state.animation_id == "walk" then
                    if not R.advance_dynamic_actor_walk_animation(state) then
                        return nil
                    end
                else
                    local facing_bearing_degrees = state.ai_target_engaged
                        and state.ai_target_bearing_degrees or
                        state.movement_current_bearing_degrees or
                        context.initial_bearing_degrees
                    state.direction = R.direction_from_bearing_degrees(
                        facing_bearing_degrees)
                    state.animation_id = "idle"
                    state.animation_frame = 0
                    if not R.advance_dynamic_actor_idle_animation(state) then
                        return nil
                    end
                end
            else
                R.advance_player_animation(state, context)
            end
        else
            state.movement_step = nil
            if profile.dynamic_encounter_only or profile.source_profile_patrol then
                if not state.transition_complete then
                    -- Source-profile list actors and dynamic encounters wait
                    -- on the seeded +0xB6 behavior delay before 0x13904
                    -- starts their patrol or target policy.
                    state.transition_started = true
                    if state.entry_delay == 0 then
                        state.transition_complete = true
                        state.behavior_timer = 0
                    else
                        state.entry_delay = state.entry_delay - 1
                    end
                else
                    local think_handled, think_audio_event =
                        R.think(state, context)
                    if think_audio_event ~= nil then
                        audio_event = think_audio_event
                    end
                    if think_handled then
                        if state.ai_deactivate_requested then
                            deactivate_record(state)
                            return R.snapshot(state, "hold", nil)
                        end
                    else
                        -- 0x13904 refreshes target/routing every 21 actor
                        -- service passes; +0x3A continues movement each pass.
                        -- Lua's typed patrol/follow destination remains active
                        -- here while the source continuation advances it [C].
                        R.requested_movement_bearing_degrees(context, profile, state)
                        local target
                        if context.action_kind == ActionKind.item or
                           context.action_kind == ActionKind.spell then
                            -- ROM 0x13FFE selects the linked source record
                            -- before dispatch. Routed default-list item actors
                            -- do not enter the dynamic-movement selector.
                            target = R.linked_target(context, state)
                            if type(target) == "table" then
                                state.ai_target_id = target.id
                                state.ai_target_bearing_degrees = target.bearing_degrees
                                state.ai_target_distance = target.distance
                            end
                        else
                            target = R.retained_linked_target(context, state)
                        end
                        if context.action_kind == ActionKind.spell then
                            -- Negative +0x56 never enters 0x14C7C / 0xB1DC.
                            state.ai_target_engaged = false
                            local spell_target = target
                            if context.action_spell_id == "heal_wounds" then
                                spell_target =
                                    R.first_wounded_party_runner(context)
                            end
                            if context.action_spell_id == "heal_wounds" and
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
                                        state.last_action_successes =
                                            spell_resolution.successes
                                        if type(spell_resolution.next_action_kind) ==
                                               "string" and
                                           type(spell_resolution.next_profile_prefix) ==
                                               "string" and
                                           ((spell_resolution.next_action_kind ==
                                                 "item" and
                                             type(spell_resolution.next_action_item_id) ==
                                                 "string") or
                                            (spell_resolution.next_action_kind ==
                                                 "none" and
                                             spell_resolution.next_action_item_id ==
                                                 nil)) then
                                            state.pending_source_action = {
                                                profile_prefix =
                                                    spell_resolution.next_profile_prefix,
                                                action_kind =
                                                    spell_resolution.next_action_kind,
                                                action_item_id =
                                                    spell_resolution.next_action_item_id,
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
                                    R.source_motion_destination_bearing_degrees(
                                        context, state, target)
                                state.movement_bearing_degrees = route_heading
                                state.movement_active = route_heading ~= nil
                                if not state.movement_active then
                                    state.movement_route_distance = nil
                                end
                            end
                        elseif context.action_kind == ActionKind.item then
                            -- Nonnegative +0x56 never enters 0x14C7C / 0xB1DC.
                            state.ai_target_engaged = false
                            local route_heading =
                                R.source_motion_destination_bearing_degrees(
                                    context, state, target)
                            state.movement_bearing_degrees = route_heading
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
                               type(target.bearing_degrees) == "number" and
                               type(target.distance) == "number" then
                                route_heading = target.bearing_degrees
                                state.movement_route_distance =
                                    target.distance & 0xffff
                            else
                                state.movement_route_distance = nil
                            end
                            state.movement_bearing_degrees = route_heading
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
                    if state.transition_complete and state.movement_active and
                       not R.action_movement_locked(state) then
                        state.movement_current_bearing_degrees = R.approach_bearing_degrees(
                            state.movement_current_bearing_degrees,
                            state.movement_bearing_degrees)
                        state.movement_step = R.finish_entity_movement(
                            context,
                            R.advance_entity_movement(context, state))
                        if state.movement_step ~= nil and
                           state.movement_step.state ~= nil then
                            state.direction = R.movement_direction_from_bearing_degrees(
                                context,
                                state.movement_step.state.animation_bearing_degrees)
                        end
                    end
                end
            else
                state.movement_bearing_degrees =
                    R.requested_movement_bearing_degrees(context, profile, state)
                if state.movement_bearing_degrees ~= nil then
                    state.movement_current_bearing_degrees = R.approach_bearing_degrees(
                        state.movement_current_bearing_degrees,
                        state.movement_bearing_degrees)
                    state.movement_step = R.finish_entity_movement(
                        context, R.advance_entity_movement(context, state))
                    if state.movement_step ~= nil and
                       state.movement_step.state ~= nil then
                        state.direction = R.direction_from_bearing_degrees(
                            state.movement_step.state.animation_bearing_degrees)
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
    state.movement_bearing_degrees = nil

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

-- The host supplies stable IDs for actors eligible for the current scene
-- effect. Lua owns the decision from each actor's current gameplay state.
function ActorAI.has_world_scene_effect_target(actor_ids)
    if type(actor_ids) ~= "table" then
        return nil
    end
    for _, actor_id in ipairs(actor_ids) do
        if type(actor_id) ~= "string" or actor_id == "" then
            return nil
        end
        local state = states[actor_id]
        local physical_condition = type(state) == "table" and
            state.physical_condition or nil
        if physical_condition == nil and type(state) == "table" and
           type(state.combatant) == "table" then
            physical_condition = state.combatant.physical_meter
        end
        if type(state) ~= "table" or
           type(state.actor_facts) ~= "table" or
           type(state.actor_facts.lifecycle) ~= "table" or
           type(state.actor_facts.lifecycle.active) ~= "boolean" or
           type(state.actor_facts.lifecycle.downed) ~= "boolean" or
           type(state.actor_facts.lifecycle.dead) ~= "boolean" or
           type(physical_condition) ~= "number" or
           physical_condition < 0 then
            return nil
        end
        local lifecycle = state.actor_facts.lifecycle
        if lifecycle.active and not lifecycle.downed and
           not lifecycle.dead and physical_condition ~= 0 then
            return true
        end
    end
    return false
end


-- register this module's definitions
return ActorAI
