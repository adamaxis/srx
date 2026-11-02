-- actor/animator.lua semantic clips, host cadence profiles, and finite effect
-- schedules.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local A = R.Animator
local E = require("enums")
local ProjectileAnimationClips =
    require("data.weapon_projectile_animation_clips")
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end

-- ROM 0x62AC lists are imported as named clips and host update durations:
-- runner death 0x7664 (8 frames, +0x200), THON death 0x7884 (1, +0x80),
-- runner hurt 0x77F8 (2, +0x300), and spell deaths 0x75E8/0x75C0
-- (10/9, +0x280); 0x5FDC/0x600E uses a 0x1000 threshold [C].
check(A.list(E.ActorVisualProfile.FIELD_RUNNER, A.Clip.WALK).frame_count == 4 and
      A.list(E.ActorVisualProfile.FIELD_RUNNER, A.Clip.WALK).looping,
      "runner walk loops 4")
check(A.list(E.ActorVisualProfile.GARGOYLE, A.Clip.WALK).frame_count == 5,
      "gargoyle walk 5")
check(A.list(E.ActorVisualProfile.THON, A.Clip.DEATH).frame_duration_updates == 32 and
      A.list(E.ActorVisualProfile.THON, A.Clip.DEATH).frame_count == 1,
      "THON death duration")
check(A.list(E.ActorVisualProfile.FIELD_RUNNER, A.Clip.DEATH).frame_count == 8 and
      A.list(E.ActorVisualProfile.FIELD_RUNNER, A.Clip.DEATH).frame_duration_updates == 8,
      "runner death duration")
check(A.list(E.ActorVisualProfile.HUMAN_GUARD, A.Clip.SPELL_DEATH_E0).frame_count == 10 and
      A.list(E.ActorVisualProfile.HUMAN_GUARD, A.Clip.SPELL_DEATH_E4).frame_count == 9,
      "E0/E4 humanoid frame counts")
check(A.list(E.ActorVisualProfile.GARGOYLE, A.Clip.SPELL).frame_count == 2,
      "gargoyle spell art list has two frames")

-- ROM 0xB6BA..0xB6DE selects C062 directions 5..7 for leftward walk headings;
-- the kind-4 Ork Ganger descriptors at 0x97C4/0x977C/0x9734 set attr 0x0800.
for _, direction in ipairs({ E.ActorFacing.SOUTH_WEST, E.ActorFacing.WEST,
                             E.ActorFacing.NORTH_WEST }) do
    local facing = R.snapshot({
        actor_id = "ganger-left-walk",
        visual_profile_id = E.ActorVisualProfile.ORK_BANDERSNATCH,
        animation_id = "walk",
        direction = direction,
        animation_frame = 0,
    }, "hold", nil)
    check(facing.sprite_flip == E.SpriteFlip.HORIZONTAL,
          "left-facing Ganger walk carries the ROM horizontal flip")
end
local right_facing = R.snapshot({
    actor_id = "ganger-right-walk",
    visual_profile_id = E.ActorVisualProfile.ORK_BANDERSNATCH,
    animation_id = "walk",
    direction = E.ActorFacing.EAST,
    animation_frame = 0,
}, "hold", nil)
check(right_facing.sprite_flip == E.SpriteFlip.NONE,
      "right-facing Ganger walk keeps the unflipped art")
check(E.FieldSpritePriority.LOW == 0 and E.FieldSpritePriority.HIGH == 1 and
      right_facing.sprite_priority == E.FieldSpritePriority.LOW,
      "field actor snapshot carries the ROM low-priority sprite layer")
local left_idle = R.snapshot({
    actor_id = "ganger-left-idle",
    visual_profile_id = E.ActorVisualProfile.ORK_BANDERSNATCH,
    animation_id = "idle",
    direction = E.ActorFacing.WEST,
    animation_frame = 0,
}, "hold", nil)
check(left_idle.sprite_flip == E.SpriteFlip.NONE,
      "walk-facing correction does not change idle art")

local imported_hurt = A.list(E.ActorVisualProfile.FIELD_RUNNER, A.Clip.HURT)
check(imported_hurt.rate == nil and imported_hurt.count == nil and
      imported_hurt.frame_duration_updates > 0,
      "runtime list has no ROM rate or selector count fields")
