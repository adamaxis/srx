# Shadowrun Genesis native reconstruction: exploratory system analysis

This document is the source map for the native C++ reconstruction. It exists
to prevent the port from becoming a collection of plausible-looking screens
whose behavior was invented independently of the ROM.

The authoritative artifact is the raw US ROM at
`C:\Github\shadowrun\Shadow Run (U) [!].bin`:

```text
size:   0x200000 bytes
SHA256: 92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E
CPU:    Motorola 68000
RAM:    0xFF0000..0xFFFFFF
VDP:    0xC00000 data, 0xC00004 control
```

The C++ port is a native translation. It does not run a 68000 interpreter,
retain Genesis VRAM as its runtime renderer, or use a palette-indexed host
surface. ROM graphics, palettes, text, tables, and sound command metadata are
decoded into ordinary native data. The source addresses below identify the
behavior being translated; they are not permission to substitute a generic UI
operation for an untraced handler.

## Evidence rules

Every entry in the implementation must be classified:

| Mark | Meaning |
| --- | --- |
| `CONFIRMED` | Direct instructions, tables, or a source record establish the behavior. |
| `CORRELATED` | The ROM path and a captured/reference frame agree, but one data meaning still needs a byte-level check. |
| `PENDING` | The entry point is known, but its complete input, drawing, side effects, or exit path is not reconstructed. |
| `REJECTED` | A previous native behavior contradicted the source and must not be reintroduced. |

The minimum trace for a screen or mechanism is:

1. entry routine and caller;
2. initialization writes and source record selected;
3. every cursor/focus field and its bounds;
4. input edge/held behavior;
5. draw/resource calls and their destination geometry;
6. sound/music commands;
7. animation state, cadence, and sprite-list/table source;
8. mutation and side effects;
9. exit and re-entry state.

If one of those is unknown, the native status remains `PENDING` even if a
placeholder panel is visible.

## 1. Execution model

### Reset and frame cadence

`0x0236` is the reset entry and the initial stack pointer is `0x00FFFFF8`.
The reset path initializes the Genesis ports, region selection, VDP registers,
RAM, palette buffers, and controller handshake before entering the game state
flow. The source repeatedly samples input, performs a screen or field update,
sets VBlank work flags, and lets the VBlank service drain graphics/audio work.

Important shared routines:

| ROM address | Role |
| ---: | --- |
| `0x0CEC` | VDP setup table and initial register configuration |
| `0x0BD2`, `0x0BF2` | frame wait/input-side synchronization used by screen loops |
| `0x1342` | counted frame/input wait used by fades and timed screens |
| `0x1872` | VBlank service entry |
| `0x195C` | controller sampling and edge-word formation |
| `0x19D2` | controller decode table |
| `0x1088`, `0x10D6` | palette interpolation/fade operations |
| `0x1232` | ordinary VDP grid/resource stream |
| `0x1728` | delta/tilemap stream writer |
| `0x1530` | packed resource stream decoder |
| `0x1378`, `0x13BE` | sprite/object list record emitters |
| `0x530DE` | CPU-to-Z80 sound command submission |
| `0x530EA` | sound/music command gate used by timed presentation paths |

The native loop retains this conceptual order:

```text
sample input edge
  -> update the active state machine
  -> update native world/record state
  -> update animation timers and selected frame
  -> submit RGBA/tile/sprite-equivalent screen writes
  -> present
```

The order matters. A menu cursor is not just a rendered arrow; the cursor
state controls which source table entry an A action reads. A field doorway is
not just a colored obstacle; its raw event byte is consumed by the area
reaction path.

### Native input boundary

`FFDD22` contains the sampled/edge controller word after the source sampler.
The native boundary is `InputState` with separate `held` and `pressed` bits.
All screen loops must consume edge-triggered actions where the source tests an
edge, and held/repeated actions where the source loops on a held direction.
The native frontend may map Windows, SDL, or another device into this
boundary; it must not change the source meaning of A/B/C/Start or directions.

## 2. Master state graph

The source flow is stateful and has multiple independent loops:

```text
Reset/region
    |
    v
Pre-title (0x5D746)  SAT guy rises $01A8->$0160, then SEGA SAT from 0x5DA20/0x5DA38
    |
    v
Title (0x1A9E)
    | Start edge
    v
Start options (0x59092)
    | New Game                  | Load Game
    v                           v
Archetype (0x59248)       Save/load slots (0x59172)
    | confirm                     | valid slot / empty slot
    v                             |             |
Timed opening (0x59C5E) <--------+             +-- stay on slot screen
    |
    v
Field runtime loop (0x075E; scene descriptor load 0x3074 -> 0x3084)
    | Start
    v
Command overview (0x10D30 / 0x10F50)
    | Inventory / Clips / Attributes / Cyberware / Magic / Pocket Secretary
    |
    +--> item-use/equipment handlers, submenus, or source-specific loops
    |
    +--> Start/B return with selected record, cursor, and persistent mutations

Field raw event + reaction state
    |
    +--> area mode transition, building interior, dialog/story state, combat,
         shop, or another source state selected by the mode/event tables
```

There are additional indirect state families. `0x1F580` indexes the reviewed
dispatch table at `0x12CCB4` for a state byte, and `0x214B0`/`0x214CE` use
the state-pair table at `0xF3E1E`. These are the story, interaction, and
special-state paths that must be followed from their callers rather than
flattened into a single `Field` or `Dialog` enum.

## 3. Live record model

The game does not store a runner as a name plus a few host statistics. The
live record is a 0x100-byte source structure copied between ROM templates,
SRAM, and RAM. The native model must preserve source bytes until their meaning
is confirmed.

The following fields are directly consumed by audited paths:

| Record field | Source meaning | Consumers |
| ---: | --- | --- |
| `+0x10` | active/class/record flags | roster eligibility, field entity dispatch |
| `+0x2C` | source position/state word used by entity setup | field initialization and animation setup |
| `+0x3E` | record type/selection boundary | `0x391A`, party selection |
| `+0x4D` | five-state posture, clamped `0..4` | `0x11AE4`, `0x12CEE`, later AI/combat |
| `+0x4E` | current ammunition/clip byte | clips and equipment conversion |
| `+0x4F` | clip count | clips loop and item use |
| `+0x56` | equipped inventory slot; `0x7F` means no weapon | equipment, overview, attack calculation |
| `+0x57` | equipped weapon/gear item code | equipment, overview, attack calculation |
| `+0x66..+0x6D` | eight possession item codes | inventory cursor/action dispatch |
| `+0x6E..+0x75` | eight item phase/flag bytes | armor/equipment/item detail paths |
| `+0x76` | equipped armor ordinal | armor action and overview |
| `+0x77..+0x85`, `+0x87..+0x8C` | attributes and skills | overview, bars, attributes screen |
| `+0x90` | fifteen-bit cyberware field | cyberware screen and calculations |
| `+0x9A..+0x9D` | source race/class/auxiliary bytes | overview, magic, item/status rendering |

The critical invariant is:

```text
possession (+0x66..+0x6D) != equipment (+0x56/+0x57/+0x76)
```

Finding an item in an inventory cell never equips it. The source equipment
handler must be executed, and the live equipped fields must change. Any native
code that derives `active_item_code` from “item exists in inventory” is
`REJECTED`.

The first three ROM records at `0x7291A`, `0x72A1A`, and `0x72B1A` are Joshua
templates for Samurai, Decker, and Gator Shaman. `0x42B0` copies exactly one
template to Joshua's live record at `0xFF0100`. These are archetype choices, not
three runners available to the C switch action. Recruited records begin at
`0x72C1A` and require separate roster eligibility/state reconstruction.

## 4. Resource and rendering model

### Text and tiles

`0xC0DA` writes source text through a tile/name-table destination. The native
decoder must preserve source tile placement when matching a reference screen;
wrapping a string into a host paragraph changes the screen and can change the
cursor/action geometry.

The primary native extraction paths are:

| Source | Native interpretation |
| ---: | --- |
| `0xC1320` | character/name table used by source text writers |
| `0x1FF0` | title/startup four-line CRAM palette block |
| `0x1F310E`, `0x67CB0`, `0x6875C` | startup/title packed pattern streams |
| `0x67DF6` | startup/title delta tilemap stream |
| `0xD4540`, stride `0x30`, 50 records | item catalog and item art/detail fields |
| `0xD2072`, 15 pointers | cyberware names |
| `0xC1A2A`, 20 pointers | attribute/skill names |
| `0xD1D28`, `0xD1D3A` | command labels |
| `0xC1418` | Pocket Secretary menu text set |

Palettes are source data, not a runtime restriction. The native renderer uses
RGBA pixels, but the index-to-color conversion must happen from the correct
source palette block. A wrong palette address is a data bug, not something to
hide with a host color approximation.

### Sprite/object output

The source builds sprite records in RAM and sends them through VBlank. `0x1378`
and `0x13BE` emit the SAT-equivalent records; fields include position,
size/link, tile attributes, palette/priority bits, and ordinal/link state. The
native renderer may draw RGBA frames directly, but it must preserve:

- the source object list order and priority;
- transparent color zero;
- tile flips and palette-line selection;
- source size/link layout;
- per-state frame timing;
- object visibility bounds and camera-relative projection.

### VBlank graphics

The native port does not reproduce VRAM writes. It does reproduce their
observable effect. The source VBlank paths at `0x6148`, `0x61F4`, `0x0EE6`,
and the map upload paths at `0x2C92`/`0x2CB2` are the boundaries for queued
tile/object updates. A host frame must not update only the background while
omitting the sprite/object list that the same source frame emits.

## 5. Title screen: exact source path and current defect register

### Source sequence

