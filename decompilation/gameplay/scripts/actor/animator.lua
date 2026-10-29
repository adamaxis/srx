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
-- Weapon projectiles still use the source clock until their signed-wrap
-- cadence is converted to authored frame timing.
Animator.PROJECTILE_PERIOD = 0x1000
-- The runner's 0x28 movement step produces rate 0x280 in 0x6092, or 0.15625
-- normalized animation progress per render update [C: 0xC002, 0xBADE, 0x6092].
Animator.PLAYER_WALK_PROGRESS_PER_UPDATE = 0x280 / 0x1000

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

-- Convert the recovered rate rule to a fraction of one animation frame per
-- render update. Lua stores normalized progress instead of the ROM's signed
-- 16-bit accumulator. ROM 0x5FDC/0x6092 [C].
function Animator.loop_progress_per_update(kind, frame, movement_step)
    local rate
    if kind == E.SpriteType.GARGOYLE then
        rate = frame == 2 and 0x0090 or 0x0180
    else
        if type(movement_step) ~= "number" or movement_step < 0 or
           movement_step > 0xffff then
            return nil
        end
        rate = (movement_step < 0x16 and movement_step * 0x1c or
                movement_step * 0x10) & 0xffff
    end
    if rate <= 0 then
        return 0
    end
    return rate / 0x1000
end

-- Advance at most one frame on each render update, preserving fractional
-- cadence across updates and frame-specific rate changes. The normalized
-- phase replaces the source accumulator while keeping its visible timing.
function Animator.advance_loop_clip(state, count, progress_per_update)
    if type(state) ~= "table" or type(count) ~= "number" or count < 1 or
       type(progress_per_update) ~= "number" or progress_per_update < 0 or
       type(state.animation_frame) ~= "number" then
        return false
    end
    local progress = state.animation_progress or 0
    if type(progress) ~= "number" or progress < 0 then
        return false
    end
    progress = progress + progress_per_update
    local advanced = progress + 1e-9 >= 1
    if advanced then
        progress = progress - 1
        state.animation_frame = (state.animation_frame + 1) % count
    end
    state.animation_progress = progress
    return advanced
end

-- 0x5FDC then 0x600E's finite branch: past the count, hold the last frame and
-- report the list finished (the caller owns pose bit 15 / its done flag).
local function tick_projectile_clock(state, rate, period)
    local word = u16(state.animation_accumulator + rate)
    local signed_word = word >= 0x8000 and word - 0x10000 or word
    local threshold = period or Animator.PROJECTILE_PERIOD
    if threshold <= signed_word then
        state.animation_accumulator = u16(word - threshold)
        if state.animation_accumulator >= 0x8000 then
            state.animation_accumulator = state.animation_accumulator - 0x10000
        end
        return true
    end
    state.animation_accumulator = signed_word
    return false
end

function Animator.step_projectile_finite(state, count, rate, period)
    if tick_projectile_clock(state, rate, period) then
        state.animation_frame = u16(state.animation_frame + 1)
    end
    if state.animation_frame >= count then
        state.animation_frame = count - 1
        return true
    end
    return false
end

R.Animator = Animator
return Animator
