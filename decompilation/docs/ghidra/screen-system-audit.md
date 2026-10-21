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
NPC/entity interaction systems remain incomplete. New Game now has physical
Stoker/Ares routes plus scene-2 records 89/90 into scene 0x14 and scene-20
record 117 into the generic maglock prompt; these do not complete general
terrain recovery or the native field loop.

### Command overview (`0x10D30`, `0x10F50`, `0x111E8`)

The command screen starts with selector `FFF0D0=6` and row `FFF0D1=0`. Left
and Right wrap the seven selector values:

| selector | source label | behavior |
| ---: | --- | --- |
| 0 | Inventory | left inventory column; row 0..3 |
| 1 | Inventory | right inventory column; row 0..3 |
| 2 | Clips | opens the clips loop |
| 3 | Attributes/Skills | A opens the view-only Attributes/Skills page; A/B leave it |
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
are drawn from the direct resource catalog. The seven command-root labels now
resolve from ROM-derived Lua text keys; the typed selector still owns focus,
drawing, and dispatch. Archetype, runner, status, power, armor, and menu
feedback labels resolve from ROM-derived `script-text.lua`; item names,
categories, labels, and detail rows resolve through `items.lua` and the same
shared text table. They all use the existing native presenter/font path.
Clips, Attributes, Cyberware, Magic, and Pocket Secretary still need remaining
source handlers and action resources connected; this hill is therefore
partial, not defended.
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
confirms its selected action. The source `Equip/Unequip`, `Discard`, `Trade`,
`Cancel`, and medical `First Aid` choices are now routed by Lua modules under
`gameplay/scripts/menu/screens/item_action.lua`, with inventory and medical
rules in the adjacent `rules/` files. `field_init.lua` registers these screens
as active route owners, so the prior C++ input handlers are bypassed while the
Lua route is open. C++ supplies typed party snapshots, capacity and roll
services, and commits; Qt presents the Lua-selected screen through the shared
field panel. Possession and equipment stay separate: an item in one of the
eight cells is not active until the source equip action sets the live record's
equipped slot/item bytes. Medical items remain on their First Aid path and do
not enter the generic Equip/Unequip branch.
The four source action positions are now declared as stable IDs by
`gameplay/scripts/items.lua`; the field text catalog checks their ROM order,
and the Qt dispatcher resolves the selected typed source action to its Lua ID.
The same module now pairs its 14 inventory/First Aid text rows and 27 power/
armor display rows with explicit ordered label keys. The catalog resolves the
ROM strings through those keys. The four source button graphics remain the
native extracted icon assets.
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
The target-effect leaf at `0x1253E` has the same typed source-derived rules in
Lua: its full-target gate, difficulty formula, roll-result fallback, meter
order, status transition, and `0xB45C` continuation. The `0x12298`
active-target/CANCEL list, wrapped target cursor, `0x12466`/`0x1247E` patch
branches, and `0x124AE` phase consumption also run in Lua. A typed host roll
service supplies the shared source RNG; the medkit rule itself remains in
Lua.
The armor-selection leaf at `0x11352` is separately represented in
`armor_selection_conversion`. The Qt inventory now presents the same typed
selection directly: item code minus `0x2B` chooses the lower armor panel at
`C820/C922/C92C/C9AC`, while the runner's equipped `+0x76` stays unchanged.
Both Wpns and Guns use six-tile rows from `CF884` at `0x0C` per modifier;
these are extracted as `armor-summary-bar-0..8`, separate from the seven-tile
weapon Damage/Drain detail tables at `CF84C` and `CF8F0`.
Debug/Release field-action QA checks a selected Vest with no armor equipped
and Clothing selected while Vest remains equipped. Independent frame 002560
supports the panel placement; the differing-state comparison is still open.
Discard now follows `0x12634`/`0x1275C`: it opens the source Yes/No
confirmation with No selected, shows the equipped-item warning, protects item
codes `0x29..0x2B` in slot 7, and clears the possession cell only after Yes.
The shared `0x57B8C` Yes/No row is declared by `gameplay/scripts/ui.lua` with
explicit `ui.option.yes` / `ui.option.no` keys in source order; the field
metric captions likewise bind their stable IDs to explicit `ui.field_metric`
keys. Discard and Ares transaction dispatch resolve the selected ID while
keeping their existing typed state and input path. Debug/Release script,
catalog, and full-screen route QA cover the bindings; see the
[shared UI Lua content review](visual-audits/shared-ui-lua-content.md).
If the discarded cell is live equipment, its weapon or armor state is cleared
with it. First Aid now follows `0x12298`/`0x1253E`: it builds the active
record target list plus the source `CANCEL` row, preserves the target cursor,
uses the `0x0DBA` Biotech roll for medkits, applies the `0x3674` physical and
`0x37EC` mental writers for patches, shows the source result strings, and
decrements/removes the selected phase byte only after a successful use. The
Trade magic-item rejection, recipient/full-inventory gate, Yes/No confirmation,
and `0x129FE` possession transfer are now connected in the Qt conversion
through typed roster and item records. The recipient HUD-panel call at
`0x110E8`, Windows loopback capture for this exact Trade route, and an independent Trade capture remain open.

