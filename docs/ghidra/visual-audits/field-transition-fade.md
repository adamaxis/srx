# Projected Interaction Fade Audit

Date: 2026-10-05  
Source trace: US ROM `0xB7B8` (`0xBA86..0xBA9E`), `0x16396..0x163D8`, and coordinator `0x095E..0x0ACC`.  
Runtime: `qt_runtime/field_scene_runtime.cpp`; QA: `shadowrun_ghidra_conversion_qt_field_transition_qa`.

## Evidence

- `0xB7B8` sets `FFDF3A=-1`, `FFDF3C=0x10`, and `FFDFE8=-1` after the world-interaction projection succeeds.
- Each ordinary field frame calls `0x16396` after camera/render updates and before world-frame logic. It services active effects, steps the palette when the countdown is even, decrements the signed counter, and publishes `FFDF3E=-1` only after the value falls below zero.
- Starting at 16 therefore yields 17 frame updates: nine even-count palette calls, the counter reaches zero on update 16, and update 17 expires it. The next coordinator pass checks `FFDF3E` before sampling controller input.
- The source path has no direct sound call in `0x16396`. Active-effect servicing is a separate call and is not treated as evidence of a new fade sound.
- Archived Debug and Release QA frames (`initial.bmp`, `fade-01.bmp`, `fade-08.bmp`, `fade-16.bmp`, `fade-17.bmp`, and `actual.bmp`) are under `build/conversion/qt/{debug,release}/qt_runtime/field-transition-qa/`.

## Question Pass

1. **Does it look correct?** Unknown against the original game: no matching independent Genesis capture of this transition was identified. Internally, the captured field steadily darkens and reaches a full black frame.
2. **Does the look match behavioral state?** Yes for the covered counters. At frame 8 the timer is 8 and four palette steps have run. At frame 16 the timer is zero, the transition remains active, and no coordinator request has been published. At frame 17 the signed counter expires, the lock clears, and `FFDF3E` becomes pending.
3. **Are the visuals correct?** Screen-wide coverage is checked: field map, player, HUD, and empty panels darken together. The host currently approximates palette changes with a uniform black overlay; exact per-color CRAM arithmetic is not established.
4. **Should it animate?** Yes. ROM `0x16396` steps on even counter values. QA captures frames 1, 8, 16, and 17 and verifies the 17-update lifetime and nine source step callbacks. This QA begins at the typed output of `0xB7B8`; physical collision triggering is not covered here.
5. **Is there a sound?** No direct sound call was found in `0x16396`. The nearby active-effect service remains its own behavior; this fade adds no sound event.
6. **Are other parts correct?** The full 320x224 frame is inspected. The HUD and player fade with the playfield, and the last capture is entirely black. No unrelated interface is open in this field-transition fixture.
7. **Do multiple frames support it?** Yes internally: initial, frame 1, frame 8, frame 16, and frame 17 are captured. Independent gameplay frames for this same transition were not found, so the captures are regression evidence, not a Genesis visual golden.
8. **Does new UI replace old UI cleanly?** Not applicable to this slice; it adds no page or controls. The pending `FFDF3E` handoff is verified, but the subsequent live world-activity selection and scene re-entry remain open.

## Result

- **Code changed:** connect the converted signed fade timer to Qt's ordinary field-frame callback and render the fade over the complete frame.
- **Behavior implemented:** 17-update timer, even-count step cadence, lock release, and next-pass field-transition request.
- **Internally checked:** Debug and Release CTest plus the multi-frame BMP set above.
- **Independently visually checked:** no matching original-game capture was found.
- **Reported defect resolved:** only the missing timer-to-transition handoff is implemented; the complete field/world-activity transition is still partial.