-- The 0x77F8 two-frame clip is still active through update 10, then completes
-- on update 11 and holds its second frame.
local timed_hurt = { animation_frame = 0, animation_elapsed_updates = 0 }
for _ = 1, 10 do
    check(not A.advance_finite_clip(timed_hurt, imported_hurt),
          "hurt clip remains active before its final update")
end
check(A.advance_finite_clip(timed_hurt, imported_hurt) and
      timed_hurt.animation_frame == imported_hurt.frame_count - 1,
      "hurt clip completes on its duration and holds its last frame")
-- Movement animation uses host cadence profiles in frames per update or map
-- unit. The oracle below checks those profiles against the ROM-derived rate
-- across the supported movement distance range [C: 0x5FDC/0x6092].
local player_walk_progress = A.PLAYER_WALK_PROGRESS_PER_UPDATE
check(player_walk_progress == 0.15625,
      "runner profile uses its host frame rate")
local distance_profiles_match_source = true
for distance = 0, 40 do
    local source_rate = distance < 22 and distance * 28 or distance * 16
    if A.progress_per_update("moving_actor_walk", 0, distance) ~=
       source_rate / 4096 then
        distance_profiles_match_source = false
        break
    end
end
check(distance_profiles_match_source,
      "moving-actor cadence profiles preserve each observed walk distance")
check(A.progress_per_update("gargoyle_walk", 2, 9) == 0.03515625 and
      A.progress_per_update("gargoyle_walk", 1, 9) == 0.09375,
      "gargoyle profile applies its frame-specific cadence")

local runner_walk = { animation_id = "walk", animation_frame = 0,
    animation_progress = 0, direction = 0 }
for _ = 1, 6 do
    R.advance_player_animation(runner_walk, { animation_mode = "walking",
        animation_direction_override = 0 })
end
check(runner_walk.animation_frame == 0 and
      runner_walk.animation_progress == 6 * player_walk_progress,
      "runner stays on frame 0 until normalized phase reaches one frame")
R.advance_player_animation(runner_walk, { animation_mode = "walking",
    animation_direction_override = 0 })
check(runner_walk.animation_frame == 1 and
      runner_walk.animation_progress == 7 * player_walk_progress - 1,
      "runner advances one frame and retains normalized fractional phase")

local creature_walk = {
    visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD, direction = 0,
    animation_frame = 0, animation_progress = 0,
    animation_cadence_profile = "moving_actor_walk",
    movement_distance_per_update = 9,
}
for _ = 1, 17 do
    check(R.advance_dynamic_actor_walk_animation(creature_walk),
          "dynamic actor walk list advances")
end
check(creature_walk.animation_frame == 1 and
      creature_walk.animation_progress ==
          (17 * 9 * 0.0068359375) - 1,
      "dynamic actor walk retains host progress across updates")

local gargoyle_idle = {
    visual_profile_id = E.ActorVisualProfile.GARGOYLE, direction = 0,
    animation_frame = 0, animation_progress = 0,
    animation_cadence_profile = "gargoyle_walk",
    movement_distance_per_update = 9,
}
for _ = 1, 11 do
    check(R.advance_dynamic_actor_idle_animation(gargoyle_idle),
          "dynamic actor idle list advances")
end
check(gargoyle_idle.animation_frame == 0 and
      gargoyle_idle.animation_progress == 11 * 0.09375 - 1,
      "gargoyle one-frame idle list wraps with host cadence")

local player_clip = {
    animation_id = "idle",
    animation_frame = 0,
    animation_elapsed_updates = 0,
    direction = 0,
}
for _ = 1, 5 do
    R.advance_player_animation(player_clip, { animation_mode = "runner_hurt",
        animation_direction_override = 0 })
end
check(player_clip.animation_frame == 0,
      "runner hurt clip holds its first frame until its authored duration")
R.advance_player_animation(player_clip, { animation_mode = "runner_hurt",
    animation_direction_override = 0 })
check(player_clip.animation_frame == 1 and
      player_clip.animation_elapsed_updates > 0 and
      player_clip.animation_accumulator == nil and
      player_clip.animation_progress == nil,
      "runner hurt advances from elapsed updates without a source clock")
