# Jump House entry and Gunderson conversation

Status: partial. The scene-2 entry description is connected to the live field
route and has ordinary-input QA. Scene-13 event 30 opens the Lua Johnson
interaction. The baseline question chain and source-gated Vigore/Errascoe
branches are connected and captured through the shared dialogue presenter.
The live event-30 movement trigger, contact/work transactions, and scene-13
exit remain open.

ROM values and control flow below are primary. Mike's map supplies a
human-readable location label; section-03 longplay frames supply independent
appearance and sequence evidence. Host screenshots are regression captures,
not Genesis goldens.

## Source path

- Scene-2 record 85 at `0x1730FC` is raw `0x00C4`, event 4. Its event-table
  byte at `0x172CE0` is `0x0D` (numeric scene 13). The dispatcher classifies
  this as the conditional maglock family with the table route as its fallback;
  it is not a direct building-name record. The site map's marker 1 labels Jump
  House and Gunderson.
- Indexed title service 22 branch `0x20484..0x204C6` checks event 4. When
  `FFF0D5` is clear, it selects profile `0x121EBC` and descriptor `0x110D1E`,
  whose first text record is `0x116C12`. This entry page has a distinct
  56x72 portrait. It has no A/B/C choices.
- On ordinary input through the live Qt route, record 85 opens the Lua-owned
  `building.present_entry_description` task. Dismissing the page resumes the
  retained typed scene-13 destination. The route QA starts at `(0x1D00,
  0x0A00)`, one collision row south of record 85, and enters it with Up.
- Scene-13 records 11/12 (`0x1B3CF8`, `0x1B3D02`) are event 30 at adjacent
  cells `(0,1)` and `(1,1)`. The source continuation is `0xB7B8 -> 0x20990`;
  its scene-13 branch selects mode row 0 from `0x12CC50`, profile `0x121E80`,
  and descriptor `0x105114`.
- Profile `0x121E80` decodes to a 56x72 image matching the Gunderson portrait
  in frame 001500. The root title is `0x106266`; labels are `0x104051`
  (information), `0x103A3A` (work), and `0x104FEB` (leave). The source
  information page is `0x1063D9`, followed by descriptor `0x10517C` and title
  `0x1064AA`. Its choices point to question/contact/other-subject branches.
- The initial information branch's “different subject” choice does not
  redraw the initial greeting. Descriptor `0x10514C` uses alternate title
  `0x106396` with the same root labels; its information choice leads to
  descriptor `0x105198`, title `0x106533`. Both alternate strings are now
  imported and the Lua interaction retains this post-information state.
- Information descriptor `0x10517C` routes Questions to dynamic builder
  `0x22F90`. The builder always offers `0x104B81` (“What exactly is a Mr.
  Johnson?”) under title `0x1069D5` (“Hurry up and ask, then.”). It adds the
  Vigore option `0x1045B0` when `FFFC1E` bit 26 is set, adds a different
  response for the same label when bit 26 is clear and `FFFC27` bit 3 is set,
  and adds the Errascoe question `0x103D24` when `FFFC1E` bit 1 is set.
- The base question selects descriptor `0x105218`: answer `0x10662B` plus
  `<MORE>`, which leads to `0x10522C`. That follow-up menu shows title
  `0x106701` and choices `0x1047F4` (another question) and `0x104817`
  (different subject). The first loops through `0x22F90`; the second returns
  through alternate root descriptor `0x10514C`.
- With world flag bit 26 set, the Vigore choice dispatches action `0xE7` to
  `0x249A4`. The ROM clears `FFFC34` bits 15 and 21, sets bit 13, and returns
  descriptor `0x105270` with response text `0x106CD2`. Lua sets
  `game.state.vigore_search_pending` through the typed host fact and presents
  that response/follow-up menu. The bit-1 Errascoe path returns descriptor
  `0x105288` with text `0x106929`; it is also represented in Lua. The
  `FFFC27` bit-3 alternate Vigore response and descriptors `0x105244` /
  `0x105258` remain unimplemented.
- The contact path includes text at `0x1069ED` and a Decker contact offer at
  `0x106AA8`. The visible work contract also uses separate ROM data around
  `0xF2196`, `0xF21DD`, and `0xF23B2..0xF241F`; salary text at
  `0x1060E0`/`0x106171` is dynamically formatted by the source. Contact
  pricing, wallet changes, acceptance, and quest effects remain incomplete.
- Scene-13 records 9/10 (`0x1B3CE4`, `0x1B3CEE`) are event 0 at cells `(2,7)`
  and `(3,7)`. Their exit/action meaning and destination have not been
  established; no return route is inferred from the known entry map.

## Independent frame observations

