-- actor/animator.lua: named clip metadata and host animation timing.
local R = require("actor.registry")
local LISTS = require("data.actor_anim_lists")
local CADENCE = require("data.actor_animation_cadence")

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
-- Named host cadence profile. ROM 0xC002/0xBADE/0x6092 provide the observed
-- timing; gameplay consumes a normalized frame rate rather than source words.
Animator.PLAYER_WALK_PROGRESS_PER_UPDATE =
    CADENCE.runner_walk.frames_per_update

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

-- Profiles express cadence in host terms: frames per update, or frames per
-- map unit for actors whose walk rate follows their movement distance.
function Animator.progress_per_update(profile_id, frame,
                                      distance_per_update)
    local profile = CADENCE[profile_id]
    if type(profile) ~= "table" then
        return nil
    end
    if type(profile.frames_per_update) == "number" then
        return profile.frames_per_update
    end
    if type(profile.default_frames_per_update) == "number" then
        local overrides = profile.frame_overrides
        local override = type(overrides) == "table" and
            overrides[frame] or nil
        return type(override) == "number"
            and override or profile.default_frames_per_update
    end
    if type(distance_per_update) ~= "number" or
       distance_per_update < 0 then
        return nil
    end
    for _, band in ipairs(profile.distance_bands or {}) do
        if type(band.frames_per_map_unit) == "number" and
           (band.max_distance_per_update == nil or
            distance_per_update <= band.max_distance_per_update) then
            return distance_per_update * band.frames_per_map_unit
        end
    end
    return nil
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

-- Advance one finite clip whose frames have explicit host update durations.
-- The final frame remains visible while animation_complete tells gameplay it
-- may retire the temporary actor.
function Animator.advance_finite_schedule(state, frame_durations)
    if type(state) ~= "table" or type(frame_durations) ~= "table" or
       type(state.animation_frame) ~= "number" then
        return false
    end
    local count = #frame_durations
    if count == 0 then
        return false
    end
    if state.animation_frame >= count then
        state.animation_frame = count - 1
        state.animation_elapsed_updates = 0
        state.animation_complete = true
        return true
    end
    if state.animation_complete == true then
        state.animation_frame = count - 1
        return true
    end
    local duration = frame_durations[state.animation_frame + 1]
    if type(duration) ~= "number" or duration < 1 then
        return false
    end
    local elapsed = state.animation_elapsed_updates or 0
    if type(elapsed) ~= "number" or elapsed < 0 then
        return false
    end
    elapsed = elapsed + 1
    if elapsed >= duration then
        elapsed = elapsed - duration
        if state.animation_frame + 1 >= count then
            state.animation_frame = count - 1
            state.animation_complete = true
        else
            state.animation_frame = state.animation_frame + 1
        end
    end
    state.animation_elapsed_updates = elapsed
    return state.animation_complete == true
end

R.Animator = Animator
return Animator
