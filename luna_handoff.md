# Luna handoff

Long-running defect review and polish for the Qt conversion in
`decompilation/ghidra_conversion/`. The game roadmap lives in the Grok plan.
This file is the queue for visual and collision bugs a player can already
hit, plus the tests that currently cannot see them.

ROM is primary. The longplay under `video analysis/` is the independent
picture. Ghidra C is a hint. Do not invent screens, collision, or menu layout.

## Rules

A previous pass reported visual fixes that only changed tests. That does not
count.

- Reproduce the bug on the Release exe
  `build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe` before
  editing. Save the full 320x224 frame.
- Name the ROM routine and the longplay frame. If no longplay frame shows
  that screen, say so. Debug/Release BMP hashes are not Genesis proof. Two
  builds can share the same wrong picture.
- Add a test that fails on the current code, then make the change, then
  re-run it. A new test that only asserts the code you just wrote is not
  evidence.
- Do not mark a `visual-defect-goals.md` row resolved from a same-source
  composite.
- If the ROM rectangle really covers the shadow, report that and stop. Do
  not shrink collision to the pretty pixels.
- Do not take Matrix, hire, save files, new maps, spell dice, `0x15830`, or
  the item-action retarget. Those are the other queue.
- Commit one job at a time. Leave these dirty paths alone:
  `decompilation/docs/visual-audits/world-activity-security-terminal-floor.md`,
  `decompilation/gameplay/world/entities/ares_weapon_emporium/entity.tsv`,
  `decompilation/tools/longplay_contact_sheets.py`,
  `grok_handoff.md`.

Each finished job needs: reproduction steps, ROM address, longplay frame or
"no frame", before/after full-screen paths, the test that failed first, and
what is still open.

## Why the tests miss this

`HILLS.md` defends hills with a CTest plus a framebuffer dump. Most dumps are
compared to themselves across Debug and Release. That catches a build skew. It
does not catch a shared wrong stop cell or a shared wrong menu.

Movement QA covers the Stoker wall and one diagonal fallback. It never walks
the runner into a table. The scene-26 priority work fixed draw order only.
`field-map-priority-composition.md` says the table draws over the actor and
the shadow stays on the base map, and it says object bounds were not proven.
That is the bug below.

## L1. Table body versus table shadow

Player walks into the table art and is stopped at the shadow.

Draw and collision are different data:

- Draw: ROM `0x3D80` priority bit, actor sprites at `0x481E`. The host paints
  map, actors, then the sparse priority overlay. Scene 26 keeps the black
  shadow on the base map and the teal table in the overlay. An actor can
  stand "inside" the table picture.
- Collision: `0xE7C8` samples terrain. Values `1..0x1F` go through `0xE9EE`
  to an object-bounds record. `0x7E` is always blocked. `0xEE10` rasterizes
  those records on a 32-unit grid. The host must stop on that record, not on
  the shadow's dark pixels and not on the priority PNG.

Do this on the scene the player used (scene-26 terminal/table is the known
report). For the contact that stops them, record:

- runner world position when the step is rejected
- the four probe corners and the terrain byte each returns
- the ROM record address, raw value, and rectangle
- which pixels of that rectangle are table body, which are shadow, which are
  floor

Then compare the host stop to a one-step ROM trace of `0xBABC` / `0xE7C8`.
Fix the host only where it rejects earlier, later, or on a different cell
than that trace. Add a test that walks into this object from the clear side
and asserts the committed position and the rejecting terrain byte. The test
must fail before the fix.

Scene 19 is the second case, already measured. Visible table pixels start at
map Y 120. Collision records 8 and 9 at `0x1D5F28` and `0x1D5F32` are `0x7E`
over `(64,128)..(127,159)` and `(160,128)..(223,159)`. Longplay section 09
frames `006990` and `007035` show the tables. Determine whether the host stop
matches those rectangles. The 8-pixel gap above Y 128 is the thing to explain,
not to paint over.

## L2. Object-bounds census

After L1's method works, walk every `1..0x1F` object-bounds record whose
rectangle overlaps a recognizable prop (tables, counters, desks, machines).
`field-map-geometry-classification.md` already counts 5,163 records. Do not
reclassify all of them. Report only rows where the host stop disagrees with
the ROM rectangle, or where the runner's on-screen feet are inside solid art
while the step is still accepted.

Output one TSV: scene, record address, rectangle, terrain byte, host stop,
ROM stop, longplay frame if one exists. Fix the host probe. Do not move ROM
rectangles onto artwork.

## L3. Four-corner probe tests

`0xBABC` / `0xBADE` / `0xB77E` probe four corners. Current movement tests do
not pin those corners.

Add cases, each with the expected rejected candidate and the position that
`0xBBE4` would keep:

- straight into a `0x7E` block from each of the four directions
- straight into one object-bounds record from each direction
- one corner blocked, the other clear (the Stoker diagonal fallback is the
  pattern; repeat it on a furniture edge)
- standing still against the block does not creep

Assert numbers from the ROM sample, not a bitmap hash.

## L4. Doorway exit retriggers

`FIELD-EXIT-01`. Entry routes exist. Leaving drops the runner on the trigger,
so the building opens again.

