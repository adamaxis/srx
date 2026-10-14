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

Action code 12 is supported by the generic spell path although it is not one
of the four action profiles in the current random-encounter catalog. Its ROM
path retargets the first present runner lane with physical meter below four,
then applies a delayed direct heal at the target's render handler.

At `0x15830`, the ROM stores the caster link at target `+0xD8`, action code at
`+0xB2`, effect slot at `+0x32`, heading at `+0x2A`, countdown 5 at `+0x02`,
and phase 0 at `+0x03`. It calls the sound service with the event indexed from
`0x4946`, then calls a separate action setup entry from `0x4964`. A different
table at `0x490A` selects the target render handler. The delayed callback is
`0x55B78`.

The four encounter clocks plus the code-12 source clock read directly from ROM
are:

| Action code | Actor profile | Render handler | Reload | Phases | Terminal behavior |
|---:|---|---:|---:|---:|---:|
| 1 | Lone Mage | `0x521E` | 1 | 4 | heartbeat 12 |
| 2 | Gator Shaman | `0x5152` | 4 | 6 | heartbeat 31 |
| 5 | Rat Shaman | `0x52BE` -> `0x5322` | 6 | 9 | heartbeat 62; serial target fan-out |
| 9 | Elven Mage | `0x4FE2` | 4 | 6 | heartbeat 31 |
| 12 | Heal Wounds | `0x56DE` | 4 | 16 | heartbeat 81; direct heal callback |

All five start with target countdown 5. Their visible phase counts are 11,
30, 61, 30, and 80 target render heartbeats respectively; the completion call
enters the terminal phase and draws no sprite. These counts come from the
ROM's per-record render service, not a wall-clock timer. Code 12 reloads its
countdown to four and completes on call 81.

The phase image pointers are sourced from `0x58B6` (code 1), `0x589E` (code 2),
`0x5876` (code 5), `0x512A` (code 9), and `0x5756` (code 12). Their effect
palette attributes are `0x2000`, `0x2000`, `0x6000`, `0x4000`, and `0x6000`.
The offline importer turns those tables and each of the 56 scene palettes
into individually named RGBA images.
The Qt renderer places the target effect at the target actor's projected
position, above that actor sprite and below the HUD.

`0x4946` supplies spell events `0x32`, `0x34`, `0x37`, `0x35`, and `0x3A`.
The event IDs are hexadecimal; table-2 asset suffixes are decimal, so the
routes are resource 50 (2.467 s), 52 (2.517 s), 55 (2.820 s), 53 (2.434 s),
and 58 (2.703 s). These are decoded FLAC one-shot lengths at 44.1 kHz. Debug
and Release audio QA verifies that each event resolves, decodes, and plays. A
same-action independent Genesis sound recording is not available, so hardware
playback lifetime remains unverified.

## Eight-question visual and behavior review

1. **Does it look correct?** The 41 full-screen captures show each action's
   source frame without an opaque rectangular fill. The flame-dart phase-1
   overlay changes 813 pixels in the playfield; the extracted color-zero pixels
   leave the underlying map visible. This supports clean compositing, not an
   independent claim that the effect exactly matches a Genesis frame.
2. **Does the look reflect the behavioral state?** Lua stores pending effect
   state with the target source slot, actor ID, scene, action, and effect-slot
   identity. The slot handoff handles the active runner's `party.runner.N` /
   `field_player_runner` behavior-render alias. The target render heartbeat
   advances the state. Script QA asserts all five cadences, delayed impact,
   effect-slot ownership, the code-2 callback branch, sequential code-5
   fan-out, and code-12's first present wounded runner selection.
3. **Are the visuals correct?** ROM effect tables, per-scene palettes, and
   directional cast poses are extracted offline. Runtime code loads named
   images and uses the scene palette result directly. The field QA confirms
   image size, in-bounds placement, and phase changes. I visually reviewed
   all sixteen code-12 phases in `heal-wounds/phase-*.bmp`; its separate
   full-screen phase-1 capture is `heal-wounds/actual.bmp`. The effect remains
   inside the playfield and leaves the runner HUD clear in every captured
   phase. No independent cast screenshot confirms the exact appearance.
4. **Is it animated, and when?** The visual sequence advances on the target's
   ROM render heartbeat. Codes 1/2/5/9/12 complete after 12/31/62/31/81 calls
   and expose 11/30/61/30/80 visible calls. Adjacent ROM phases may
   intentionally repeat the same image; the capture review and Lua phase trace
   preserve those holds.
5. **Does it have sound, and how long does it play?** A successful `0x15830`
   slot allocation requests one action-specific sound event. The corresponding
   decoded resources for codes 1/2/5/9/12 last
   2.467/2.517/2.820/2.434/2.703 seconds. Playback tests pass in Debug and
   Release. The longplay does not provide a confirmed same-action Genesis
   spell sound to measure against.
6. **Were neighboring screen areas checked?** QA checks all 41 phase captures
   as 320x224 frames. The right HUD is pixel-identical to the pre-effect frame,
   and the effect stays left of it. The player, ROM encounter
   actors, floor, and background remain present. Scene 1's broad crosshatched
   surround already appears in the pre-effect image; this effect change does
   not account for it, and its independent map fidelity remains a separate
   open check.
7. **Are there multiple frames to support the interpretation?** Yes. The
   capture set has all four flame-dart, six flame-bolt, nine sleep, six
   mana-blast, and sixteen heal-wounds source phases. The Lua test also records
   every timer step through the target callback. No independent longplay cast
   sequence matching code 12 was found.
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

Code 5's `0x1597E` scan stores markers for active records whose direct route
cost is below `0x500` and whose source `+0x10` bit 6 is clear. At heartbeat 62,
`0x5322` clears the completed record and starts the same nine-phase effect on
the first still-marked active, unlocked record; each terminal pass advances
the next record in source order. Lua models the scan marks and pending effect
by source slot, and QA verifies two eligible records plus the strict range
edge, inactive, and locked exclusions. Code 2 now reads the current typed
`FFE186` scene state and the caster's current service flags from the live actor
candidate list; it skips `0x55B78` only when the scene state is 3 and caster
`+0x10` bit 3 is clear. Both outcomes are tested. The code-12 lane scan uses
active party slots and their typed physical meters, selects in source order,
and suppresses the cast when no present lane is below four. Its `0x55C2C`
delayed heal callback, effect clock, meter/source-flag persistence, and `0x3A`
sound route are implemented. The ROM-selected `0x4964` setup-table dispatch
to `0xB574` remains an explicit boundary. Independent Genesis cast
pixels/audio, other spell codes, and the Scene 1 map surround remain
unresolved. See
[`actor-ai-heartbeat-source-audit.md`](../actor-ai-heartbeat-source-audit.md)
and [`audio-playback-verification.md`](../visual-analysis/audio-playback-verification.md).
