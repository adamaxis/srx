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

Native status: extracted logo/circuit/text layers. The title starts black,
explodes the logo left-to-right, then fades in a wrapping circuit scroll.
Copyright after DD5A and PRESS START after D7<$C6C with inverted 10/64
blanking are implemented. Remaining: 0x10D6 palette interpolation and a
phase-matched Genesis capture. Live SAT compositing is not used at runtime.

### Start Options (`0x59092`, layout `0x1B6498`)

Options are exactly:

1. `New Game`
2. `Load Game`

The selector is a single byte clamped to `0..1` by the shared menu helper at
`0xC376`. Up/Down changes it; A dispatches the selected option. New Game
clears the load state and proceeds to the archetype screen. Load Game enters
the save/load slot screen; it does not silently become New Game when a slot is
empty.

Native status: 0x10B5A CRT frame, source layout text (command-menu green
font), cursor asset, two-choice bounds, and the New/Load dispatch are
implemented.

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

Native status: 0x10B5A CRT frame, large runner portraits that follow the
selector, three-choice bounds, exact split Gator/Shaman layout, selected
template storage, and one-time Joshua initialization are implemented.

### New-game opening (`0x59C5E`, `0x59D7C`, `0x5AA96`, `0x5AD1A`)

New Game runs timed presentation groups and may be skipped with the source
input edge. The recovered text groups follow the 0x5ADD0/0x5ADF0 script tables and the
stage DBF counters: Seattle hold `0x3C` then lines 0/1/2-4, wilderness from
`0x267`, massacre lines at `0x394`, tri-vid at `0x4F9`, nuyen at `0x60E`,
Sea-Tac at `0x956`; the sequence ends at `0xB37`.

Native status: source text, 0x5AF62/0x59D7C/0x5AA96/0x5AD1A stage switch,
skip, direct per-frame SAT choreography, source-positioned wilderness map overlays,
camera pan, and the airport/cab layers are implemented in the separate Ghidra
conversion Qt target. Independent Genesis visual comparison, source audio
playback, and the remaining fill/palette presentation leaves are not yet
defended.

## Field and command screens

### Field runtime (`0x075E`, scene load `0x3074`/`0x3084`, `0x2B34`, `0x391A`, `0x5A62`, `0xB7B8`)

The field runtime loop at `0x075E` projects the controlled record, probes the four-corner terrain
footprint, updates movement state/animation, dispatches raw interaction cells,
and changes areas through the mode event table. Start enters the command
screen without resetting the selected record or camera. C calls `0x391A` and
searches the initialized roster for the next eligible active record. If no
other record passes the flag tests, the source produces the action sound but
leaves the controlled runner unchanged.

Native status: the reviewed player movement/collision path now follows the
source `0x19D2` input-angle table, `0xB530` angle slew, `0xC002` fixed-point
vectors, and `0xE7C8` probe. Extracted area maps, walking animation state, Start
entry, and no-eligible-runner C behavior are present. Source camera transitions,
the right-side runner/HUD entity presentation, recruited roster population, and
NPC/entity interaction systems remain incomplete; the native build must not
present this as a finished field loop.

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
position; it is not a vertical checklist of all seven commands. `0x110BC`
submits the command-overview HUD as ten rows of eight words at `0xE104`, with
the tile word derived from the active record's `+0x3E` and its `+0x2C` passed
as the panel parameter. The overview reads the active record's race, archetype, physical/mental values, active
item, attack/defense tables, armor record, weapon/gun bars, five-state
posture, and inventory cells.

The five small command buttons are a separate source focus surface. ROM
`0x11454` selects a 2x2 tile block from `0xCD3F2` for selectors `2..6` and
writes it at x coordinates `128, 152, 176, 200, 224`, row `184`. That focus
surface is distinct from the lower selected-command label and from the
inventory cursor.

Native status: the separate Ghidra-conversion Qt runtime now enters this state
through the converted field-coordinator Start edge and keeps the converted
command selector/cursor state in a non-blocking host bridge. Source background
geometry, the typed-record-derived overview fields, inventory boxes/icons/art,
source command-tab focus blocks, selected-cell equipment overlays, and the
source-derived status strip are presented at their reviewed positions. The
diagnostic `--field-command-dump` exercises that same Start edge and writes the
resulting 320x224 frame. The normal command-overview path now also consumes
the converted typed equipment combat result: source-indexed Damage/Power and
Wpns/Guns rows, selected-item art, phase-value tiles, and the Atk/Def strips
are drawn from the direct resource catalog. The command, archetype, runner,
status, power, armor, item-detail, and item text catalog is extracted from the
reviewed ROM tables into `menu.field.text` and rendered with the extracted
menu font. Clips, Attributes, Cyberware, Magic, and Pocket Secretary still
need their source handlers and action-detail resources connected; this hill is
therefore partial, not defended.
The negative `+0x56` special attack branch and the field combat consumers of
these values remain explicitly incomplete.

