# Ghidra-conversion Qt startup QA

The separate executable `shadowrun_ghidra_conversion_qt` now has one
deterministic startup walk:

```text
shadowrun_ghidra_conversion_qt \
  --assets decompilation/assets \
  --startup-qa <output-directory>
```

It drives the converted source contracts through the same typed state used by
the Qt window and records `startup-qa.tsv`, `event-log.txt`, presentation
boundaries, and BMP checkpoints. The output directory is intentionally outside
`decompilation/assets`; these are QA artifacts, not runtime resources.

The CTest entry
`shadowrun_ghidra_conversion_qt_startup_qa` runs this walk once. It covers:

- reset/pre-title to title-ready;
- Start Options New Game and Load Game;
- the empty Save/Load slot message and return to Start Options;
- all three Joshua choices (Samurai, Decker, and Gator Shaman);
- opening checkpoints at source frames 300 through 3000 and every typed stage
  transition;
- all five taxi-cursor outline phases and settled holds for both mode 4 and
  mode 2; the full-screen assertions check all eight cursor tiles against
  their ROM-derived Plane-A positions;
- the opening-to-field handoff; and
- opening, closing, and reopening the field command overview.

## Source boundaries exercised

The walk is organized around the ROM entry points already recorded in the
conversion audit:

| Behavior | ROM/source boundary |
| --- | --- |
| reset and pre-title logos | `0x0236`, `0x5D746` |
| Start Options | `0x59092` |
| Save/Load | `0x59172` |
| Joshua archetype selection | `0x59248`, `0x42B0` |
| timed opening | `0x59C5E` and its stage chain |
| recurring field pass | `0x075E`, `0x0B08` |
| command overview | `0x10D30` |

The archetype portrait placement is source-derived from the `0x592C2` call to
`0x1F38A`: the selected 80-word resource is uploaded at the source `(8, 8)`
tile origin, which is `(64, 64)` in the host scene. This replaced the earlier
unverified `(80, 56)` placement.

## Defect caught by the walk

The command overview close path previously dismissed the Qt panel but left the
converted field display-service word at `0xFFFF`. The normal field coordinator
therefore rejected every later Start edge. The close transition now publishes
the source completion (`display_service_word = 0`) before returning to the
normal field pass. The QA artifacts contain both command-open frames and the
field frame between them, so a one-shot menu regression fails the walk.

## Visual interpretation

The independent reference is the 30-fps extraction under
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`.
The converted source loop is advanced at its source cadence, so checkpoint
numbers are source frames and must not be compared to equal-number video
frames. The video remains the visual authority for scene order, layer presence,
placement, and clearing behavior; the QA BMPs are evidence of the current
native presentation and are not themselves a Genesis golden.

This walk verifies the startup/field handoff hill. It does not claim that the
full field, combat, inventory, dialogue, save restoration, or audio systems
are complete; their remaining status stays in `HILLS.md` and
`decompiled-function-status.tsv`.
