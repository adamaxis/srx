# Shadowrun Genesis screen and system audit

This is the working reconstruction contract for the native port. The ROM
listing and extracted data are authoritative. A screen is not considered
reconstructed merely because it has a similar-looking panel: its entry path,
cursor state, input dispatch, source records, exits, and side effects must be
accounted for.

The detailed source-oriented exploration, evidence ledger, and reconstruction
worksheet are maintained in [reconstruction-exploratory-analysis.md](reconstruction-exploratory-analysis.md).
That document is the required audit map for states and implementation order;
this file remains the concise screen contract.

## Global rules

- The source runs one screen loop per VBlank and stores cursor/focus state in
  RAM. The native update loop preserves the audited state transitions and
  adapts VDP writes to RGBA screen writes. Unaudited handlers remain explicit
  boundaries; they are not treated as reconstructed merely because a panel is
  visible.
- Genesis palette indexes are converted to host RGBA during extraction. The
  native runtime does not emulate VRAM, the VDP, or a 68000.
- A source record is not automatically a party member. Eligibility comes from
  the source record flags and roster initialization, not from the existence of
  a coordinate or portrait resource.
- The first three records at ROM `0x7291A`, `0x72A1A`, and `0x72B1A` are
  Joshua's Samurai, Decker, and Gator Shaman templates. `0x42B0` copies one
  template into Joshua's live record. They are not three runners for C to
  cycle through. Recruited roster records begin at `0x72C1A`.

## Startup and title screens

### Title (`0x0BF2`, `0x1A9E`, `0x1C6A`, `0x1DCA`, `0x1E4E`)

The title loop clears the display, waits through the source reveal delay,
writes the logo columns incrementally, advances the sprite list on its own
cadence, writes the copyright rows late in the countdown, and flashes PRESS
START from the countdown low bits. Start is an edge-triggered exit into the
startup options block at `0x59092`.

Native status: the title layers, reveal timing, sprite cadence, copyright,
and PRESS START timing are extracted and implemented. The remaining audit
item is frame-by-frame visual comparison of every sprite-list phase against a
Genesis capture; a settled title screenshot is not sufficient evidence.

### Start Options (`0x59092`, layout `0x1B6498`)

Options are exactly:

1. `New Game`
2. `Load Game`

The selector is a single byte clamped to `0..1` by the shared menu helper at
`0xC376`. Up/Down changes it; A dispatches the selected option. New Game
clears the load state and proceeds to the archetype screen. Load Game enters
the save/load slot screen; it does not silently become New Game when a slot is
empty.

Native status: source layout text, cursor asset, two-choice bounds, and the
New/Load dispatch are implemented.

### Save/Load (`0x59172`, layout `0x1B6518`)

The source screen presents the extracted save/load layout and a two-position
selector. A checks the selected SRAM slot. An invalid/empty slot stays on the
screen and writes the no-save message from `0x1B6527`; a valid load sets
`FFE108`, restores the saved state, and skips the new-game opening. Start/B
returns to Start Options.

Native status: the two-position selector, empty-slot result, source message,
and return path are implemented. SRAM serialization and valid-load state
restoration remain pending; until then every slot check is intentionally the
source-visible empty-slot result.

### Choose Archetype (`0x59248`, layout `0x1B64CF`)

The source clamps the selector with `D3=2`, so there are three choices:

1. `Samurai`
2. `Decker`
3. `Gator Shaman` (the extracted layout writes `Gator` and `Shaman` on
   separate rows)

A/B confirms the selected template. The call to `0x42B0` copies the selected
   0x100-byte Joshua template into `0xFF0100`, then the opening path starts.
There is no standalone Shaman choice and no later command-screen operation
that changes Joshua's archetype.

Native status: three-choice bounds, exact split Gator/Shaman layout, selected
template storage, and one-time Joshua initialization are implemented.

### New-game opening (`0x59C5E`, `0x59D7C`, `0x5AA96`, `0x5AD1A`)

