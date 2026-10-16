# Ares Weapon Emporium physical entry audit

**Status:** normal-control field entry is connected and verified in Debug and
Release. A separate scene-20 route now reaches the physical maglock prompt and
returns with Walk Away; see [the scene-20 audit](scene20-maglock-physical-entry.md).
The broader FIELD-02/PLAY-01 goals remain partial because maglock action
results, Security Terminal, building item-use, and other building routes are
not all connected.

## ROM and typed-state route

The US ROM bytes at `0x173142` are
`04 C0 04 DF 02 20 02 3F 00 CB`: scene-2 terrain record 92, bounds
`0x4C0..0x4DF` by `0x220..0x23F`, value `0x00CB`. The low five bits publish
event 11. At New Game, scene 2 starts at fixed position `(0x3CE0, 0x2D00)`
with the movement world-mode byte clear and field resource mode 2. Ordinary
Right input moves to the doorway's horizontal lane; Up reaches contact at
world coordinate `(1231, 582)`, within the source rectangle's expanded
collision probe. The movement path publishes raw/projected `0xFFCB` and event
11.

The event is handled by the existing field-exit coordinator. Field mode 2 / event
11 selects the Ares service at `0x1FA78`, which opens the generic mode-text
prompt and waits 30 frames for a fresh controller event. Ordinary A input
returns to the Ares caller and opens the shop controller. Start closes the
shop and restores scene 2. This is distinct from scene-2 record 91 / `0xC2`,
which enters the Stoker mode-2/event-2 dialogue route.

The normal route exposed a presentation bug: the fade overlay remained at its
black endpoint after the Ares mode-text panel opened, making the entire prompt
black. The mode-text handoff now clears the Qt fade overlay after opening the
replacement panel. Source boundary
`field.mode_text_prompt.display_palette_refreshed` records that full-screen
handoff. The existing Ares transaction QA remains separate and uses its typed
test seam for menu branch coverage; this route QA does not inject an event,
mode, or prompt.

## Eight-question visual review

1. **Does it look correct?** I reviewed all 14 full-screen 320x224 Release
   captures: spawn, two eastward movement states, east waypoint, two northward
   states, doorway contact, fade 8/16/17, Ares introduction, the same prompt
   after 30 frames, Ares shop category, and field return. Movement stays on the
   scene-2 map with the field HUD. The two all-black fade frames are the final
   source fade states. The prompt and shop screens are readable and occupy the
   full expected panel; the return is a field screen with the player and HUD.
2. **Does appearance reflect behavior?** CTest checks scene 2, field mode 2,
   initial world mode 0, record 92, raw/projected `0xFFCB`, event 11, contact
   bounds, fade counter, and the mode-text/shop ownership transitions. The
   prompt has no speaker actor, uses `field.mode.entry.ares.sprite`, and
   remains open with 30 frames before input. A opens the weapons category;
   Start returns to the field.
3. **Are the visuals correct?** The native Ares introduction matches the
   independently extracted longplay frame
   [`ares-introduction-5983.png`](../visual-analysis/reference-frames/ares-introduction-5983.png)
   in logo, text well, green text, and empty lower well. The category layout
   matches [`ares-shop-5985.png`](../visual-analysis/reference-frames/ares-shop-5985.png)
   in title, tabs, offer row, and right status panel. The independent save uses
   a different runner portrait and 7,923 nuyen; the native New Game fixture
   correctly shows Joshua and 20 nuyen. The longplay frames are visual
   references, not input or collision logs.
4. **Is it animated?** The doorway fade advances through the source checkpoints:
   collision frame (timer 15 / one palette step), fade 8 (timer 8 / four
   steps), fade 16 (timer 0 / eight steps), and fade 17 (timer -1 / nine
   steps). The Ares title-only prompt is static through its 30-frame delay;
   the before/after images are byte-identical. Shop movement and transaction
   cadence are covered separately in the shop QA.
5. **Does sound accompany it?** The generic Ares title prompt has no direct
   sound request in the `0x1F246`/`0x1F3C4` path, and this route records no
   audio callback while the prompt owns the screen. No clip duration applies
   to the prompt. The existing shop audit records source `0x25` selection and
   `0x24` confirm/cancel resources and durations. Movement sound remains open
   under FIELD-01/SOUND-03; an empty Qt event log alone does not prove the ROM
   is silent during field movement.
6. **Are related parts correct?** The review includes the full field and HUD
   during approach, the complete fade, Ares title panel, category shell and
   right status panel, then the full field after Start. The return checkpoint
   requires the shop to be closed, the HUD and playfield to be present, and
   repeated full-screen renders to match.
7. **Do multiple frames support the interpretation?** The independent longplay
   review covers the Ares prompt and shop sequence across frames 5983-5994;
   the saved independent references are frames 5983 and 5985. The native route
   uses the 14 checkpoints listed above, including entry, intermediate fade,
   a 30-frame static hold, shop entry, and return. The independent capture has
   no controller or collision-event trace, so it supports screen appearance
   and order but not record 92's trigger state.
8. **Was the previous screen cleared before the new one was drawn?** Prompt
   ownership replaces the field screen after the fade; the old field fade is
   reset before paint. The shop category replaces the prompt. Start removes
   the shop, and the resulting full field screen has the proper HUD and
   playfield with no prompt or shop controls left over. The route captures
   every owning screen as a complete image.

## Reproduction and artifacts

Run `shadowrun_ghidra_conversion_qt_qa --assets assets
--field-ares-collision-shop-qa <output-dir>`. CTest name:
`shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa`.

The configured Release and Debug artifacts are under
`decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/` and
`decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/field-ares-collision-shop-qa/`.
`route.tsv` records typed state at contact, fade, prompt, shop, and return;
`source-boundaries.txt` records the coordinator handoff; `audio-events.txt`
records event requests. The BMPs are full-screen captures, not crops.
