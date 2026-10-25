-- actor/actions.lua: logic: action delays, target markers, world target selection, spell/item dispatch.
-- Split from the pre-module gameplay/scripts/actor_ai.lua; see docs/ghidra/lua-actor-ai-module-boundaries.md.
local R = require("actor.registry")
local E = require("enums")
local I = require("lib.int68k")
local Delay = require("actor.delay")
local u8, u16, s16 = I.u8, I.u16, I.s16
local BehaviorFlag, SoundId, SceneId, Collision, EntryDelayMax, AiTuning =
    E.BehaviorFlag, E.SoundId, E.SceneId, E.Collision, E.EntryDelayMax, E.AiTuning
local states = game.actors
R.pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot or {}
R.spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot or {}
local pending_spell_effects_by_source_slot = R.pending_spell_effects_by_source_slot
local spell_effect_scans_by_source_slot = R.spell_effect_scans_by_source_slot
local BehaviorFlag = R.BehaviorFlag   -- from actor/tables.lua
local ActorStatus = R.ActorStatus   -- from actor/tables.lua
local HitSpark = E.HitSpark
local item_action_pose_selectors = R.item_action_pose_selectors   -- from actor/tables.lua
local spell_pose_selectors = R.spell_pose_selectors   -- from actor/anim.lua
local Animator = R.Animator   -- from actor/animator.lua

local source_action_delay = Delay.actor_action_delay

local function advance_source_action_delay(state, context)
    if context.phase ~= "behavior" or
       not context.dynamic_world_encounter_actor then
        return
    end
    -- ROM 0xA8EA decrements active actor +0xDC once before the actor-service
    -- continuation, while the value is nonnegative.
    if state.action_delay >= 0 then
        state.action_delay = state.action_delay - 1
    end
end

local function advance_target_motion_effect(state, context)
    state.target_motion_effect_slot_request = nil
    state.target_motion_effect_slot_released = false
    state.target_motion_effect_visible = false
    if context.phase ~= "render" or
       not state.target_motion_effect_active then
        return
    end

    -- ROM 0x4E86 decrements the signed byte at +0x02, reloads 6 on
    -- underflow, and advances +0x03. Phases 1..3 draw; phase 4 clears the
    -- shared slot and +0x11.6 without emitting a sprite.
    state.target_motion_effect_countdown =
        state.target_motion_effect_countdown - 1
    if state.target_motion_effect_countdown < 0 then
        state.target_motion_effect_countdown = 6
        state.target_motion_effect_phase =
            state.target_motion_effect_phase + 1
        if state.target_motion_effect_phase >= 4 then
            state.target_motion_effect_active = false
            state.target_motion_effect_slot = nil
            state.target_motion_effect_slot_released = true
            return
        end
    end
    state.target_motion_effect_visible = true
end

local function request_target_motion_effect(state, context, heading, target)
    for slot = 0, 17 do
        if context.world_effect_slot_occupancy[slot + 1] ~= true then
            state.target_motion_effect_active = true
            state.target_motion_effect_slot = slot
            state.target_motion_effect_slot_request = slot
            state.target_motion_effect_countdown = 0
            state.target_motion_effect_phase = 0
            state.target_motion_effect_direction =
                R.direction_from_heading(heading)
            -- ROM 0x14C7C sets +0x11.6 for a close, delayed Hell Hound with
            -- a free effect slot; 0x13FFE dispatches 0xB11C. That path uses
            -- 0x55B16 and spawns its 0xB146 child only when +0x4C or +0x9C
            -- changes [C].
            local combat = game.modules.combat
            local update = type(combat) == "table" and
                type(combat.resolve_default_bonus_action) == "function" and
                combat.resolve_default_bonus_action(context, target) or nil
            if type(update) == "table" and
               type(update.combatant) == "table" then
                state.combatant = update.combatant
                state.action_result = update.combatant.action_result or
                    state.action_result
            end
            state.combat_meter_update = update
            if type(update) == "table" and update.changed == true and
               type(target) == "table" and
               type(target.world_x) == "number" and
               type(target.world_y) == "number" then
                state.hit_spark_spawn_request = {
                    target_id = target.id,
                    world_x = target.world_x,
                    world_y = target.world_y,
                }
            end
            -- ROM 0xB19A resets +0xDC after the optional child allocation.
            state.action_delay = source_action_delay(context, state)
            return true
        end
    end
    return false
end

