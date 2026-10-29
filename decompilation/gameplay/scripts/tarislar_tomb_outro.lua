-- Scene-55 ending sequence. Lua owns the progression state; the host supplies
-- eligible actor identities and queues the field continuation on completion.
local ActorAI = require("actor.heartbeat")

local Scene55OutroPhase = {
    initial_wait = "initial_wait",
    effect_wait = "effect_wait",
    actor_scan = "actor_scan",
}

local EFFECT_FRAME_COUNT = 8 -- ROM effect frame +0x18 wraps after frame 7.
local ACTOR_SCAN_FRAME = 3
local FRAME_HOLD_COUNTDOWN = 0x0A -- ROM reload; the next pass makes 11.

local TarislarTombOutro = {}
local sequence

local function new_sequence()
    return {
        phase = Scene55OutroPhase.initial_wait,
        phase_timer = 0x00B4,
        animation_frame = 0,
        animation_countdown = 0,
    }
end

local function advance_effect_animation(state)
    -- ROM 0x5D55C: the eight-frame effect holds each frame for eleven
    -- render passes (timer reload 0xA followed by the decrementing pass).
    state.animation_countdown = state.animation_countdown - 1
    if state.animation_countdown < 0 then
        state.animation_countdown = FRAME_HOLD_COUNTDOWN
        state.animation_frame =
            (state.animation_frame + 1) % EFFECT_FRAME_COUNT
    end
end

function TarislarTombOutro.advance(active, actor_ids, arm)
    if type(active) ~= "boolean" or type(actor_ids) ~= "table" or
       type(arm) ~= "boolean" then
        return nil
    end

    if not active then
        sequence = nil
        return "inactive"
    end

    if arm then
        sequence = new_sequence()
        return "armed"
    end

    if sequence == nil then
        return "inactive"
    end

    if sequence.phase == Scene55OutroPhase.initial_wait then
        sequence.phase_timer = sequence.phase_timer - 1
        if sequence.phase_timer < 0 then
            sequence.phase = Scene55OutroPhase.effect_wait
            sequence.phase_timer = 0x0100
        end
        return "progressed"
    end

    if sequence.phase == Scene55OutroPhase.effect_wait then
        advance_effect_animation(sequence)
        sequence.phase_timer = sequence.phase_timer - 1
        if sequence.phase_timer < 0 then
            sequence.phase = Scene55OutroPhase.actor_scan
            sequence.phase_timer = 0x0028
        end
        return "progressed"
    end

    if sequence.phase ~= Scene55OutroPhase.actor_scan then
        return nil
    end

    advance_effect_animation(sequence)
    if sequence.animation_frame ~= ACTOR_SCAN_FRAME then
        return "progressed"
    end

    -- The host has already selected the actors covered by this effect.
    local target_present = ActorAI.has_world_scene_effect_target(actor_ids)
    if type(target_present) ~= "boolean" then
        return "unresolved"
    end
    if target_present then
        return "actor_target_present"
    end

    sequence.phase_timer = sequence.phase_timer - 1
    if sequence.phase_timer < 0 then
        return "completion_ready"
    end
    return "progressed"
end

return TarislarTombOutro