The title entry is `0x1A9E`. It initializes these source variables before its
loop:

| RAM field | Initial value | Role |
| ---: | ---: | --- |
| `FFDD2C` | `0x5A` | initial reveal delay |
| `FFDD3C` | `0` | sprite phase cadence counter |
| `FFDD3E` | `0` | six-entry title sprite-list index |
| `FFDD40` | `0` | logo/audio cadence counter |
| `FFDD42` | `0x98` | SAT X origin before `0x1C6A` adds four |
| `FFDD5A` | `0` | logo/text completion state |
| `FFDD5C` | `0x5A` | delayed copyright/text timer |
| `FFF1E2` | `0x2090` | logo column source pointer |
| `FFF1E6` | `0` | revealed logo-column index |
| `D7` | `0xE10` | title loop countdown |

`0x1D7A` performs title resource setup. It calls `0x1406`, installs the
startup/title resources, initializes the title palette and screen buffers, and
sets the title-specific source pointers. The title circuit/background path
uses the pattern streams and tilemap data above. It is not equivalent to a
single static bitmap.

Each title-loop pass performs input/frame synchronization and then:

1. waits through the `0x5A`-pass delay;
2. calls `0x1E4E` on its source cadence to emit the next logo column;
3. calls `0x1C6A` while the reveal is active;
4. advances one of six sprite lists from pointer table `0x1CE4`;
5. moves the sprite origin by four pixels per `0x1C6A` call;
6. writes PRESS START from `0x1DCA` or the blank companion from `0x1DE0`
   according to `D7 & 0x3F`;
7. writes the four copyright rows through `0x1DF6` after the completion timer;
8. exits only after the source Start edge sets the title exit state.

`0x1C6A` advances the six sprite-list phase entries at a three-frame counter
cadence. The source records are terminated by `0x8000` and are expanded by
`0x1378`; they are not six arbitrary full-screen animation frames.

The `0x1A9E` branch calls `0x1C6A` on every active reveal pass, including
passes between logo-column writes. It continues while `FFF1E6 <= 0x25`; the
last `0x1E4E` column write advances that index to `0x26`, and the following
loop pass takes `0x0E34` instead of rebuilding the flame projection. The Qt
conversion state now carries that projection lifetime explicitly, so the last
sprite record is not painted after the reveal branch stops.

`0x1E4E` reads successive columns from `0x2090`, writes the title plane at the
source name-table destination, increments `FFF1E6`, and on its sound cadence
submits command `0x5D` through `0x530DE`. When the logo completes, `0x1C1E`
advances the delayed text state, submits the `0x530EA` completion command, and
calls `0x1DF6` for the copyright block.

The text resources are written at these source locations:

| Content | ROM data | Source destination |
| --- | ---: | ---: |
| PRESS START | `0x1F81` | `0xCA84` |
| blank PRESS START row | `0x1F99` | `0xCA84` |
| copyright row 1 | `0x1F00` | `0xCB04` |
| copyright row 2 | `0x1F24` | `0xCB84` |
| copyright row 3 | `0x1F48` | `0xCC04` |
| copyright row 4 | `0x1F68` | `0xCC84` |

### Confirmed native defect

The title circuit plane and the title palette transition are separate source
operations. The current host decoder uses the 64-word dark circuit-plane
palette beginning at `0x1C024` (`0x1C064` is its third line), which matches the
settled reference background; `0x1FF0` is the
64-word target palette consumed by the source transition helper at `0x10D6`,
not a horizontal-scroll table. The native host currently expands the settled
plane to RGBA at extraction time, so the exact per-pass palette interpolation
still needs to be represented as a source-indexed layer. The background writer
also emits the source `0x4000` VDP attribute; its host priority interpretation
remains `PENDING` until a phase-matched capture is compared.

### Remaining title work

The title is not considered complete yet. The next comparison must capture the
same source loop phases and compare:

- first frame before the reveal delay;
- each one-pass logo reveal step after the initial delay;
- each four-call sprite-list phase;
- SAT X translation and clipping;
- source plane/sprite priority;
- PRESS START blink phase;
- copyright timing;
- the settled screen after the source completion command.

The six native sprite images must be inspected as source SAT compositions,
not judged from a single settled screenshot. Any mismatch is recorded with the
source phase, pointer-list entry, tile index, palette line, and destination
coordinates.

## 6. Startup and opening presentation

### Start Options: `0x59092`

`0x59092` writes the source layout from `0x1B6498`, initializes selector byte
`FFF0DC`, and calls `0xC376` with bounds `0..1`. The choices are exactly:

```text
New Game
Load Game
```

An A edge dispatches the selected choice. New Game clears the load state and
enters `0x59248`; Load Game enters `0x59172`. An empty load slot is not New
Game.

### Save/load: `0x59172`

The source writes the two-slot layout from `0x1B6518`, clamps the selector to
two positions, and checks SRAM only after the selected slot action. The valid
path sets `FFE108`, restores the copied state block, and returns to the game
without the new-game opening. The invalid path writes the no-save message at
`0x1B6527` and remains on the screen. The success text is at `0x1B6577`.

The save data includes the source record fields copied by `0x597EE`/`0x598B0`,
field/mode words, party records, and a checksum block. Native SRAM must use
that source structure; a host-only “slot exists” boolean is not valid.

The converted field bootstrap now preserves the successful-load continuation:
after `0x594E0` restores the saved scene/camera/live-runner records, it runs
the source `0x028C2` re-entry before the 31-frame field warm-up. The current Qt
composition accepts this only when the saved scene is mode 2, whose direct
terrain and scene records are available. A different saved scene reports a
missing source-resource boundary until its own layer/stream/descriptor records
are extracted; it is never silently shown with mode-2 terrain.

### Archetype: `0x59248` and `0x42B0`

The selector bound is `0..2`. The visible choices are:

```text
Samurai
Decker
Gator
Shaman
```

`Gator` and `Shaman` are two rows of one choice. The selected index is copied
from the template table at `0x7291A` with a `0x100` stride and written to
Joshua's live record by `0x42B0`. There is no command-screen archetype switch.

### Opening: `0x59C5E`

The opening is a sequence of timed presentation groups, not a text-only menu:

| Stage | Source path | Sound command observed |
| ---: | ---: | ---: |
| 1 | `0x5AF62` | `0x10` through `0x530EA` |
| 2 | `0x59D7C` | `0x11` through `0x530EA` |
| 3 | `0x5AA96` | `0x12` through `0x530EA` |
| 4 | `0x5AD1A` | presentation continuation |
| 5 | `0x59D12`, `0x134D2` | opening-to-field transition |

The source uses VBlank flags, palette fades, sprite/object lists, map/tile
uploads, camera movement, and controller abort checks inside these stages. The
native text timing is only one recovered layer. Animated artwork, sprite
choreography, and music remain `PENDING` until each stage is traced and
compared.

## 7. Field, movement, animation, and transitions

### Area initialization

`0x3074` selects the active field descriptor from the pointer table at `0x3232`
using mode `FFDDC4`, copies the descriptor into `FFDDC6`, initializes the map
and camera words, and copies the two 0xE0F-word map/name-table buffers. There
are 56 mode descriptors. Mode 2 is the reviewed Redmond Barrens startup area;
indoor modes occupy the small mode range used by the source building paths.

The reviewed mode-2 field resources include:

- raw pattern data at `0x166B82`;
- compressed name/tile stamps at `0x170102`;
- map streams at `0x16D9A2` and `0x16ED52`;
- source entity/object records and event/reaction bytes in the mode descriptor.

Native extraction converts these to RGBA and typed terrain data, but the source
mode/event descriptors remain authoritative for collisions and transitions.

### Movement and collision

The main field loop at `0x075E` updates the field state and calls the
movement/reaction chain. The `0x03074 -> 0x03084 -> 0x0313A` path preserves
the scene id while loading the selected row from the complete `0x3232` profile
table and then copies that row's terrain layers before the loop runs.
`0x3032` only classifies a subset of scene ids for `FFFC0E`; it does not
replace the profile-table index. The
relevant pieces are:

| Source | Responsibility |
| ---: | --- |
| `0x2B34` | field record/camera and startup record setup |
| `0xBABC` | directional movement and movement-state selection |
| `0xE7C8` | four-corner terrain/entity footprint probe |
| `0xB7B8` | raw terrain/event reaction pass |
| `0x5A62` | fixed-point world-to-screen projection |
| `0x5FDC` | timer/animation update used by field records |
| `0x600E`/`0x60FA` | object state, packed sprite, and VBlank queue path |
| `0x391A` | controlled-runner selection on C |

The source probes the footprint, chooses the highest relevant raw status, and
then dispatches the event/reaction state. A blocked movement can still carry a
raw interaction byte. Replacing that with a center-cell collision check changes
doorway behavior and is `REJECTED`.

The controller and step path are now pinned down rather than approximated:

- `0x195C` maps the normalized D-pad nibble through `0x19D2` to one of eight
  16-bit target angles; opposing/invalid combinations return `0xFFFF`.
- `0xB530` slews record `+0x2E` toward `+0x34` in `0x0800` increments, with
  the source's unsigned wrap comparisons.
- `0xBABC`/`0xB77E` convert the current angle to an ordinal and add the signed
  fixed-point vector pair at `0xC002` before calling `0xB48C`/`0xE7C8`.
- `0xBBE4` commits the accepted D5/D6 position; a rejected step leaves the
  record coordinates unchanged while the raw terrain result remains available
  to the event path.

The native field loop now follows those angle, vector, and fixed-point rules.
The descriptor camera remains fixed during ordinary movement because the only
reviewed camera writes are the descriptor/transition paths at `0x2B34` and
`0x3074`; the right-hand HUD/entity panel and transition-specific camera writes
are still separate work items. This is a source-derived movement slice, not a
claim that field presentation is complete.