local function advance_target_marker(state, context)
    if context.phase ~= "render" then
        return
    end
    if context.target_selection_latched ~= true then
        state.target_selection_latched = false
        state.target_marker_visible = false
        state.target_marker_animating = false
        state.target_marker_style = 0
        state.target_marker_radius = 0
        state.target_marker_countdown = 0
        return
    end

    -- ROM 0x5C7E tiers the TARGET's condition meter, not an action byte:
    -- d0 = +0x4C (physical), or +0x9C (stun) when the leader record's
    -- weapon rule says so (target_marker_reads_stun_meter, 0x5C82..0x5CBA).
    -- <3 -> 0x5D92 (tile 0x5E7 pal 1), 3..5 -> 0x5DBE (0x5E5),
    -- 6..8 -> 0x5DA8 (0x5E6), >=9 -> 0x5DD4 (0x5E7 pal 3).
    local combatant = type(state.combatant) == "table" and state.combatant
        or (type(context.combatant) == "table" and context.combatant or nil)
    local meter
    if context.target_marker_reads_stun_meter == true then
        meter = combatant and combatant.mental_meter
    else
        meter = combatant and combatant.physical_meter
        if type(meter) ~= "number" then
            meter = context.source_physical_meter
        end
    end
    if type(meter) ~= "number" then
        meter = 0
    end
    if meter >= 9 then
        state.target_marker_style = 3
    elseif meter >= 6 then
        state.target_marker_style = 2
    elseif meter >= 3 then
        state.target_marker_style = 1
    else
        state.target_marker_style = 0
    end

    if not state.target_selection_latched then
        -- ROM 0x1DC4C seeds +0xA6 with -0x7FF7. The render pass then calls
        -- 0x5C7E, which decrements that signed word before placing its four
        -- corner sprites at an 8-pixel-per-step shrinking radius.
        state.target_marker_countdown = -0x7ff7
    end
    state.target_selection_latched = true
    state.target_marker_visible = true

    local countdown = state.target_marker_countdown or 0
    if countdown < 0 then
        local next_word = u16(countdown - 1)
        local next_countdown = next_word >= 0x8000
            and next_word - 0x10000 or next_word
        if next_countdown < 0 then
            state.target_marker_countdown = next_countdown
            state.target_marker_animating = true
            state.target_marker_radius = (next_word & 0x7f) * 8
        else
            state.target_marker_countdown = 0
            state.target_marker_animating = false
            state.target_marker_radius = 0
        end
    else
        state.target_marker_animating = false
        state.target_marker_radius = 0
    end
end

local function advance_world_target_selection(state, context)
    state.target_selection_processed = false
    if context.phase ~= "behavior" or
       context.target_selection_requested ~= true then
        return
    end

    local candidates_by_slot = {}
    for _, candidate in ipairs(context.world_target_selection_candidates or {}) do
        local slot = candidate.source_record_slot
        if type(slot) == "number" and slot % 1 == 0 and
           slot >= 0 and slot < 16 then
            candidates_by_slot[slot] = candidate
        end
    end

    local special_path = context.target_selection_special_marker == true and
        (context.target_selection_mode == 0x0c or
         context.target_selection_mode == 0x04 or
         context.target_selection_mode == 0x0b)
    state.selected_world_actor_id = nil
    local start
    if special_path then
        start = context.target_selection_current_slot
        if start == nil then
            start = context.target_selection_owner_slot
        end
        if start ~= nil and context.target_selection_advances_cursor then
            start = start + 1
        end
        if start ~= nil and
           (start >= 16 or
            ((candidates_by_slot[start] or {}).descriptor_class or 0) >= 3) then
            start = 0
        end
    else
        start = 3
        local previous = context.target_selection_current_slot
        if previous ~= nil then
            if previous < 9 then
                start = previous
                if context.target_selection_advances_cursor then
                    start = start + 1
                end
            end
        elseif context.target_selection_linked_slot ~= nil then
            start = context.target_selection_linked_slot
        end
        if start >= 16 then
            start = 3
        end
        if ((candidates_by_slot[start] or {}).descriptor_class or 0) < 3 then
            start = 3
        end
    end

    if start ~= nil then
        for offset = 0, 7 do
            local candidate = candidates_by_slot[start + offset]
            if candidate ~= nil then
                local available =
                    (candidate.service_flags & ActorStatus.ACTIVE) ~= 0
                if special_path then
                    if available and type(candidate.id) == "string" and
                       candidate.id ~= "" then
                        state.selected_world_actor_id = candidate.id
                        break
                    end
                else
                    available = available and
                        (candidate.service_flags &
                         ActorStatus.DEAD) == 0 and
                        (candidate.service_flags &
                         ActorStatus.DOWN) == 0 and
                        (candidate.behavior_flags &
                         BehaviorFlag.ESCORT) == 0 and
                        candidate.projected_y_nonzero == true
                    if available and type(candidate.id) == "string" and
                       candidate.id ~= "" then
                        state.selected_world_actor_id = candidate.id
                        break
                    end
                end
            end
        end
    end

    state.target_selection_processed = true
end