### Inventory (`0x11506`, `0x11564`, `0x11592`, `0x11678`, `0x12148`)

Inventory is an 8-cell, two-column by four-row grid. The selector is a source
cursor sprite over a box, not a checklist. Up/Down clamps the row to `0..3`;
Left/Right chooses the column. The selected item is read as
`record + 2*row + column`. A dispatches the source item action, including the
weapon/gear equip toggle and armor branch that updates the record's equipped
armor field; it is not a generic "check" action.

Native status: cell geometry, the extracted 3x4 source cursor sprite from
`0xD29F6`, extracted item icons/art, selected-item details, row/column state,
and the source action loop are implemented. In the Qt conversion the visible
yellowish selector is a one-pixel outline directly on the 24x16 item art; it is
not the larger equipment-cell rectangle.
first A/B edge enters the four-choice `0x11740` loop; the second A/B edge
confirms its selected action. The native loop uses the source `Equip/Unequip`,
`Discard`, `Trade`, `Cancel`, and medical `First Aid` labels. Equip/Unequip,
Discard, and the reviewed Trade branch now route through their source-backed
native handlers. Possession and
equipment are separate: an item in one of the eight cells is not active until
the source equip action sets the live record's equipped slot/item bytes.
Medical items stay in their source-specific First Aid path and never become
weapons through the generic Equip/Unequip branch.
The reviewed utility/magic branch is also source-specific: `0x12168` routes
item codes `0x13`, `0x14`, `0x16`, and `0x18..0x2B` to `0x121C0`, which writes
the three D202F source lines at C620, waits for a fresh event through `0x0D94`,
and returns without changing possession or equipment. The Qt conversion now
connects this path to its typed acknowledgement state and directly extracted
text; its QA checks the outline, item/detail/shell/HUD layers, no-mutation
result, static wait, and fresh-edge return. The exact automatic-use screen is
not present in the supplied longplay, so visual fidelity remains partial.
Inventory-action confirm also now emits the source `0x24` request before the
item handler; the mapped audio file runs 2.484 seconds including its release
tail. The wait itself does not issue another sound request.
The target-effect leaf at `0x1253E` is also converted separately in
`first_aid_conversion`: its full-target gate, difficulty formula, roll-result
fallback, meter order, status transition, and `0xB45C` continuation are
covered without importing ROM state into the runtime. The same isolated module
now includes the `0x12298` active-target/CANCEL list, wrapped target cursor,
the `0x12466`/`0x1247E` patch branches, and `0x124AE` phase consumption.
The armor-selection leaf at `0x11352` is separately represented in
`armor_selection_conversion`; its preview write/rollback, no-armor gate, and
text/effect/grid ordering remain outside the native runtime.
Discard now follows `0x12634`/`0x1275C`: it opens the source Yes/No
confirmation with No selected, shows the equipped-item warning, protects item
codes `0x29..0x2B` in slot 7, and clears the possession cell only after Yes.
If the discarded cell is live equipment, its weapon or armor state is cleared
with it. First Aid now follows `0x12298`/`0x1253E`: it builds the active
record target list plus the source `CANCEL` row, preserves the target cursor,
uses the `0x0DBA` Biotech roll for medkits, applies the `0x3674` physical and
`0x37EC` mental writers for patches, shows the source result strings, and
decrements/removes the selected phase byte only after a successful use. The
Trade magic-item rejection, recipient/full-inventory gate, Yes/No confirmation,
and `0x129FE` possession transfer are now connected in the Qt conversion
through typed roster and item records. The recipient HUD-panel call at
`0x110E8`, measured Qt speaker output for event `0x25`, and an independent Trade capture remain open.

The separate Qt conversion retains the selected inventory slot while the
First Aid state is active, accepts the source A edge for target choice, waits
for a fresh high-byte event to dismiss the result, and returns to the same
Inventory page. Its CTest writes `actual.bmp` and target/CANCEL/use/return
checkpoints and checks the live HUD and one-pixel item selector. This remains
partial while the host random seed differs from source startup, event `0x25` maps to resource 037;
Qt speaker output remains unmeasured, and the recruited-runner visual roster is unavailable.

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
and ammunition bytes. Its top-level source labels are `Reload`, `Trade`,
`Distribute`, and `Cancel` from `D1DDC`; `Increase Clips`, `Decrease Clips`,
`Trade Clips`, and `Change Recipient` belong to the nested trade table at
`D1E24`. A dispatch is gated by the extracted clip count; an empty count
produces the source `D1AC6`/`D1AD3` result (`is out of ammo clips!`) rather
than changing the weapon state. Reload also rejects no-equipment and
non-clip-using items through `D1B40` (`This item does not use clips!!`).

