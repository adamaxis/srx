# OPEN-09: Ambush magic effect stops when the attacker is hit

## Source trace

The US ROM uses the same word at `$FFF146` to control the four-phase paired
effect and the persistent attacker hit sequence. `0x5A4C2` draws two paired
descriptors every six updates while that word is below four; its table wraps
the effect phase through the four pairs. At `0x5A252`, the hit transition
writes four to `$FFF146` before it starts the white palette state at `0x5A2D0`.
The next `0x5A4C2` call therefore returns without drawing either magic sprite.
The call order at `0x5A1EA` runs the hit sequence before the paired effect on
each update, so the stop takes effect on the same source update as the hit
transition.

The raw US ROM bytes at `0x5A252` and `0x5A4C2` are the authority for this
shared state and call order. Ghidra `fn_0005A1EA.cpp`, `fn_0005A252.cpp`, and
`fn_0005A4C2.cpp` corroborate the writes, comparison, six-update deadline,
phase wrap, and call sequence.

## Native change and connected QA

The prior conversion kept the persistent hit sequence and dual magic effect in
separate typed states. The hit callback now stops `TitleIntroDualSpriteState`
at the shared source transition. The native state exposes this as an `active`
flag; reset enables the effect, the source hit transition disables it, and
later calls emit no paired sprites. This preserves the behavior without making
the ROM RAM address a runtime dependency.

`shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa` saves full-screen
320x224 `magic-before-death-0.bmp` and `magic-before-death-1.bmp` frames with
the two magic sprites at different six-update phases, then checks that the
stage-5 hit frame has neither sprite. It also retains the head/body flash,
timed smoke, and cleanup checkpoints. The connected
`shadowrun_ghidra_conversion_qt_opening_death_layer_qa` advances every source
update through the attacker death and cockpit handoff and fails if a magic
sprite reappears after the hit transition. The direct typed-state CTest
checks reset, phase cadence, and stop behavior. All three CTests pass in Debug
and Release; the 14 named ambush and 14 death-layer BMP checkpoints are
byte-identical across configurations.

Independent section 02 frames `000660`, `000666`, `000672`, and `000678` show
the magician in the hit/fade/blast sequence without the earlier detached
paired sparkle. The recording supports appearance and order, not exact
60-Hz/VBlank alignment. The hit event `0x2F` selects silent table-2 slot 47;
the paired sprite routine itself has no sound request or audible duration.

## Nine-question visual review

1. **Does it look correct? Yes for the effect lifetime.** The native full-screen
   checkpoints show the paired spark before the hit, changing phase, and absent
   from the hit frame. Independent section 02 frames `000660-000678` support
   the effect stopping as the attacker is hit.
2. **Does the look match behavior? Yes.** ROM `0x5A252` writes the shared
   control value four; the next `0x5A4C2` call sees the stop value and submits
   no sprites. The runtime callback disables its typed pair state at that
   transition, and QA checks there are no `ambush_flash` requests.
3. **Are the visuals correct? Yes for the two spell sprites and their
   placement.** The four source descriptor pairs remain at `(0x183,0xBE)` and
   `(0x1AB,0xD7)` with low-priority palette flags. No asset or palette was
   changed. The separate square-border defect on the troll's explosion is
   tracked under OPEN-10 and is not considered fixed by this change.
4. **Should it animate? Yes, until the hit transition.** The paired effect
   changes phase every six source updates. The sequence starts with wilderness
   ambush choreography and stops on the same source update as the persistent
   hit transition. It does not restart during the later smoke or death pose.
5. **Does it have sound?** The paired effect routine has no sound call. The
   adjacent hit transition requests `0x2F`, which the ROM audio path sends to
   silent table-2 slot 47. There is no audible duration to reproduce.
6. **Are the other visible parts correct? Yes for the reviewed frames.** The
   whole 320x224 captures retain the foreground runner, story text, projectiles,
   the actor's white hit image, and later smoke. The troll explosion's square
   border remains a separate open visual defect.
7. **Do multiple frames support the interpretation? Yes.** Two full native
   frames show different active spell phases; the hit and later smoke frames
   show the stopped state. Independent section 02 frames `000660`, `000666`,
   `000672`, and `000678` support the ordering. The scene is automatic, so
   there is no player click-through path.
8. **Does new UI replace old UI cleanly?** No menu is involved. The frame
   presenter clears its one-frame sprite request batch before each source
   update. Stopping the spell pair removes only those requests; story text,
   projectiles, the hit sprite, and the smoke continue on their own source
   states.
9. **Was the whole time window reviewed? Partial.** The post-hit QA advances
   every source update through the complete death-layer interval and cockpit
   entry, checking that no paired spell sprite returns. Its named screenshots
   cover active phases, hit, smoke, clear, and handoff. A refreshed consecutive
   full-intro image capture is still needed after the separate OPEN-10 border
   correction so both intro fixes can be reviewed together.

## Outcome

- **Code changed:** the persistent hit transition stops the typed paired
  magic-sprite state.
- **Behavior implemented:** the paired six-update spell animation runs before
  the hit, stops on the hit transition, and stays stopped through smoke and
  death cleanup.
- **Internally checked:** direct-state, ambush-render, and death-layer CTests
  pass in Debug and Release; all 28 named BMPs match by SHA-256.
- **Independently visually checked:** section 02 frames `000660-000678` show
  the hit/blast sequence without the paired sparkle. Timing is not aligned to
  individual 60-Hz source updates.
- **Reported defect status:** the persistent magic animation is corrected in
  the connected native route. The final consecutive full-intro review remains
  open until the adjacent OPEN-10 extraction fix is complete.