local function action_line_clear(context, target)
    local line_clear = context.action_line_clear
    return type(line_clear) == "function" and
        line_clear(target.world_x, target.world_y) == true
end

local function first_wounded_party_runner(context)
    -- ROM 0x13C20 checks the three runner records in order and selects the
    -- first present lane whose physical meter is below four. The runtime
    -- candidate list contains party slots only when their roster marker is
    -- present; temporary world actors are never heal lanes.
    local by_slot = {}
    for _, candidate in ipairs(context.ai_target_candidates or {}) do
        local slot = type(candidate) == "table"
            and candidate.source_record_slot or nil
        if type(candidate) == "table" and
           candidate.world_activity_actor ~= true and
           type(candidate.service_flags) == "number" and
           (candidate.service_flags & ActorStatus.ACTIVE) ~= 0 and
           type(slot) == "number" and slot >= 0 and slot <= 2 and
           type(candidate.combatant) == "table" and
           type(candidate.combatant.physical_meter) == "number" then
            by_slot[slot] = candidate
        end
    end
    for slot = 0, 2 do
        local candidate = by_slot[slot]
        if type(candidate) == "table" and
           candidate.combatant.physical_meter < 4 then
            return candidate, slot
        end
    end
    return nil, nil
end

local function classify_spell_dispatch(state, context, target)
    -- 0x13FFE reaches the spell branch only with a linked target. Delay and
    -- the projected +0x0C word are tested before the code-4/11 RTS.
    if type(target) ~= "table" then
        return nil
    end
    if state.action_delay >= 0 then
        return "waiting"
    end
    if context.source_projection_active ~= true then
        return "offscreen"
    end
    if context.action_code == 4 or context.action_code == 11 then
        return "suppressed"
    end
    if not action_line_clear(context, target) then
        return "blocked"
    end
    return "spell"
end

local function classify_item_dispatch(state, context, target)
    -- 0x13FFE tests delay and +0x0C before the nonnegative +0x56 branch.
    -- Codes 13..15 reach 0x15646. Other nonzero codes reach the 0x1411E
    -- gate, which needs FFE186 or +0xB0 bit 4. Those facts are not hosted,
    -- so a clear probe is not a spawn and not a C042/C092 pose.
    if type(target) ~= "table" then
        return nil
    end
    if state.action_delay >= 0 then
        return "waiting"
    end
    if context.source_projection_active ~= true then
        return "offscreen"
    end
    if (state.source_animation_selector & 0x07ff) > 0x003c then
        return "item_pose_gated"
    end
    if not action_line_clear(context, target) then
        return "blocked"
    end
    local code = context.action_code
    if code >= 13 and code <= 15 then
        return "item_effect"
    end
    if code < 1 or code > 12 then
        return "item_gated"
    end
    if type(target.source_record_heading) ~= "number" then
        return "item_target_heading_unresolved"
    end
    if context.world_scene_state ~= 0 or
       (state.source_behavior_flags &
        BehaviorFlag.IN_BAND) ~= 0 then
        if state.source_action_ammo == 0 then
            return "item_empty"
        end
        return "item_spawn_requested"
    end
    return "item_gated"
end

local function select_c082_directional_pose(state, heading)
    if type(heading) ~= "number" then
        return nil
    end
    local direction = R.direction_from_heading(u16(heading))
    local selector = spell_pose_selectors[direction + 1]
    if type(selector) ~= "number" then
        return nil
    end
    state.direction = direction
    state.animation_id = "spell"
    state.animation_selector = selector
    state.source_animation_selector = selector
    state.animation_frame = 0
    state.animation_accumulator = 0
    return direction
end

local function begin_thon_action(state, target, line_clear)
    if type(target) ~= "table" or
       type(target.heading) ~= "number" then
        return false
    end
    local direction = select_c082_directional_pose(state, target.heading)
    if direction == nil then
        return false
    end

    -- ROM 0xAE6E aims +0x2E at the linked record; both 0xAF7C and 0xAFAE
    -- select the same directional +0x16 entry from the C082 list [C].
    state.movement_current_heading = u16(target.heading)
    state.ai_target_id = target.id
    state.ai_target_heading = u16(target.heading)
    state.ai_target_distance = target.distance
    state.ai_target_engaged = line_clear == true
    state.action_animation_complete = false
    return true
end

local function begin_dynamic_item_action(state, context, target)
    local heading = type(target) == "table" and
        target.source_record_heading or nil
    if type(heading) ~= "number" then
        return false
    end
    local direction = R.direction_from_heading(heading)
    local animation = Animator.list(state.entity_kind, Animator.Pose.ITEM,
                                    direction)
    if animation == nil or animation.finite ~= true then
        return false
    end

    -- ROM 0xB39A installs the C042 selector for the linked target heading,
    -- resets the finite animation list, and reloads the ordinary mode-0
    -- action delay. The host has already run 0x1514E and reports its result.
    state.direction = direction
    state.animation_id = "weapon"
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_increment = animation.rate
    state.animation_period = Animator.PERIOD
    state.source_animation_selector =
        item_action_pose_selectors[direction + 1]
    state.animation_selector = state.source_animation_selector
    state.item_action_complete = false
    state.action_delay = source_action_delay(context, state, 0)
    state.ai_target_engaged = false
    return true