Native status: selector bounds, source labels, extracted ammunition fields,
action gates, exact reviewed error text, and the native transition are
implemented in the separate Qt conversion runtime. The Clips screen replaces
the complete five-button command row with the ROM's four-entry row plus its
trailing blank; it is not a transparent focus overlay on the ordinary command
tabs. Executable QA walks Reload -> Trade -> Distribute -> Cancel, verifies
the 5 -> 2 wrap, deselection/reselection, the stable surrounding shell, and
the reload arithmetic. The nested Trade loop follows
`D1E24`/`0x1209A`/`0x120E4` for quantity wrapping, recipient changes, and
`Exchange complete`; the positive top-level Distribute average follows
`0x11FDE`/`0x12014` with the source remainder-bit rule. Reload follows
`0x11CCA`/`0x11D86`/`0x36FC`: it gates non-clip items, asks for
confirmation when ammunition remains, consumes one clip, clamps loaded
ammunition to the extracted item capacity, and emits the source
`Rock & Roll, chummers!!` result. The top-level screen is connected; the
recruited-recipient trade presentation and an independent visual golden
remain open.

### Attributes/Skills (`0xD01C`)

This is a two-column attribute/skill view over the active record, but it is
also a real Karma upgrade screen. The cursor selects one of eight attributes
or twelve skills. A valid selection computes a next value and source cost;
the confirmation branch writes the selected record byte and consumes the
record's available Karma at `+0x8D`.

Native conversion status: source names, values, auxiliary bytes, two-column
navigation, species limits, class gating, cost formulas, confirmation, and
record mutation are implemented in `attributes_skills_menu_conversion`.
Presentation text and the surrounding VDP callbacks remain separate. The
species limit rows are the five six-byte records at `0xC22A6`; attributes 6
and 7 are display-only in the upgrade path. The skill branch retains the
source distinction between 2x costs and the left-column middle-skill 1.5x
cost path, including its linked-skill zero-value dependency.

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
| 5 | Magic | `0xFBA6` | page-5 A opens the typed Magical Defense fallback for the source class/resource gates that select it; spell-panel cursor/force, target/effect, and result remain |
| 6 | Pocket Secretary | `0xC2A6` | five/six-entry cursor loop, source-priority wrapping, handler/mode-action dispatch, and exit masks converted; handler table and presentation remain |

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
ordinary field-mode labels are the `0xC1418` table written by `0xC2A6` through
`0xC0DA`; `0xD1D94`/`0xD1E6C` belong to the separate inventory-action menus and
must not be substituted here. The source handler also checks the selected
runner's class and the field mode before dispatching an option. The documented
gameplay options include runner and contact information, group items,
cyberdeck access, dismiss runner, and save/load. Save/Load is disabled in
buildings and during combat. A blank entry under Save/Load is an intentional
debug-menu path in the shipped ROM, not a missing label to fill with a made-up
option.

