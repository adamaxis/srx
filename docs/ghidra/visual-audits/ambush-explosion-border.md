# OPEN-10: Ambush explosion patch transparency

## Source trace and native change

The US ROM sequence at `0x5A0B2` reads four pointer/delay records from
`0x5A1D2`. Their holds are 12, 24, 60, and 12 updates. Each record selects a
13x12 tile map that is composited as a patch over the existing wilderness
scene. Empty map entries and color-zero pixels must leave the earlier scene
visible.

The offline `decode_genesis_intro_ambush_background_resources_impl` now treats
zero entries as transparent while decoding all four patches. It regenerates
the individually named RGBA assets. The typed intro phase list now carries
the ROM hold values in source order. The runtime still consumes the named
assets; no ROM address or tile decoder is used for live drawing.

## Nine-question visual review

1. **Does it look correct? Yes for the reported square.** Native full-screen
   frame `frame-004446.png` in the final 6,782-frame Release capture has no
   patch-sized rectangle behind the center attacker explosion. The former
   tone block is gone while the blast remains visible.
2. **Does the look match behavior? Yes.** The four ROM map records carry the
   overlay content and 12/24/60/12 update holds. Color-zero cells are now
   transparent, preserving the previously drawn wilderness layer.
3. **Are the visuals correct? Yes for this patch defect.** All four extracted
   assets contain both transparent and drawn pixels. The attacker, player,
   story text, projectiles, hit effect, smoke, and later layer cleanup remain
   present in the reviewed full-screen sequence.
4. **Should it animate?** The patch images are static per phase. The phase
   changes after 12, 24, 60, and 12 source updates. Explosion and smoke
   sprites keep their separate source animation schedules.
5. **Does it have sound?** Drawing the patch makes no sound request. The
   adjacent hit event `0x46` selects silent table-2 slot 70 in the traced ROM
   audio path; it has no audible clip or playback duration.
6. **Are other screen regions correct?** The full 320x224 frames were scanned
   from title through the map handoff and first field dialogue. The changed
   ambush frames were additionally reviewed side by side with the baseline;
   later scenes were not claimed as resolved by this patch.
7. **Do multiple frames support this interpretation? Yes.** Independent
   Genesis section 02 frames `000640`, `000642`, `000644`, `000646`, and
   `000648` show the same explosion interval without the rectangle. Native
   frames `004332` through `004446` cover the patch transition and hit. This
   is an automatic cutscene, so there is no click-through sequence.
8. **Are new graphics layered cleanly? Yes.** Zero map cells preserve the
   earlier scene. The patch adds no panel or control; no old button or
   interface is involved. Full-screen frames retain underlying actors and
   effects without a rectangular overdraw.
9. **Was the full time window reviewed? Yes, as a whole-screen overview.** All
   6,782 captured frames are represented in 106 chronological contact sheets
   (eight by eight images per sheet; the final sheet is partial). All 535
   pixel-differing baseline/fix frames were also reviewed in full-resolution
   comparison sheets; the 48 phase-timing changes were reviewed separately.
   Direct 320x224 frames were inspected at the key explosion and handoff
   points. The overview is a scan for sequence-wide anomalies; it does not
   replace pixel-level inspection of every unchanged duplicate frame.

## Outcome

- **Code changed:** the offline patch extractor preserves transparent zero
  cells; the four phase durations now match the ROM records.
- **Behavior implemented:** the four ambush overlays reveal the existing
  background through empty cells and advance after 12/24/60/12 updates.
- **Internally checked:** graphics and connected startup QA passed in Debug
  and Release; regenerated assets were loaded by the Qt capture.
- **Independently visually checked:** Genesis section 02 frames
  `000640-000648` show no rectangular patch; native full-screen frames and
  transition comparisons show the native layer is transparent.
- **Reported defect status:** OPEN-10 is resolved for the square border. The
  overall intro hill remains partial for other open behaviors.