end

local function begin_spell_cast(state, context, target, resolution)
    -- 0xAE6E aims +0x2E at the link, then AF7C/AFAE install C082 and reload
    -- +0xDC with mode 0. A successful 0x1033E action has already reserved an
    -- effect record through 0x15830; its 0x521E phase clock owns the delayed
    -- target callback.
    local heading = target.heading & 0xffff
    local direction = R.direction_from_heading(heading)
    state.direction = direction
    state.animation_id = "spell"
    state.animation_selector = spell_pose_selectors[direction + 1]
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.animation_increment = 0
    state.action_animation_complete = false
    state.action_delay = source_action_delay(context, state, 0)
    state.ai_target_engaged = false
    state.movement_heading = nil
    state.movement_active = false
    state.movement_route_distance = nil
    state.movement_step = nil
    state.combat_meter_update = nil
    state.combat_action_requested = false
    state.spell_effect_visible = false
    state.spell_effect_phase = 0
    state.spell_effect_action_code = 0
    state.spell_effect_slot_released = nil
    state.spell_effect_resolution = nil
    R.queue_spell_effect(state, context, target, resolution)
end

-- 0x1033E's successful dispatch reserves a shared effect record; the target
-- record's heartbeat owns the delayed 0x521E callback. Shared by NPC casts
-- (0x13FFE) and the field player's held-A cast (0xAE6E).
function R.queue_spell_effect(state, context, target, resolution)
    if type(resolution) == "table" and resolution.effect_started == true and
       type(resolution.spell_effect_slot) == "number" and
       type(target.id) == "string" and
       type(target.source_record_slot) == "number" then
        local magic = game.modules.magic
        local effect_profile = type(magic) == "table" and
            type(magic.actor_effect_profile) == "function" and
            magic.actor_effect_profile(context.action_code) or nil
        if type(effect_profile) ~= "table" then
            return
        end
        local pending = {
            action_code = context.action_code,
            action_result = resolution.action_result,
            target_id = target.id,
            source_record_slot = target.source_record_slot,
            effect_slot = resolution.spell_effect_slot,
            initial_countdown = effect_profile.initial_countdown,
            countdown = effect_profile.initial_countdown,
            phase = 0,
            phase_count = effect_profile.phase_count,
            repeat_countdown = effect_profile.repeat_countdown,
            caster_combatant = resolution.combatant,
            ai_target_candidates = context.ai_target_candidates,
            scene_id = context.movement_scene_id,
            caster_actor_id = state.actor_id,
            caster_source_flags = context.source_record_flags or 0,
            owns_effect_slot = true,
        }

        -- ROM 0x1597E's type-5 scan marks every live record under the 0x500
        -- route-cost bound. 0x5322 later consumes those marks one record at
        -- a time as the preceding sleep animation finishes.
        if context.action_code == 5 and
           type(context.ai_target_candidates) == "table" then
            for _, candidate in ipairs(context.ai_target_candidates) do
                if type(candidate) == "table" and
                   type(candidate.id) == "string" and
                   type(candidate.source_record_slot) == "number" and
                   type(candidate.service_flags) == "number" and
                   type(candidate.direct_route_step_cost) == "number" and
                   (candidate.service_flags & ActorStatus.ACTIVE) ~= 0 and
                   (candidate.service_flags & ActorStatus.DEAD) == 0 and
                   candidate.direct_route_step_cost < 0x0500 then
                    local marker = {
                        action_code = 5,
                        actor_id = candidate.id,
                        scene_id = context.movement_scene_id,
                    }
                    spell_effect_scans_by_source_slot[
                        candidate.source_record_slot] = marker
                    local candidate_state = type(candidate.id) == "string" and
                        states[candidate.id] or nil
                    if type(candidate_state) == "table" then
                        candidate_state.spell_effect_scan_marker = marker
                    end
                end
            end
        end

        -- Source record slots remain stable across the field-player alias
        -- used by the behavior and render passes (party.runner.N vs
        -- field_player_runner). Lua uses the slot to transfer the pending
        -- target-owned effect to the actor heartbeat that renders that record.
        if type(target.source_record_slot) == "number" then
            pending_spell_effects_by_source_slot[
                target.source_record_slot] = pending
        end
    end
end