New Game runs timed presentation groups and may be skipped with the source
input edge. The recovered text groups begin at host frames `0x000`, `0x0A0`,
`0x290`, `0x3A4`, and `0x6EC`; the full sequence ends at `0x8CC`.

Native status: source text, timing groups, and skip behavior are implemented.
The original cutscene's animated artwork, sprite choreography, and music are
not yet reconstructed. The native text-only opening must not be presented as
the finished intro.

## Field and command screens

### Field loop (`0x3084`, `0x2B34`, `0x391A`, `0x5A62`, `0xB7B8`)

The field loop projects the controlled record, probes the four-corner terrain
footprint, updates movement state/animation, dispatches raw interaction cells,
and changes areas through the mode event table. Start enters the command
screen without resetting the selected record or camera. C calls `0x391A` and
searches the initialized roster for the next eligible active record. If no
other record passes the flag tests, the source produces the action sound but
leaves the controlled runner unchanged.

Native status: movement, collision/event transitions, extracted area maps,
walking animation state, Start entry, and no-eligible-runner C behavior are
implemented. Recruited roster population and NPC/entity interaction systems
remain incomplete.

### Command overview (`0x10D30`, `0x10F50`, `0x111E8`)

The command screen starts with selector `FFF0D0=6` and row `FFF0D1=0`. Left
and Right wrap the seven selector values:

| selector | source label | behavior |
| ---: | --- | --- |
| 0 | Inventory | left inventory column; row 0..3 |
| 1 | Inventory | right inventory column; row 0..3 |
| 2 | Clips | opens the clips loop |
| 3 | Attributes/Skills | opens the display loop |
| 4 | Cyberware | opens the display loop |
| 5 | Magic | source magic handler |
| 6 | Pocket Secretary | source pocket-secretary handler |

The source writes only the selected command label at the bottom command
position; it is not a vertical checklist of all seven commands. The overview
reads the active record's race, archetype, physical/mental values, active
item, attack/defense tables, armor record, weapon/gun bars, five-state
posture, and inventory cells.

Native status: source background geometry, record-derived overview fields,
inventory boxes/icons/art, selected-cell overlays, source-derived attack and
defense strips for the normal equipped-item branch, and command selector state
are implemented. Magic and Pocket Secretary are still only their selected
labels; their source handlers need a dedicated audit. The negative `+0x56`
special attack branch and the field combat consumers of these values remain
explicitly incomplete.

### Inventory (`0x11506`, `0x11564`, `0x11592`, `0x11678`, `0x12148`)

Inventory is an 8-cell, two-column by four-row grid. The selector is a source
cursor sprite over a box, not a checklist. Up/Down clamps the row to `0..3`;
Left/Right chooses the column. The selected item is read as
`record + 2*row + column`. A dispatches the source item action, including the
weapon/gear equip toggle and armor branch that updates the record's equipped
armor field; it is not a generic "check" action.

Native status: cell geometry, cursor, extracted item icons/art, selected-item
details, row/column state, and the source weapon/gear and armor state changes
for the audited `0x12168` path are implemented. Possession and equipment are
separate: an item in one of the eight cells is not active until the source
equip action sets the live record's equipped slot/item bytes. The other item
action branches (`0x12634`, `0x12836`, and `0x11040`) are still not ported.

### Posture (`0x11928`, `0x119CE`, `0x11ABE`, `0x11AE4`)

When the command selector is `2..6`, Up enters the dedicated posture focus.
The five source values are:

`Full Defense`, `Med. Defense`, `Neutral`, `Med. Offense`, `Full Offense`.

Left/Right mutates the active record's byte `+0x4D`, clamped to `0..4`, then
calls `0x11ABE`. That routine redraws the posture strip and calls `0x12CEE`
to recompute the attack/defense source bars. This is a gameplay state used by
the field behavior, not a decorative slider. B returns focus to the command
bar; Start exits the command screen.