The active Lua route retains the selected inventory slot while First Aid is
open, accepts A for target choice, waits for a fresh controller edge to dismiss
the result, and returns to the same Inventory page. Its CTest writes
`actual.bmp` and target/CANCEL/use/return checkpoints and checks the live HUD
and one-pixel item selector. Debug and Release route tests cover the Lua rule
and the whole-screen output; their BMP hashes match and the captured screens
were reviewed. The hill remains partial while the host random seed differs
from source startup, Windows loopback for this exact route remains unrecorded,
and the recruited-runner visual roster is unavailable.

### Posture (`0x11928`, `0x119CE`, `0x11ABE`, `0x11AE4`)

When the command selector is `2..6`, Up enters the dedicated posture focus;
Down with no horizontal direction held deselects it and returns to the same
command row. Left/Right adjust the active record's posture byte (`+0x4D`),
clamped to `0..4`. The five source values are:

`Full Defense`, `Med. Defense`, `Neutral`, `Med. Offense`, `Full Offense`.

Each horizontal change calls `0x11ABE`, which redraws the value label and calls
`0x12CEE` to recompute the attack/defense source bars. The `0x119CE` posture
focus image comes from the grid at `0xCA9E` (`120,168`); the separate five-state
bar begins at `0xCAAC` (`176,168`). The focus image replaces the panel's static
POSTURE label while preserving that bar. It is static while selected; only a
Left/Right edge requests sound event `0x25` (`audio.effects.resource.037`,
2.134 seconds). Down closes focus without a sound. Start redraws the final
posture and bars, sets `FFF0B1`, and exits the complete command menu on that
input. Posture is gameplay state, not a decorative slider.

Native status: the active posture focus, source five-state values, clamping,
record persistence, rendering, and the normal `0x12CEE` attack/defense
calculation are implemented. The native results match the Joshua source record
for all five posture values (`attack 3,4,5,6,7`; `defense 19,18,17,16,15`).
The Qt posture CTest drives real command inputs on pages 2..6, compares the
selector pixels with the named source asset, checks a 30-frame static hold,
tests both horizontal directions and both clamps, verifies event-0x25 cadence,
and compares the complete command frame after Down with its pre-focus image.
Start then returns to a full-frame field image identical to the one before the
menu opened. These are internal renderer checks; the supplied independent
longplay shows neighboring command pages but not this selected posture route.
The negative `+0x56` special branch (`0x12D7C`/`0x12FA4`) and the field
AI/combat code that consumes posture are not yet reconstructed, so their effect
outside the audited command path remains pending.

### Clips (`0x11B58`, `0x12062`)

The clips screen has its own selector loop and uses the active record's clip
and ammunition bytes. Its top-level source labels are `Reload`, `Trade`,
`Distribute`, and `Cancel` from `D1DDC`; `Increase Clips`, `Decrease Clips`,
`Trade Clips`, and `Change Recipient` belong to the nested trade table at
`D1E24`. A dispatch is gated by the signed byte at `+0x4F` using `TST.B` and
`BGT`; zero and values `0x80..0xFF` enter the no-reserve path. `0x12062`
temporarily renders the runner name and the `D1AC6`/`D1AD3` “is out of ammo
clips!” text, clears the count, and returns to `0x11C46`; the signed `bgt`
then fails and immediately enters `0x11C9C`, which clears the panel and
displays the final `Function Aborted!` message from `D1ED8`. Reload also rejects no-equipment and
non-clip-using items through `D1B40` (`This item does not use clips!!`).