### Walking animation

Ordinary accepted field movement uses the eight direction selectors at
`0xC062`, which point to four-frame positive-marker loops (`0x20..0x3C`). The
blocked/stopped reset at `0xB442` selects the static direction poses at
`0xC052` (`0x00..0x1C`). The distinct `0xC0B2` selectors (`0xE8..0x104`) are
finite eight-descriptor behavior transitions installed by `0xB330` and the
immediate branch of `0xAFF2`; using those as a held-D-pad walk cycle leaves the
actor on the transition's final pose. `0x600E` loops the positive `0xC062`
list and calculates +0x18 from the live +0xCA timing word. For the kind-3
runner, `0xBABC` supplies +0xCA=`0x28`, so +0x18=`0x280` against the `0x1000`
period at +0x1C, advancing at six- or seven-update intervals according to the
accumulator phase. Selector
changes through `0xB6DE` do not clear the frame or accumulator; `0x600E`
resolves the frame bound on the next render pass.

Other motion/action families remain distinct around `0xC042`, `0xC072`,
`0xC082`, `0xC092`, `0xC0A2`, `0x15318`, `0x15336`, and the entity
state/phase tables at `0x6AB3E`, `0x6AB96`, `0x6ABEE`, and `0x6AC46`. The field
animation path also consults `0x13E58`, `0x13E7C`, and the timing tables at
`0xD4540`/`0xD4EA0` for object-specific cadence.

The Qt presenter now consumes separately named C062 movement frames and
typed selector/timer state. The C0B2 transition assets remain a separate
family and are not selected for ordinary D-pad movement.

### Area/building transitions

The source transition is a composition of:

```text
current mode
  + raw terrain/event byte
  + reaction substate/status bits
  + mode reaction table
  + destination descriptor
  -> next mode/state handler
```

For the reviewed Redmond Barrens startup map, the C0 motel doorway event
selects mode `0x14`, Stoker's Coffin Motel. Ares Weapon Emporium and every
other building have their own source event cells and destinations. A generic
“blocked cell means motel/security building” rule is invalid.

The full building pass must enumerate, for each doorway:

| Field | Required source evidence |
| --- | --- |
| entry cell | mode map raw byte and four-corner selection |
| reaction index | `FFFBF9`/mode reaction table value |
| destination | mode table/pointer entry and destination mode |
| entry placement | destination descriptor camera/record coordinates |
| exit placement | destination's reverse reaction or explicit exit cell |
| sound/fade | source calls around the transition |
| special state | story/combat/shop/alarm state set before entry |

## 8. Command overview and menus

### Overview loop: `0x10D30` / `0x10F50`

`0x10F50` initializes selector `FFF0D0=6` and row `FFF0D1=0`, uploads the
overview background and active record fields, then returns to the loop. The
seven selector positions are:

| Selector | Source label | Source route |
| ---: | --- | --- |
| `0` | Inventory | left column, four rows |
| `1` | Inventory | right column, four rows |
| `2` | Clips | `0x11B58` |
| `3` | Attributes/Skills | `0xD01C` |
| `4` | Cyberware | `0x11950` |
| `5` | Magic | `0xFBA6` |
| `6` | Pocket Secretary | `0xC2A6` |

The source displays the selected command label at the command position. It is
not a vertical checklist containing all seven labels. `0x114A0` wraps the
selector and sets the source redraw/action flags.

The overview reads the active record's source race/class values, attributes,
skills, equipped item, armor, ammunition, posture, bars, and inventory cells.
These values must be read from the selected live record after field mutations.

### Inventory cursor: `0x11506`, `0x11564`, `0x11592`

The inventory is an 8-cell, two-column by four-row grid. `FFF0D0` chooses the
column and `FFF0D1` chooses the row. `0x12148` reads:

```text
slot = 2 * FFF0D1 + FFF0D0
item = active_record[0x66 + slot]
```

The selector is a source cursor sprite over a box. It is not a checklist and
it is not an equip marker. A dispatches through the source item action table.

### Equipment/action dispatch

The source dispatch at `0x12168` first classifies the selected item:

| Item code | Source branch |
| ---: | --- |
| `0x13`, `0x14`, `0x16`, `0x18..0x2B` | automatic-use acknowledgement at `0x121C0` |
| `< 0x18` otherwise | weapon/gear equip path at `0x121E0` |
| `0x2C+` | armor path |