-- ROM 0xACAE -> 0xAD86: a negative +0x56 sends held A to 0xAE6E. With a
-- linked +0xA2, 0xAE6E aims +0x2E and runs the 0x14D8E line probe. With no
-- link it skips the probe but still jumps to 0x1033E. That resolver's
-- 0x55740 reads +0x9B/+0x11 through A1, which 0xAE6E did not load. In the
-- player actor loop 0xA8EA, 0xAAA6 loads the prior record's +0x3A callback
-- into A1 and 0xAAB0 executes it after that record's action; the callback
-- can change A1 [C]. The first record also inherits A1 from the outer caller
-- [C].
-- The no-link 0x1033E path leaves A1 untouched through 0x55740, so its
-- defender reads are state-dependent and not a stable target [H]. We retain
-- the known attacker-side modifiers and omit those undefined stale bytes.
-- The deterministic no-link dispatch at 0x15830 is still modeled: only
-- action codes 7/14 reach the directional field barrier at 0x49A2 [C].
-- Every path returns through 0xAD94 to 0xAFAE/0xAF7C, both of which reload
-- delay mode 0 and set +0x16 = C082[dir(+0x2E)] [C].
function R.begin_player_spell_cast(state, context)
    if context.phase ~= "behavior" or
       context.player_spell_available ~= true or
       context.combat_confirm_held ~= true or
       context.combat_action_ready ~= true or
       context.combat_input_blocked == true then
        return nil
    end
    if context.world_scene_state == 3 then
        state.source_action_dispatch = "player_spell_suppressed"
        return nil
    end
    local target = type(context.ai_target_candidates) == "table" and
        context.ai_target_candidates[1] or nil
    if type(target) ~= "table" then
        local combat = game.modules.combat
        if type(combat) ~= "table" or
           type(combat.resolve_untargeted_spell_action) ~= "function" then
            return nil
        end
        local cast_context = setmetatable(
            {action_code = context.player_spell_code},
            {__index = context})
        local replaces_field_barrier =
            context.player_spell_code == E.SpellActionCode.SUPER_BARRIER or
            context.player_spell_code == E.SpellActionCode.BARRIER
        -- 0x49A2 resets the previous field barrier before reserving its new
        -- shared effect slot. Present that Lua-owned slot as free to the roll [C].
        local old_field_effect = state.player_field_spell_effect
        if replaces_field_barrier and
           type(old_field_effect) == "table" and
           type(context.world_effect_slot_occupancy) == "table" then
            local occupancy = {}
            for slot = 1, 18 do
                occupancy[slot] = context.world_effect_slot_occupancy[slot]
            end
            if type(old_field_effect.effect_slot) == "number" then
                occupancy[old_field_effect.effect_slot + 1] = false
            end
            cast_context.world_effect_slot_occupancy = occupancy
        end
        local resolution = combat.resolve_untargeted_spell_action(
            cast_context)
        if resolution == nil then
            return nil
        end
        state.source_action_dispatch = "player_spell"
        state.spell_action_resolution = resolution
        state.action_result = resolution.action_result
        state.combat_action_requested = false
        state.combatant = resolution.combatant
        if resolution.field_effect_kind == E.SpellEffectKind.FIELD_BARRIER then
            state.player_field_spell_effect = nil
            if resolution.effect_started and
               type(resolution.spell_effect_slot) == "number" then
                state.player_field_spell_effect = {
                    action_code = resolution.action_code,
                    effect_slot = resolution.spell_effect_slot,
                    remaining = resolution.action_result *
                        E.SpellTuning.FIELD_BARRIER_TICKS_PER_SUCCESS,
                }
            end
        end
        return resolution.audio_event
    end
    if not action_line_clear(context, target) then
        state.source_action_dispatch = "player_spell_blocked"
        return nil
    end
    local combat = game.modules.combat
    if type(combat) ~= "table" or
       type(combat.resolve_spell_action) ~= "function" then
        return nil
    end
    local cast_context = setmetatable(
        {action_code = context.player_spell_code},
        {__index = context})
    local resolution = combat.resolve_spell_action(cast_context, target)
    if resolution == nil then
        return nil
    end
    state.source_action_dispatch = "player_spell"
    state.spell_action_resolution = resolution
    state.combat_action_requested = false
    R.queue_spell_effect(state, cast_context, target, resolution)
    return resolution.audio_event
end

