# Actor spell/item line probe review

## Scope and source trace

This slice connects the existing Lua spell/item dispatch gate to the ROM's
`0x14D8E` collision-window check. The ROM trace is recorded in
[`actor-ai-heartbeat-source-audit.md`](../actor-ai-heartbeat-source-audit.md):
`0x13FFE` reaches the probe only after its target, delay, projection, and action
code gates; `0x14D8E` initializes a linked scratch record through `0x14FF8`,
tries 27 projected positions, and returns carry only for a blocking terrain
result with `FFDFE2` clear. The nested `0x15ABA` carry is cleared at `0x14DEE`.

The placement tables were read directly from the US ROM at `0x15A4C` and
`0x15A6C`. The new conversion assertions cover all eight heading sectors.
`0x14F26` consumes signed fixed-point coordinates; the field bridge now passes
the high-word source coordinate to the map collision service and carries the
boundary marker from that same collision sample.

## Eight-question visual and behavior review

1. **Does it look correct?** The probe is an action gate and draws no pixels.
   Its earlier field frames show no new rectangle or selector. The separate
   spell-effect field capture now shows the ROM-selected Lone Star mage and
   target effect; that capture verifies internal rendering, not Genesis
   fidelity.
2. **Does the look reflect the behavioral state?** The callback is installed
   on temporary world actors. Lua invokes it only after the linked-target,
   expired-delay, active-projection, and unsuppressed-code gates. A blocked
   terrain result with a clear `FFDFE2` sample returns blocked; boundary-active
   terrain and the terminal inner collision branch preserve the ROM's clear
   outer carry. An unresolved probe fails closed in the Qt bridge. Conversion
   tests cover these outcomes and stale-marker avoidance.
3. **Are the visuals correct?** The offline importer now extracts the
   directional `C082` cast poses and per-scene spell-effect frames. The field
   renderer uses those named assets. The full-screen spell audit checks their
   placement and transparency; no independent Genesis cast capture is
   available to confirm exact visual fidelity.
4. **Is it animated, and when?** The collision window is a synchronous
   27-probe routine, not a timed animation. A successful Lua spell dispatch
   selects a directional `C082` pose. The target effect runs on its render
   heartbeat for 12, 31, 62, or 31 calls for action codes 1, 2, 5, or 9.
   The shared source callback resolves on the final call; full phase and
   visibility traces are covered by the Lua test.
5. **Does it make a sound?** `0x14D8E` itself has no sound request. Once
   `0x15830` accepts an effect slot, its `0x4946` table selects events
   `0x32`/`0x34`/`0x37`/`0x35`, routed to decimal table-2 resources
   50/52/55/53. Their decoded host one-shot lengths are 2.467/2.517/2.820/
   2.434 seconds. No independent Genesis spell sound is matched yet.
6. **Were neighboring screen areas checked?** All 27 full 320x224 Scene 2
   random-encounter BMPs and all 8 Hell Hound effect BMPs were opened: route
   entry and checkpoints, the visible QA fixture, both walk and action poses,
   both target-selector states, all five production-dispatch checkpoints, all
   four C072 hit-response captures, and all four C0B2 down-state captures,
   plus the Hound encounter/effect phases and release. The map, floor, runner,
   and right HUD remain composed; no old menu or button is drawn beneath a new
   control. The five production-dispatch images are byte-identical because the
   selected actors stay outside the viewport and their entry cue is silent.
   The hit captures retain the selected actor's red corner marker; the QA
   sends B to cycle away from the downed record only after those screenshots.
   The separate Scene 1 spell QA reviews all 25 effect-phase captures and
   checks that the right HUD is pixel-identical to the baseline. Its broad
   crosshatched map surround is present in the pre-effect image too; this
   slice does not establish that underlying scene's independent fidelity.
   Debug/Release image hashes match for the existing routes. The collision
   callback does not draw or publish global movement markers while probing.
7. **Are there multiple frames and an independent example?** The encounter
   frames show entry, approach, two walk poses, and both action frames. The
   target-selection captures distinguish an active marker from the preceding
   field frame. Independent longplay frames 2190/2220 and 2430/2460 in section
   06 show separated yellow and red target brackets. They corroborate marker
   shape and hue variation, but they do not show this actor's spell gate or a
   cast pose. The spell capture contains every extracted source phase,
   including repeated images where the ROM holds a phase across calls.
8. **Are new graphics covering stale graphics?** The bridge adds no drawing
   operation or UI layer. The existing full-frame captures show no additional
   button, menu, or stale panel underneath the actor and HUD.

## Validation and open work

Debug and Release passed `shadowrun_ghidra_entity_transition`,
`shadowrun_ghidra_entity_animation_spawn_probe`,
`shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa`, and
`shadowrun_ghidra_conversion_qt_field_hound_close_effect_qa`. These protect the
typed probe, source placement tables, and existing full-frame actor
composition. The full-screen review confirms there is no new rectangle,
selector, or stale UI layer from this bridge. The line-probe callback requests
no sound; the downstream spell action requests its own table-driven cue.
The four spell profiles now traverse Lua action/effect resolution, target-owned
render timing, named cast/effect assets, sound routing, and a full-screen
renderer capture. Code-5 propagation and the code-2 `FFE186` callback gate are
now covered by source-driven Lua state and script QA; independent Genesis
visuals/audio for a same-action cast remain unavailable. See
[`actor-spell-effect.md`](actor-spell-effect.md) for the source trace and
screen-by-screen review.