Native status: the `0xC2A6` panel now uses the extracted C1418 option order,
has a five-row cursor loop in ordinary field mode, returns through B/Start,
and sends Save/Load to the existing two-slot `0x59172` screen. Group Items now
uses the `0xC8C8` bitfield/catalog path and its source empty-catalog message.
The Cyberdeck row now follows the first branch of `0x18150`: with the native
new-game state equivalent to clear `FF018F` bit 0, it writes the exact three
lines at `0xE43D1`, `0xE43E2`, and `0xE43F3` into the Pocket Secretary panel.
Notebook now follows the base `0xF236`/`0xF33E` path: it extracts the title,
Current Run, Tips & Clues, and the three event-gated labels. The New Game
path clears `FFFC2C/27/2A` at `0x82A`, then `0x59C48` seeds `FFFC2C` to
`0x1C7F`, so Contacts is visible in the fresh post-opening session. The Qt
runtime now builds the three row gates from typed field state instead of
static text-asset data. Stoker command `0x30` adds `FFFC30` bit 1, exposing
the next Tips & Clues record; command `0x31` sets `FFFC27` bit 0, exposing
Mr. Johnsons. `FFFC2A` remains clear until a runner-information event writes
it. A restored save that omits these flags stops at an explicit boundary. The
native screen displays the exact `0xF2E2` no-current-run response.
The 96 Tips & Clues source entries are also extracted from the three
`0xC2716`/`0xC2796`/`0xC2816` pointer tables into a named direct text asset.
Startup `0x084E` sets `FFFC30` bit 0, and `0xF9A4` scans each group LSB-first.
Qt renders the filtered page list, Left/Right clamped cursor, source page
counter and source text placement. A/B/Start return to Notebook; C invokes
`0x391A` and stays on Tips. The panel is fully cleared before replacement, and
QA captures `actual.bmp`, another unlocked page, held animation, and all three
returns. Event `0x25` is delivered by the mode-action path and maps to
`audio.effects.resource.037`; Qt speaker output is unmeasured. Story-event writers and restored-save tip flags
remain pending; no independent Notebook/Tips Genesis frame is in the supplied
longplay. The Mr. Johnsons handler at
`0xF416` has also been audited far enough to extract its five fixed source
names in source order; its FFFC27-gated list/detail loop remains pending, as
do the conditional Contacts and Shadowrunners detail screens.
Dismiss Runner dispatches to Qt's typed `0xC448` route. The source places
Dismiss/Who?? at `(88,32)/(88,40)`, its visible separator at `(72,48)`, fixed
Abort at `(88,64)`, and active names from `(88,80)` in source record order.
Qt uses the compact party markers for dismissal and preserves the live
record flags. The child redraws the shared blank panel, and its QA now checks
that Pocket Secretary options disappear on entry and that the complete inner
panel matches the parent after B, Abort, and Start returns. The route remains
partial: Qt draws the per-VBlank `0xE660` cells at twelve lower-bezel
positions and uses the `0x110E8` selected-runner portrait preview. Event
`0x25` maps to resource 037; Qt speaker output and an independent
Genesis Dismiss Runner frame remain unverified. The separate NativePort Dismiss
path has known source mismatches and is not used as validation. The
deck-present Matrix path is intentionally still pending, as are restricted
Notebook detail handlers and their state gates.

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
image. The movement loop at `0x075E`/`0x2B34` projects the active record;
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
| Opening | `0x59C5E` and timed groups | Partial | Typed stage chain and direct timed/overlay assets are present in the separate conversion target; independent visual comparison and source audio playback remain pending |
| Field movement | `0x075E`, `0x03084`, `0x2B34`, `0xE7C8`, `0x5FDC`, `0x600E` | Partial | Scene descriptor loading is source-traced; reviewed player collision and animation path is present, while movement step/timing, source camera transitions, right-side HUD/entity presentation, and NPC/entity logic remain pending |
| Area transitions | `0x5A62`, `0x172CDC`, `0x20E48`, `0x20E7A`, `0x20E92`, `0x20EC2` | Partial | The current ROM trace covers the mode-2 event table and the electromagnetic-lock/Security Terminal prompt continuations. The earlier Stoker attribution was from the old native-port layer and is not source evidence; building destinations, NPC/dialogue records, and reverse transitions remain open |
| NPC dialogue panels | `0x1F65A`, `0x1F246`, `0x1F3C4`, `0xC168`, `0xF4AA0..0xF4B2C`, `0x121FA0`, `0x122420`, `0x213CA`, `0x213E2`, `0x213FA` | Partial | The shared panel consumes typed speaker/prompt views and renders the 27-count source wrap scan as at most 26 visible glyphs per line. When the typed Stoker mode-2/event-2 tuple selects the arrival record, A on `<MORE>` dispatches command `0xE9`; rolls 0..79 open `0xF4AB4`. Package C dispatches `0x30` to set `FFFC30` bit 1 and open F4E8C; debt-page B dispatches `0x31` to set `FFFC27` bit 0 and open F5AB0. Debt-page A now runs `0x05`; the fresh 20-nuyen wallet follows the insufficient-funds branch to `0x213E2`, retains its balance, and opens the same referral page. These flags feed the dynamic Notebook rows and Tips list; Debug/Release CTests write prompt and Notebook `actual.bmp` files. Independent section 03 frames 000600-000602 show debt, while 000690/000750/000780/000840 show the referral page. The room-offer command `0x06` spends the source 20-nuyen charge, selects `0xF5AB8` paid or `0xF5AF8` insufficient-funds feedback, then requests Hotel Options after the paid message. The Qt Hotel Options screen remains an explicit boundary. The live field object/property trigger, loaded-save wallet/flags, 80..99 `0x1E1FC` effect, three-option package-state property, other speaker records, source RNG initialization, successful `0x213FA` package/payment transaction, unresolved indirect audio hook, and live field return remain open |
| Command overview | `0x10D30`, `0x10F50`, `0x111E8`, `0x11454` | Partial | Normal overview, source command-tab focus blocks, and selector are present; Magic target/effect and remaining Pocket Secretary row handlers remain pending |
| Inventory navigation | `0x11506`..`0x11678` | Implemented | Eight-cell cursor and source column/row semantics are covered |
| Equipment action | `0x11740`..`0x118C6`, `0x12148`, `0x12168`..`0x125F8`, `0x12634`..`0x129FE` | Partial | Two-step source action loop, Equip/Unequip, armor, Discard, and typed Trade recipient/transfer routes are present; First Aid is separately connected through active-target/CANCEL, source A-confirm, effect, result wait, and inventory return; recipient HUD projection, independent Trade capture, utility playback, and later item handlers remain |
| Posture and bars | `0x119CE`, `0x11ABE`, `0x11AE4`, `0x12CEE` | Partial | Normal equipped-item attack/defense calculation is source-derived; special negative `+0x56` path and combat consumers are pending |
| Clips | `0x11B58`, `0x11CCA`, `0x11D86`, `0x11DD2`, `0x11E7E`, `0x11FDE`, `0x12014`, `0x12062`, `0x1209A`, `0x120E4`, `0x36FC` | Partial | Source top-level/nested selectors, typed Joshua/save roster projection, +0x58 selected-runner names, ammo gates, Reload confirmation/consumption/clamp, recipient filtering/cycling, quantity wrap, clip transfer, completion message, and Distribute average are present; loaded-save field composition and final panel art remain |
| Attributes/skills | `0xD01C` | Partial | Source grid, cursor, species limits, Karma costs, class gate, confirmation, and record mutation are converted; presentation callbacks remain |
| Cyberware | `0x11950` | Partial | Empty branch and source bit 0..14 installed-row scan are converted; text/input service and mutation flows remain |
| Runner switching | `0x391A` | Partial | Source type-2 wrap, eligibility/suppression gate, and no-eligible no-install behavior are converted; recruited roster data and native integration remain |
| Pocket Secretary / Notebook | `0xC2A6`, `0xC3E6`, `0xC376`, `0xC8C8`, `0xF236`, `0xF2E2`, `0xF33E`, `0xF312`, `0x18150`, `0xC448` | Partial | Qt connects the source five-row base, Group Items empty/populated branches, Cyberdeck no-deck response, Notebook base list, and Current Run no-run wait/return. The Notebook dump checks ROM-derived coordinates and verifies replacement text clears prior rows. The Dismiss route checks fixed Abort/name placement, compact-marker state, selected-runner preview, current-runner replacement, and full inner-panel replacement on return. F312/C448 call E660 once per VBlank; Qt draws the typed random 1x2 effect in twelve lower-bezel positions and QA checks that it does not overlap child text or controls. Normal quest acquisition and restored-save FFFC22, Contacts/Shadowrunners detail handlers, Mr. Johnsons, Save/Load, deck-present Matrix, restricted-mode gates, measured Qt output for event 0x25, and independent Pocket Secretary Genesis frames remain open. Existing NativePort menu paths do not count as Qt implementation evidence. |
| Magic | `0xFBA6` | Partial | The separate Qt executable opens the typed fallback, computes physical/mana defense rows from ready-party `+0x10`/class/item-0x18 state and active `+0x77`/`+0x7C`, presents the source rows, forwards parent event `0x24`, and returns on one fresh edge. Spell-panel cursor/force, target/effect, and independent Magic visuals remain |
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
2. Finish Magic target/effect behavior and Pocket Secretary row handlers as stateful screens.
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
  title background comparison currently uses the 64-word dark circuit palette
  beginning at `0x1C024`; frame-by-frame timing and audio still remain to be
  verified.
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
- The reviewed `0x12168` item-code split now sends `0x14`, `0x16`, and
  `0x18..0x2B` through the source `0x121C0` automatic-use acknowledgement;
  native tests verify that the item remains in its possession cell and does
  not become equipped.
- Unequipping no longer removes the projected runner body. The item sprite
  selection path and the body projection path are separate source effects;
  the native fallback marker is not a character sprite.
- The recruited template table at `0x72C1A` is now extracted as ten complete
  named `0x100`-byte runner records. The following zero-kind record is treated as
  the source terminator. These are candidate templates only; New Game still
  starts with Joshua, and the event-owned recruitment/installation state is
  not invented until its writers are traced.

These are correction slices, not a completion claim. The right-hand party
status/sidebar layer, recruited roster activation, source submenu layouts, animated
opening artwork/music, valid SRAM load, and the remaining item-action branches
still require their own source traces and focused comparisons.