The ROM does not keep an independent magazine count in each inventory slot.
`+0x4E` is the active gun's loaded rounds; `0x125FA` converts that load to a
percentage when the gun is put away, and `0x121E0` applies the percentage to
the newly equipped gun's `0xD4540 + 0x24` capacity. `+0x4F` is the runner's
shared Universal Gun Clips count. The live Clips route resolves reload capacity
from the currently equipped item's `+0x24` value on each input step.

The top-level and nested selector IDs plus Clips panel/message text IDs live
in `gameplay/scripts/clips.lua` and `assets/dialogue/script-text.lua`. The
screen and math now run in `gameplay/scripts/menu/screens/clips.lua` and
`gameplay/scripts/menu/rules/clips.lua`; C++ supplies typed runner records,
active slot, magazine capacity, and ammo writes. `FieldSceneRuntime` presents
Lua's current route snapshot. ROM 0x11B98 installs button row CD33E for Clips,
0x11E7E installs CD37A for nested Trade, and 0x11D06/0x127B0 replaces that
row with CD3B6 for reload Yes/No. The presenter consumes these three separate
extracted rows and paints their selected focus blocks. The source text at
0xD1ED8, `Function Aborted!`, is also available through the Lua text catalog.
The source 0x12C36 clear starts at name-table C118 (screen 96,16), covers its
20x20-cell center region, and leaves the separately composed field HUD intact.
That clear also reaches the top 16 pixels of the shared posture panel. The
subsequent 0x11B5C call into 0x11900 redraws the common shell, so the Qt Clips
presenter now restores the posture frame, current bar, and status label after
its center clear. Its four-button row starts at x=120,y=184; focus blocks start
at x=136,160,184,208 to align with the extracted button art. The selected
root and Trade captions come from D1DDC and D1E24; reload Yes/No uses its
prompt table. Each caption is centered in the 17-character field beginning at
x=112.
The result path at `0x11CB6` uploads `CD2C6` while waiting for a fresh input
edge, so result and abort messages show the shared five-command tab row. The
child Clips/Trade row is not left under the message. The `CD3B6` two-choice
confirmation art places Yes and No at x=160 and x=184; the selected Yes block
must align with the existing Yes control at x=160. The Trade recipient name is
centered in its 15-character field spanning x=120..240. Clips QA compares the
posture strip with the parent frame pixel-for-pixel and checks these button,
focus, name, and caption pixels.

Debug and Release CLI QA captures all four selector positions and wrap,
Function Aborted and message acknowledgement, every nested Trade focus and a
source-template Ricky transfer, centered Ricky/Winston recipient names, reload
No and Yes, decline without mutation, and reload success. It compares focus
pixels and requires the parent command tabs on every result wait; acknowledging
Cancel restores the full parent image exactly. The common zero-
reserve gate follows 0x12062 into the 0x11C9C Function Aborted path. The
nested Trade loop follows D1E24/0x1209A/0x120E4 for quantity and recipient
changes. Distribution follows 0x11FDE/0x12014: it accumulates a byte total,
writes the quotient to all three records when nonzero, and gives the remainder
to the record marked by status bit 1. Lua route QA covers both the inactive
record write and the byte-total wrap case. Reload follows 0x11CCA/0x11D86/0x36FC:
it gates non-clip items, consumes one clip, clamps loaded ammunition to the
equipped item's capacity, and emits the source “Rock & Roll, chummers!!” result.

Menu movement and Yes/No toggling request event 0x25; action/confirmation
requests event 0x24. Shared audio QA resolves them to resources 037 and 024
with durations 2.134 s and 2.484 s. The exact Clips route's speaker-output
loopback has not been recorded. The supplied longplay has neighboring
command/menu frames but no matching Clips selector, nested Trade, or reload
confirmation sequence, so independent visual fidelity remains unverified.
Complete loaded-save field composition also remains open.

### Attributes/Skills view and Hotel Karma upgrades (`0xD01C`, `0xD024`)

These are two different user-facing screens that share the ROM's
Attributes/Skills grid and drawing routine. Command selector 3 dispatches to
`0xD01C`, which sets `FFF116=-1` before drawing the view-only page. It shows
the current Karma total, with no next-value/cost preview; `0xD33A` sends A or
B back to the command shell without mutation. Hotel option 0 points to
`0xD024`, inside the shared ROM page body but past the `FFF116=-1` assignment.
Its lower detail line shows the selected row's next value and Karma cost, and
A opens the spending prompt through `0xD3EA`/`0xD53C`.

