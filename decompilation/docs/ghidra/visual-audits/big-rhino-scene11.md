# The Big Rhino (field scene 11)

Status: partial. Both event-30 cells now route to Mortimer Reed's shared Lua
Johnson interaction. The work choice opens the ROM's context-2 offer table.
The information branch now follows the ROM's question gates and flag writes,
and its $1,500 Alesandro contact uses a typed wallet transaction. Context-2
non-Ghoul run settlement remains open; the independent first-entry/question
comparison is also open.

## ROM trace

- The Scene-3 Penumbra event at `0x18ABAA` is event 5. Its route-table fallback
  selects field scene 11; the Penumbra guide's marker 6 and ROM contract
  location option 5 name the Big Rhino.
- Scene 11 event records 10 and 11 (`0x1D5B0C` and `0x1D5B16`) are adjacent
  event-30 cells at map pixels `(288,32)` and `(256,32)`. Their combined
  rectangle overlays the dark booth alcove.
- ROM `0xB7B8` sends event 30 to `0x20990`. The Scene-11 branch sets dialogue
  slot 2; the mode table at `0x12CC50` resolves portrait profile `0x121FF4`
  (profile index 7) and opening descriptor `0x105544`.
- Descriptor `0x105544` begins at text `0x107D56`, uses the shared `<MORE>`
  continuation, then links to `0x105558`. That menu title is `0x107DE0`; its
  three source labels are `0x104051` (information), `0x103A3A` (work), and
  `0x104FEB` (leave).
- The work choice's command `0x0D` selects the context-2 callback `0x215DA`.
  With no active run, it displays text `0x108501` before selecting one of the
  eight offer records at `0xF3EBC`. The Lua location profile now uses the
  existing ROM-derived Big Rhino offer table and the extracted work-opening
  text. Its eight run rows remain shared data; they are not rebuilt in the
  building script.
- The dispatcher at `0x20AFC` selects conditional descriptor `0x10596C` when
  `FFFBEC` is negative, `FFFC1C` bit 7 is set, and the low byte of `FFFBEC`
  matches the active dialogue slot. That descriptor begins at `0x105B7A` with
  Mortimer saying he owes Joshua credits, then presents the same information,
  work, and leave labels. The flag's story meaning and the selected choices'
  later effects remain unresolved. Text records at `0x1154AE`, `0x11555C`, `0x1155B5`, and
  `0x1155E1` describe the restaurant and package-related interactions, but
  have not been linked to these event-30 cells; the viewer labels them related
  source text.
- The extracted ROM default actor list for scene 11 is empty. Mortimer's
  portrait belongs to the dialogue profile and does not establish a standing
  map actor.

## Visual verification questions

1. **Does it look correct?** The map annotations follow the composed map art:
   the service counter, dark booth, potted plants, dining tables, and green wall
   panels have separate outlines. The outlines do not replace or recolor the
   source map image.
2. **Does the look reflect game state?** The two booth event cells link to the
   Mortimer Reed conversation profile, including its ROM-gated alternate
   opening. The default actor list remains empty; the portrait is not shown as
   a map character. Event-8 and event-9 markers remain unresolved instead of
   receiving invented destinations.
3. **Are the visuals correct?** The source map composite and priority plane
   were inspected separately. Furniture bodies, foreground lamp pixels,
   plant pots, and plant foliage use separate parts where the source planes
   distinguish them. The `0x121FF4` portrait was decoded at 56x72. I inspected
   the active Qt contact-offer and paid-result captures beside the independent
   longplay frames at original resolution; portrait placement, panel bounds,
   text wrapping, and option positions agree. Independent captures for the
   first-entry and information-question pages remain open.
4. **Is anything animated?** The map fixtures are static. The source actor list
   is empty, and the event-30 dialogue dispatch does not add a standing actor.
   No new animation cadence was inferred for the scenery. A complete trace of
   animation during the dialogue branch remains open.
5. **Is there sound?** `0x20990` clears `FFF10C` before entering the shared
   prompt path. No direct sound request is identified in this scene branch;
   actual playback and duration have not been measured.
6. **Are the other screen elements clean?** The viewer still presents the
   composed map, priority overlay, route bounds, map profile, and audio profile
   as separate evidence layers. Entity previews are selection-driven; this
   slice adds no gameplay buttons or overlays over existing controls.
7. **Do multiple frames support the interpretation?** The inspected contact
   offer is visible in
   `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_23_source_5220-5520/frame_000000.png`.
   `frame_000150.png` shows the purchase result with Alesandro Hobbs in the
   Pocket Secretary. Both are independent game captures; they corroborate the
   portrait, offer copy, panel layout, option placement, and purchase result.
   Booth-entry and question pages still need matching independent captures.

The information path from `0x105558` now plays `0x107FA9`, opens the category
page at `0x108049`, and builds Julius, Mako, and Aragorn questions from the
ROM gates in `0x2317A`. Mako's `0x23222` action sets FFFC30 bit 21; Aragorn's
`0x2323A` action clears bit 11 and sets bit 12. Contact purchase command
`0x50` at `0x221BA` charges 1,500 nuyen and sets FFFC2C bit 1. Fresh New Game
already seeds that bit in `0x1C7F`, so the write does not prove a unique
contact acquisition for Alesandro Hobbs. The shared Lua interaction and host
wallet transaction are covered by Debug/Release script and full-screen
dialogue tests, with `actual.bmp` captures under
`qt_runtime/big-rhino-mortimer-visual-audit/`. A new Debug/Release
physical-input CTest walks ordinary Up into event 30 and captures the first
page and three-choice root under `qt_runtime/field-big-rhino-entry-qa/`; the
Debug and Release captures match by SHA-256 and were inspected. No separate
Big Rhino menu implementation is used. Context-2 non-Ghoul run settlement and
independent first-entry/question visuals remain open.