for _ = 1, 5 do
    R.advance_player_animation(player_clip, { animation_mode = "runner_hurt",
        animation_direction_override = 0 })
end
check(player_clip.player_hurt_animation_complete_pending == true and
      player_clip.animation_frame == 1,
      "runner hurt reports completion while holding its final frame")
player_clip = { animation_id = "runner_hurt", animation_frame = 0,
    animation_elapsed_updates = 0, animation_progress = 0.75,
    player_hurt_pose_active = true, direction = 0 }
R.advance_player_animation(player_clip, { animation_mode = "idle",
    animation_clock_reset_requested = true,
    animation_direction_override = 0 })
check(player_clip.animation_progress == 0 and
      player_clip.animation_accumulator == nil,
      "save restore resets Lua-owned progress while the hurt clip remains active")
player_clip = { animation_id = "idle", animation_frame = 0,
    animation_elapsed_updates = 0, direction = 0 }
for _ = 1, 8 do
    R.advance_player_animation(player_clip, { animation_mode = "strike",
        animation_direction_override = 0 })
end
check(player_clip.animation_frame == 1 and
      player_clip.animation_elapsed_updates == 0,
      "runner strike advances from its named clip duration")

-- ROM 0xB39A selects Joshua's C042 item-action list at 0x784C: one frame,
-- rate 0x80 over a 0x1000 period, then the following 0xA8EA pass selects
-- B45C/C052 even while the separate +0xDC action delay remains positive.
local player_weapon_action = {
    animation_id = "idle",
    animation_frame = 0,
    animation_elapsed_updates = 0,
    direction = E.ActorFacing.EAST,
    player_weapon_animation_start_pending = true,
}
local player_weapon_clip = A.list(E.ActorVisualProfile.FIELD_RUNNER,
                                  A.Clip.ITEM)
check(player_weapon_clip.frame_count == 1 and
      player_weapon_clip.frame_duration_updates == 32,
      "runner C042 metadata keeps its source one-frame, 32-update lifetime")
for _ = 1, 31 do
    R.advance_player_animation(player_weapon_action, {
        animation_mode = E.ActorAnimationMode.ATTACK,
        animation_direction_override = E.ActorFacing.EAST,
    })
end
check(player_weapon_action.animation_id == "attack" and
      player_weapon_action.animation_frame == 0 and
      player_weapon_action.animation_elapsed_updates == 31 and
      player_weapon_action.player_weapon_animation_complete ~= true,
      "C042 pose stays visible until its 32nd render update")
R.advance_player_animation(player_weapon_action, {
    animation_mode = E.ActorAnimationMode.ATTACK,
    animation_direction_override = E.ActorFacing.EAST,
})
check(player_weapon_action.animation_id == "attack" and
      player_weapon_action.player_weapon_animation_complete == true,
      "C042 action list completes independently of held attack mode")
R.advance_player_animation(player_weapon_action, {
    animation_mode = E.ActorAnimationMode.ATTACK,
    animation_direction_override = E.ActorFacing.EAST,
})
for _ = 1, 20 do
    R.advance_player_animation(player_weapon_action, {
        animation_mode = E.ActorAnimationMode.ATTACK,
        animation_direction_override = E.ActorFacing.EAST,
    })
end
check(player_weapon_action.animation_id == "idle" and
      player_weapon_action.player_weapon_animation_complete == true,
      "held attack mode does not pin the runner to the C042 pose")
player_weapon_action.player_weapon_animation_start_pending = true
R.advance_player_animation(player_weapon_action, {
    animation_mode = E.ActorAnimationMode.ATTACK,
    animation_direction_override = E.ActorFacing.EAST,
})
check(player_weapon_action.animation_id == "attack" and
      player_weapon_action.animation_frame == 0 and
      player_weapon_action.animation_elapsed_updates == 1 and
      player_weapon_action.player_weapon_animation_complete ~= true and
      player_weapon_action.player_weapon_animation_start_pending == nil,
      "a later held-fire action restarts C042 from its first update")

player_clip = { animation_id = "idle", animation_frame = 0,
    animation_elapsed_updates = 0, direction = 0 }
for _ = 1, 64 do
    R.advance_player_animation(player_clip, { animation_mode = "runner_down",
        animation_direction_override = 0 })
end
check(player_clip.animation_frame == 7,
      "runner down advances through the semantic death clip and holds")