Reviewed section-03 captures at frames 001050, 001200, 001350, 001500, 001650,
001800, 001950, 002000, 002100, 002150, 002200, 002250, 002400, 002550,
002700, 002850, 003000, 003150, 003300, and 003450. Key comparisons:

- Frame 001200 shows the entry text and its portrait but no choices. The new
  Qt `entry-description.bmp` visually matches this entire 320x224 composition.
- Frame 001350 shows the scene-13 map after the entry page. The new route's
  final field frame has a clean map/HUD and no old dialogue controls; the
  runner placement differs slightly from frame 001350. Source spawn and
  transition-position behavior still need tracing before claiming exact
  positional fidelity.
- Frame 001500 shows Gunderson's portrait, greeting, and three A/B/C rows.
  The Qt full-screen root capture matches the sampled layout and labels.
- Frames 001650/001800 show the static information page and `< MORE >`;
  frame 001950 shows its separate topic menu. Frame 002100 shows the
  “Hurry up and ask” selector; 002200/002250 show the Johnson answer and
  `< MORE >`; 002550/002700 show its two-choice follow-up; and 002850 shows
  the alternate root. The Qt full-screen captures match these samples in
  panel placement, portrait, text, and option labels. Each replaces the prior
  page cleanly.
- Frames 003000/003150 show the contract text; 003300 shows the generated
  salary offer; 003450 shows the acceptance result. Those work and offer
  screens remain absent from the native flow.
- Across sampled modal frames the old map/HUD/buttons are replaced rather than
  layered under the new page. The route QA captures full 320x224 frames at
  collision, entry page, transition start/midpoint, and settled scene.

The selected captures establish sampled appearance and order, not every
30-fps frame, Genesis VBlank cadence, controller edges, or sound duration.

## Seven-question verification record

1. **Does it look correct?** The entry portrait is present and matches the
   ROM-decoded `0x121EBC` asset in the reference frame. The distinct
   `0x121E80` portrait matches Gunderson in frame 001500. Qt full-frame
   captures were reopened and compared visually with the corresponding
   longplay samples.
2. **Does the look match behavior?** Ordinary Up input in route QA contacts
   scene-2 record 85/event 4, opens the entry page, and A resumes scene 13.
   Scene-13 event 30 is wired to the Johnson script but still needs its own
   ordinary-movement end-to-end QA. Lua reads the typed Vigore/Errascoe facts;
   selecting the eligible Vigore question sets the pending search fact.
3. **Are the visuals correct?** The entry text, portrait, panel, and absence of
   choices match frame 001200. The root, information, topic, question selector,
   answer, follow-up, and alternate-root captures match their selected
   samples. Question frames 002100, 002200, 002550, and 002850 were compared
   directly against the numbered longplay. Debug and Release captures for
   fourteen states match byte-for-byte; that establishes host repeatability,
   while the compressed longplay remains the independent visual reference.
   The contract, salary, and acceptance screens remain absent from the native
   flow.
4. **Should anything animate?** The sampled dialogue panels and portraits are
   static. The native entry-to-map handoff currently uses the host's 60-frame
   modal-return fade; this is transition/input handling, not a ROM timing claim.
   Exact source animation cadence and fade duration remain unverified.
5. **Is there sound?** Entry/dialogue-specific sound requests and lifetimes
   have not been traced. Do not call these pages silent or audio-correct yet.
6. **Are surrounding visuals and stale layers correct?** Full-screen QA shows
   a black transition start, the new scene-13 map during the fade, then a clean
   settled map/HUD. No entry text or dialogue buttons remain underneath. The
   runner placement difference at frame 001350 is open.
7. **Are multiple frames available?** Yes: the numbered longplay contains
   samples through the entry, interior, root, information, question, work,
   offer, and acceptance stages. This review uses selected frames and does
   not claim every frame was individually inspected.

## Current QA and remaining work

- Debug and Release CTest
  `shadowrun_ghidra_conversion_qt_field_jump_house_entry_qa` exercise the
  collision and retained scene route. Captures are under
  `build/conversion/qt/<config>/qt_runtime/field-jump-house-entry-qa/`.
- `shadowrun_ghidra_field_dialogue_runtime` exercises the entry, root,
  information, topic menu, question chain, conditional Vigore/Errascoe pages,
  and post-information root/menu through the shared Lua presenter. Debug and
  Release runs each capture fourteen full 320x224 screens under
  `qt_runtime/jump-house-visual-audit/`.
- Still required: ordinary-input QA for scene-13 event 30; the FFFC27 bit-3
  alternate response; contact/work flows and their state/wallet effects;
  event-0 exit trace; sound and duration trace; source actor placement and
  transition timing trace.