The weapon/gear path uses `+0x56` and `+0x57` as equipment state. It converts
ammunition using the selected item record's factor at `D4540 + item*0x30 +
0x24`, preserves source quotient/remainder behavior from `0x125FA`, and
stores the result as a source byte. Selecting the already equipped slot writes
`+0x56=0x7F`, clears `+0x57`, and returns the source state to “no weapon”.

The armor path at `0x1219C`/`0x121B0` uses `+0x76` and the selected slot's
`+0x6E` phase byte. If any armor is equipped, the next armor action clears
`+0x76` and the selected cell's phase, even when a different armor is selected;
a later action equips that armor. The presence of an armor code in a cell alone
does not equip it.

The selected-item detail path is a separate source classification. `0x115F2`
calls the weapon Damage/Power writer only for codes below `0x10`. The
`0x12B8E` branch selects literal labels from `0xD29C4`, `0xD29CA`, and
`0xD29D0`: `ammo` for `<0x0D`, `uses` for `0x0D..0x13`, and `rtng` for
`0x14`/`0x18..0x28`. The medkit code `0x10` therefore uses `uses`; it is not
a weapon and must not display a Damage row. Its value is a two-tile
source-indexed entry selected from `0x6AC46` by the selected slot's `+0x6E`
byte. The native port now preserves the label classification and exposes the
raw selector while the source tile table is promoted into native assets.

The separate branches at `0x12634`, `0x12836`, and `0x11040` have their own
messages, target/runner selection, and mutations. They must not be collapsed
into the audited `0x12168` path.

### Posture: `0x11928`, `0x119CE`, `0x11ABE`, `0x11AE4`

When the selector is `2..6`, Up enters posture focus. The five source labels
are:

```text
Full Defense, Med. Defense, Neutral, Med. Offense, Full Offense
```

Left/Right changes `+0x4D`, clamps it to `0..4`, redraws the strip/name, and
calls `0x12CEE` for the source attack and defense calculations. `0x12CEE` is
not a generic multiplier on an existing bar. It calls the source average and
item/skill helpers, checks equipped-item state, adds defense/cyberware values,
and selects one of the extracted source strips.

The normal equipped-item branch has been translated and tested against Joshua:

```text
posture:  0  1  2  3  4
attack:   3  4  5  6  7
defense: 19 18 17 16 15
```

The negative `+0x56` special attack branch at `0x12D7C`/`0x12FA4` now has a
typed D4EA0 spell-row boundary and source-matched residual test coverage, but
the complete attack-strip branch and the field AI/combat consumers remain
`PENDING`.

## 9. The remaining command panels

### Clips: `0x11B58`

The clips panel is an action panel, not a passive ammunition display. Its
source code draws the clip rows at `0x11B58`, then dispatches the selected row
through the action branches at `0x11CCA`, `0x11DD2`, and `0x11FDE`.
`0x12062` is the empty-clip gate. A row with no usable clip reaches the source
error/result path; it must not silently refill, disappear, or become an
equipped weapon.

The separate Qt conversion runtime now uses the source-specific menu rows:
`0xCD33E` for top-level Clips, `0xCD37A` for nested Trade, and `0xCD3B6` for
Reload confirmation. It previously reused the ordinary inventory action row
at `0xCD302`, leaving unrelated icons behind the Yes/No prompt. The source
clear at `0x12C36` covers the full 20-by-20-cell center region before the new
panel is drawn. Executable QA walks every selector entry and wraparound, checks
deselection/reselection without record mutation, compares the surrounding
portrait/inventory/status shell while moving, and verifies reload confirmation
and the `0x26` load increment. Debug and Release produce identical full-screen
captures across top-level, nested, confirmation, return, and reload states.
Those captures compare the UI with extracted ROM resources and are internal
conversion checks; there is no independent Genesis Clips sequence in the
supplied longplay, so visual fidelity remains unverified. The converted reload
path gates non-clip items,
consumes one clip, clamps loaded ammunition to the extracted item capacity, and
emits the source `Rock & Roll, chummers!!` result. The nested trade path now
uses the same typed three-slot party roster as the field scene: source-record
exclusion, active/hidden recipient filtering, recipient cycling, quantity
wrap, transfer arithmetic, the source completion message, and the selected
recipient's live-record `+0x58` name are tested. The source `????` prompt is
retained only as the fallback for an absent name; an independent visual golden
also remains open.

### Attributes and skills: `0xD01C`

The panel obtains twenty labels from `0xC1A2A`: eight attributes followed by
twelve skills. The source lays them out as two columns of ten rows. This is a
readout of the active runner record, not a character-class selection screen.
The values live in the record attribute/skill bytes beginning at `+0x77` and
must reflect mutations from combat, cyberware, items, and story events. The
panel's display path is correlated; every mutation path that can change these
bytes still needs an explicit source trace.

### Cyberware: `0x11950`

Cyberware is a bit-set panel. The active record carries the owned/active bits
at `+0x90`; bit 15 is handled separately and bits 14..0 index the pointer/data
table at `0xD2072`. The source panel enumerates those bits and resolves each
label through that table. Possession, availability, active effect, and display
selection are separate questions here, just as inventory and equipment are
separate questions. The list is source-correlated, while the item-specific
mutation/effect paths remain `PENDING`.

### Magic: `0xFBA6`

The Magic command first compares runner class `+0x9B` with 2 at `0xFBA6`.
Classes 0/1 enter the spell-panel branch only when resource/cap byte `+0x7F`
is nonzero. That path uses `0x11900`/`0x12C36`, the `+0xA0` table selector,
`FFF10E` spell-panel state, and later spell/force/target helpers. If the class
is 2 or greater, or class 0/1 has `+0x7F == 0`, ROM takes the Magical Defense
fallback instead. It prepares the command row and clears/rebuilds the center
content grid through `0x12C36`, then writes `C89C0` (`Magical Defense`) to
`C31C` and calls `0x106DA` twice with `D2=0` and `D2=-1`.
Those calls build independent physical and mana values from active-party
`+0x10`/class/item-0x18 state and the active runner's `+0x77`/`+0x7C`, then
write 13-tile rows at `C51E` and `C69E` from `0xCF610` (stride `0x1A`,
palette `0x2000`). It waits through `0x0D94` for a fresh controller event
before `0x11928` restores the command row. It replaces the center content but
does not clear the whole menu plane, so the runner frame and right HUD remain.
The Qt executable connects this static subpath and its parent `0x24` action
event.

For the spell-capable branch, `0x10260` builds the selected spell/force state;
`0x10276` writes the selected spell's 3x3 glyph beside its title, text, and
force/posture values; `0x102EC` writes the multiline metric and Force/Posture
labels; and `0x11190` transfers complete 3x3 spell icons to the lower strip.
The five runner spell tables at `0x166AD8` store a DBF last index, so native
`entry_count` is source word + 1. The typed panel draws the four source metric
rows refreshed through `0x1055E`, calculates the Damage/None row through
`0x105E6`, and applies the signed availability gate at `0x1088C`. The Qt QA
walks the spell list, selection, four cursor phases, return, and reopen; it
checks the retained side HUD and portrait and verifies the old command-center
controls are cleared. Its runtime dump is
`qt_runtime/field-magic-spell-qa/actual.bmp`.

Visual review used the independent longplay frames
`section_06_combat_inventory_delivery_0510-0600/frame_000001.png`,
`frame_000002.png`, and `frame_000005.png`; frames 000000 through 000006 show
the cursor changing while the page is otherwise stable. The source cursor
counter advances phase every seven field frames. Opening requests event 0x24,
which maps to `menu-024.flac` (2.484 seconds from FLAC metadata); actual Qt
output-device playback has not been measured. The spell target/effect action
route and broader independent Genesis coverage remain open, so this is a
partial hill and the internal CTest dump is not a visual golden.

### Pocket Secretary: `0xC2A6`, Group Items: `0xC8C8`

Pocket Secretary draws through `0xC3E6`, with the source option/label table at
`0xC1418`; the cursor is maintained at `0xC376`. The source debug/state byte
at `FFF101` changes the visible bounds between four and five entries, which is
evidence that this panel has state-dependent contents and is not a decorative
list. The existing NativePort base presents the five-row field-mode order and
sends Save/Load to the existing `0x59172` screen. Qt draws the same source row
order; its Save/Load handler remains a separate unconnected route.

Selecting Group Items dispatches to `0xC8C8`. That handler reads the persistent
global bitfield at `FFFC22`, emits the names from the fifteen-entry pointer
table at `0xC25C4` in descending bit order, or writes the exact empty message
at `0xC1471` when no bits are set. It then waits through `0xF312` for any
controller edge and returns to the command loop. The NativePort has a separate
catalog path. Qt now connects the empty and populated display branches using
typed FFFC22 state and the extracted item-name table. For a fresh New Game,
FFFC22 is zero: C8C8 clears the inner panel, writes the two C1471/C1486 source
lines at C50E, then waits at F312. For populated diagnostic flags, the typed
view visits bits 14 down to 0 and draws names at C208, advancing two tile rows
per visible entry to match the source +0x100-byte destination stride. The Qt
QA holds each state for 30 idle frames and returns to the same selected row.
Normal quest acquisition and restored-save FFFC22 are not connected, and the
supplied longplay sections do not show this screen; these dumps are regression
evidence, not Genesis visual proof.

The Cyberdeck row dispatches to `0x18150`. The first instruction tests
`FF018F` bit 0, the source deck-present flag. When it is clear, the handler
calls `0xC3E6`, passes `0xE43D1` to `0xC0DA` with destination `0xC40E`, and
returns to the Pocket Secretary loop. The three fixed source strings are:
`" You do not have"`, `"  a cyberdeck!!"`, and `" Keep looking."`. The native
Qt extracts those lines. The active Lua route reads the typed deck-present
fact and pushes generic `MESSAGE` with `NO_CYBERDECK` when the bit is clear;
the Qt presenter draws the extracted lines and returns to the same selected
row on a fresh button edge. When the flag is set, the registered Lua route
pushes the same typed Cyberdeck screen used by Matrix entry and returns to the
selected Pocket Secretary row on Start. Debug/Release ordinary-input QA now
captures this `0x18150` branch and its clean return; the full deck image was
inspected against independent Section 15 frame `004220`. This path opens the
offline deck menu; the separate Cyberspace/System Search route starts a Matrix
session. The inline branch beginning at `0x1817E` is now source-traced as the
deck-present entry shell: it prepares/copies the Matrix resource block at
`0xE2E3A`, establishes ten menu slots, enters `0x1822C`, then refreshes the
mode data/palette and restores field resources. The `0x1822C` deck/program loop
and its state tables remain a separate audit target; a generic Matrix
placeholder is not an acceptable substitute.

The separately exported `0x181F8` path is now named
`run_matrix_session_entry`. Its confirmed prelude is represented by
`prepare_matrix_session_entry` in the cyberspace entry module: the source
display commands, `0xE2E3A` copy, `FFEF14` dirty bit, menu-resource upload, and
`FFFBCA=10` slot initialization are preserved before the `0x1822C` fall-through.
The remaining session loop/finalizer is kept as an explicit boundary until its
source state is converted rather than being conflated with the `0x1817E`
inline caller shell.

Dismiss Runner dispatches to `0xC448`. It clears the inner panel through
`0xC3E6`, then writes the fixed text from `0xC1499` at `0xC216`: `Dismiss`,
`Who??`, the visible `0xC14AE` separator (`[` plus twelve backslashes), a
blank row, and `Abort`. The separator is visible text, not name-table
padding. `Abort` is always at `0xC416` / host `(88,64)`, independent of the
candidate count. The handler enumerates the live records at `FF0200` then
`FF0300`, tests bit 0 of each record's `+0x10`, and advances the destination
by `0x100` before writing each active name. Thus the first candidate begins
at `(88,80)`, followed by 16-pixel rows. Cursor base `0xC412` is `(72,64)`;
each selected row advances it by 16 pixels. Up/down requests sound event
`0x25`; A/B/C confirm with `0x24`, while Start returns to the parent.

Selecting a candidate enters `0x59798` and clears its compact party marker
(`FFFC0C` or `FFFC0D`); it does not clear the live record's `+0x10` bit.
Follow-up counter, effect, rendering, and active-runner paths include
`0xC8B0`, `0x6AA7E`/`0x33AA`, `0x1E514`, `0x3404`, `0x38FA`, and `0x391A` when
the dismissed runner is current. Candidate selection also calls `0x110E8`
for a HUD preview. The Qt route now uses typed candidate/cursor state, the
fixed Abort row, source text positions, compact-marker removal, and active
runner replacement. It advances the `0xE660` random two-cell effect once per
frame and paints the persistent cells in twelve lower-bezel positions. The
`0x110E8` preview resolves each live record's portrait table, offset, and
palette attributes and draws its 32x32 image at source destination `C928`,
which maps to `(160,144)` with the `0x80`-byte tile-row stride. Held-frame QA
checks that lamps do not cover child text, cursor, or portrait, and that the
portrait changes with candidate selection. Event `0x25` has no identified
alias or duration. The independent longplay contains no Dismiss Runner screen,
so source visual cadence remains unverified. Current Qt captures and tests
cover state transitions and internal draw consistency. The separate NativePort route is not
evidence of ROM fidelity: it filters the current record, moves Abort after
the names, omits the visible separator, and clears a live-record flag.

Notebook dispatches to `0xF236`. Its setup helper `0xF33E` writes the title
and two ungated rows from `0xC567E`, then appends `Contacts`, `Mr. Johnsons`,
and `Shadowrunners` only when `FFFC2C`, `FFFC27`, and `FFFC2A` are nonzero.
The New Game initializer at `0x59C48` stores `0x1C7F` in `FFFC2C`, so a
fresh post-opening session has Contacts visible; the typed Qt facts preserve
that initial gate rather than hiding the row.
The copied action table at `0xC6648` resolves those rows to `0xF2E2`,
`0xF9A4`, `0xF840`, `0xF416`, and `0xF6DC`; `FFF0B9` is the inclusive
selector bound and `FFF0DC` is the cursor. The Qt text catalog loads the
Notebook labels and the `0xC56E7` three-line response. In a fresh session, where
the current-run flag is not active, selecting `Current Run` follows `0xF2E2`
and displays `You are not currently / engaged in a shadowrun. / Get a job!!`,
then waits for an input edge before rebuilding Notebook. The Tips & Clues
page and conditional child routes are now connected. The positive Current Run
branch and story-event writers for conditional Notebook rows remain explicit
boundaries; child lists filter typed source masks rather than use an invented
always-visible list.

The raw US ROM placement is now recorded for the Qt base screen. `0xF33E`
clears the inner panel through `0xC3E6`, draws `0xC567E` at `0xC216`, and
draws its separator at `0xC292`; the `0x80` markers in `0xC0DA` advance one
8-pixel name-table row. Thus the title, Current Run, and Tips & Clues begin at
`(88,32)`, `(88,56)`, and `(88,72)`. Each visible event-gated row begins 16
pixels after the prior option, starting at `(88,88)`, and the cursor base
`0xC392` is `(72,56)`. The no-run branch at `0xF2F6` clears the same panel
before drawing `0xC56E7` at `0xC30C`, or `(48,48)`, with 8-pixel line spacing.
The Qt painter redraws the blank inner grid before either screen's text. Its
QA checks that Notebook rows are gone during the response and that response
text is gone after Notebook is rebuilt. Independent Pocket Secretary frames
are still absent from the supplied longplay. Qt renders the shared `0xE660`
wait animation inside the twelve source-derived lower-bezel destinations;
independent Genesis frames are needed to verify its appearance and cadence.

The Notebook loop at `0xF236` treats C separately from its row handler table:
it calls the shared `0x391A` mode-action routine and branches back to the same
Notebook loop. That routine requests audio event `0x25`. The Qt Notebook C
route now issues that event while retaining its open state and selected row;
its route QA checks the event and same-page return. The Notebook A dispatch,
Up/Down selection, and B/Start return paths do not request audio. The Tips
page has the same source C-to-`0x391A` event, while its page movement and
A/B/Start return are silent. Event `0x25` resolves to the extracted
`audio.effects.resource.037` clip (2.134 seconds including its release tail);
default-output playback state and measured duration are covered by the shared
audio playback QA. Route-specific Windows loopback remains unrecorded.

The Tips & Clues handler at `0xF9A4` is backed by three 32-entry pointer
tables: `0xC2716` selected by `FFFC30`, `0xC2796` by `FFFC34`, and `0xC2816`
by `FFFC38`. It starts `FFF0B9` at `0xFF`, clears D1, then uses `BTST.L D1,D0`
while advancing the table pointer and D1; table entry `n` is enabled by flag
bit `n` (LSB first). Startup at `0x084E` sets `FFFC30` to `1`, so a fresh
New Game has source entry zero available. Set pointers are appended to
`FFEF16`, and `FFF0DC` selects the compact visible-page list. Left and Right
move that page cursor with a zero/final-index clamp. A and B return to Notebook,
C calls the shared `0x391A` mode-action routine and resumes Tips, and Start
returns to Notebook.

Qt loads all 96 source strings from the ROM-generated Lua text table in
`dialogue/script-text.json`; source `0x80` line breaks become host newlines. The
screen draws the title/separator at `(88,32)/(72,40)`, page text at `(40,56)`,
the `C663A` page-count label at `(136,184)`, the right-aligned current page at
`(184,184)`, and the total at `(232,184)`. `C6645` is the two-cell current-page
clear span. The Qt panel is rebuilt from its blank inner grid before drawing
the tip, so parent buttons do not remain underneath the child; returning to
Notebook similarly clears the tip and counter. E660 advances once per VBlank
in the twelve lower-bezel cells, outside the tip body and page counter. F9A4
does not request sound. Its C action reaches event `0x25` through `0x391A`, but
that event still has no verified Qt audio alias or measured clip. Story-event
writers and restored-save tip flags remain open; independent Notebook/Tips
Genesis frames are absent from the supplied longplay, so visual fidelity stays
partial.

The conditional Mr. Johnsons handler at `0xF416` is a separate five-name
screen, not another Tips page. It writes the title from `0xC56AE`, then tests
bits 0 through 4 of `FFFC27` and emits these source-order names into its
dynamic list: `Mr. Gunderson` (`0xC5728`), `Julius Strouther` (`0xC5755`),
`Mortimer Reed` (`0xC5766`), `Vigore & Jarl` (`0xC5736`), and
`Caleb Brightmore` (`0xC5744`). Its selector starts at zero and uses the same
`C376` bound/cursor family, but A writes the selected entry through a separate
`C38C` destination; C returns, B calls `0x391A`, and Start continues the loop.
The five fixed names are extracted into the Notebook text catalog in source
order. Its Lua list/detail loop is connected to the Qt presenter; the story
writers for `FFFC27` bits 1 through 4 remain unresolved.

Contacts (`0xF840`) and Shadowrunners (`0xF6DC`) are separate Notebook child
screens. They share the `0xF5F4` flag-filtered list builder, but read different
source masks and record tables: Contacts uses `FFFC2C` with 13 names at
`0xC66AC` and 13 descriptions at `0xC66E0`; Shadowrunners uses `FFFC2A` with
10 names at `0xC665C` and 10 descriptions at `0xC6684`. A set bit selects its
same-index record, preserving source order. The builder shows up to eight
records on one page. When more than eight bits are set, it replaces row eight
with the source `<Next Screen>` label (`0xC661D`), shows remaining records on
page two, and appends `<Prior Screen>` (`0xC662B`). Exactly eight selected
records do not create a page control. A/B opens a selected record's separate
description wait; a fresh input edge returns to the list. C uses the shared
`0x391A` action; Start returns to Notebook.

The field route now gives each page its own Lua screen state and cursor. Lua
filters records, paginates, handles controls, and selects detail records;
Qt receives the typed visible-index snapshot and renders the ROM-extracted
text from `assets/dialogue/script-text.json`. The offline importer is
`decompilation/ghidra_conversion/tools/rom_research/extract_notebook_records.py`; the runtime does not read ROM
pointers. Debug and Release route QA covers both tables, details, forward and
prior paging, parent return without stale child text, and the exact-eight
boundary. The QA masks `0x1C7F` and `0x03FF` are diagnostic fixtures; they do
not establish every story writer for these flags. No independent Genesis
Notebook captures have been identified in the supplied longplay, so the
current captures verify the converted screens' full-frame cleanliness and
state transitions, not independent visual fidelity.

### Matrix

Matrix is a separate state family from Magic and from the command overview.
The deck-present entry/return shell at `0x1817E` is now converted, including
its source resource addresses and `0x1822C` loop boundary. The deterministic
opening of `0x1822C` is also converted in the separate Ghidra conversion
module: it preserves the source upload/HUD callback order, performs the
source row-3 staging write before clearing the live selection to column 0 /
row 0, and translates row/column moves to ordinary host coordinates
(`0x94 + row * 0x28` and `0x8C + column * 0x20`).
The adjacent `0x18A92` leaf is now named as the Matrix program-key helper:
it clears D7, multiplies the row byte by four, and adds the column to D7's
low byte only. Normal selections linearize to 0..15; the typed conversion
also preserves the full register result and source byte wrapping for
malformed/restored selections without exposing FFF0D0/FFF0D1 to host code.
The `0x1882E` panel path now has its source-controlled render order: the common
header is written first; an unavailable index 0..11 renders only the locked
message and returns; available entries render their icon and detail resources,
run two description passes, and add runner-only statistics for indices 0..11.
Indices 12..15 bypass the availability table and runner-stat path. The actual
resource presentation remains a callback boundary.
The inline action branch at `0x18408` is also separated: source action bits
write the three proven `FFF0B7` modes in priority order, bit 15 exits Matrix,
rows 0..2 enter the program-slot path, row 3/column 2 is blocked without an
active FFF0B0 run, and row 3/column 3 becomes the pull-the-plug confirmation
only when that run gate is active. The four indirect column handlers remain
explicit boundaries until their targets are traced.
The `0x19238` program-slot path now exposes its record lookup: it searches
the five source slot bytes for an exact selected-program match, otherwise
chooses the first negative/free slot, and reports no available slot when all
five are unavailable. The subsequent load/execute and visual-refresh paths
remain separate boundaries. Its concrete outcomes are now typed as already
loaded, no available slot, insufficient memory, or ready to install; the
loaded-program Yes/No prompt routes Yes to the existing slot command and No
back to the Matrix panel.
The source HUD-mode leaf at `0x19BAE` is now converted separately from the
Matrix UI: a selected 0x14-byte record is eligible only when `+0x0F` is
positive and signed `+0x13` is nonnegative; alternate mode interleaves a
second table entry only for codes 7..16. The larger `0x19B40` wrapper now
applies the record eligibility gate, uses the record mode when `FFDF84` is
negative, and otherwise requires a positive `0x17C3C` result before the same
table lookup; that resolver remains an explicit boundary.
The old identity label at `0x19BF6` was also disproven. The source preserves
its incoming value in D0 while selecting the current or adjacent record's
`+0x11` field and adding one or two according to the signed `FFF0A0` variant;
the bounded host conversion now exposes both results.
The adjacent `0x19C54` leaf is also no longer treated as two empty exported
functions: it follows the selected slot through its loaded program code and
reads that program's raw availability byte. That byte is deliberately kept as
an integer because the callers at `0x55FD8` and `0x56506` compare, add, and
scale it; reducing it to `bool` would change the action/HUD behavior whenever
the source value is greater than one. `0x19C5C` is an interior instruction
address in the same leaf, so the native conversion exposes one bounded lookup
with the original byte value intact.
The CyberInfo screen at `0x18AA6` is no longer a generic world routine:
selection zero is Deck Stats and selection one is Run Info. The new native
screen driver preserves the source setup, per-frame VBlank/latch/sample/history
order, up-before-down priority, three-button page activation, and bit-15 return
to the Matrix panel. Its Run Info branch at `0x18C42` classifies signed
`FFF0E3` as active when negative, absent when zero, and complete when
positive. For an active run, `FFFBF2` selects CPU versus Datastore and its
node-function label, while signed `FFFBF6` selects Delete File, Upload File,
Crash System, or Download File through the `0xE44C8` pointer table. The Lua
CyberInfo route now presents the same active/absent/complete result from the
saved Matrix objective state; Deck Stats field extraction and exact panel
placement remain open callbacks.
The adjacent Matrix program-slot path at `0x19238` now has its complete
interactive prompt control flow in `matrix_program_slot_screen_conversion`:
the source enters on No, wraps the two-entry cursor, prioritizes activation
over return, applies the slot mutation only for Yes with a live ready timer,
and otherwise performs the clear-timer/slot-refresh branch before the common
Matrix redraw. This is separate from the existing typed slot/memory planner;
the remaining resource/text presentation is still an explicit callback.
The parent Matrix loop at `0x1822C` now has the same control-flow treatment:
the host conversion services VBlank/input/history in source order, gives
vertical movement priority over horizontal movement, redraws the typed
four-by-four cursor, routes rows 0..2 to the program-slot screen, preserves
the row-3 inactive-run gate, and exposes the column-action/disconnect handlers
as explicit native boundaries. Bit 15 is handled only after the action bits,
matching the source's Matrix-to-field exit priority.
The storage screen path at `0x18D1A` now has its first concrete record math:
`0x1954A` reads the five loaded-slot usage bytes, `0x1952C` loads each of the
twelve availability flags, and its `0x19538` arithmetic tail squares the flag
before applying the source E2DF2 footprint table. Normal flags are 0/1, but
the native conversion preserves the source operation, and
`0x19558` subtracts both totals from the Matrix's total memory word. The new
`matrix_storage_conversion` module returns those values as ordinary host
integers. The Lua Storage screen now models the `0x18F5E` reverse five-slot
datafile walk, ROM filename/separator/size rows, Start-to-return behavior,
empty-storage return, and `0x18DE0` erase prompt. No cycles through occupied
records; Yes clears type/size/value and returns through the file-list branch.
The Qt layer receives typed phase/name/size fields and places the text at the
traced plane coordinates. Debug/Release route QA writes complete frames for
the root, empty view, list, prompt, post-erase list, and empty result. The full
Cyberdeck/Matrix hill remains partial pending broader traversal coverage and an
independent Genesis Storage comparison.
The callable sub-entries are retained separately in the ledger: `0x195A6`
selects the current program before entering timing arithmetic at `0x195AA`,
`0x19538` is the callable footprint-multiplier tail of `0x1952C`, and
`0x19C54` enters the shared slot lookup at `0x19C5C`.
The neighboring helpers were corrected from the ROM trace: `0x1957E` measures
capacity consumed by the five programs already present in the loaded-program
slots, while `0x195A6` computes a selected program's load-step count from its
footprint and deck divisor. `0x195D8` publishes that program code, clears its
pending flag, stores the ten-times step offset, and refreshes the source grid
and row-render boundaries. The earlier generic visual-slot helper in the field
batch was removed because the ROM trace does not support that interpretation.
The utility handler at `0x19018` now has a named two-entry selector: source
selection zero dispatches “Show Map,” while selection one dispatches “Retreat”
unless `FFDF84` reports combat, in which case it takes the explicit “Retreat
not possible during combat!” branch. The deeper `0x19180` routine is now
typed as a node transition: `FFF118` is the current node, `FFF11A` is the
previous node, a negative previous node takes the JACK OUT warning, and an
action swaps the two node ids while publishing the transition marker. Bit 15
remains the return-to-Matrix route.
The Matrix state table, program actions, and rendered resources inside the
loop must still be recovered before claiming the Matrix panel complete. Until
then the Matrix hill remains `PENDING`, rather than represented by a generic
screen.

## 10. Interaction, story, shops, and combat

### Field interaction and story dispatch

Field actions do not directly select an arbitrary destination. The source
dispatch writes a command/state value to `0x1F580`; that value indexes the
250-entry mode-command table at `0x12CCB4` (commands `0x00..0xF9`). A second state-pair table at `0xF3E1E`
selects the routine through `0xF3DC6`. The portable implementation must retain
both levels: the field produces the source event, then the transition table
chooses the destination/routine and its presentation.

Opening-story text and related narrative blocks occupy `0x5B7D8` through
`0x5BA4C`. This is evidence that the intro is a sequence of source stages,
not a single static title-to-field jump. The known opening stages are listed
in Section 6; exact frame timing, artwork, actor sprites, and music command
boundaries are still `PENDING`.

### Building entry

Building entry must be audited as a tuple:

```text
current field mode
  -> raw interaction/event value
  -> source transition-table row
  -> destination mode/map and entry position
  -> building presentation/action state
```

The Redmond Barrens Stoker's Coffin Motel interaction is a confirmed native
regression target: the source action must resolve to the motel destination,
not to a generic security-building shell. A “building” fallback without the
source event and destination row is rejected. The working worksheet must record
the source coordinates, collision tile, event value, table row, destination,
entry coordinates, and return transition for every tested building.

The earlier native-port note incorrectly attributed the Stoker motel text
(`0x1167EB..0x116A79`) to `0x20E48`. The ROM bytes at `0x20E48..0x20EDD`
instead select the electromagnetic-lock doorway descriptor at `0x11CCCE`,
the four event-specific choice grids written by `0x20E92`, and the building
Security Terminal descriptor at `0x11CD62` through `0x20EC2`. The converted
field-interaction continuation now records those typed prompt/resource plans;
the Stoker/Jump House text remains a separate unresolved source-trace task.
No native-port dialog behavior is being used as proof for this conversion.

### Shops and equipment

The item catalog begins at `0xD4540`; cyberware data begins at `0xD2072`.
Shop presentation and purchase/use mutations must consume those source data
records. A shop may grant possession without equipping an item. Equipment is a
separate action through the inventory dispatch in Section 8, and a save/load
round trip must preserve both states independently.

### Combat and posture

The posture bar is a combat input/state calculation, not a UI decoration. Its
source handler recomputes attack and defense from posture, skills, equipped
item state, armor/cyberware, and source helper routines. The current native
normal branch is tested, but the special attack branch and combat consumers
are not yet complete. Until those paths are traced, combat should not claim
feature completeness merely because the posture strip changes on screen.

## 11. Persistence and runner state

The save/load entry at `0x59360` validates the save record, restores the
source state, and copies runner records through `0x597EE`/`0x598B0`. The
valid/invalid save messages and checksum behavior are distinct source paths.
The portable save format may use a Windows-native representation, but it must
round-trip the source semantics: active runner, each live record, inventory,
equipment, armor, posture, ammunition/clips, cyberware, attributes/skills,
story flags, field mode/position, and any audio/intro state that the source
persists.

A runner switch is legal only when the source roster contains another
eligible runner. The source still emits the switch attempt sound/message when
there is no eligible runner, but it does not replace the active runner with an
archetype or invent a new character. The native implementation's previous
“cycle Samurai/Decker/Shaman” behavior is explicitly rejected.

## 12. Audio and timing

The source sound command boundary is `0x530DE`; the completion/status path is
`0x530EA`. The command ring is initialized and advanced around `0x52F3C`,
with command/data helpers at `0x52E96` and `0x52EA2`. `FFFC0A` is a source
audio gate/state byte that must be preserved in any timing model.

Known command observations are:

| Context | Observed source command(s) | Status |
| --- | --- | --- |
| title reveal/settle | `0x5D` | confirmed boundary, timing still to trace |
| opening stages | `0x10`, `0x11`, `0x12`, `0x2E` | Paired `0x10`/`0x12` tracks mapped; paired `0x11` trace has no YM key-on and remains unmapped; `0x2E` is verified silent slot 46 |
| command/menu activity | `0x24`, `0x25` | confirmed boundary, per-action mapping pending |

The native port can expose a platform-neutral `playMusic`, `playEffect`,
`stopMusic`, and `isAudioBusy` interface, but the call sites must remain
source-driven. A silent screen, a guessed loop, or one generic menu beep is
not evidence of correct audio behavior. Every screen worksheet needs its
entry command, loop/stop condition, action sounds, and exit command.

## 13. Native status and evidence boundary

| Area | Native status | Evidence boundary |
| --- | --- | --- |
| title palette/background | partial | source palette is `0x1FF0`; the background writer's `0x4000` VDP attribute still needs source-frame comparison |
| title animation | partial | source loop and sprite lists identified; exact timing/priority/clipping pending |
| startup options | confirmed | selector, New/Load rows, source layout and transition identified |
| save/load | partial | valid/invalid/success paths identified; full record/checksum trace pending |
| archetype setup | confirmed | source rows/templates identified; no runtime archetype cycling allowed |
| opening | pending | stage entry points identified; artwork/timing/audio/transition trace incomplete |
| field rendering | correlated | mode/resources identified; full plane/object priority still under audit |
| field movement | correlated | source direction/frame tables identified; all directional animation frames pending |
| building entry | partial | motel regression identified; destination matrix incomplete |
| command overview | correlated | selector and seven command dispatches identified |
| inventory | confirmed | two-column/four-row source grid and cursor path identified |
| equipment | audited path | possession/equipment distinction and major dispatch branches identified |
| posture | partial | normal calculation branch translated/tested; special/combat consumers pending |
| clips/attributes/cyberware | partial | panel sources identified; action/effect mutations pending |
| Magic | partial | class/resource gate and two-row cursor bounds translated; spell-specific action paths and transitions incomplete |
| Pocket Secretary | partial | five-row field-mode menu and Save/Load entry translated; row effects and restricted-mode gates pending |
| Matrix | pending | state-specific action paths and transitions incomplete |
| combat | pending | posture consumers, actions, damage, AI, and result presentation incomplete |
| audio | partial | command boundary/ring and ROM bank window traced; 65 complete WAV assets extracted from the 94-entry table-2 resource set; screen-specific playback wiring remains |

## 14. Rejected shortcuts

The following implementation strategies are not acceptable as reconstruction
evidence:

1. A static title image in place of the title state machine.
2. The wrong palette block or a palette derived from unrelated pointer/data bytes.
3. Arbitrary title frames, sprite order, or reveal timing chosen for visual appeal.
4. A generic building destination or “security building” fallback when the
   source transition row is unknown.
5. Treating inventory possession as equipment.
6. Replacing the source inventory grid with a checklist.
7. Applying a generic posture multiplier instead of the source `0x12CEE`
   calculation and its equipment branches.
8. Enabling three archetypes as switchable party members when the source only
   uses the archetype screen for initial setup.
9. Claiming a visible panel is functional without tracing its action, mutation,
   message, and return paths.

## 15. Reconstruction worksheet and work order

Each screen/state entry must be recorded with these fields before its native
implementation is called complete:

```text
state id / source entry
entry condition and predecessor
exit conditions and destination state(s)
input bytes, cursor bounds, repeat/debounce behavior
RAM fields read
RAM fields written
tilemap/plane resources and decoded format
sprite/object list, frame source, priority, clipping
palette source and conversion
text source and coordinates
music/effect commands and timing
save/load impact
known source comparison image or trace
native test and remaining evidence gap
```

The implementation order follows the dependency graph rather than the visual
surface area:

1. Finish title frame extraction and source-timed reveal/audio comparison.
2. Finish opening stage transitions, artwork/sprites, text, and music.
3. Complete field movement frames and directional object animation.
4. Recover the building transition matrix, starting with Stoker's Coffin Motel.
5. Rebuild the command overview and every panel as source cursor/action state.
6. Finish runner roster, save/load, equipment, clips, cyberware, and attributes.
7. Trace Magic, Pocket Secretary, Matrix, shops, combat, and posture consumers.
8. Replace native placeholder audio/assets with extracted source resources and
   verify each state against capture traces.

No later item should be used to mask an earlier unresolved transition. When a
source path is unknown, the code and this document must label it `PENDING` and
capture the missing evidence needed to close it.
Matrix/menu service leaves converted.
The selected-program action service at `0x19976` is now typed as well. It rejects empty or still-loading slots and the source's active selection values `2..5`, requires overlay stage `0x0C`, derives action mode with `FFDD22` bit-14 then bit-12 priority, dispatches through the unresolved program-code table, and resets the overlay. Normal program codes publish the exact next selection value (`<3` -> `3`, `<8` -> `5`, code `11` -> `2`, otherwise `4`). Selection `7` enters the runner-record parity path: even record fields clear the program availability flag and use seed mode `5`, odd fields use seed mode `4`; both set the HUD variant and initialize the world-effect seed through explicit callbacks.
The next three Matrix/menu leaves are now typed in `matrix_menu_service_conversion`.
`0x19632` clears one loaded slot using the source program-code-specific flag cleanup, writes the empty program/status values, and preserves the repeated-grid/grid/row refresh order.
`0x19788` decrements the unsigned overlay countdown, services the overlay timer, then advances only the first non-complete one of five visual slot states when the signed inter-step delay expires; completion publishes the selected slot and renders its text.
`0x1989A` normalizes the signed HUD variant to `-1`, `0`, or `1`, exposes the upward completion fade/world-effect/vblank/controller sequence, and publishes audio only when the source's last-published variant changes. These callbacks are source-service boundaries and are not linked to the emulator or native renderer.
The paired runner-record leaves are now no longer generic: `0x1A4EA` selects the compact timing record at `FF1100 + (FFF118 * 0x14)`, reads its `+0x10`, and returns the complete signed table at ROM `0x1A50E..0x1A536` (`100, 20, 10, 9, ... -8`), while `0x1A538` tests the same selected record's `+0x0F` as the active-effect marker. They are deliberately separate from the 0x100-byte live runner record model and are exposed through the typed record-access module with explicit invalid-index handling for host callers.

`0x1BA6E` is now identified as the four-frame title burst animation called from the larger title/world sequence at `0x1ADE6`. It advances the shared `FFE0B0`/`FFE0AE` countdown and phase, appends sprite key `0x780`, resolves the phase entry from the ROM `0x1D970` table, and builds it at the shared `FFE04A`/`FFE04C` position with attributes `0x6000`. Phase four is terminal and clears the source completion marker without emitting another sprite; the native conversion leaves the sprite-data boundary explicit.

The adjacent title profile pair is now typed. `0x1BEBA` copies the source profile destination's dirty bit, selects the four-entry sprite source at `0x1C136` from the signed countdown, builds the two `0x15E`/`0xE0` sprites at `y=0xEE`, and on expiry runs `copy_title_profile`, behavior attachment, menu service, and world-effect initialization in source order. `0x1BF7A` is the separate signed timer service: values through `-8` select the seven-word profile offset, while values below `-8` reload from the current compact record's signed `+0x06` through ROM `0x1BFFC`. The source eligibility helper remains a callback boundary.

`0x1BB6A` is now `advance_title_noise_effect`. Its bit-2 active branch decrements `FFE0CA`, renders only when the low three bits are at least four, and finalizes by clearing source bits 2 and 0 before the behavior/menu callbacks. Its inactive branch arms bit 2 with a `0x3C` countdown at expiry; otherwise it consumes one source PRNG word, performs the exact two 16-bit rotates used by the ROM selector, and emits seven `0xE8,0xEC` sprites from the four source tables at `0x1DAB8`/`0x1DAC2`/`0x1DACC`/`0x1DAD6`.

`0x1BCE8` is now `advance_title_profile_motion`. The conversion separates the 13-frame profile phase selected from the `0x1BE4E` records from the later 60-step movement path. The movement path preserves the source `0x15E56` easing lookup, signed fixed-point scaling, `0x1BE3E` four-frame sprite selection, the `0x0F` secondary countdown, and the two finalization entry points that clear `FFE058`/`FFE0D6` and conditionally publish `FFE0F4` before the menu refresh boundary.

`0x1C61C` is now `advance_title_sweep_animation`. It decrements the record's
byte delay, advances through the 32 four-byte records at `0x1C6D4`, resolves
one of the six direct sprite streams at `0x1C6BC`, and emits the same stream at
`base_x + signed_offset` and `base_x - signed_offset` with attributes `0x6000`.
The typed conversion now contains every source index, signed offset, and low
delay byte instead of requiring a resolver for the normal path. Source bit 3
suppresses the pair without suppressing timing; phase `0x20` clears bits 0 and
3, attaches behavior B, clears `FFE0D6`, and returns through menu-selection
code four. The source reads the low byte immediately after the 32-entry table
before that terminal branch; the conversion preserves the resulting `-1`
delay as state without treating code bytes as runtime data.

The paired title-layer path at `0x1C3FA`/`0x1C506` is now typed. The outer
routine updates the secondary (+0x08 descriptor) phase first and the primary
(+0x04 descriptor) phase second, then renders the primary pointer as A2/the
first layer and the secondary pointer as A4/the second layer; the alternate
path uses fixed sources `0x1D428`/`0x1D452`. The renderer emits both layers at
the record position and optionally applies the four direct status-offset rows
at `0x1C5FC`, `0x1C604`, `0x1C60C`, and `0x1C614` to both layers. Its effect row
is the source signed `FFE169 - 1`, clamped only above three and then masked by
three, while its column is `FFDF88 & 3`. It invokes the source double-build
gate only when `FFE084` is nonzero and the low selector nibble is at least
eight. The focused regression covers the layer order, all four extracted
offset rows, and the timer-zero signed wrap to row three.

`0x1C866` is now `advance_title_effect_motion`. It decrements `FFE066` at a
four-tick cadence, advances twelve frames, and resolves the direct sprite
stream from the exact 12-entry table at `0x1C926` or `0x1C956`. The table
choice preserves the ROM's nonzero `FFE0E2` versus `FFE060 >= 0x120` split;
each selected stream is built at the current `FFE060`/`FFE062` position with
attributes `0x2000`. At frame twelve it installs `0x1C288` into the secondary
record slot (`FF1588`, stage `FF158C`) on the left path or `0x1C28E` into the
primary slot (`FF1584`, stage `FF158D`) on the right path, clears
`FFE0D6`/`FFE05E`, and services the menu.

`0x1CA12` is now `advance_world_effect_motion`. It finalizes when the
decremented `FFE070` word is negative; otherwise it advances `FFE072`, selects
the exact byte from `0x1CA8C`, applies the ROM's +5 source-index adjustment
only for nonnegative `FFE06C`, and resolves one of the nine direct streams at
`0x1CAAC`. It then adds `FFE06C` to fixed-point X, subtracts `0x1A` from
fixed-point Y, shifts both by four, and builds with attributes `0x2000`.

The record helpers behind the adjacent title/world path are now typed as
`0x1D048` `advance_world_effect_record` and `0x1D0DE`
`resolve_world_effect_sprite`. The first owns the mixed byte/word record's
60-step lifetime, 20-frame source-table timer, exact 64-entry `0x15E56`
easing data, fixed-point movement, and three-phase animation counter. The
second selects the direct `0x1D134`/`0x1D11C`/`0x1D128` source table from the
record variant and returns shifted host coordinates; neither helper exposes a
ROM-backed runtime memory model.

`0x1CB6A` is now `spawn_world_effect_record`. It chooses the normal or cleared
work record, issues audio command `0x17` through a callback, marks the record
active, seeds its source variant and timers, and derives the signed 60-step
fixed-point velocity from the offset source position to the destination
record. The native model exposes those record fields directly instead of
recreating the original address-selected A1 record.

`0x1CC24` is now `advance_persistent_world_effect`. It advances the shared
effect record, updates the record's six-slot ring byte (+0x08) on the source's
`FFDF88 & 3 == 0` cadence, writes the selected direct sprite stream and
shifted coordinates, then draws seven wrapped entries beginning after the
selected slot with attributes `0x6000`. Expiry clears `FFE0D6` and returns
through the menu-selection variant boundary.

`0x1CD5A` is now `advance_title_secondary_effect_service`. It selects the
direct `0x1D158`/`0x1D140`/`0x1D14C` stream table, advances the three source
timers, applies the record's `-8/-0x0E` fixed-point motion before the main
`0x2000` build, and preserves the final moved build on expiry. Variant 0 and
variant 1 seed the paired/expanded position records at their source frame
boundaries; only variant 2 calls the six-position and paired-position motion
helpers before returning to menu progression.

`0x1D280` is now `initialize_world_sequence`, paired with the existing typed
`advance_world_sequence` implementation. It seeds the source table token
`0x1D428`, source buffer token `0xDA038`, destination `0x811B`, row range
`0x70..0xAF`, current row `0x51`, four-frame countdown, and inactive substage
state exactly once at the native boundary.

`0x1E43C` is now `handle_world_activity_request`. It clears the four request
side fields, consumes the pending flag, scans at most the source's thirteen
effect records for the active/high-effect combination, publishes availability
as `0` or `-1`, installs handler token `0x1F16A`, and begins world activity.

`0x1E166` is now `trigger_world_effect_sequence`. Its native gate preserves
the current-entity protection bit, activity/placement locks, sequence-mode
override, and the exact `FFDF88 & 0x3FF == 10` cadence. A successful random
threshold selects the source's forward-existing-sequence, begin-activity, or
spawn-new-sequence branch.

`0x1CB20` is now `advance_world_effect_activation`. Its typed state preserves
the bit-6 secondary-record seed (`0x1400/0x1070`, `-8/-14`, and the source
timers), the negative setup-marker path (`0x134`/`0x102`, behavior token
`0x1C27C`, audio command `0x16`), the motion-initializer branch, and the
`D5=0x20`, `D6=-0x28`, `D1=0` spawn boundary. The source's cross-entry tail
adjustment remains an explicit callback rather than a hidden address alias.

The next title/world-effect pair is also no longer generic. `0x1A818` starts
the descriptor transition, choosing between its six-frame loading path and the
active descriptor-token path; its source word `0xD280` is recorded as such
rather than using the stale unsigned Ghidra rendering. `0x1A908` selects the
effect mode, handles the fixed negative descriptor and the `0x45` termination
sentinel, arms modes 6 and 7 once, and initializes the shared work record at
`FF163E`. The ROM descriptor table remains an explicit data-resolution boundary,
which keeps this conversion useful without turning source pointers into native
runtime address arithmetic.

The three following overlay leaves are now typed as well. `0x1AB6E` and
`0x1AC6A` are the primary and secondary moving overlays, including their
source-exact flag equality checks, threshold behavior, transition cutoff, and
different cadence limits. `0x1AD42` is the background overlay with the intro
marker gate and its separate position shift. Their phase-table selections are
callbacks returning host data tokens; the conversion does not make the native
runtime decode or dereference the Genesis tables.

`0x1E69C` is the world-effect initializer preceding the indirect state
dispatcher. It clears the setup marker and effect globals, chooses one of
eight records in the current context group, derives the second random bound
from the record weight and `FF018C`, publishes the selected handler fields,
and invokes the selected initializer through the `F3DC6` table boundary. The
native conversion represents those records as typed host data and leaves the
initializer as an explicit callback.

`0x1DF34` resolves nearby projected world effects. It scans ten 0x12-byte
records with strict +/-0x100 coordinate bounds, delegates the interaction
code to `0x133A`, clears the active flag and services audio command `0x24`
for non-negative records, and routes interaction code `0x65` to the effect
slot cleanup boundary. The native version keeps those source decisions in a
typed record loop instead of exposing the original record addresses.

`0x1DD94` is the frame-level world coordinator. Its native conversion keeps
the source order: scene/activity gates and timers, low-mode service cleanup or
target selection, inactive-effect cleanup, optional terrain/trigger work,
world-effect triggering, and nearby-effect resolution. The calls are typed
service boundaries rather than a runtime copy of the original RAM schedule.

`0x1DC22` is the outer entity-service gate: it scans ten `0x100`-byte records
for `+0x11` bit 4 and only then enters `0x1DC4C` with the `0x678CC` cursor
sentinel. It is not a circular iterator. `0x1DC4C` releases the owner's
previous `+0xA2` target, derives a descriptor-aware start at `FF0400` or
`FF0100`, scans at most eight consecutive records without wrapping, and uses
different publication rules for normal and special modes. Normal selection
requires active/unlocked/unselected/non-high-effect/nonzero-action records and
writes the owner back-link; modes `0x0B`, `0x04`, and `0x0C` use the active-only
path and preserve the service marker. `0x1DEA0` independently clears only
bit-6, non-bit-3 records with a zero action word.

The larger `0x1AA40` leaf is now named `tick_world_effect_animation` and
converted separately. Its normal path chooses runner-derived timing versus the
fixed descriptor-state cadence, its loading/transition path uses the six-phase
`0xF1C46` sequence and reset boundary, and its final order is sprite build then
the three overlay updates. The decompiler's indeterminate D1/A0 values are
represented as explicit timing and sprite-data callbacks rather than guessed
register state.

The next compact group is also named and typed: `0x1B282` is the Matrix
program-transition stage loop, `0x1B364` is the four-table title-figure pass,
and `0x1B45C` is the PRNG-driven paired word-stream animation. The latter's
two loop lengths and ROM limit table are preserved as data behavior, while
stream writing and menu finalization remain explicit callbacks. `0x1B7D0` now
uses the existing behavior-record initializer name everywhere in the export.

### World-effect controller source correction: 0x1A370..0x1AC50

The manual ROM pass corrected the generated Ghidra body for `0x1A370`. The
entry gate is the active-work word at `FF163E+0x0C`, not the animation-active
flag at `FF164A`. The descriptor-transition branch is selected by `FFE082`;
its byte cadence decrements `FF163E+0x04` and `+0x06`, streams the current
phase through the vblank/transfer/noise boundary, and calls `0x1A8F4` when the
phase expires. The ordinary branch uses the return from `0x1A538`, starts
`0x1A818` for an inactive compact-runner effect, and calls `0x17E78` before
mode-specific setup. The generated C reuses the `FFE082` predicate after the
`0x1A538` call, so its printed normal branch is not a faithful control-flow
description.

The adjacent leaves were also separated by their actual writes. `0x1A54E`
initializes only the default mode, position, and countdown fields;
`0x1AC50` writes the independent `FFE084/FFE086` threshold pair; and
`0x1A8F4` clears the transition latch, requests exit, and clears the active
work record. The conversion no longer represents those leaves as one generic
effect structure or invents velocity/lifetime/palette writes.

### Title/world sprite argument correction: 0x1A570, 0x1A5A0, 0x1A7BE

The direct listing shows that `0x1A570` is two distinct calls to the common
sprite-list builder, not an argument-free repeated rebuild. It passes source
list `0xF1B1E` at `(0x012A, 0x0100)` and source list `0xF1B28` at
`(0x0115, 0x0100)`, with builder flags `0x2000` for both. The converted
world-mode branch now emits two `TitleSpriteBatchBuildRequest` values in that
order.

`0x1A7BE` selects the eighth-through-eleventh entries of the `0xF1AEE`
pointer table using `((FFDF88 >> 1) & 3) + 8`, appends `0x6E0`, increments
the fixed-point Y field by `0x60`, and stops on the unsigned `Y >= 0x1280`
boundary. The live build coordinates are both shifted right by four and the
attribute-offset builder receives key `0x6E0` with flags `0xA000`. The typed
conversion exposes this as `TitleVerticalSpriteBuildRequest` and does not
replace the source table selection with a generic vertical callback.

Finally, mode 6 of `0x1A5A0` calls the existing `0x5668C`
`advance_menu_selection_service_and_refresh` service before clearing the
world-effect mode. That boundary is now named accordingly in the conversion.

### World/title scene-loop correction: 0x1ADE6

The scene-loop source order now names `0x19788` as
`advance_matrix_program_loading`. The two persistent-effect callees at
`0x1CCDC` and `0x1CC24` are unconditional in the ROM, so the conversion no
longer places them behind invented outer activity flags.

The source uses different conditions for the shared `FFF0B1` status byte:
`BMI` after the world-effect controller and `BNE` during the field refresh.
The typed state retains the signed byte so a positive field-refresh status is
not silently treated as clear. The prologue also leaves the external
`FFF0A1` noise bit, `FFE0DE` Matrix-transition request, and pre-existing
`FFEF14` dirty bits intact, matching the writes actually present in the ROM.

### World-effect animation/overlay branch correction: 0x1AA40..0x1AD42

The ROM uses `FFE0F2`, not the combined predicate `FFE0F2 == 0 &&
FFE082 == 0`, to select the transition-table branch. With `FFE0F2` clear,
`FFE082` only suppresses the normal frame-cadence update and sprite-record
append; it does not switch the sprite data to `0xF1C46`, and the source still
calls the primary, secondary, and background overlay routines. The converted
animation tick now follows that source control flow.

The overlay sprite builders now expose the exact source table and coordinate
origins: `0xF1BDE`/`0xF1BCE`/`0xF1BBE` for primary/secondary/background,
`D1=position>>6` for the moving overlays, `D1=position>>4` for the
background, `D2=0xB9` or `0xD0`, `D4=0x7A8`, and `D6=0x2000`. The primary
phase limit is also corrected to the source's valid `0..0x19` range; the
secondary and background ranges remain `0..3`.
