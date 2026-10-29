-- actor/animator.lua: named clip metadata and host animation timing.
local R = require("actor.registry")
local I = require("lib.int68k")
local E = require("enums")
local u16 = I.u16
local LISTS = require("data.actor_anim_lists")

local Animator = {}
Animator.LISTS = LISTS
-- Gameplay names for authored actor clips. ROM pose words are used only by
-- the offline list importer; runtime policy selects clips by these names.
Animator.Clip = {
    IDLE = "idle",
    WALK = "walk",
    ITEM = "item_action",
    HURT = "hurt",
    SPELL = "spell",
    WEAPON = "weapon",
    ACTION = "action",
    SPELL_DEATH_E0 = "spell_death_e0",
    SPELL_DEATH_E4 = "spell_death_e4",
    DEATH = "death",
    HIT_SPARK = "hit_spark",
}
-- Legacy source-clock support remains for projectile and movement paths that
-- have not yet been converted to duration-based animation state.
Animator.PERIOD = 0x1000
Animator.Pose = E.ActorPose
-- 0xBABC moves the controlled runner with vector table 0xC002, whose +4 word
-- (0x28) 0xBADE copies to +0xCA [C].
Animator.PLAYER_SPEED = 0x28

function Animator.list(kind, clip, direction)
    local by_kind = LISTS[kind]
    local list = by_kind and by_kind[clip] or nil
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

-- Finite character clips use an ordinary duration measured in animation
-- updates. The offline importer converts ROM 0x62AC list rates against the
-- source 0x1000 threshold [C: 0x5FDC/0x600E]; gameplay stores elapsed updates.
function Animator.advance_finite_clip(state, clip)
    if type(state) ~= "table" or type(clip) ~= "table" or
       clip.looping ~= false or type(clip.frame_count) ~= "number" or
       clip.frame_count < 1 or
       type(clip.frame_duration_updates) ~= "number" or
       clip.frame_duration_updates <= 0 then
        return false
    end

    if (state.animation_elapsed_updates ~= nil and
        type(state.animation_elapsed_updates) ~= "number") or
       (state.animation_frame ~= nil and
        type(state.animation_frame) ~= "number") then
        return false
    end
    local elapsed = (state.animation_elapsed_updates or 0) + 1
    local frame = state.animation_frame or 0
    local duration = clip.frame_duration_updates
    while elapsed + 1e-9 >= duration and frame < clip.frame_count do
        elapsed = elapsed - duration
        frame = frame + 1
    end
    if frame >= clip.frame_count then
        state.animation_frame = clip.frame_count - 1
        state.animation_elapsed_updates = 0
        return true
    end

    state.animation_frame = frame
    state.animation_elapsed_updates = elapsed
    return false
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