-- ROM 0xAD86: a non-negative +0x56 with +0x57 == 0 jumps to 0xB1DC, the
-- unarmed strike. It reloads the delay in mode 1 (0xABFC) and requests 0x4C
-- (0x55 when +0x0E == 0x0C). With a +0xA2 link it turns toward the target;
-- 0x15C84/0x15C58 then accepts only an active target closer than 0x200.
-- In range it requests 0x4D (0x4E when FFF9F8 bit 7) and runs the shared
-- 0x55C76 reaction. Returns swing audio and the optional heading audio.
-- [C] the target's 0xAFF2 hit pose is the incoming_hit reaction that
-- combat.commit_combatant_meters raises for world actors.
-- 0xB29E..0xB2EA installs the kind-1 child through 0x15A8C only when the
-- shared reaction changes a meter; this player-side decision returns that
-- spawn fact without changing enemy incoming_hit handling [C].
function R.begin_player_melee(state, context)
    if context.phase ~= "behavior" or
       context.player_melee_available ~= true or
       context.combat_confirm_held ~= true or
       context.combat_action_ready ~= true or
       context.combat_input_blocked == true then
        return nil
    end
    state.combat_action_requested = false
    local swing = context.entity_kind == 0x0C and SoundId.MELEE_SWING_ALT
        or SoundId.MELEE_SWING
    local target = type(context.ai_target_candidates) == "table" and
        context.ai_target_candidates[1] or nil
    if type(target) == "table" and type(target.heading) == "number" then
        -- ROM 0xB212: 0x15D3C self->target bearing to +0x34 and +0x2E.
        state.strike_heading = target.heading & 0xffff
    end
    -- ROM 0x15C58: target +0x10 bit 0 set, bit 6 clear, 0x14D58 < 0x200.
    local flags = type(target) == "table" and target.service_flags or nil
    if type(target) ~= "table" or type(target.distance) ~= "number" or
       type(flags) ~= "number" or
       (flags & ActorStatus.ACTIVE) == 0 or
       (flags & ActorStatus.DEAD) ~= 0 or
       (target.distance & 0xffff) >= HitSpark.RANGE_LIMIT then
        state.source_action_dispatch = "player_melee"
        return swing
    end
    local combat = game.modules.combat
    local update = type(combat) == "table" and
        type(combat.resolve_reaction) == "function" and
        combat.resolve_reaction(context, target) or nil
    state.source_action_dispatch = "player_melee_hit"
    state.combat_meter_update = update
    if update ~= nil and update.changed == true and
       type(target.world_x) == "number" and
       type(target.world_y) == "number" then
        -- ROM 0xB29E compares defender +0x4C/+0x9C after 0x55C76 and only
        -- allocates the 0x15A8C kind-1 child when either meter changed [C].
        state.hit_spark_spawn_request = {
            target_id = target.id,
            world_x = target.world_x,
            world_y = target.world_y,
        }
    end
    local heading_event = context.target_heading_audio_alternate
        and SoundId.TARGET_HEADING_ALT or SoundId.TARGET_HEADING
    return swing, heading_event
end

local function actor_matches_pending_spell_target(context, pending)
    if context.actor_id == pending.target_id then
        return true
    end
    local slot = pending.source_record_slot
    return slot ~= nil and context.source_record_slot == slot and
        pending.target_id == "party.runner." .. tostring(slot) and
        context.actor_id == "field_player_runner"
end

local function transfer_pending_spell_effect(state, context)
    local slot = context.source_record_slot
    if type(slot) ~= "number" then
        return
    end
    local pending = pending_spell_effects_by_source_slot[slot]
    if type(pending) ~= "table" then
        return
    end
    if pending.scene_id ~= context.movement_scene_id then
        pending_spell_effects_by_source_slot[slot] = nil
        return
    end
    if not actor_matches_pending_spell_target(context, pending) then
        return
    end
    state.spell_effect_pending = pending
    pending_spell_effects_by_source_slot[slot] = nil
end

local function spell_scan_marker(candidate, scene_id)
    if type(candidate) ~= "table" or
       type(candidate.source_record_slot) ~= "number" then
        return nil
    end
    local marker = spell_effect_scans_by_source_slot[
        candidate.source_record_slot]
    if type(marker) == "table" and marker.scene_id ~= scene_id then
        spell_effect_scans_by_source_slot[
            candidate.source_record_slot] = nil
        marker = nil
    end
    if type(marker) == "table" and marker.scene_id == scene_id and
       marker.actor_id == candidate.id then
        return marker
    end
    local candidate_state = type(candidate.id) == "string" and
        states[candidate.id] or nil
    marker = type(candidate_state) == "table" and
        candidate_state.spell_effect_scan_marker or nil
    if type(marker) == "table" and marker.scene_id == scene_id and
       marker.actor_id == candidate.id then
        return marker
    end
    return nil
end

local function clear_spell_scan_marker(slot, actor_id)
    local marker = spell_effect_scans_by_source_slot[slot]
    if type(marker) == "table" and marker.actor_id == actor_id then
        spell_effect_scans_by_source_slot[slot] = nil
    end
    local candidate_state = states[actor_id]
    if type(candidate_state) == "table" and
       type(candidate_state.spell_effect_scan_marker) == "table" and
       candidate_state.spell_effect_scan_marker.actor_id == actor_id then
        candidate_state.spell_effect_scan_marker = nil
    end
