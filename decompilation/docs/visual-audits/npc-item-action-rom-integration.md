# NPC item action from a ROM encounter row

## Source trace

The deterministic field QA selects Scene 2's source random-sum-8 entry by
feeding the selector pair `4+4`. It does not skip or fabricate the row's actor
records. The cell at `0x1D71DC` points to the ordered list at `0x1D6E2A`, which
creates three Weenrs:

| Source order | Profile | Action | Source ammo |
|---:|---|---|---:|
| 0 | `weenr2_k08_item01` | item template 1 | 40 |
| 1 | `weenr1_k08_none00` | none | 0 |
| 2 | `weenr1_k08_none00` | none | 0 |

The ROM action path reaches `0x1514E` after the delay, projection, selected
animation, linked-target, and terrain-probe gates. The spawn cycle services the
source ammunition before publishing its projectile records and requests sound
event `0x1C`. On success, `0x141E8` passes the linked record's own `+0x2E`
heading to `0xB39A`; that routine installs the directional C042 selector and
reloads the source action delay. The C042 pose therefore comes from the
*linked actor's record heading*, not the relative bearing from shooter to
target. `0x600E` advances the selected finite list in the separate render
service.

The ROM row is retained in
[`dynamic-world-activity-actors.tsv`](../../assets/field/entities/dynamic-world-activity-actors.tsv).
The offline graphics importer exports named directional item-action frames
and their per-stem cadence in
[`actor-item-action-profiles.tsv`](../../assets/field/entities/actor-item-action-profiles.tsv).
The executable loads those files directly; it does not decode ROM data while
running.

## Connected behavior

The Lua actor heartbeat chooses the item action from the typed encounter
record, opens the source `+0xB0.4` action window when its route policy reaches
the required range, and returns the action decision. The C++ boundary validates
the returned Lua state for that same service pass. It then calls the typed
spawn-cycle service, consumes one round, routes the source sound event, retains
the projectile record, and makes the result available to the next Lua
animation heartbeat.

Both behavior and render heartbeats now use the same runner identity and carry
the active runner's position and source heading. This preserves the linked
record Lua selected during behavior when the following render pass installs
C042. A missing source heading remains an explicit unresolved case rather than
falling back to a guessed relative direction.

## QA and artifacts

`shadowrun_ghidra_conversion_qt_field_npc_item_action_qa` runs in Debug and
Release. It verifies that all three source rows appear, only source order 0 has
an item, the first shot creates one projectile, source ammo changes `40 -> 39`,
event `0x1C` is requested once, and the item's animation selector matches the
active runner's `+0x2E` heading. The captured trace records every field frame
before the first shot and for 64 frames after it.

Each build writes ten full-screen 320x224 BMPs under
`build/conversion/qt/{debug,release}/qt_runtime/field-npc-item-action-qa/`:
`actual.bmp`, the initial and encounter-entry frames, the C042 shot frame,
frames 8/16/31/32 after the shot, the first return-to-movement frame, and the
64-frame follow-up. `npc-item-action-trace.tsv` records animation, frame,
selector, ammo, distance, projectile count, and sound count. The
Debug/Release BMPs match pairwise by SHA-256. The existing Lua script-runtime,
random-encounter, and Hell Hound effect CTests also pass in both builds.

## Nine-question full-screen review

1. **Does it look correct?** The C042 frame uses the extracted directional
   actor art; the field scene shows the two source actors that enter the
   viewport while the third source row remains outside it. The frame is
   plausible, but the exact NPC C042 pose has not been isolated in a Genesis
   capture, so pixel fidelity remains open.
2. **Does the appearance match the actual state?** Yes for the captured host
   state: source order 0 has item 1 and 40 rounds; one spawn changes it to 39,
   produces one projectile record and one event `0x1C`, and selects the C042
   direction from the active runner's source heading. The other two records
   stay unarmed.
3. **Are the visuals correct?** The actor sprite is selected from the ROM
   extracted C042 directional frame and uses its recorded origin. No new
   rectangle or opaque black border appears around the sprite. The matching
   Genesis action frame is not confidently identified, so the extracted art's
   exact in-game pose and timing remain unverified.
4. **Is it animated, and when?** The Weenr item-action list contains one frame
   with increment `0x0080` and period `0x1000`. The live trace shows selector
   `0x0040` on the shot frame, then the moving actor selects walk selector
   `0x002C` on the next field frame. The other checkpoints follow movement
   through frame 64. The source code supports the C042 list and separate render
   cadence; an independent capture is still needed to settle the exact duration
   when the actor is moving.
5. **What sound plays, and for how long?** The action requests event `0x1C`,
   which maps to resource 028. The existing audio playback QA measures that
   resource at 2.483 seconds. This route verifies one request at the shot; it
   does not measure Windows loopback for this NPC shot specifically.
6. **Were the rest of the screen checked?** Yes. Every QA image was inspected
   at native 320x224 size. The map, runner, right HUD, and control strip remain
   intact through the encounter, shot, movement, and 64-frame follow-up. The
   encounter-entry image is identical to the initial frame because these
   source edge placements are outside the viewport at that point.
7. **Do multiple frames support the interpretation?** The full field-frame
   trace surrounds the shot, with whole-screen captures before it, at the
   action, at four follow-up points, on movement return, and after 64 frames.
   Independent longplay frames from
   `section_04_field_corridor_npc_dialogue_0238-0390`, frames 294, 297, 300,
   303, 306, 309, 312, and 315 (04:07.8-04:08.5), show several moving actors,
   target brackets, and changing combat state. They corroborate the field
   combat presentation, but do not identify this exact NPC action or its ROM
   action bytes.
8. **Are new visuals drawn over stale controls or art?** No. The actor's
   selected sprite replaces its previous animation in the actor draw pass; the
   projectile is a separate source record in the field entity draw order. The
   feature opens no menu and leaves no old buttons, panels, or duplicate actor
   sprite behind.
9. **Was the whole affected interval reviewed?** The trace covers every field
   frame from encounter setup through the first shot and 64 frames after it.
   All ten named outputs were reviewed; `actual.bmp` is byte-identical to the
   shot-frame alias. All Debug/Release image pairs are byte-identical. The
   longplay comparison covers eight consecutive reference samples rather than
   a single still.

## Status

The Scene 2 item-1 NPC path is connected through Lua decision, source ammo,
projectile allocation, event request, C042 selection, and the separate render
heartbeat. This is a partial combat hill. Other NPC item codes and item-effect
branches, projectile target hits/status/damage, and an independently matched
Genesis firing pose remain open. See the broader
[creature targeting review](field-random-encounter-creature-targeting.md) and
[player weapon spawn audit](weapon-spawn-ammo-consumption.md).