-- The two weapon-effect list lengths become explicit host timing data. The
-- cadence table records the visible behavior of ROM 0x5FDC/0x600E without
-- preserving the signed accumulator or its completion bit in Lua state.
local short_projectile = {
    animation_frame = 0, animation_elapsed_updates = 0,
    animation_complete = false,
}
local short_projectile_frames = {}
for update = 1, 3 do
    local complete = A.advance_finite_schedule(short_projectile,
        ProjectileAnimationClips[3].frame_durations)
    short_projectile_frames[update] = short_projectile.animation_frame
    if update < 3 then
        check(not complete, "three-frame projectile remains active before its last update")
    else
        check(complete, "three-frame projectile completes on update three")
    end
end
check(short_projectile_frames[1] == 1 and
      short_projectile_frames[2] == 2 and
      short_projectile_frames[3] == 2 and
      short_projectile.animation_accumulator == nil and
      short_projectile.animation_increment == nil,
      "three-frame projectile holds its terminal frame without source counters")

local special_projectile = {
    animation_frame = 0, animation_elapsed_updates = 0,
    animation_complete = false,
}
local special_projectile_frames = {}
for update = 1, 19 do
    local complete = A.advance_finite_schedule(special_projectile,
        ProjectileAnimationClips[11].frame_durations)
    special_projectile_frames[update] = special_projectile.animation_frame
    if update < 19 then
        check(not complete, "eleven-frame projectile remains active before update nineteen")
    else
        check(complete, "eleven-frame projectile completes on update nineteen")
    end
end
local expected_special_projectile_frames = {
    1, 2, 3, 4, 4, 4, 4, 4, 5, 6,
    7, 8, 9, 9, 9, 9, 9, 10, 10,
}
local special_projectile_schedule_matches = true
for update, frame in ipairs(expected_special_projectile_frames) do
    if special_projectile_frames[update] ~= frame then
        special_projectile_schedule_matches = false
    end
end
check(special_projectile_schedule_matches and
      special_projectile.animation_accumulator == nil and
      special_projectile.animation_increment == nil,
      "eleven-frame projectile follows the source-visible hold schedule in semantic state")

-- Exhaust the live source phase domain. Both runner idle and walk use looping
-- lists, so compare their visible frame and fractional phase with the source
-- threshold step while requiring that no source accumulator leaks into state.
local same = true
for _, mode in ipairs({ "walking", "idle" }) do
    for f = 0, (mode == "walking" and 3 or 0) do
      for source_phase = 0, 0xFFF do
        local next_phase = source_phase + 0x280
        local expected_frame = f
        if next_phase >= 0x1000 then
            next_phase = next_phase - 0x1000
            expected_frame = (expected_frame + 1) %
                (mode == "walking" and 4 or 1)
        end
        local state = { animation_id = mode == "walking" and "walk" or "idle",
            animation_progress = source_phase / 0x1000,
            animation_frame = f, direction = 0 }
        R.advance_player_animation(state, { animation_mode = mode,
            animation_direction_override = 0 })
        if state.animation_frame ~= expected_frame or
           math.abs(state.animation_progress - next_phase / 0x1000) > 1e-12 or
           state.animation_accumulator ~= nil then
            same = false
        end
      end
    end
end
check(same, "runner idle/walk matches source frame cadence with normalized state")

-- Death clips select frame count and duration from the actor's visual family.
local death = R.death_list({ visual_profile_id = E.ActorVisualProfile.THON })
check(death.frame_duration_updates == 32 and death.frame_count == 1,
      "THON down duration")
death = R.death_list({ visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD,
                       spell_death_variant = "spell_death_e0" })
check(death.frame_duration_updates == 6.4 and death.frame_count == 10,
      "E0 list host duration")
check(A.hurt_audio(E.ActorVisualProfile.HUMAN_GUARD) ==
          A.LISTS.audio.hurt_by_visual_profile[
              E.ActorVisualProfile.HUMAN_GUARD] and
      A.down_audio(E.ActorVisualProfile.HELL_HOUND) ==
          A.LISTS.audio.down_by_visual_profile[
              E.ActorVisualProfile.HELL_HOUND],
      "hurt and down cues come from the generated ROM tables")
print(string.format("animator tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