Native status: the active posture focus, source five-state values, clamping,
record persistence, rendering, and the normal `0x12CEE` attack/defense
calculation are implemented. The native results match the Joshua source record
for all five posture values (`attack 3,4,5,6,7`; `defense 19,18,17,16,15`).
The negative `+0x56` special branch (`0x12D7C`/`0x12FA4`) and the field
AI/combat code that consumes posture are not yet reconstructed, so their effect
outside the audited command path remains pending.

### Clips (`0x11B58`, `0x12062`)

The clips screen has its own selector loop and uses the active record's clip
and ammunition bytes. A dispatch is gated by the extracted clip count; an
empty count produces the source no-ammunition result rather than changing the
weapon state.

Native status: selector bounds, extracted ammunition fields, action gate, and
the native transition are implemented. The final source-authentic panel art
and exact action text still need the same asset extraction treatment as the
overview.

### Attributes/Skills (`0xD01C`)

This is a display loop over the source attribute/skill tables and the active
record's value bytes. It has a two-column cursor and row movement. The source
also contains upgrade/mutation branches whose A/B/C semantics are not yet
fully traced.

Native status: source names, values, auxiliary bytes, two-column navigation,
and read-only rendering are implemented. Mutation behavior is intentionally
disabled until those branches are traced.

### Cyberware (`0x11950`)

This handler is display-oriented: it reads the active record's fifteen-bit
cyberware field at `+0x90` and resolves installed entries through the source
name table.

Native status: bitfield extraction, source names, and installed-state display
are implemented. Purchase/install/removal flows, if reached through another
source path, remain to be traced.

## Command-screen dispatch map

The command bar is a dispatcher, not a collection of decorative buttons. The
seven labels at `0xD1D28`/`0xD1D3A` are selected by `FFF0D0`; the indirect table
at `0xD2A0A` routes the five non-inventory commands as follows:

| selector | source label | handler | current boundary |
| ---: | --- | ---: | --- |
| 2 | Clips | `0x11B58` | native selector, ammunition gate, and return path |
| 3 | Attributes/Skills | `0xD01C` | native read-only two-column view |
| 4 | Cyberware | `0x11950` | native installed-bit display |
| 5 | Magic | `0xFBA6` | source handler still to be decoded |
| 6 | Pocket Secretary | `0xC2A6` | source handler still to be decoded |

The second indirect table at `0xD2A1E` is the selected-inventory action
dispatch. `0x12168` is the item/armor path; `0x12634`, `0x12836`, and
`0x11040` are separate item-action branches. They must not be collapsed into
one generic checklist action. In the audited `0x12168` path, item codes below
`0x18` take the weapon/gear or special-use split, item codes `0x18..0x2B`
take the separate item-use path, and item codes `0x2C+` take the armor path.
The first two selector values are both labeled `Inventory`, because they
identify the left and right columns; Up/Down changes the row and Left/Right
changes the column.

## Pocket Secretary systems

The Pocket Secretary is a second-level menu under command selector six. The
source text table at `0xD1D94` and `0xD1E6C` supplies the two visible menu sets;
the source handler also checks the selected runner's class and the field mode
before dispatching an option. The documented gameplay options include runner
and contact information, group items, cyberdeck access, dismiss runner, and
save/load. Save/Load is disabled in buildings and during combat. A blank entry
under Save/Load is an intentional debug-menu path in the shipped ROM, not a
missing label to fill with a made-up option.

Native status: the command selector reaches the source Pocket Secretary label,
but its submenu rows, class/mode gates, save/load effects, runner/contact
records, and debug path are not yet ported. The next pass must recover the
`0xC2A6` loop and its text/table pointers before presenting those choices.

## Magic, Matrix, and combat screens

Magic and Matrix are separate systems. The Magic command is only legal for a
runner whose record supports magic; its screen operates on the runner's spell
list and spell levels. The cyberdeck command opens a different program/deck
screen with its own cursor, load state, program memory, and A/B/C posture
launch semantics. The overhead field combat loop is also not the command
screen: it selects targets, consumes ammunition or spell drain, updates health,
and uses the runner's posture to bias offensive/defensive behavior.

