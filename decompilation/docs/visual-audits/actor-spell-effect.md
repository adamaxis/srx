# Dynamic actor spell effect review

## ROM trace and connected path

The active encounter catalog has four spell-action profiles:
`lonemage_k0a_spell01`, `gator_shaman_k09_spell02`,
`rat_shaman_k0b_spell05`, and `elven_mage_k06_spell09`. The spell gate at
`0x13FFE` now reaches Lua's `combat.resolve_spell_action` after the source
target, delay, projection, action-code, and `0x14D8E` collision-window gates.
On success, `0x1033E`'s typed action result reserves an effect slot and starts
the corresponding target effect state in Lua. These actions do not consume
weapon ammunition.

At `0x15830`, the ROM stores the caster link at target `+0xD8`, action code at
`+0xB2`, effect slot at `+0x32`, heading at `+0x2A`, countdown 5 at `+0x02`,
and phase 0 at `+0x03`. It calls the sound service with the event indexed from
`0x4946`, then calls a separate action setup entry from `0x4964`. A different
table at `0x490A` selects the target render handler. The delayed callback is
`0x55B78`.

The four live render clocks read directly from ROM are:

| Action code | Actor profile | Render handler | Reload | Phases | Delayed callback |
|---:|---|---:|---:|---:|---:|
| 1 | Lone Mage | `0x521E` | 1 | 4 | heartbeat 12 |
| 2 | Gator Shaman | `0x5152` | 4 | 6 | heartbeat 31 |
| 5 | Rat Shaman | `0x52BE` -> `0x5322` | 6 | 9 | heartbeat 62 |
| 9 | Elven Mage | `0x4FE2` | 4 | 6 | heartbeat 31 |

All four start with target countdown 5. Their visible phase counts are 11,
30, 61, and 30 target render heartbeats respectively; the completion call
enters the terminal phase and draws no sprite. These counts come from the
ROM's per-record render service, not a wall-clock timer.

The phase image pointers are sourced from `0x58B6` (code 1), `0x589E` (code 2),
`0x5876` (code 5), and `0x512A` (code 9). Their effect palette attributes are
`0x2000`, `0x2000`, `0x6000`, and `0x4000`. The offline importer turns those
tables and each of the 56 scene palettes into individually named RGBA images.
The Qt renderer places the target effect at the target actor's projected
position, above that actor sprite and below the HUD.

`0x4946` supplies spell events `0x32`, `0x34`, `0x37`, and `0x35`. The event
IDs are hexadecimal; table-2 asset suffixes are decimal, so the routes are
resource 50 (2.467 s), 52 (2.517 s), 55 (2.820 s), and 53 (2.434 s). These are
decoded FLAC one-shot lengths at 44.1 kHz. Debug and Release audio QA verifies
that each event resolves, decodes, and plays. A same-action independent Genesis
sound recording is not available, so hardware playback lifetime remains
unverified.

## Eight-question visual and behavior review

1. **Does it look correct?** The 25 full-screen captures show each action's
   source frame without an opaque rectangular fill. The flame-dart phase-1
   overlay changes 813 pixels in the playfield; the extracted color-zero pixels
   leave the underlying map visible. This supports clean compositing, not an
   independent claim that the effect exactly matches a Genesis frame.
2. **Does the look reflect the behavioral state?** Lua stores pending effect
   state under the target's stable actor ID, with source slot, scene, action,
   and effect-slot identity. The target render heartbeat advances that state;
   the script test asserts all four cadences, delayed impact, and slot release.
3. **Are the visuals correct?** ROM effect tables, per-scene palettes, and
   directional cast poses are extracted offline. Runtime code loads named
   images and uses the scene palette result directly. The field QA confirms
   image size, in-bounds placement, and phase changes; no independent cast
   screenshot confirms the exact appearance.
4. **Is it animated, and when?** The visual sequence advances on the target's
   ROM render heartbeat. Codes 1/2/5/9 complete after 12/31/62/31 calls and
   expose 11/30/61/30 visible calls. Adjacent ROM phases may intentionally
   repeat the same image; the capture review and Lua phase trace preserve those
   holds.
5. **Does it have sound, and how long does it play?** A successful `0x15830`
   slot allocation requests one action-specific sound event. The corresponding
   decoded resources last 2.467/2.517/2.820/2.434 seconds. Playback tests pass
   in Debug and Release. The longplay does not provide a confirmed same-action
   Genesis spell sound to measure against.
6. **Were neighboring screen areas checked?** All 25 phase captures were
   inspected as 320x224 frames. The right HUD is pixel-identical to the
   pre-effect frame, and the effect stays left of it. The player, ROM encounter
   actors, floor, and background remain present. Scene 1's broad crosshatched
   surround already appears in the pre-effect image; this effect change does
   not account for it, and its independent map fidelity remains a separate
   open check.
7. **Are there multiple frames to support the interpretation?** Yes. The
   capture set has all four flame-dart, six flame-bolt, nine sleep, and six
   mana-blast source phases. The Lua test also records every timer step through
   the target callback. I found no independent longplay cast sequence that
   matches one of these four action profiles.
8. **Are new graphics drawn over stale controls or panels?** This change adds
   a sprite to the field entity pass; it does not add a button, panel, or menu.
   The complete field/HUD frame was reviewed for all phases, and the HUD region
   compares byte-for-byte with the baseline.

## Validation and remaining source boundaries

`shadowrun_gameplay_script_runtime_qa`,
`shadowrun_ghidra_conversion_qt_field_spell_effect_qa`, and
`shadowrun_ghidra_audio_playback` pass in Debug and Release. The field capture
uses the ROM-selected Scene 1 Lone Star sequence and moves its mage beside the
runner for clear inspection; that placement is diagnostic, not a claimed
natural encounter replay. The four script timers use a separate target actor
render heartbeat, not the caster's behavior heartbeat.

Code 5's source handler clones the completed effect to active, non-excluded
records with `+0xDE == 5`; Lua propagation is still open. The code-2 handler
also checks `FFE186 == 3` and source `+0x10` bit 3 before calling the delayed
callback; those facts are not yet in the typed Lua context. Other spell codes,
independent visual/audio references, and the Scene 1 map surround remain
unresolved. See [`actor-ai-heartbeat-source-audit.md`](../actor-ai-heartbeat-source-audit.md)
and [`audio-playback-verification.md`](../visual-analysis/audio-playback-verification.md).
