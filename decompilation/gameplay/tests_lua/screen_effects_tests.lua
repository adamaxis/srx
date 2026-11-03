local E = require("enums")
local ScreenEffects = require("effects.screen_effects")

local checks = 0
local function check(condition, message)
    checks = checks + 1
    if not condition then
        error(message or "screen effect check failed")
    end
end

local reveal = ScreenEffects.create(E.ScreenEffectKind.LINE_SCROLL_REVEAL, {
    duration_vblanks = E.PreTitleLogoReveal.DURATION_VBLANKS,
    start_counter = E.PreTitleLogoReveal.START_COUNTER,
    counter_cadence = E.PreTitleLogoReveal.COUNTER_STEP_CADENCE,
    first_reveal_counter = E.PreTitleLogoReveal.FIRST_REVEAL_COUNTER,
    full_reveal_counter = E.PreTitleLogoReveal.FULL_REVEAL_COUNTER,
    line_count = E.PreTitleLogoReveal.LOGO_HEIGHT,
    shimmer_tail_rows = E.PreTitleLogoReveal.SHIMMER_TAIL_ROWS,
    max_line_offset = E.PreTitleLogoReveal.MAX_LINE_OFFSET,
})
check(reveal.status == E.ScreenEffectStatus.ACTIVE and
      reveal.elapsed_vblanks == 0 and
      reveal.duration_vblanks == 337 and
      reveal.effect_counter == 0x41 and
      reveal.visible_rows == 0,
      "SEGA line reveal starts hidden at source counter $41 [C: 0x5D746]")

local line_scroll_values = {}
for row = 1, E.PreTitleLogoReveal.LOGO_HEIGHT do
    line_scroll_values[row] = (row * 7) % 32
end

local counter_step_updates = {}
local last_step_eligible_vblank = 0
for vblank = 1, E.PreTitleLogoReveal.DURATION_VBLANKS do
    local before_counter = reveal.effect_counter
    local before_eligible = reveal.eligible_vblanks
    ScreenEffects.update(reveal, {
        counter_eligible = vblank >= 93,
        line_scroll_values = line_scroll_values,
        shimmer_phase = (vblank - 1) % E.PreTitleLogoReveal.SHIMMER_PHASE_COUNT,
    })

    check(reveal.elapsed_vblanks == vblank,
          "effect duration counts one source VBlank per update [C: 0x5D746]")
    check(reveal.eligible_vblanks - before_eligible ==
              (vblank >= 93 and 1 or 0),
          "only source-eligible VBlanks advance the counter cadence [C: 0x5D7AA..0x5D7CE]")

    if reveal.effect_counter ~= before_counter then
        counter_step_updates[#counter_step_updates + 1] = vblank
        check(reveal.eligible_vblanks - last_step_eligible_vblank == 2,
              "counter steps on every second eligible VBlank [C: 0x5D7D2..0x5D802]")
        last_step_eligible_vblank = reveal.eligible_vblanks
    end

    if vblank < 114 then
        check(reveal.stage == E.PreTitleLogoReveal.STAGE_HIDDEN,
              "logo remains hidden before counter reaches $36 [C: GPGX e_00240]")
    end
    if reveal.effect_counter == E.PreTitleLogoReveal.FIRST_REVEAL_COUNTER then
        check(reveal.stage == E.PreTitleLogoReveal.STAGE_REVEAL and
              reveal.visible_rows > 0 and reveal.edge_row == reveal.visible_rows - 1,
              "first source reveal counter exposes a top-down shimmer edge [C: GPGX e_00240]")
        check(reveal.settled_rows ==
                  reveal.visible_rows - E.PreTitleLogoReveal.SHIMMER_TAIL_ROWS,
              "the new reveal starts with a bounded unsettled line band")
        for row = 1, reveal.visible_rows do
            check(reveal.line_offsets[row] >= 0 and
                  reveal.line_offsets[row] <= E.PreTitleLogoReveal.MAX_LINE_OFFSET,
                  "per-line offset stays within the source PRNG range [C: 0x5DAC4/0x0D44]")
        end
    end
    if reveal.effect_counter == 50 then
        check(reveal.settled_rows > 0 and
              reveal.line_offsets[reveal.settled_rows] == 0 and
              reveal.edge_row == reveal.visible_rows - 1,
              "settled rows advance from the top while the lower edge shimmers")
    end
end

check(#counter_step_updates == 122 and
      counter_step_updates[1] == 94 and
      reveal.eligible_vblanks == 245 and
      reveal.effect_counter == -57 and
      reveal.elapsed_vblanks == 337 and
      reveal.status == E.ScreenEffectStatus.FINISHED and
      reveal.stage == E.PreTitleLogoReveal.STAGE_COMPLETE and
      reveal.visible_rows == E.PreTitleLogoReveal.LOGO_HEIGHT,
      "337-VBlank SEGA effect finishes after 122 every-other eligible counter steps [C: 0x5D746]")
check(reveal.counter_step_count == 122,
      "effect reports the measured source counter-step count [C: 0x5D7D2..0x5D802]")
local finished_elapsed = reveal.elapsed_vblanks
ScreenEffects.update(reveal, { counter_eligible = true })
check(reveal.elapsed_vblanks == finished_elapsed,
      "finished effects ignore later updates")

local prng = ScreenEffects.create(E.ScreenEffectKind.LINE_SCROLL_REVEAL, {
    duration_vblanks = 8,
    start_counter = 1,
    counter_cadence = 1,
    first_reveal_counter = 0,
    full_reveal_counter = -1,
    initial_visible_rows = 4,
    line_count = 4,
    shimmer_tail_rows = 4,
    random_seed = 1,
})
ScreenEffects.update(prng, { counter_eligible = true })
check(prng.line_offsets[1] == 7 and prng.line_offsets[2] == 13 and
      prng.line_offsets[3] == 30 and prng.line_offsets[4] == 30 and
      prng.random_state == 56269,
      "Lua line jitter uses 0x0D44 state*0x20E5+1 and the five-bit sample [C]")

local fade = ScreenEffects.create(E.ScreenEffectKind.PALETTE_FADE, {
    duration_vblanks = E.FieldSceneTransitionVBlanks.BUILDING_FADE,
    direction = E.ScreenEffectFadeDirection.OUT,
})
for frame = 1, E.FieldSceneTransitionVBlanks.BUILDING_FADE do
    ScreenEffects.update(fade, {})
end
check(fade.status == E.ScreenEffectStatus.FINISHED and
      fade.elapsed_vblanks == 34,
      "shared palette fade accepts the existing 34-VBlank building duration [C: 0x131C/0x12F6]")

print(string.format("screen_effects_tests=%d", checks))
