# Random encounter and creature targeting review

## ROM evidence

- `0x59986` selects a field outcome from two source bounded draws; Lua's
  `field_encounters.trigger_attempt` now applies `0x1E166`'s blocker,
  cadence, and probability gates before the host consumes the source PRNG.
- A zero-cell result reaches `0x1E216`; `0x1E25A` initializes each listed
  actor, sets the encounter flags, writes its edge coordinate, and schedules
  the shared timeout. `0x1E302` uses live camera words `FFDDC6/FFDDC8` and map
  bounds `FFDDCE/FFDDD0`; `0x1E41E` checks the normalized terrain coordinate.
- `0xA8EA` services actor behavior separately from `0x4618`'s render pass.
  `0x13904` refreshes actor target policy on its source cadence, while the
  continuation advances accepted movement on each actor-service pass.
- `0x14492` chooses the target pool from actor `+0x10.3`; it does not select
  the nearest screen sprite. B's newly pressed bit reaches `0x1DC4C`, which
  sets the selected actor's `+0x11.4` latch and initializes `+0xA6` for the
  `0x5C7E` marker animation. The B path itself makes no sound call.

## Runtime evidence

Release CTest
`shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` passed. It
injects selector rolls `1+2` for Scene 2's sum-3 row and checks the two-Weenr
sequence, active actor records, source entry cue requests, Lua target link,
close-range movement, creature walk frames, and B selection surviving button
release. The script-runtime CTest also checks the four action-code marker
groups, the 64-to-0 shrink sequence, the persistent marker, and latch clearing.
The field QA matches all 14 opaque source pixels in each of the four 8x8
marker corners and confirms the right-side HUD is unchanged. These are not
tests of the production random-roll sequence or the full combat route. The
Release `shadowrun_gameplay_script_runtime_qa` separately checks each
`0x1E166` blocker, the `(frame_counter & 0x3FF) == 10` cadence, threshold
accept/reject behavior, the sequence-forward branch, and that Lua requests
the source probability draw only after state/cadence eligibility passes.

The real edge-spawn captures (`encounter-entry`, `after-30`, `after-180`, and
`after-480`) keep the full map and HUD composition and contain no visible
creature because the source candidates are beyond the current viewport. The
separate `encounter-visible-qa-fixture.bmp` moves the runner close to the
first creature to inspect its approach and walking art; it is a QA fixture,
not a simulated gameplay route. `creature-walk-frame-0.bmp` and
`creature-walk-frame-3.bmp` show distinct source walk poses. The target-marker
capture keeps the production spawn frames untouched, then repositions the
selected live record beside the runner only for an on-screen renderer check.
`target-selection-b.bmp` captures the shrinking stage and
`target-selection-b-static.bmp` captures the settled selector. Both retain the
full map and HUD; the HUD crop remains identical to the baseline.

ROM `0x5C7E` selects shared object-atlas tiles `0x5E5..0x5E7`, the palette line,
and four flipped 8x8 corner descriptors. The importer now exports each scene's
four variants as alpha-preserving PNGs. Lua owns the `+0xA6` render-pass
countdown: the first frame begins at radius 64, then the corners move inward by
8 pixels per render pass until the four-corner marker remains at radius zero.
The renderer draws those corners after the actor body to preserve the Genesis
SAT's earlier-sprite overlap priority. B itself has no sound call.

Independent combat footage in section 06 corroborates the marker shape:
`frame_002190.png` and `frame_002220.png` (9:43-9:44) show yellow corner
brackets; `frame_002430.png` and `frame_002460.png` (9:51-9:52) show red
brackets around a combatant. This confirms the separated-corner treatment and
that the hue varies in play. The footage does not expose actor action bytes, so
it cannot independently assign those hues to a specific action-code group or
prove a frame-for-frame match with this Scene 2 fixture.

## Eight-question screen review

1. **Does it look correct?** The extracted walk poses differ as expected, and
   the settled marker is a slim four-corner selector rather than a filled box.
   Independent combat frames show the same separated brackets in yellow and
   red. The exact Scene 2 hue is source-palette-derived, not matched to that
   different combat encounter.
2. **Does the look reflect the behavioral state?** B sets the selected actor's
   `+0x11.4` latch and initializes `+0xA6`; Lua retains selection after release
   and returns the marker style from the actor action byte. The pixel QA checks
   each of the four drawn corners, not just the state flag.
3. **Are the visuals correct?** The marker tiles, palette lines, flips, and
   descriptor offsets are from the ROM. PNG alpha prevents black tile
   backgrounds. The independent frames corroborate shape, but not the exact
   Scene 2 palette/action pairing.
4. **What animates, and when?** Creature movement uses named C062 directional
   frames on the render heartbeat. The marker shrinks 64, 56, ... 0 on render
   passes and stays static while the target latch remains set; clearing the
   latch removes it.
5. **What sound accompanies it?** B selection makes no ROM sound request. The
   tested entry sequence requests `0x2E`, which resolves to a verified silent
   or no-op resource; no audible duration is claimed.
6. **Were neighboring areas checked?** All 11 full 320x224 QA frames were
   opened, including production edge-spawn frames, the close fixture, two walk
   poses, and both selector stages. The full HUD remains unchanged.
7. **Are there multiple frames?** Yes: 0, 30, 180, and 480 service frames,
   two walk poses, selector shrink and settled frames, plus independent
   section-06 longplay frames 2190, 2220, 2430, and 2460.
8. **Are new UI elements covering stale ones?** The selector adds only the
   four source corner sprites to the field layer. Their transparent pixels
   remain transparent, the marker follows the selected actor, and the existing
   HUD is pixel-identical; no old menu or button layer is reused here.

## Status

ROM-selected spawn, entry state, Lua target acquisition, close-range movement,
walk animation, and the B selector's typed state and source pixels are covered
by focused Release QA. The ordinary gate policy is now Lua-owned and its
blocker/cadence/chance branches are covered by script-runtime QA; the field
spawn QA still injects the already-bounded selector rolls, so production RNG
consumption and event dispatch are not covered end to end. This hill remains
partial: the alternate target action-byte route through `+0x9C`, the full
actor pool, creature item/spell actions and hit resolution/damage/death, and
natural approach from the edge are unresolved. The independent longplay
corroborates the marker shape, but does not provide the actor bytes needed to
verify every palette variant.
