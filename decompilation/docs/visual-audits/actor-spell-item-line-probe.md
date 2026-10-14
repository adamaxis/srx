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
   The reviewed field frames show no new rectangle or selector. This does not
   establish the appearance of a spell cast because the production encounter
   rows reviewed here contain no spell-action actor.
2. **Does the look reflect the behavioral state?** The callback is installed
   on temporary world actors. Lua invokes it only after the linked-target,
   expired-delay, active-projection, and unsuppressed-code gates. A blocked
   terrain result with a clear `FFDFE2` sample returns blocked; boundary-active
   terrain and the terminal inner collision branch preserve the ROM's clear
   outer carry. An unresolved probe fails closed in the Qt bridge. Conversion
   tests cover these outcomes and stale-marker avoidance.
3. **Are the visuals correct?** This change adds no graphic. If a future source
   actor reaches the successful spell branch, Lua selects a `C082` pose, but
   those directional frames are not extracted and the presenter currently
   draws no spell sprite. That visual remains unresolved.
4. **Is it animated, and when?** The collision window is a synchronous
   27-probe routine, not a timed animation. A successful Lua spell dispatch
   records frame 0 and holds it on the render heartbeat. The visible `C082`
   animation cadence remains unverified until its frames are extracted and
   compared with a same-action game capture.
5. **Does it make a sound?** The traced `0x14D8E` path has no sound request and
   this bridge adds none. The downstream `0x1033E`/`0x15830` effect and any
   associated cast audio are still open, so no cast duration is claimed.
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
   Debug/Release image hashes match for these routes. The collision callback
   does not draw or publish global movement markers while probing.
7. **Are there multiple frames and an independent example?** The encounter
   frames show entry, approach, two walk poses, and both action frames. The
   target-selection captures distinguish an active marker from the preceding
   field frame. Independent longplay frames 2190/2220 and 2430/2460 in section
   06 show separated yellow and red target brackets. They corroborate marker
   shape and hue variation, but they do not show this actor's spell gate or a
   cast pose.
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
no sound, and the field captures do not trigger the new spell/item callback.
The next spell work still needs an actual source-backed actor action, the
`0x1033E` dice/effect path, `0x15830` target effect, `C082` assets, and an
independent same-action capture before visual correctness can be claimed.