The manual and the source both make posture consequential: the five settings
are Full Defense, Med Defense, Neutral, Med Offense, and Full Offense; computer
controlled runners use that setting to decide whether to hang back/heal or
advance/attack. The native command screen now updates the source attack and
defense strips for all five settings in the normal equipped-item branch. The
special negative-`+0x56` calculation, field/combat consumers, spell drain, and
the Matrix program loop remain explicit implementation boundaries rather than
placeholder UI.

## Field interaction, buildings, and area transitions

The field is an entity/collision system, not a free camera over a background
image. The movement loop at `0x3084`/`0x2B34` projects the active record,
probes the four corners through `0xE7C8`, advances its directional animation,
and then dispatches the raw terrain/event byte. A doorway's event index is
looked up in the active mode table at `0x172CDC`; it is the combination of the
current area, raw cell, reaction substate, and destination descriptor that
chooses the next area. A generic "building" transition is therefore invalid.

For the Redmond Barrens startup map, the reviewed C0 doorway at the motel
entrance selects mode `0x14` (Stoker's Coffin Motel). Ares Weapon Emporium and
other buildings have different event cells and must be reached through their
own source table entries. The native area catalog currently contains the
reviewed mode-2 destinations and the motel transition. The mode-2 event-0
tuple now enters the source Stoker interaction state after loading mode `0x14`;
other building interiors, exit reaction context, shops, and combat triggers
still need their individual source dispatches.

## Audit matrix

This is the implementation boundary for the current native build. `Implemented`
means that the listed source path has been traced to its native counterpart and
has a focused test or source-record check. `Partial` means that only the named
subpath is present. `Pending` means that a visible placeholder is not evidence
of a port.

| System | Source path audited | Native status | Explicit boundary |
| --- | --- | --- | --- |
| Title | `0x0BF2` and reveal/sprite helpers | Partial | Needs frame-by-frame sprite comparison and final audio timing |
| Startup options | `0x59092`, `0xC376` | Implemented | Source layout and New/Load dispatch are covered |
| Save/load | `0x59172` | Partial | Empty-slot result is covered; SRAM restore is pending |
| Archetype selection | `0x59248`, `0x42B0` | Implemented | Only the three Joshua templates are selectable |
| Opening | `0x59C5E` and timed groups | Partial | Text/timing/skip are present; animated art and music are pending |
| Field movement | `0x3084`, `0x2B34`, `0xE7C8`, `0x5FDC`, `0x600E` | Partial | Movement, four-corner collision, eight directional two-frame states, and source timer classification are present; NPC/entity logic outside the reviewed player path remains pending |
| Area transitions | `0x5A62`, `0x172CDC`, `0x20E48` | Partial | The reviewed mode-2 C0/event-0 tuple loads mode `0x14` and enters the source Stoker dialogue state; every building reaction and reverse transition is not |
| Command overview | `0x10D30`, `0x10F50`, `0x111E8` | Partial | Normal overview and selector are present; Magic/Pocket Secretary handlers are pending |
| Inventory navigation | `0x11506`..`0x11678` | Implemented | Eight-cell cursor and source column/row semantics are covered |
| Equipment action | `0x12148`, `0x12168`..`0x1228C` | Partial | Audited weapon/gear toggle and armor mutation are present; other item-action branches are pending |
| Posture and bars | `0x119CE`, `0x11ABE`, `0x11AE4`, `0x12CEE` | Partial | Normal equipped-item attack/defense calculation is source-derived; special negative `+0x56` path and combat consumers are pending |
| Clips | `0x11B58`, `0x12062` | Partial | Selector, ammo fields, and empty gate are present; final action panel remains |
| Attributes/skills | `0xD01C` | Partial | Read-only display is present; mutation branches are pending |
| Cyberware | `0x11950` | Partial | Installed-bit display is present; mutation flows are pending |
| Runner switching | `0x391A` | Partial | No-eligible-runner behavior is covered; recruited roster is pending |
| Pocket Secretary | `0xC2A6` | Pending | Label only; submenu, mode gates, and effects are pending |
| Magic | `0xFBA6` | Pending | Handler and spell state are pending |
| Matrix/cyberdeck | source deck/program loop | Pending | Separate from Magic and command overview |
| Field combat | source target/action loops | Pending | Posture consumers, damage, ammo/spell drain, and animation are pending |

The equipment rows are intentionally separate from the inventory rows. The
source record has both possession cells and live equipped-state bytes (`+0x56`,
`+0x57`, and the armor field). Native code must preserve that distinction; it
must not infer that an item is equipped from its presence in a cell.

## Startup, persistence, and non-game screens

The source startup flow is stateful:

`Title -> Start Options -> Save/Load or Choose Archetype -> timed opening ->
Field`

Start is an edge-triggered exit on each of these loops. A failed SRAM check
stays on Save/Load and writes the source no-save message; it does not silently
start a new game. A valid load sets the load flag and bypasses the new-game
opening. New Game selects exactly one of the three Joshua templates, copies it
to the live record through `0x42B0`, and only then starts the opening. The
native state transitions and empty-slot behavior are present; SRAM contents,
valid-load restoration, and the full animated/music opening are not.

The title also has a source debug input sequence and the Pocket Secretary has
an intentional blank debug entry. Those are part of the shipped ROM's input
surface, but they are not ordinary gameplay options and will be reconstructed
only after their handlers and persistence effects are mapped.

## Reconstruction order

The next system passes should be completed in this order:

1. Finish source-authentic startup/title visual comparison, including the
   animated intro artwork and audio timing.
2. Extract and implement Magic and Pocket Secretary as stateful screens.
3. Decode the recruited roster at `0x72C1A` and populate runtime party slots;
   only then should C-switching expose additional runners.
4. Trace posture consumers in field/combat logic so posture changes affect
   behavior, not just the record and display.
5. Recover valid SRAM save/load and the remaining Attributes/Skills actions.

This list is deliberately explicit so new native UI code cannot introduce a
screen, option, or state transition that the ROM does not have.

## Correction ledger (2026-09-18)

The following source checks were completed during the current visual audit;
they are recorded here because each one corrects a previously invented native
behavior:

- Title input is now two-stage: the first Start edge settles the source reveal
  and the next edge enters startup options. The transient reveal sprite is
  removed once the source column counter reaches its terminal value. The
  title background comparison currently uses the dark circuit map at `0x1C064`;
  frame-by-frame timing and audio still remain to be verified.
- The startup archetype plane is cleared independently from the title plane.
  It now renders the source choices and cursor on black instead of retaining
  the title circuit background.
- The command overview keeps the inventory cursor separate from the source
  equipment highlight. `0x11624` highlights only the live weapon byte at
  `+0x56` and the inventory cell matching the armor byte at `+0x76`.
- The overview class text uses the three source template records at
  `0x7291A` (`Gator Shaman`, `Samurai`, `Decker`) rather than a made-up class
  list. The active runner remains the only initialized party record; the
  recruited roster is still pending.
- The item detail writer now follows the `0x115F2`/`0x12E9E` boundary:
  weapon Damage/Power rows are emitted only for item codes below `0x10`.
  Medkits and the later item categories therefore cannot acquire a weapon
  damage stat merely because a table record has a nonzero field. The adjacent
  `0x12B8E` branch now selects the source `ammo`, `uses`, and `rtng` labels;
  the value tile table at `0x6AC46` is still pending native asset extraction.
- Unequipping no longer removes the projected runner body. The item sprite
  selection path and the body projection path are separate source effects;
  the native fallback marker is not a character sprite.

These are correction slices, not a completion claim. The right-hand party
status/sidebar layer, recruited roster, source submenu layouts, animated
opening artwork/music, valid SRAM load, and remaining item-action branches
still require their own source traces and focused comparisons.
