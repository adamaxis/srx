# Package run origin and delivery event

Status: **partial**. The run logic is connected to the field-exit
orchestrator's first dispatch stage, but there is no full-screen capture of an
accepted run continuing through both selected map events. The script-runtime
test proves event matching and once-only payout; it does not prove that whole
playthrough.

## ROM trace and host behavior

- ROM `0x1E69C` selects the run type and stores its source progress byte.
- `0x1EEDC` dispatches `FFFBED` through the six-pointer table at `0xF3E56`.
  Entries 1 and 2 both point to the shared handler at `0x1EF2C`.
- At the origin, `0x1EF2C` compares `FFFBF8/FFFBF9` to `FFFBEE/FFFBEF` and
  sets bit 0 of `FFFC1C`, which gates the later destination comparison. It
  sets alternate result `FFF0D5` to `0xFF` and bit 9 of `FFFC22` for run type
  2.
- After that bit is set, the handler compares the current tuple to
  `FFFBF1/FFFBF2`. For type 2 it sets `FFF0D5` to 1 and clears `FFFC22` bit 9.
  Common payout at `0x1F00A` adds `FFFBF4` to funds and adds `FFFBF3` to the
  `+0x8D` byte of each of the three runner records.
- The type-2 path contains no sound request, actor spawn, or animation start.
  The Qt adapter now calls Lua at the `0x1EEDC` dispatch stage, applies the
  typed alternate result/active flag, then applies the payout and progress
  bytes. It does not add a panel, sprite, animation, or audio request.

## Nine-question visual check

1. **Does it look correct? - Unknown for the event result screens.** No
   event-result screen is captured yet. The existing package offer layout is
   supported by independent section-03 frames 003000, 003150, 003300, and
   003450; those show the selected package route, payment prompt, and accepted
   confirmation.
2. **Does the visible look match behavioral state? - Partial.** The runtime
   test checks wrong-location, destination-before-pickup, origin, an
   intermediate tuple, destination, and duplicate destination. It confirms
   state/reward outputs, but the event callback has not been driven in a
   captured field session.
3. **Are the visuals themselves correct? - Partial.** This change adds no
   drawing code or asset changes. The full-screen Release `actual.bmp` from
   `field-jump-house-entry-qa` was compared with section-03 frames 001350,
   001365, and 001380. The blue grid-like lines remain present across those
   independent frames, so they are part of the scene, not a newly drawn debug
   overlay. The runner placement differs slightly in the existing QA capture.
4. **Should anything animate? - Yes: no run-event animation is called by the
   type-2 ROM path.** The grid remains visually stable in reference frames
   001350/001365/001380 while nearby actors and the runner change pose/position.
   No new animation timer or actor was added for package pickup/delivery.
5. **Does it have sound? - Yes: no call traced.** The type-2 origin/destination
   branches at `0x1EF2C` and common payout at `0x1F00A` contain no sound request;
   this change adds no audio mapping or playback call. This is source-call
   evidence, not a playback-duration measurement.
6. **Are the other visible parts correct? - Partial.** Whole-screen Release
   captures `field-jump-house-entry-qa/actual.bmp`, `gunderson-root.bmp`,
   `field-ares-collision-shop-qa/ares-shop-category.bmp`, and
   `field-stoker-collision-to-dialogue-qa/actual.bmp` were reviewed after the
   callback moved. The screens show no new controls or stale overlay. These
   all four Debug/Release pairs are byte-identical by SHA-256. These routes have no active package offer, so they do not cover the event result.
7. **Do multiple frames support the interpretation? - Partial.** Independent
   frames 001350/001365/001380 support the field-map layer; 003000/003150/
   003300/003450 support the package-offer dialog sequence. No supplied frame
   was identified for the player visiting the selected pickup and destination
   as one run.
8. **Does new UI replace old UI cleanly? - Yes for this change.** The event
   callback draws no UI and creates no overlapping controls. A captured
   pickup/delivery transition is still needed to check the following generic
   field prompt after `FFF0D5` changes.
9. **Was the whole affected time window reviewed? - No.** The selected run
   offer through pickup, delivery, payout, and the first stable field frame
   has not been captured continuously. Keep this hill partial.

## Internal verification

`shadowrun_gameplay_script_runtime_qa` passes in Debug and Release. It verifies
that the route generated from the ROM offer data is origin `area_2/location_4`,
destination `area_2/location_5`, negotiated payout 45, and progress delta 1;
pickup arms the package, only the later selected destination completes it, and
the repeated destination cannot pay again. Four focused whole-screen field
route CTests also pass in Debug and Release. They guard adjacent menus and
field screens but do not exercise an active package run.