The Qt field scene exposes these routes through separate fixed-mode screen
types: `CommandMenuKarmaViewRuntime` and `HotelKarmaUpgradeRuntime`. They share
the typed grid/input controller, while each route fixes its own interaction
mode; the command view has no route-level API for enabling Karma spending.

The native runtime keeps separate route state for both screens. The command
view paints the current Karma total and cannot mutate the runner. The Hotel
child paints next-value/cost details, confirms Yes/No, updates the selected
record byte and Karma at `+0x8D`, and returns to Hotel Options. Both use the
source two-column, ten-row grid and selection highlighting. The five six-byte
species-limit records are at `0xC22A6`; attributes 6 and 7 cannot be upgraded.
The skill branch preserves the source 2x and left-column middle-skill 1.5x
costs, including its linked-skill zero-value dependency.

The shared static copy comes from `gameplay/scripts/attributes_skills.lua` as
explicit ordered keys for the 20 labels and five upgrade messages. The command
view and Hotel spend screen intentionally share those keys while retaining
separate lower detail lines and behavior. Script QA checks exact ID/key/text
order, and the full-screen route QA checks that the view-only page has no cost
preview while the Hotel page does.

Debug QA covers both routes with separate multi-frame dump directories. A
pixel comparison requires the same shared grid and current Karma amount in
both captures, plus upgrade/cost text only on the Hotel screen's lower detail
line. Entry and return checks ensure the child screens replace and
rebuild their parent panels cleanly. The supplied longplay has no matching
frames for either Attributes/Skills page, so these checks do not establish
independent Genesis visual fidelity.

### Cyberware (`0x11950`)

This handler is display-oriented: it reads the active record's fifteen-bit
cyberware field at `+0x90` and resolves installed entries through the source
name table. It prepares the center name-plane grid and writes the child panel
without clearing the complete command screen. The native presenter now
redraws only that center content region, preserving the shared runner portrait,
inventory, posture bar, command tab, and right HUD.

Native status: bitfield extraction, source names, installed-state display, and
the shared-panel redraw are implemented. Debug/Release QA captures parent,
child, held, return, and reopen frames, and checks shared regions plus
pixel-identical parent restoration. No matching Genesis Cyberware capture is
available; purchase/install/removal flows, if reached through another source
path, remain to be traced.

The 18 strings now have explicit ordered `cyberware.text.option.*` keys in
`gameplay/scripts/cyberware.lua`; the script test checks their ROM text, while
field-catalog QA checks the resulting title, separator, empty message, and
implant rows. This changes content routing only, not the installed-bit logic
or the shared-panel composition.

## Command-screen dispatch map

The command bar is a dispatcher, not a collection of decorative buttons. The
seven labels at `0xD1D28`/`0xD1D3A` are selected by `FFF0D0`; the indirect table
at `0xD2A0A` routes the five non-inventory commands as follows:

| selector | source label | handler | current boundary |
| ---: | --- | ---: | --- |
| 2 | Clips | `0x11B58` | native selector, ammunition gate, and return path |
| 3 | Attributes/Skills | `0xD01C` | native read-only two-column view; A/B/Start return to the command shell |
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

The seven root choice IDs and visible labels are now declared in
`gameplay/scripts/command_overview.lua` and `assets/dialogue/script-text.lua`.
`FieldSceneRuntime` validates their ROM order and binds the labels to the
existing command-menu view. This moves menu content into Lua without giving
Lua or Qt a second copy of the native page-selection state.

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

The root cursor path is source-silent: the `0xC2A6` loop advances `0xE660`,
branches on Up/Down, and calls `0xC376` without a sound service. Event `0x24`
is requested once by the parent command overview when A opens this page; it is
not a movement cue. Debug/Release route QA verifies the entry request and
silent root selection/wrap sequence. See
[`pocket-secretary-navigation-audio.md`](visual-audits/pocket-secretary-navigation-audio.md).