For Stoker, Ares, and the scene-20 door: trace the ROM return coordinate,
facing, and input latch. Reproduce leave, then held direction, then a fresh
direction. The runner must clear the trigger once and stay on the field.
Independent longplay of that exit may not exist. The ROM coordinate is the
oracle. A test that only checks "dialogue closed" is not done.

## L5. Menus, one screen at a time

Use the contact sheets in
`video analysis/longplay_review_contact_sheets/targeted_menu_evidence/`
and section 06 / section 08. For each row, capture the ordinary Release
route at 320x224, blink or hold through the source cadence, and write the
pixel difference against the named longplay frame. Fix the presenter only
where the ROM draw/clear order and the longplay agree.

| Job | Defect | What is wrong | Compare |
| --- | --- | --- | --- |
| L5a | `SHOP-01` | Ares category focus and side panel do not follow the ROM cursor sequence | Ares frames around the shop entry the longplay shows; do not restyle |
| L5b | `SHOP-02` | Unaffordable feedback leaves too fast | ROM wait at the rejected-purchase branch, full interval captured |
| L5c | `MENU-KARMA-01` | Attributes, Skills, selection color, Magic, Essence disagree with the live runner | Section 06 near 09:55 and section 08 near 14:40, Gator Shaman record |
| L5d | `MENU-POSTURE-01` | Highlight sits on the label and misses the selected attack/defense bar | Ordinary Start-menu posture route, Up/Down across all five values |
| L5e | `MENU-CLIPS-01` | Child selector misses the four buttons and leaves a black panel | Parent, Clips, reload, trade, confirm, decline, return |
| L5f | `MENU-REDRAW-01` | Defense bars and child pages draw over controls the ROM clears | Entry and return of each child. No leftover buttons, portraits, or bars |
| L5g | `MENU-CANCEL-01` | B is not mapped as cancel on every screen that has it. Start is not a substitute | Inventory of B and Start per root, child, confirm, and field menu. Test them separately |
| L5h | `MENU-BLINK-01` | Notebook lower buttons use the wrong frame, color, and black flashes | One full ROM blink cycle, consecutive frames, text must stay put |
| L5i | `HUD-CURRENCY-01` | Nuyen is missing where the ROM draws the balance | Pocket Secretary and shop, before and after a buy |
| L5j | `DIALOGUE-01-RECHECK` | `< MORE >` brackets, manager expression, and punctuation still look wrong | Section 03 Stoker and manager panels. The earlier HFlip audit is not the closer |
| L5k | `COMBAT-STATE-01` | Unarmed and Spurs lack the attack pose. A sold gun shows `00`, which is not the swing | Equip, sell, unequip, then attack, full frames |

`INVENTORY-01` Trade stays open until a second runner exists. Do not fake one.

## L6. Runner under the HUD

Scene-2 event 1 draws the runner under the opaque HUD column. Section 03
shows the street and the HUD together. Find a frame where the runner passes
that column and compare feet, portrait, and ammo. Fix clip or punch only to
match the ROM layer order. Do not move the HUD to a new place.

## L7. Tests that can fail

For L1 through L6, the new tests are part of the job, not a follow-up.

- Position and terrain-byte asserts, not "BMP exists".
- A menu test names the ROM cursor coordinate and the string id. A full-frame
  hash against a previous dump of ourselves is extra, not the assert.
- Keep one Debug/Release hash check so a build skew still fails. Do not treat
  it as the behavior check.
- Prefer extending `shadowrun_ghidra_conversion_qt_field_movement_qa` and the
  existing menu QA executables. Do not add a new harness.

## L8. Timing rechecks, after the pictures are honest

These routes already run. The complaint is cadence.

- `OPEN-SKIP-TIMING-01`. Start-skip into the Stoker auto-walk. Compare the
  black hold and the per-update step to consecutive section 03 frames. Do
  not change the speed to taste.
- `TITLE-LOOP-01`. Run the ordinary window with no input through the idle
  restart. The internal 3,601-update test is not a substitute for that window.
- `AUDIO-RECHECK-02`. Inventory intro-stage sound requests and stops. One
  cue per stage boundary, loopback duration written down. No new music.

## Done when

L1 has a failing-then-passing position test and a full-frame capture where
the feet stop on the ROM cell. L5 has one reviewed screen with a longplay
diff, not a batch of menu tweaks in one commit. The rest can sit in the
queue.

## Progress log

### L1 Scene 26 table collision: source stop condition reached

Release reproduction follows the ordinary Scene 2 event-1 route into Scene
26, then captures the first blocked Down step from a visible clear approach.
The trace and full-screen frames are documented in
`decompilation/docs/visual-audits/scene26-table-collision.md`. No independent
longplay frame of this table was found. ROM record 70 at `0x1D9C2A` is
`(96..191, 224..255, 0x7E)`; its lower edge overlaps the visible shadow. The
host and ROM terrain sampler both classify the candidate as blocked. Per the
stop rule above, collision was not changed. The QA now asserts the sampled
coordinates, bytes, candidate, and retained position; it passed before and
after adding those assertions, so this is a source-confirmed report, not a
failing-then-passing behavior fix. The broader Scene 19 and object census
remain open.
