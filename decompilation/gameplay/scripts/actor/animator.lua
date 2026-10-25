-- actor/animator.lua: the one ROM animation clock (0x5FDC + 0x600E), data-driven
-- from data/actor_anim_lists.lua (ROM 0x62AC). Every actor animation routine in
-- anim.lua steps through here; none keeps its own accumulator arithmetic.
local R = require("actor.registry")
local I = require("lib.int68k")
local E = require("enums")
local u16 = I.u16
local LISTS = require("data.actor_anim_lists")

local Animator = {}
Animator.LISTS = LISTS
-- +0x1C: every actor list is timed against 0x1000 (spawn template) [C].
Animator.PERIOD = 0x1000
-- Pose bases (+0x16 & 0x7FFF without the direction*4) [C].
Animator.Pose = E.ActorPose
-- 0xBABC moves the controlled runner with vector table 0xC002, whose +4 word
-- (0x28) 0xBADE copies to +0xCA [C].
Animator.PLAYER_SPEED = 0x28

function Animator.list(kind, pose, direction)
    local by_kind = LISTS[kind]
    local list = by_kind and by_kind[pose] or nil
    if list == nil or direction == nil or list.by_dir == nil then
        return list
    end
    if type(direction) ~= "number" or direction < 0 or direction > 7 then
        return nil
    end
    return list.by_dir[direction + 1]
end

function Animator.hurt_audio(kind)
    return LISTS.audio and LISTS.audio.hurt_by_kind[kind] or nil
end

function Animator.down_audio(kind)
    return LISTS.audio and LISTS.audio.down_by_kind[kind] or nil
end

local function signed(word)
    return word >= 0x8000 and word - 0x10000 or word
end

-- ROM 0x5FDC: +0x1A += +0x18 (16-bit); when signed +0x1A >= +0x1C, +0x1E++ and
-- +0x1A -= +0x1C. Returns true when the frame advanced.
function Animator.tick(state, rate, period)
    local word = u16(state.animation_accumulator + rate)
    if period <= signed(word) then
        state.animation_frame = u16(state.animation_frame + 1)
        state.animation_accumulator = signed(u16(word - period))
        return true
    end
    state.animation_accumulator = signed(word)
    return false
end

-- ROM 0x6092: a looping list ignores its stored rate. Kind 0xD uses 0x180
-- (0x90 on frame 2); every other kind uses +0xCA * 0x1C below 0x16, else * 0x10.
function Animator.loop_rate(kind, frame, speed)
    if kind == 0x0D then
        return frame == 2 and 0x0090 or 0x0180
    end
    if type(speed) ~= "number" or speed < 0 or speed > 0xffff then
        return nil
    end
    return (speed < 0x16 and speed * 0x1c or speed * 0x10) & 0xffff
end

-- 0x5FDC then 0x600E's finite branch: past the count, hold the last frame and
-- report the list finished (the caller owns pose bit 15 / its done flag).
function Animator.step_finite(state, count, rate, period)
    Animator.tick(state, rate, period or Animator.PERIOD)
    if state.animation_frame >= count then
        state.animation_frame = count - 1
        return true
    end
    return false
end

-- 0x5FDC then 0x600E's looping branch: wrap to frame 0 at the count.
function Animator.step_loop(state, count, rate, period)
    local advanced = Animator.tick(state, rate, period or Animator.PERIOD)
    if state.animation_frame >= count then
        state.animation_frame = 0
    end
    return advanced
end

R.Animator = Animator
return Animator