Native status: the `0xC2A6` panel now uses the extracted C1418 option order,
has a five-row cursor loop in ordinary field mode, returns through B/Start,
and sends Save/Load to the existing two-slot `0x59172` screen. Group Items now
uses the `0xC8C8` bitfield/catalog path and its source empty-catalog message.
The Cyberdeck row now follows the first branch of `0x18150`: Lua reads the
typed `FF018F` bit-0 fact and, when clear, pushes the generic `MESSAGE` screen
with `NO_CYBERDECK`. The Qt presenter writes the exact three lines at
`0xE43D1`, `0xE43E2`, and `0xE43F3` into the Pocket Secretary panel. The
message carries the shared `E660` effect, so the lower bezel keeps updating
while the text waits; any fresh controller edge returns to the same row. The
no-deck branch adds no separate sound request. A restored save with an unknown
flag and the deck-present Matrix path remain explicit boundaries.
Notebook now follows the base `0xF236`/`0xF33E` path: it extracts the title,
Current Run, Tips & Clues, and the three event-gated labels. Its five
potential rows keep ROM order in `gameplay/scripts/pocket_secretary.lua`; the
typed model applies the event gates and dispatches the selected stable ID.
The New Game path clears `FFFC2C/27/2A` at `0x82A`, then `0x59C48` seeds
`FFFC2C` to
`0x1C7F`, so Contacts is visible in the fresh post-opening session. The Qt
runtime now builds the three row gates from typed field state instead of
static text-asset data. Stoker command `0x30` adds `FFFC30` bit 1, exposing
the next Tips & Clues record; command `0x31` sets `FFFC27` bit 0, exposing
Mr. Johnsons. `FFFC2A` remains clear until a runner-information event writes
it. A restored save that omits these flags stops at an explicit boundary. The
native screen displays the exact `0xF2E2` no-current-run response.
The `0xF236` Notebook loop sends C through `0x391A`, which requests event
`0x25` and returns to the same selected Notebook row. Qt now preserves that
page/selection and emits the event; QA checks both the callback and the full
320x224 frame. Notebook A dispatch, Up/Down, and B/Start return are silent.
Event `0x25` maps to `audio.effects.resource.037` (2.134 seconds including
release tail); shared audio QA verifies decoded playback, default-output state,
and duration, but route-specific loopback remains unrecorded.
The 96 Tips & Clues source entries are also extracted from the three
`0xC2716`/`0xC2796`/`0xC2816` pointer tables into a named direct text asset.
Startup `0x084E` sets `FFFC30` bit 0, and `0xF9A4` scans each group LSB-first.
Qt renders the filtered page list, Left/Right clamped cursor, source page
counter and source text placement. A/B/Start return to Notebook; C invokes
`0x391A` and stays on Tips. The panel is fully cleared before replacement, and
QA captures `actual.bmp`, another unlocked page, held animation, and all three
returns. Event `0x25` is delivered by the mode-action path and maps to
`audio.effects.resource.037`; shared audio QA checks decoded playback, default-output state, and duration, while Windows loopback for this route remains unrecorded. Story-event writers and restored-save tip flags
remain pending; no independent Notebook/Tips Genesis frame is in the supplied
longplay. The Mr. Johnsons handler at
`0xF416`, `0xF840`, and `0xF6DC` now have live Lua list/detail routes. The
Mr. Johnsons page uses its five ROM names and FFFC27 bits. Contacts and
Shadowrunners share the ROM `0xF5F4` paging rule, with separate FFFC2C and
FFFC2A masks and ROM-derived name/detail catalogs. Full-screen Debug/Release
QA covers their two-page lists, details, clean returns, and the exact-eight
no-pager boundary. The flag writers and independent Genesis frames for these
pages remain unresolved, so these routes remain partial.
Dismiss Runner dispatches to Qt's typed `0xC448` route. The source places
Dismiss/Who?? at `(88,32)/(88,40)`, its visible separator at `(72,48)`, fixed
Abort at `(88,64)`, and active names from `(88,80)` in source record order.
Qt uses the compact party markers for dismissal and preserves the live
record flags. The child redraws the shared blank panel, and its QA now checks
that Pocket Secretary options disappear on entry and that the complete inner
panel matches the parent after B, Abort, and Start returns. The route remains
partial: Qt draws the per-VBlank `0xE660` cells at twelve lower-bezel
positions and uses the `0x110E8` selected-runner portrait preview. Event
`0x25` maps to resource 037 and shared audio QA checks playback on the default output; Windows loopback for this route and an independent
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
The spell text table at ROM `0xD4EA0` now has semantic IDs in
`gameplay/scripts/magic.lua` source order, with explicit ordered Lua keys for
all 13 names, all 13 descriptions, and seven shared panel labels. The field
text catalog resolves those keys into the typed presenter; the native runner
spell table still controls which spells appear for each archetype. For the
non-magic fallback at `0xFBA6`, the source rebuilds the center menu grid before
writing a 16-cell ` Magical Defense` title row (including its leading blank)
and the two defense strips. The outer runner frame, posture control, command
tab, and right HUD remain. Qt now redraws the center panel before the title
and bars, so old status/item text is not visible behind the fallback.

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

