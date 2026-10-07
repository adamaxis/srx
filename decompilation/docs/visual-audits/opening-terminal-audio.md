# Terminal death-pose audio source audit

Date: 2026-10-06
Scope: `OPEN-07`, the `0x2E` request recorded with Michael's terminal death
pose and left-plate clear.

## ROM command path

US-ROM bytes at `0x59FA6` load `0x002E` into `D0`; `0x59FAA` calls
`0x530DE`. That routine tests `FFFC0A` and branches to `0x530FE` when audio is
enabled. `0x530FE` preserves `D0` and calls `0x52F3C`. The 68000 routine at
`0x52F3C` appends opcode `0x10` followed by the saved `D0` argument, so this
event reaches the Z80 as `FF 10 2E`: start table-2 slot 46. The `FFFC08` check
is in separate routine `0x530EA` and is not part of this event's call path.

The same `0x59F8E` branch runs only when elapsed time equals `0x168`, then
returns after the final pose request. The source submits the sound command
once; it is not a loop or animation-timed retrigger.

## Resource output

I ran the ROM-backed offline extractor on the adjacent known-active slot 37,
known-silent slot 44, and requested slot 46:

```text
python decompilation/tools/audio/extract_genesis_audio.py \
  decompilation/input/shadowrun_genesis_us.bin \
  build/visual-audits/open07-resource-trace/compare \
  --ids 37,44,46 --seconds 5
```

The probe reported 38 chip writes for slot 37, but nine each for slots 44 and
46. Slot 46's writes are the four PSG maximum-attenuation setup values
(`9F BF DF FF`) and five YM setup/mode writes; it produces no YM key-on or
tone-data sequence. It is absent from `assets/resources.tsv` and has no
`resource-046.flac`. The existing audio extraction audit records that all 94
table-2 slots were tested and the 29 silent/no-op slots were omitted. This
source trace and targeted probe place slot 46 in that silent group.

There is therefore no audible effect file or playback duration to add. The
runtime correctly leaves event `0x2E` unmapped. The playback CTest now includes
`0x2E` among unmapped events and verifies it never reaches the asset resolver.

## Nine-question review

1. **Does it look correct?** The `0x2E` callback is recorded at the terminal
   death-pose event. It adds no image or UI layer.
2. **Does visible state match behavior?** Yes. The typed event is emitted once
   at elapsed `0x168`, alongside the left-region clear and final-pose draw.
3. **Are the visuals correct?** No visual code changed for this audio review.
   The Release full intro remains byte-identical to the reviewed post-clear
   capture in all 6,782 frames.
4. **Is it animated?** The death pose is static after its terminal draw. The
   source issues `0x2E` once, not on each pose frame.
5. **Is there sound, and how long does it play?** The command is a start
   request for table-2 slot 46, but the ROM-backed extraction produces only
   setup/quieting writes. It is a verified silent/no-op slot, so no sound or
   playback duration is expected.
6. **Are related screen parts also correct?** The full intro review includes
   the surrounding wilderness scene, projectile motion, tri-vid replacement,
   later airport/cab-map scenes, menus, and field arrival.
7. **Do multiple frames support this interpretation?** Native frames
   1208-1211 and 1266/1354/1366 were inspected; independent section 02 frames
   730-780 show the prior pose, clear, stable terminal pose, and later
   projectile motion.
8. **Are old controls or graphics left under new ones?** This event adds no
   controls. The same full-screen run shows the left actor plate cleared before
   the terminal pose and the later scene replacement removing that pose.
9. **Was the whole window reviewed?** Yes. The complete 6,782-frame Release
   capture, including the stable field hold, was compared and reviewed as
   documented in the [full intro capture](full-intro-capture-2026-10-06-open03-clear.md).

## Result

- **Code changed:** no runtime audio alias was added; the playback test now
  guards against assigning a sound to `0x2E`.
- **Behavior implemented:** unchanged; the converted callback records the
  source command, and the runtime emits no audio for the silent resource.
- **Independently checked:** terminal visual frames match the independent
  section 02 sequence; ROM-backed Z80 extraction found no audible resource at
  slot 46. The supplied longplay audio is a mixed capture and does not provide
  an isolated stem or precise VBlank-to-audio timing.
- **Defect status:** `OPEN-07` is closed as a non-defect. The missing playback
  alias is correct for this source slot.
