# Opening Story Text in Lua

Date: 2026-10-09  
ROM: US Genesis Shadowrun  
Screen: opening story captions, ROM text stream `0x5B7D8..0x5BA4B`

## Source and ownership

The 22 opening captions are separate printable ROM strings, each terminated
by `0xFF`. The offline graphics importer reads the bytes at their traced
offsets, validates the terminator and visible-byte range, and emits stable
`opening.story.*` keys in `assets/dialogue/script-text.lua`. The
`startup.lua` module binds the typed `OpeningIntroTextAsset` identities to
those keys. The Qt intro presenter resolves copy from Lua; the native intro
schedule still owns when each caption is written, its row, palette bank, and
clear lifetime. The C++ conversion retains no copy of the captions.

## Review checklist

1. **Does it look correct?** The Seattle date, location, team, ambush, tri-vid,
   airport, credstick, and Stoker captions match the ROM wording. The supplied
   independent longplay shows the Seattle story in section 02 frames 000300,
   000400, 000420, and 000430; the opening strings appear in their expected
   lines above the scene.
2. **Does the look reflect the behavioral state?** The typed intro schedule
   selects each record and the Lua key map is validated at startup. Missing,
   empty, or extra story keys prevent the Qt frontend from starting with an
   incomplete opening-content table.
3. **Are the visuals correct?** The migration changes the text source only.
   Whole-screen captures preserve the source font, row positions, green ink,
   Seattle skyline, shrinking focus outline, actor layers, tri-vid, airport,
   and cab map. The independent 30-fps capture supports appearance and order,
   not exact VBlank timing.
4. **Is it animated?** Captions themselves are static. The existing typed
   schedule writes and clears them at source events; the Seattle focus frame
   continues through its five two-VBlank phases. No animation cadence changed.
5. **Is there a sound?** Caption records do not request an SFX. Opening music
   remains a separate stage service: command `0x10` starts the existing
   14,833 ms stage-1 track, as recorded in
   [the opening music audit](opening-music-start.md). This text migration adds
   no sound request and changes no playback duration.
6. **Are related screen parts correct too?** I inspected the complete startup
   and opening composition around the change: title options, all three
   archetype entries, Seattle and its outline, wilderness, tri-vid, airport,
   cab map, Stoker arrival/manager/farewell, and the first field return.
7. **Do multiple frames support the interpretation?** All 38 opening-scene
   captures from the Debug startup QA run were inspected, including all saved
   text phases and scene transitions. The independent longplay frames
   000300/000400/000420/000430 and 000470/000480 support the Seattle and
   wilderness sequence. Debug and Release hashes match for all 38 captures.
8. **Does new UI replace old UI cleanly?** These are captions rather than
   buttons. The existing blank text marker removes the prior row, scene
   changes clear the previous scene's text requests, and the inspected
   fade/scene/field returns show no caption or Stoker control left over the
   next page.

## Verification and disposition

- Debug and Release `shadowrun_graphics_extract --all-assets` each wrote
  1,562 extracted assets with zero missing; their full generated
  `script-text.lua` SHA-256 is identical:
  `779440032A49DC428D365649C2EC82B8E7F149938F61D009F2BB3CA355D9E5A2`.
- In each configuration, gameplay-script QA, full startup QA, and startup
  integration QA passed (3/3). The workbench startup integration CTest also
  passed in Debug and Release.
- All 38 Debug opening captures have byte-identical Release counterparts.
  Full-screen title/archetype and post-intro field captures were also reviewed
  for neighboring controls and stale overlays.
- Opening captions now resolve through Lua in the live Qt startup path. This
  does not complete the remaining unported building and NPC dialogue trees.