end

local function advance_spell_effect(state, context)
    state.spell_effect_slot_released = nil
    state.spell_effect_resolution = nil
    state.spell_effect_visible = false
    state.spell_effect_phase = 0
    state.spell_effect_action_code = 0
    state.spell_effect_completed = false
    state.field_barrier_remaining = nil
    transfer_pending_spell_effect(state, context)
    local field_effect = state.player_field_spell_effect
    if type(field_effect) == "table" and context.phase == "render" then
        -- ROM 0x49F2 stores action_result * 0x190 in the field effect timer;
        -- 0x4ADC decrements that timer on the effect service pass [C]. The host
        -- owns the allocated shared slot; Lua owns this source clock.
        field_effect.remaining = field_effect.remaining - 1
        if field_effect.remaining <= 0 then
            state.spell_effect_action_code = field_effect.action_code
            state.spell_effect_phase = 1
            state.spell_effect_slot_released = field_effect.effect_slot
            state.spell_effect_completed = true
            state.player_field_spell_effect = nil
            return
        end
        state.field_barrier_remaining = field_effect.remaining
    end
    if context.phase ~= "render" or
       type(state.spell_effect_pending) ~= "table" then
        return
    end

    local pending = state.spell_effect_pending
    if pending.scene_id ~= nil and
       pending.scene_id ~= context.movement_scene_id then
        state.spell_effect_pending = nil
        return
    end
    if not actor_matches_pending_spell_target(context, pending) then
        return
    end
    state.spell_effect_action_code = pending.action_code
    if context.source_record_slot ~= pending.source_record_slot then
        state.spell_effect_phase = pending.phase_count
        if pending.owns_effect_slot ~= false then
            state.spell_effect_slot_released = pending.effect_slot
        end
        state.spell_effect_pending = nil
        state.spell_effect_completed = true
        return
    end
    pending.countdown = pending.countdown - 1
    if pending.countdown < 0 then
        pending.countdown = pending.repeat_countdown
        pending.phase = pending.phase + 1
        state.spell_effect_phase = pending.phase
        if pending.phase >= pending.phase_count then
            local combat = game.modules.combat
            local caster_source_flags = pending.caster_source_flags
            if type(context.ai_target_candidates) == "table" then
                for _, candidate in ipairs(context.ai_target_candidates) do
                    if type(candidate) == "table" and
                       candidate.id == pending.caster_actor_id and
                       type(candidate.service_flags) == "number" then
                        caster_source_flags = candidate.service_flags
                        break
                    end
                end
            end
            local skip_delayed_callback = pending.action_code == 2 and
                context.world_scene_state == 3 and
                (caster_source_flags & 0x08) == 0
            if pending.action_code ~= 5 and not skip_delayed_callback and
               type(combat) == "table" and
               type(combat.resolve_spell_impact) == "function" and
               type(context.combatant) == "table" and
               type(pending.caster_combatant) == "table" then
                local effect_context = {
                    combatant = pending.caster_combatant,
                    ai_random_bound = context.ai_random_bound,
                    ai_target_candidates =
                        pending.ai_target_candidates,
                    combat_meter_service_blocked =
                        context.combat_meter_service_blocked,
                }
                local target = {
                    id = context.actor_id,
                    source_record_slot = context.source_record_slot,
                    combatant = context.combatant,
                }
                state.spell_effect_resolution =
                    combat.resolve_spell_impact(
                        effect_context, pending, target)
            end
            if pending.owns_effect_slot ~= false then
                state.spell_effect_slot_released = pending.effect_slot
            end
            state.spell_effect_pending = nil
            state.spell_effect_completed = true

            if pending.action_code == 5 then
                clear_spell_scan_marker(
                    pending.source_record_slot, pending.target_id)
                local candidates = type(context.ai_target_candidates) == "table"
                    and context.ai_target_candidates or
                    pending.ai_target_candidates
                if type(candidates) == "table" then
                    for _, candidate in ipairs(candidates) do
                        local marker = spell_scan_marker(
                            candidate, pending.scene_id)
                        if type(candidate) == "table" and
                           type(candidate.source_record_slot) == "number" and
                           type(candidate.service_flags) == "number" and
                           (candidate.service_flags & ActorStatus.ACTIVE) ~= 0 and
                           (candidate.service_flags & ActorStatus.DEAD) == 0 and
                           marker ~= nil then
                            local next_pending = {
                                action_code = pending.action_code,
                                action_result = pending.action_result,
                                target_id = candidate.id,
                                source_record_slot =
                                    candidate.source_record_slot,
                                effect_slot = pending.effect_slot,
                                initial_countdown =
                                    pending.initial_countdown,
                                countdown = pending.initial_countdown,
                                phase = 0,
                                phase_count = pending.phase_count,
                                repeat_countdown = pending.repeat_countdown,
                                caster_combatant = pending.caster_combatant,
                                ai_target_candidates = candidates,
                                scene_id = pending.scene_id,
                                caster_source_flags =
                                    pending.caster_source_flags,
                                caster_actor_id = pending.caster_actor_id,
                                owns_effect_slot = false,
                            }
                            pending_spell_effects_by_source_slot[
                                candidate.source_record_slot] = next_pending
                            break
                        end
                    end
                end
            end
            return
        end
    end
    state.spell_effect_visible = true
    state.spell_effect_phase = pending.phase
