-- actor/animator.lua semantic clips, normalized locomotion phase, and the
-- isolated weapon-projectile clock.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local A = R.Animator
local E = require("enums")
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end

-- ROM 0x62AC lists are imported as named clips and host update durations:
-- runner death 0x7664 (8 frames, +0x200), THON death 0x7884 (1, +0x80),
-- runner hurt 0x77F8 (2, +0x300), and spell deaths 0x75E8/0x75C0
-- (10/9, +0x280); 0x5FDC/0x600E uses a 0x1000 threshold [C].
check(A.list(3, A.Clip.WALK).frame_count == 4 and
      A.list(3, A.Clip.WALK).looping, "runner walk loops 4")
check(A.list(E.SpriteType.GARGOYLE, A.Clip.WALK).frame_count == 5,
      "gargoyle walk 5")
check(A.list(E.SpriteType.THON, A.Clip.DEATH).frame_duration_updates == 32 and
      A.list(E.SpriteType.THON, A.Clip.DEATH).frame_count == 1,
      "THON death duration")
check(A.list(3, A.Clip.DEATH).frame_count == 8 and
      A.list(3, A.Clip.DEATH).frame_duration_updates == 8,
      "runner death duration")
check(A.list(5, A.Clip.SPELL_DEATH_E0).frame_count == 10 and
      A.list(5, A.Clip.SPELL_DEATH_E4).frame_count == 9,
      "E0/E4 humanoid frame counts")
check(A.list(E.SpriteType.GARGOYLE, A.Clip.SPELL).frame_count == 2,
      "gargoyle spell art list has two frames")
local imported_hurt = A.list(3, A.Clip.HURT)
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
-- The ROM's 0x5FDC/0x6092 rates become a fraction of one animation frame per
-- render update. Character state stores that normalized fraction rather than
-- the source accumulator. Rates below are direct source-derived expectations.
local player_walk_progress = A.PLAYER_WALK_PROGRESS_PER_UPDATE
check(player_walk_progress == 0x280 / 0x1000,
      "runner 0x28 movement step becomes normalized progress")
check(A.loop_progress_per_update(E.SpriteType.CORP_GUARD, 0, 9) ==
          (9 * 0x1c) / 0x1000 and
      A.loop_progress_per_update(E.SpriteType.CORP_GUARD, 0, 0x16) ==
          0x160 / 0x1000,
      "creature movement step selects the source rate on either side of 0x16")
check(A.loop_progress_per_update(E.SpriteType.GARGOYLE, 2, 9) ==
          0x90 / 0x1000 and
      A.loop_progress_per_update(E.SpriteType.GARGOYLE, 1, 9) ==
          0x180 / 0x1000,
      "gargoyle frame 2 uses its slower source rate")

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
    entity_kind = E.SpriteType.CORP_GUARD, direction = 0,
    animation_frame = 0, animation_progress = 0,
    movement_step_distance = 9,
}
for _ = 1, 17 do
    check(R.advance_dynamic_actor_walk_animation(creature_walk),
          "dynamic actor walk list advances")
end
check(creature_walk.animation_frame == 1 and
      creature_walk.animation_progress == (17 * 9 * 0x1c / 0x1000) - 1,
      "dynamic actor walk retains the source rate as normalized progress")

local gargoyle_idle = {
    entity_kind = E.SpriteType.GARGOYLE, direction = 0,
    animation_frame = 0, animation_progress = 0,
    movement_step_distance = 9,
}
for _ = 1, 11 do
    check(R.advance_dynamic_actor_idle_animation(gargoyle_idle),
          "dynamic actor idle list advances")
end
check(gargoyle_idle.animation_frame == 0 and
      gargoyle_idle.animation_progress == 11 * 0x180 / 0x1000 - 1,
      "gargoyle one-frame idle list wraps with normalized progress")

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
player_clip = { animation_id = "idle", animation_frame = 0,
    animation_elapsed_updates = 0, direction = 0 }
for _ = 1, 64 do
    R.advance_player_animation(player_clip, { animation_mode = "runner_down",
        animation_direction_override = 0 })
end
check(player_clip.animation_frame == 7,
      "runner down advances through the semantic death clip and holds")
-- Weapon projectile effects retain a private source clock until their signed
-- overflow cadence is converted to authored clip durations.
local st = { animation_accumulator = 0xE00, animation_frame = 1 }
check(A.step_projectile_finite(st, 2, 0x200) and st.animation_frame == 1 and st.animation_accumulator == 0, "projectile finite list clamps and reports done")
st = { animation_accumulator = 0x7F00, animation_frame = 0 }
A.step_projectile_finite(st, 4, 0x200, 0x1000); check(st.animation_frame == 0 and st.animation_accumulator < 0, "projectile signed overflow does not advance")

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
local death = R.death_list({ entity_kind = 15 })
check(death.frame_duration_updates == 32 and death.frame_count == 1,
      "THON down duration")
death = R.death_list({ entity_kind = 5,
                       spell_death_variant = "spell_death_e0" })
check(death.frame_duration_updates == 6.4 and death.frame_count == 10,
      "E0 list host duration")
check(A.hurt_audio(5) == A.LISTS.audio.hurt_by_kind[5] and
      A.down_audio(12) == A.LISTS.audio.down_by_kind[12],
      "hurt and down cues come from the generated ROM tables")
print(string.format("animator tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
