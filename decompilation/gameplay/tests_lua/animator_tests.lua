-- actor/animator.lua (ROM 0x5FDC/0x600E/0x6092) and the routines built on it.
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local A = R.Animator
local E = require("enums")
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end

-- ROM list data [C 0x62AC] spot checks.
check(A.list(3, A.Pose.WALK).count == 4 and not A.list(3, A.Pose.WALK).finite, "runner walk loops 4")
check(A.list(E.SpriteType.GARGOYLE, A.Pose.WALK).count == 5, "gargoyle walk 5")
check(A.list(E.SpriteType.THON, A.Pose.DEATH).rate == 0x80 and
      A.list(E.SpriteType.THON, A.Pose.DEATH).count == 1, "THON death 1 @0x80")
check(A.list(3, A.Pose.DEATH).count == 8 and
      A.list(3, A.Pose.DEATH).rate == 0x200, "runner death 8 @0x200")
check(A.list(5, A.Pose.SPELL_DEATH_E0).count == 10 and
      A.list(5, A.Pose.SPELL_DEATH_E4).count == 9, "E0/E4 humanoid 10/9")
check(A.list(E.SpriteType.GARGOYLE, A.Pose.SPELL).count == 2,
      "gargoyle spell art list has two frames")
-- 0x6092
check(A.loop_rate(3, 0, A.PLAYER_SPEED) == 0x280, "player 0x28*0x10")
check(A.loop_rate(5, 0, 9) == 9 * 0x1c and A.loop_rate(5, 0, 0x16) == 0x160, "speed < 0x16 *0x1C else *0x10")
check(A.loop_rate(13, 2, 9) == 0x90 and A.loop_rate(13, 1, 9) == 0x180, "kind 0xD cadence")
-- 0x5FDC signed compare / 0x600E clamp & wrap
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

-- Death list rate per kind: THON 0x80, spell death uses the E0/E4 list.
local death = R.death_list({ entity_kind = 15 })
check(death.rate == 0x80 and death.count == 1, "THON down rate 0x80")
death = R.death_list({ entity_kind = 5,
                       spell_death_pose = A.Pose.SPELL_DEATH_E0 })
check(death.rate == 0x280 and death.count == 10, "E0 list 10 @0x280")
check(A.hurt_audio(5) == A.LISTS.audio.hurt_by_kind[5] and
      A.down_audio(12) == A.LISTS.audio.down_by_kind[12],
      "hurt and down cues come from the generated ROM tables")
print(string.format("animator tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
