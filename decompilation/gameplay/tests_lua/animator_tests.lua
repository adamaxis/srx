-- actor/animator.lua semantic finite clips and retained movement clock.
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
-- 0x6092
check(A.loop_rate(3, 0, A.PLAYER_SPEED) == 0x280, "player 0x28*0x10")
check(A.loop_rate(5, 0, 9) == 9 * 0x1c and A.loop_rate(5, 0, 0x16) == 0x160, "speed < 0x16 *0x1C else *0x10")
check(A.loop_rate(13, 2, 9) == 0x90 and A.loop_rate(13, 1, 9) == 0x180, "kind 0xD cadence")
local player_clip = {
    animation_id = "idle",
    animation_frame = 0,
    animation_elapsed_updates = 0,
    animation_accumulator = 0,
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
      player_clip.animation_accumulator == 0,
      "runner hurt advances from elapsed clip updates without a source accumulator")
for _ = 1, 5 do
    R.advance_player_animation(player_clip, { animation_mode = "runner_hurt",
        animation_direction_override = 0 })
end
check(player_clip.player_hurt_animation_complete_pending == true and
      player_clip.animation_frame == 1,
      "runner hurt reports completion while holding its final frame")
player_clip = { animation_id = "runner_hurt", animation_frame = 0,
    animation_elapsed_updates = 0, animation_accumulator = 0,
    player_hurt_pose_active = true, direction = 0 }
R.advance_player_animation(player_clip, { animation_mode = "idle",
    animation_accumulator_override = 0x0900,
    animation_direction_override = 0 })
check(player_clip.animation_accumulator == 0,
      "runner hurt ignores locomotion clock overrides while its clip is active")
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
-- The source-clock helper is retained only for unconverted movement/effects.
local st = { animation_accumulator = 0xE00, animation_frame = 1 }
check(A.step_finite(st, 2, 0x200) and st.animation_frame == 1 and st.animation_accumulator == 0, "finite clamps & reports done")
st = { animation_accumulator = 0xF00, animation_frame = 3 }
A.step_loop(st, 4, 0x280); check(st.animation_frame == 0 and st.animation_accumulator == 0x180, "loop wraps")
st = { animation_accumulator = 0x7F00, animation_frame = 0 }
A.tick(st, 0x200, 0x1000); check(st.animation_frame == 0 and st.animation_accumulator < 0, "signed overflow does not advance")

-- Player: new data-driven path equals the pre-animator hardcoded 0x280/%4 path on
-- the live domain (frame 0..3, accumulator 0..0xFFF).
local function old_player(state, mode)
    local acc = state.animation_accumulator + 0x0280
    local adv = acc >= 0x1000
    if adv then acc = acc - 0x1000 end
    state.animation_accumulator = acc
    if mode == "walking" then
        if adv then state.animation_frame = (state.animation_frame + 1) % 4 end
    else state.animation_frame = 0 end
end
local same = true
for _, mode in ipairs({ "walking", "idle" }) do
    for f = 0, 3 do for acc = 0, 0xFFF, 0x20 do
        local a = { animation_accumulator = acc, animation_frame = f }
        local b = { animation_accumulator = acc, animation_frame = f, direction = 0 }
        old_player(a, mode)
        R.advance_player_animation(b, { animation_mode = mode, animation_direction_override = 0 })
        if a.animation_frame ~= b.animation_frame or a.animation_accumulator ~= b.animation_accumulator then same = false end
    end end
end
check(same, "player path unchanged on live domain")

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
