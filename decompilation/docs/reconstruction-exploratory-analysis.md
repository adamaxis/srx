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
Field area loop (0x3084)
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

The main field loop at `0x3084` loads the active mode, updates the field state,
and calls the movement/reaction chain. The relevant pieces are:

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

Walking is driven by source direction/state tables, not a host “move one frame
every N milliseconds” animation. The source uses the animation/resource tables
around `0xC042`, `0xC072`, `0xC082`, `0xC092`, `0xC0A2`, `0x15318`, `0x15336`,
and the entity state/phase tables at `0x6AB3E`, `0x6AB96`, `0x6ABEE`, and
`0x6AC46`. The field animation path also consults `0x13E58`, `0x13E7C`, and
the timing tables at `0xD4540`/`0xD4EA0` for object-specific cadence.

The native port must select frames by the source direction and state marker,
keep the source timer/phase, and return to that direction's idle frame when an
accepted movement step ends. A single reused walking image or a mirrored
frame for every direction is not source reconstruction.

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
| `0x13`, `0x14`, `0x16` | separate use/error path at `0x121C0` |
| `< 0x18` otherwise | weapon/gear equip path at `0x121E0` |
| `0x18..0x2B` | separate item-action path |
| `0x2C+` | armor path |

The weapon/gear path uses `+0x56` and `+0x57` as equipment state. It converts
ammunition using the selected item record's factor at `D4540 + item*0x30 +
0x24`, preserves source quotient/remainder behavior from `0x125FA`, and
stores the result as a source byte. Selecting the already equipped slot writes
`+0x56=0x7F`, clears `+0x57`, and returns the source state to “no weapon”.

The armor path at `0x1219C`/`0x121B0` uses `+0x76` and the selected slot's
`+0x6E` phase byte. A second activation clears the equipped armor state. The
presence of an armor code in a cell alone does not equip it.

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

The negative `+0x56` special attack branch at `0x12D7C`/`0x12FA4` and the
field AI/combat consumers remain `PENDING`.

## 9. The remaining command panels

### Clips: `0x11B58`

The clips panel is an action panel, not a passive ammunition display. Its
source code draws the clip rows at `0x11B58`, then dispatches the selected row
through the action branches at `0x11CCA`, `0x11DD2`, and `0x11FDE`.
`0x12062` is the empty-clip gate. A row with no usable clip reaches the source
error/result path; it must not silently refill, disappear, or become an
equipped weapon. The exact item-specific effects and message timing remain
`PENDING` until the branch writes and the corresponding message tables have
been traced together.

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

The Magic command first checks the runner class at `+0x9B`; the ordinary magic
branch requires a class value below 2 and a nonzero magic/resource byte at
`+0x7F`. It then enters the source magic
panel through `0x11900`/`0x12C36`, with its own spell cursor and panel state
(`FFF10E` is part of that state). Magic is therefore not an alias for the
cyberware panel and must not be enabled merely because the command selector
can reach index 5. The display shell is identified; spell availability,
selection, costs, effects, and failure messages remain `PENDING`.

### Pocket Secretary: `0xC2A6`, Group Items: `0xC8C8`

Pocket Secretary draws through `0xC3E6`, with the source option/label table at
`0xC1418`; the cursor is maintained at `0xC376`. The source debug/state byte
at `FFF101` changes the visible bounds between four and five entries, which is
evidence that this panel has state-dependent contents and is not a decorative
list. The native base presents the verified five-row field-mode order and sends
Save/Load to the existing `0x59172` screen.

Selecting Group Items dispatches to `0xC8C8`. That handler reads the persistent
global bitfield at `FFFC22`, emits the names from the fifteen-entry pointer
table at `0xC25C4` in descending bit order, or writes the exact empty message
at `0xC1471` when no bits are set. It then waits through `0xF312` for any
controller edge and returns to the command loop. The native port has extracted
the catalog into the native state data and covers the empty-catalog path; event
setters for the persistent bits remain pending.

The Cyberdeck row dispatches to `0x18150`. The first instruction tests
`FF018F` bit 0, the source deck-present flag. When it is clear, the handler
calls `0xC3E6`, passes `0xE43D1` to `0xC0DA` with destination `0xC40E`, and
returns to the Pocket Secretary loop. The three fixed source strings are:
`" You do not have"`, `"  a cyberdeck!!"`, and `" Keep looking."`. The native
port extracts those lines and keeps the Pocket Secretary open while displaying
them. The inline branch beginning at `0x1817E` is now source-traced as the
deck-present entry shell: it prepares/copies the Matrix resource block at
`0xE2E3A`, establishes ten menu slots, enters `0x1822C`, then refreshes the
mode data/palette and restores field resources. The `0x1822C` deck/program loop
and its state tables remain a separate audit target; a generic Matrix
placeholder is not an acceptable substitute.

Dismiss Runner dispatches to `0xC448`. It writes the fixed lines at `0xC1499`
(`Dismiss`), `0xC14A4` (`Who??`), and `0xC14BD` (`Abort`), then checks the
active-record bit at `+0x10` while enumerating the source runner records. The
selector starts at zero; `0xC376` clamps it to the active-list bound, and the
zero row returns without changing party state. A nonzero selection resolves a
live runner and enters the source dismissal event path (`0x6AA7E`/`0x33AA`),
after which that record is no longer eligible as an active party member. The
native port extracts the fixed labels, lists live non-current host records,
preserves the Abort boundary, and clears the selected record's active bit.
The source event's narrative/audio side effects remain a separate boundary.

Notebook dispatches to `0xF236`. Its setup helper `0xF33E` writes the title
and two ungated rows from `0xC567E`, then appends `Contacts`, `Mr. Johnsons`,
and `Shadowrunners` only when `FFFC2C`, `FFFC27`, and `FFFC2A` are nonzero.
The New Game initializer at `0x59C48` stores `0x1C7F` in `FFFC2C`, so a
fresh post-opening session has Contacts visible; the native state data carries
that initial gate rather than hiding the row.
The copied action table at `0xC6648` resolves those rows to `0xF2E2`,
`0xF9A4`, `0xF840`, `0xF416`, and `0xF6DC`; `FFF0B9` is the inclusive
selector bound and `FFF0DC` is the cursor. The native port now extracts all
six labels and the `0xC56E7` three-line response. In a fresh session, where
the current-run flag is not active, selecting `Current Run` follows `0xF2E2`
and displays `You are not currently / engaged in a shadowrun. / Get a job!!`,
then waits for an input edge before rebuilding Notebook. The Tips & Clues
pointer lists and the three conditional event setters remain explicit next
boundaries; they must be ported from their source tables rather than replaced
with an always-visible invented list.

The Tips & Clues handler at `0xF9A4` is backed by three 32-entry pointer
tables: `0xC2716` selected by `FFFC30`, `0xC2796` by `FFFC34`, and `0xC2816`
by `FFFC38`. It starts `FFF0B9` at `0xFF`, walks each table pointer in order,
and tests D7 from 31 down to 0; therefore table entry `n` is enabled by flag
bit `31-n`. Set pointers are appended to `FFEF56` and use the same
`FFF0DC`/`C376` cursor family as the other Notebook screens. The native
extractor resolves all 96 pointers and converts each source `0x80` separator
to a host newline in native state data. The native Tips screen now preserves the
three source flag longwords, builds the MSB-first visible page list, writes the
source page counter boundary from `C663A`, renders the selected entry's source
lines, and returns on A/C/Start; B follows the already ported `0x391A` runner
switch path and redraws the page. Story/event writers that set the three tip
flag longwords remain pending, so a fresh decoded session correctly has zero
visible tips until a test or future event path sets them.

The conditional Mr. Johnsons handler at `0xF416` is a separate five-name
screen, not another Tips page. It writes the title from `0xC56AE`, then tests
bits 0 through 4 of `FFFC27` and emits these source-order names into its
dynamic list: `Mr. Gunderson` (`0xC5728`), `Julius Strouther` (`0xC5755`),
`Mortimer Reed` (`0xC5766`), `Vigore & Jarl` (`0xC5736`), and
`Caleb Brightmore` (`0xC5744`). Its selector starts at zero and uses the same
`C376` bound/cursor family, but A writes the selected entry through a separate
`C38C` destination; C returns, B calls `0x391A`, and Start continues the loop.
The five fixed names are now extracted into the organized Notebook state data
with their source order preserved. The native list/detail loop and the story
writers that set `FFFC27` remain pending until the omitted middle block is
resolved against the bytecode.

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
it linearizes the selected row/column as `row * 4 + column`, without exposing
the original FFF0D0/FFF0D1 addresses to host code.
The `0x1882E` panel path now has the corresponding typed gate: indices 0..11
are available only when their FFFBCC flag is nonzero, while indices 12..15
bypass that table; only the first twelve enter the runner-detail/stat branch.
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
remain separate boundaries.
The source HUD-mode leaf at `0x19BAE` is now converted separately from the
Matrix UI: a selected 0x14-byte record is eligible only when `+0x0F` is
positive and signed `+0x13` is nonnegative; alternate mode interleaves a
second table entry only for codes 7..16. The unresolved `0x17C3C` adjustment
used by the larger `0x19B40` wrapper remains an explicit boundary.
The Matrix state table, program actions, and rendered resources inside the
loop must still be recovered before claiming the Matrix panel complete. Until
then the Matrix hill remains `PENDING`, rather than represented by a generic
screen.

## 10. Interaction, story, shops, and combat

### Field interaction and story dispatch

Field actions do not directly select an arbitrary destination. The native
dispatch writes an event/state value to `0x1F580`; that value indexes the
source transition table at `0x12CCB4`. A second state-pair table at `0xF3E1E`
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

The reviewed mode-2 event-0 tuple is now wired through the native state machine:
the raw `0xC0` cell selects destination mode `0x14`, and the port enters a
source-text dialogue state using the callback/text resources at `0x20E48` and
`0x1167EB..0x116A79`. The dialogue can be advanced and returns to the loaded
motel map. This is intentionally a scoped Stoker slice; it does not claim that
the other building callbacks, NPC records, or reverse exits are complete.

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
| opening stages | `0x10`, `0x11`, `0x12`, `0x2E` | confirmed boundary, track identity pending |
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
| audio | partial | command boundary/ring identified; source timing and asset mapping pending |

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