For the Redmond Barrens startup map, scene-2 record 91 at ROM `0x173138`
(`0xC2`, bounds `0x3C0..0x3DF` by `0x280..0x29F`) is the Stoker doorway;
normal Up publishes raw collision `0xFFC2`/event 2 and selects Stoker's
mode-2 arrival prompt through `0x1F65A` and `0xF4AA0`. Separately, paired
scene-2 records 89/90 at `0x173124` (`0xC0`) publish event 0; event-table row
0 at `0x172CDC` selects scene `0x14`. The native physical-entry QA now follows
that New Game route into scene 20. Scene-20 record 117 at `0x1D9724` (`0x82`)
then opens the generic maglock panel on projected `0xFF82`/event 2. The old
scene-2 record-2 candidate at `0x172DBE` remains unrelated and still produces
event 0. Ares, maglock, and Security Terminal continue to use distinct
property/event handlers.

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
| Field movement | `0x075E`, `0x03084`, `0x2B34`, `0xE7C8`, `0x5FDC`, `0x600E` | Partial | Scene descriptor loading is source-traced; field movement QA covers accepted movement and blocked reset. Stoker QA follows ordinary Up to record 91; Ares QA follows ordinary Right/Up to record 92; scene-20 QA follows ordinary New Game input through scene-2 records 89/90, scene entry, and record 117 in scene 20. All three capture full-screen transitions; Debug/Release checks the scene-20 dialog return against a field-only render. General movement timing, other camera transitions, broader HUD/entity presentation, and NPC/entity logic remain pending |
| Area transitions | `0x5A62`, `0x172CDC`, `0x20E48`, `0x20E7A`, `0x20E92`, `0x20EC2`, `0x21628`, `0x21692` | Partial | Scene-2 record 91 (`0x173138`, `0xC2`) publishes event 2 and selects mode-2 Stoker dialogue; record 92 (`0x173142`, `0xCB`) is reached by ordinary Right/Up, publishes `0xFFCB`/event 11, opens the generic Ares prompt, then A enters the shop and Start returns. Paired records 89/90 at `0x173124` (`0xC0`) publish event 0; row 0 at `0x172CDC` selects scene `0x14`. In scene 20, record 117 at `0x1D9724` (`0x82`) publishes projected `0xFF82`/event 2 and opens the source-backed maglock dialog; A reveals choices, C/Walk Away restores the field, and B with no active-party Frag Grenade displays the generic unavailable-item page before A restores a clean field. A quality-2 QA passkey succeeds on ordinary A; quality 1 against selection 2 keeps the generic insufficient-quality choices, leaves key/map unchanged, adds no action sound, and C restores the field. The ordinary Samurai path selects Electronics with A; ROM 0x21692 returns the +0x8A=0 no-skill page before any roll. The ordinary Decker path (skill 1/no kit) gets one threshold success, reduces the current selection 2->1, and holds the partial-result page without sound or map change. A returns to the field; fresh Up contact reloads selection 2 from the collision event at ROM 0xB994. The QA then seeds kit item 0x15 and a reproducible PRNG state; two of three target-3 rolls select the success page and request 0x4B. The full map stays 0x82 while the result is open and 0x2BD2 changes cell 483 to 0x81 after ordinary A returns. The kit is a test fixture. The unrelated record-2 candidate at `0x172DBE` remains event 0. Qt seeds the movement gate from the selected active runner's source record `+0x10` byte (all three `0x7291A` archetypes have `0x0F`); it no longer substitutes the separate spawned-entity `0x44BC` default `0x05`. Debug/Release QA captures the route and checks the full-screen redraw without stale modal controls. The Ares fade handoff clears its old overlay; scene re-entry also releases the display gate after replacement buffers are prepared. Scene-26 D8 terminal physical entry has a collision-backed route with a QA-seeded passkey. Other passkey qualities, story acquisition and kit acquisition, downstream alert-entity selection, grenade-consumption outcomes, terminal world-activity outcome, other buildings, and reverse transitions remain open. The `0x4B`, `0x2C`, and `0x30` sound results retain their separate resource/stop findings in HILLS |
| NPC dialogue panels | `0x1F65A`, `0x1F246`, `0x1F3C4`, `0xC168`, `0xF4AA0..0xF4B2C`, `0x121FA0`, `0x122420`, `0x213CA`, `0x213E2`, `0x213FA`, `0x20E48`, `0x20E7A`, `0x20E92`, `0x20EC2`, `0x21692` | Partial | The shared panel consumes typed speaker/prompt views and renders the 27-count source wrap scan as at most 26 visible glyphs per line. Physical scene-2 record 91 opens the Stoker arrival record; A on `<MORE>` dispatches command `0xE9`, and the deterministic QA seed selects the 0..79 manager branch. Ordinary B/A advances the manager Leave choice and returns to the field. A second ordinary-input path chooses Rent, pays the 20-nuyen room charge, enters Hotel status/options, opens the distinct Karma-spending page, and checks out; it remains in scene 2 and loads no motel interior. Scene-20 record 117 (`0x1D9724`, value `0x82`) now opens the shared speaker-none maglock prompt from normal Up input; A displays the source-backed choices and C/Walk Away returns to a clean field. A separate physical route seeds active Joshua with passkey `0x14`/quality 2; ordinary A shows the source result, requests `0x4B`, preserves the item, changes map index 483 from `0x82` to `0x81`, and returns without stale controls. Scene-26 descriptor-spawn QA also reaches the D8 prompt after two QA-seeded passkey successes. The physical scene-20 Samurai route follows Electronics to the `0x21692` no-skill result, holds it for 30 frames, and clears it with ordinary A. A second ordinary Decker route covers a no-kit partial result and a QA-kit success; ROM `0xB994` resets selection to the collision event value on recontact, and `0x2BD2` commits the success map counter after result acknowledgement. The kit and PRNG are test fixtures. Story-driven item acquisition, The zero-roll Electronics alert and 120-frame timeout handoff are now covered through the entity-target selection boundary; the resulting actor/effect and terminal world-activity outcome remain open. Independent section 03 frames 000080..000082 show the Stoker panel; section 05 frame 000600 shows a generic A/B/C choice panel. The longplay has no scene-20 maglock content or input/event log. Package C dispatches `0x30` to set `FFFC30` bit 1 and open F4E8C; debt-page B dispatches `0x31` to set `FFFC27` bit 0 and open F5AB0. Debt-page A runs `0x05`; fresh 20-nuyen state follows the insufficient-funds branch. The room-offer command `0x06` spends 20 nuyen or selects the insufficient-funds response, then requests Hotel Options after the paid message. Hotel status/options/checkout have typed routes and internal full redraw checks. The New Game route now exercises the paid-debt transaction and package handoff with a QA-seeded 250-nuyen wallet. The live package-state property, loaded-save wallet/story flags, 80..99 `0x1E1FC` effect, other speakers, source RNG initialization, physical repeat-entry movement, independent paid-handoff capture, unresolved indirect audio hook, and Hotel independent capture remain open |
| Command overview | `0x10D30`, `0x10F50`, `0x111E8`, `0x11454` | Partial | Normal overview, source command-tab focus blocks, and selector are present; Magic target/effect and remaining Pocket Secretary row handlers remain pending |
| Inventory navigation | `0x11506`..`0x11678` | Implemented | Eight-cell cursor and source column/row semantics are covered |
| Equipment action | `0x11740`..`0x118C6`, `0x12148`, `0x12168`..`0x125F8`, `0x12634`..`0x129FE` | Partial | Two-step source action loop, Equip/Unequip, armor, Discard, and typed Trade recipient/transfer routes are present; First Aid is separately connected through active-target/CANCEL, source A-confirm, effect, result wait, and inventory return; C928 recipient-panel projection and clean return pass Debug/Release route QA; independent Trade capture, utility playback, and later item handlers remain |
| Posture and bars | `0x119CE`, `0x11ABE`, `0x11AE4`, `0x12CEE` | Partial | Normal equipped-item attack/defense calculation is source-derived; special negative `+0x56` path and combat consumers are pending |
| Clips | `0x11B58`, `0x11CCA`, `0x11D86`, `0x11DD2`, `0x11E7E`, `0x11FDE`, `0x12014`, `0x12062`, `0x1209A`, `0x120E4`, `0x36FC` | Partial | Source top-level/nested selectors, typed Joshua/save roster projection, +0x58 selected-runner names, ammo gates, Reload confirmation/consumption/clamp, recipient filtering/cycling, quantity wrap, clip transfer, completion message, and Distribute average are present; loaded-save field composition and final panel art remain |
| Attributes/skills view | `0xD01C` | Implemented | Command-menu route is view-only: FFF116 sentinel, live values, selector, A/B/Start return, full redraw, and no runner mutation are covered by the Qt QA |
| Hotel Karma upgrades | `0xD024` via `0xCF38` | Implemented | Hotel option opens the spend-enabled child route; source cost/limits, Yes/No mutation, Karma change, child/parent redraw, and Hotel return are covered by Qt QA |
| Hospital Options | `0x1FA02`, `0xC928`, `0xCE7C` | Partial | Scene-4 record 93/event 4 is reached by ordinary Up and opens the nurse prompt; A opens the same-map three-row menu, with Cyberware and No/Yes exit handoff covered in Debug/Release physical and full-frame QA. Medical Attention `0xCA2E`, B/action `0x391A`, and independent Genesis hospital visuals remain open; see `visual-audits/council-island-hospital-options.md` |
| Cyberware | `0x11950` | Partial | Empty branch and source bit 0..14 installed-row scan use a typed model; the center child panel now preserves the shared command portrait, inventory, posture, tab, and right HUD. Debug/Release full-screen QA checks entry, 30-frame hold, return, reopen, and both populated/empty states. No independent Genesis Cyberware image or mutation flow is available |
| Runner switching | `0x391A` | Partial | Qt C input now syncs `RunnerSwitchState` with the typed active party, moves the source `+0x10` selection marker, requests event `0x25`, and refreshes the active record/HUD used by held-A actions. Debug/Release route QA confirms no-eligible Joshua retention and C-selects a source save-template Ricky before reloading his record. Normal story recruitment/save restore, the source world-target/fade/animation tail, and an independent Genesis C-switch capture remain open |
| Pocket Secretary / Notebook | `0xC2A6`, `0xC3E6`, `0xC376`, `0xC8C8`, `0xF236`, `0xF2E2`, `0xF33E`, `0xF312`, `0xF5F4`, `0xF6DC`, `0xF840`, `0x18150`, `0xC448` | Partial | Qt connects the source five-row base, Group Items empty/populated branches, Cyberdeck no-deck response, Notebook base list, Current Run no-run wait/return, Mr. Johnsons, Contacts, and Shadowrunners pages. ROM-derived full-frame Debug/Release QA covers Contacts/Shadowrunners list, details, paging, exact-eight behavior, and clean parent returns. The Notebook dump checks coordinates and stale-row clearing. The Dismiss route checks fixed Abort/name placement, compact-marker state, selected-runner preview, current-runner replacement, and inner-panel replacement on return. F312/C448 call E660 once per VBlank; the typed random 1x2 effect is checked against child text and controls. Positive current-run records, normal quest acquisition and restored-save flags, Cyberdeck, Save/Load, restricted-mode gates, route-specific Windows audio loopback, and independent Pocket Secretary Genesis frames remain open. Shared audio QA verifies decoded playback/default-output state/duration. Existing NativePort menu paths do not count as Qt implementation evidence. |
| Magic | `0xFBA6`, `0x10260`, `0x10276`, `0x102EC`, `0x1055E`, `0x105E6`, `0x1088C`, `0x11190` | Partial | Qt connects the typed Magical Defense fallback and spell-panel display/browsing. Spell QA checks the corrected DBF spell-table bounds, complete 3x3 icons, selected header glyph, four metric rows, live selection/four cursor phases, command-center clearing, retained HUD/portrait, and return/reopen; independent longplay frames 000001/000002/000005 were inspected. Source cursor phase advances every seven field frames. Parent event 0x24 maps to `menu-024.flac` (2.484 s metadata); shared audio QA verifies decoding, PlayingState on the default output, duration, and stop, while Windows loopback for the exact Magic route remains unrecorded. Target/effect action and broader independent visual coverage remain open |
| Matrix/cyberdeck | `0x181F8`, `0x1822C`, `0x54662` and typed Lua session/menu paths | Partial | Cyberdeck and static node traversal routes render the typed node, fixed blue traversal floor, HUD, deck footer, and ROM-selected world-effect phase. Debug/Release QA rejects node-security recoloring of the floor. The 56-frame entry projection, moving camera, raised action/hand effects, live Matrix actor cadence, and matched active-alert visual check remain open. See `visual-audits/matrix-traversal-viewport.md` |
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