end

local function begin_dynamic_target_action(state, context, target)
    if not context.dynamic_world_encounter_actor or
       state.action_delay >= 0 or state.target_motion_effect_active then
        return nil
    end

    -- ROM 0xB1FC..0xB21C re-measures the linked target with 0x15D3C and
    -- stores it in +0x34 and +0x2E before picking the C0A2 direction, so the
    -- strike faces the target, not the last walk heading [C].
    local heading = state.movement_current_heading or context.initial_heading or 0
    if type(target) == "table" and type(target.heading) == "number" then
        heading = target.heading
    end
    heading = u16(heading)
    local direction = R.direction_from_heading(heading)
    local animation = Animator.list(state.entity_kind,
                                    Animator.Pose.ACTION, direction)
    if animation == nil or animation.finite ~= true then
        return nil
    end
    state.movement_current_heading = heading

    -- ROM 0xB1DC selects C0A2 for the accepted target heading, requests the
    -- kind-specific action cue, resets +0x1A/+0x1E, and applies delay mode 1.
    state.animation_id = "action"
    state.animation_frame = 0
    state.animation_accumulator = 0
    state.direction = direction
    state.animation_selector = Animator.Pose.ACTION + direction * 4
    state.source_animation_selector = state.animation_selector
    state.animation_increment = animation.rate
    state.animation_period = Animator.PERIOD
    state.action_animation_complete = false
    state.action_delay = source_action_delay(context, state, 1)
    -- ROM 0xB25E calls 0x1DBAA once the strike is accepted in range [C].
    -- The typed heartbeat result lets the host set FFDF84/FFDF86 [H].
    state.world_activity_requested = true
    -- ROM 0xB284 invokes the shared 0x55C76 reaction only after the linked
    -- heading is accepted. Lua resolves the source-derived action now and
    -- returns a one-heartbeat meter update for the host to apply.
    local update = game.modules.combat.resolve_reaction(context, target)
    state.combat_meter_update = update
    if type(update) == "table" and update.changed == true and
       type(target) == "table" and
       type(target.world_x) == "number" and
       type(target.world_y) == "number" then
        -- ROM 0xB278..0xB29A saves target +0x4C/+0x9C around 0x55C76 and
        -- enters the 0xB29E child init only when either byte changed [C].
        -- 0xB1DC (entered from the 0x141D8 accept path) reaches this for
        -- enemy strikes too, so this is the same child core wires in
        -- begin_player_melee (core 5d5c24ef). The Hell Hound 0xB11C path
        -- uses 0x55B16 and the 0xB146 child with the same init [C].
        state.hit_spark_spawn_request = {
            target_id = target.id,
            world_x = target.world_x,
            world_y = target.world_y,
        }
    end
    local heading_event = context.target_heading_audio_alternate
        and SoundId.TARGET_HEADING_ALT or SoundId.TARGET_HEADING
    local action_audio_event = state.entity_kind == E.SpriteType.HELL_HOUND
        and SoundId.MELEE_SWING_ALT or SoundId.MELEE_SWING
    return action_audio_event, heading_event
end


-- register this module's definitions
R.source_action_delay = source_action_delay
R.advance_source_action_delay = advance_source_action_delay
R.advance_target_motion_effect = advance_target_motion_effect
R.request_target_motion_effect = request_target_motion_effect
R.advance_target_marker = advance_target_marker
R.advance_world_target_selection = advance_world_target_selection
R.action_line_clear = action_line_clear
R.first_wounded_party_runner = first_wounded_party_runner
R.classify_spell_dispatch = classify_spell_dispatch
R.classify_item_dispatch = classify_item_dispatch
R.begin_thon_action = begin_thon_action
R.begin_dynamic_item_action = begin_dynamic_item_action
R.begin_spell_cast = begin_spell_cast
R.actor_matches_pending_spell_target = actor_matches_pending_spell_target
R.transfer_pending_spell_effect = transfer_pending_spell_effect
R.spell_scan_marker = spell_scan_marker
R.clear_spell_scan_marker = clear_spell_scan_marker
R.advance_spell_effect = advance_spell_effect
R.begin_dynamic_target_action = begin_dynamic_target_action
return R
