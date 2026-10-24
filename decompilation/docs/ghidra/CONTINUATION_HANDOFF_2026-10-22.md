# Shadowrun Reconstruction Handoff

Date: 2026-10-22
Workspace: `C:\\Github\\shadowrun`

This handoff is for the next chat continuing the current authorized
implementation. The current worktree is shared; inspect it before editing.

## Project rules

The active project is exclusively `decompilation/ghidra_conversion/` and its
shared inputs under `decompilation/input/`, `export/`, `assets/`,
`gameplay/`, and `docs/ghidra/`. Do not extend or build
`archive/legacy_native_port/`. Keep the source addresses as provenance and
express recovered behavior through typed C++ host services and Lua-owned
gameplay/content. Do not add runtime bytecode copies, ROM-address decoders,
VDP/name-table emulation, or invented screens/routes.

Use the US ROM as primary evidence, Ghidra C as a cross-check, and the current
longplay video as a visual reference. Internal renderer captures prove host
output only; they are not independent Genesis evidence. Review full native
320x224 frames, including the bottom choice area, before making visual claims.
For a defended hill, the repository instructions require a ROM trace, typed
native state, a regression CTest, and a CLI `actual.bmp`; keep the status
partial when any requirement is missing.

The user wants dialogue pages/options and conditional branches in JSON, with
generic typed outcomes for continuation, exit, entry, and cinematics. Study the
ROM and longplay permutations before filling data. Keep the model readable,
extensible, testable, and Lua-owned. The user also expects focused commits and
preservation of unrelated worktree changes.

The user previously called out incorrect cyberdeck/menu art, misalignment,
scrambled lower areas, and cropped frames. Continue to inspect actual images at
native resolution and verify the selected UI graphics against ROM behavior and
the longplay. They also asked to use the newer debugger methodology in the
`grok_bot_2` plan. Read that plan and follow its exact methodology before
further debugger-based source work; do not assume or claim it was used already.

## Completed adjacent slice: Tarislar City Inn / Tabatha

The Tabatha JSON graph work described in the prior handoff was completed in
commits `ed1202d6c5` and `539d0cb906`. Its Debug focused CTests passed. Do not
repeat that slice unless new evidence identifies a defect.

## Current work: Sinsearach gate caller for command `0x44`

The prior note that Frosty's command `0x44` caller was unknown is stale. Static
US-ROM tracing connected the route:

1. Scene 9 event record 230 at `0x1B578E` covers the gate opening. In mode 9,
   event 0, finalizer `0x1FD80` selects its dialogue from story bits 18/20 and
   Group Item bit 5, then returns D6=-1 instead of taking generic event-table
   row 0 to field scene 13.
2. The guard's Elders choice reaches command `0x34` at `0x21CE4`. Its success
   gate requires story bits 13/17/18/19/20 set and Group Item bit 5/story bit
   30 clear; success opens Lady Gillian group `0xF57E4`, otherwise the Lodge
   escort/busy pages at `0xF4D14`.
3. Lady Gillian's Frosty choice dispatches command `0xF9` at `0x24DFE`. Story
   bit 9 selects `0xF57BC` (expecting you) or `0xF57D0` (not ready). MORE on
   `0xF57BC` dispatches command `0x44`; handler `0x21FD0` sets story bit 30,
   loads profile `0x121F4C`, and opens `0xF57F8`.

The active project now has an entity-owned Scene-9 route, Lua entry module, and
JSON dialogue graph through Frosty's first conversation pages. The graph uses
typed story conditions and actions for command `0x49`: it clears FFFC34 bit 4,
sets bit 5, and applies helper `0x5D130`'s runner source-byte triplet `+3`.
The ROM sequence then writes FFF0BE=9, selects data at `0x0F5870`/`0x1FE690`,
and calls the shared blocking story-page routine `0x5D1F6`. Its table begins
with page pointers `0x0FBAFB`, `0x0FBBBD`, and `0x0FBC6E`, followed by a
sentinel. The three ROM text records are imported under named keys in
`assets/dialogue/script-text.json`; `cinematics.json` records their order,
controller-event input, and source-context return, and the graph ends with a
typed `play_cinematic` outcome. The native presenter, FFF0BE/FFE172 state
handling around the sequence, and route QA remain unfinished. Commands
`0x39`/`0x3A` select
roots `0x11B6D4`/`0x11B6EC`; their fare command `0x38` attempts 1,000 nuyen.
The insufficient branch selects `0x11B51D`; payment selects `0x11B450` and
initializes mode state 6. The refusal return and final field transition remain
unmodeled.

The shared story-page source trace is now expanded in
`decompilation/docs/ghidra/cinematic-story-pages-source-audit.md`. ROM
`0x5D1F6` renders each ordered text pointer, waits for one controller event,
and advances after a mode-sized repeated tile-region write. Harlequin selects
ten rows and Spirit Eyes eight; both use 40 columns. The selected resource
profiles still need source-backed extraction and named host assets. No runtime
presenter, route test, CLI frame, or independent capture was added in this
continuation.

The initial route slice imported 32 Sinsearach keys over the existing 1,416
keys. For this continuation, the explicit asset extractor was rebuilt and run
with `--all-assets` into a temporary output root. Its generated catalog added
the three exposition keys but omitted 17 keys already present in the checked-in
catalog, so only those three extracted entries were merged into the active
asset. No route-specific CTest, Qt physical-input QA, CLI `actual.bmp`, or
independent Genesis capture has been produced. The route is partial and must
remain so.

Relevant files in the current slice:

- `decompilation/ghidra_conversion/tools/rom_research/graphics_extract.cpp`
- `decompilation/assets/dialogue/script-text.json`
- `decompilation/gameplay/dialogue/conversations/salish_sinsearach_gate.json`
- `decompilation/gameplay/dialogue/conditions.json`
- `decompilation/gameplay/dialogue/menus.json`
- `decompilation/gameplay/dialogue/speakers.json`
- `decompilation/gameplay/scripts/salish_sinsearach_gate.lua`
- `decompilation/gameplay/world/entities/salish_sinsearach_village_gate/entity.tsv`
- `decompilation/gameplay/world/entities/salish_sinsearach_village_gate/interactions.json`
- `decompilation/gameplay/world/areas/scene-09-site-routes.tsv`
- `decompilation/docs/ghidra/HILLS.md`
- `decompilation/docs/ghidra/decompiled-function-status.tsv`
- `decompilation/docs/ghidra/field-map-trigger-profile.md`
- `decompilation/docs/ghidra/visual-audits/puyallup-tarislar-apartments.md`

## Current worktree and next steps

The new gate route has not been run through a CTest or physical Qt scenario.
Before calling it defended, add focused Debug/Release route coverage, drive the
Scene-9 event with ordinary input, capture and inspect complete 320x224
`actual.bmp` frames, and verify the clean return after dialogue. Keep host
captures distinct from independent Genesis visual evidence.

The shared story-page routine is source-understood well enough to catalog the
three Sinsearach pages, but its native presenter and surrounding mode/service
state are not implemented. Add focused route CTest and physical-input QA,
capture and inspect full-frame actual.bmp output, then implement the presenter
and remaining state handling. The `0x39`/`0x3A` roots and `0x38` 1,000-nuyen
fare branch are traced but remain unmodeled; preserve their explicit graph
boundary until the response choices and field return are converted. The
Tarislar pending `FFE162`
descriptor route and Scene-54 activity/exit/return remain separate open work.

At the last known status, preserve these unrelated untracked directories and
files; do not use blanket staging:

- `decompilation/ghidra_conversion/big-rhino-mortimer-visual-audit/`
- `decompilation/ghidra_conversion/julius-strouther-visual-audit/`
- `decompilation/ghidra_conversion/tarislar-city-inn-visual-audit/`
- `grok_bot_2/`
- `decompilation/docs/ghidra/CONTINUATION_HANDOFF_2026-10-22.md`

## Continuation after this slice

First review the current entity route and dialogue graph, then add the named
Debug/Release CTest and physical-input QA needed to defend the Scene-9 route.
Implement and capture the catalogued three-page cinematic, including the
remaining FFF0BE/FFE172 state handling. Then model the traced `0x39`/`0x3A`
roots, `0x38` fare result branches, and unresolved field return.
Keep Tarislar's pending descriptor and Scene-54 completion/return as separate
source-backed slices. Continue broader work on
menu/cyberdeck fidelity, Lua building and vid-phone routing, and the reusable
JSON dialogue/outcome model. Record implementation, verification, independent
visual evidence, and unresolved source boundaries separately.

## 2026-10-23 continuation update

This update supersedes the unfinished-work statements above for the
Sinsearach cinematic and fare branches. `cinematic-story-pages-source-audit.md`
now records the named extracted assets, Lua page progression, typed Qt story
presenter, source-context return, and Debug/Release physical QA for both
Sinsearach and Spirit Eyes. The Sinsearach gate also has physical QA for the
main Frosty route, 999-nuyen and exact-funds fare outcomes, and three
additional ordinary-input permutations covering the question/answer loop,
Frosty-left self-loop and exits, Elders fallback, fare refusal, and settled
Scene-9 return. The new branch captures and the 27-file Debug/Release hash
comparison are recorded in
`visual-audits/sinsearach-gate-choice-permutation-audit-2026-10-23.md`.

Keep the gate hill partial: the hostile-root prompt lacks a dedicated Qt
physical-input capture and no independent Genesis encounter frame has been
identified. The story-page visual and Spirit Eyes visual claims also remain
host-only until a matching independent capture is found. The next separate
source-backed slice is the Tarislar City Inn paid field route, followed by the
unresolved post-attack world/combat outcome; its independent longplay frames
cover the intro through Tabatha's apartment prompt, but not those branches.
Preserve all previously listed unrelated worktree paths and avoid blanket
staging.

## 2026-10-23 City Inn payment continuation

The City Inn paid room route is now physically exercised from scene-1 record
68 with ordinary Up input. The QA checks the manager's Crash/Leave/Tabatha
labels, the fixed 30-nuyen prompt, and three wallet/action permutations:
exactly 30 then Pay debits to zero and reaches Hotel status/options through
the JSON graph's real `open_service hotel.options` result; 29 then Pay selects
the insufficient page, preserves 29, and returns to scene 1; 30 then No shows
farewell, preserves 30, and returns to scene 1. Debug and Release CTests pass,
the original Leave route still passes, and all twenty-three payment-route BMP hashes
match across configurations. Key full-frame states were inspected at 320x224.
The price is fixed by the ROM and the dynamic transaction input is the wallet;
this Inn route has no class/race price modifier. Do not generalize that
observation to profile-dependent character statistics or combat bars.

The audit remains partial. The independent longplay does not show the room
payment alternatives, Tabatha paths are graph-level rather than ordinary
field-entry QA, and the post-attack actor/combat result is still unestablished.
The next requested audit focus is the class/race preset matrix and numerical
displays: enumerate supported combinations, source-trace starting stats,
abilities, per-stat minima/maxima, and formula inputs such as the Off/Def
bars; capture action/state permutations and keep host screenshots separate
from independent Genesis evidence. The following untracked audits already
exist and should be reviewed before adding duplicate work:
`archetype-profile-permutation-audit-2026-10-23.md`,
`attributes-skills-profile-permutation-audit-2026-10-23.md`,
`equipment-combat-bar-profile-permutation-audit-2026-10-23.md`, and
`magic-defense-party-composition-audit-2026-10-23.md`.

## 2026-10-23 City Inn Tabatha physical permutations

The ordinary field-entry audit now covers the full City Inn action cross
product: Crash, Leave, and Tabatha at the manager menu; Pay and No at the room
offer; Caleb-first and Errascoe-first; then Recruit and Thanks. Four dedicated
Debug/Release CTests physically enter scene-1 record 68, traverse each Tabatha
route through command `0x14`, the refusal/attack branches, Renraku and Tabatha
profile changes, David's Holopix, and a clean scene-1 return. A/B option labels
and story text are asserted against their source-linked graph IDs.

The Holopix route checks the 30-frame input wait and discards an early A press,
then samples a neutral release before accepting the next A edge. Command `0x14`
now preflights each FFFC30/FFFC27 store it will mutate before the Karma service
runs; the pure-Lua test verifies unresolved saved flags and unavailable runner
state produce a source boundary with no partial acceptance update. The known
fresh-game QA baseline is bit 0 set in FFFC30, the other action bits clear, and
FFFC27 clear. The focused `tarislar_city_inn_tests.lua` and the nine City Inn /
shared-dialogue CTests pass in both Debug and Release. The eight field-route QA
outputs contain 101 full-frame BMPs per configuration, all matching by SHA-256.

At 320x224, I inspected both topic responses, the attack page, Renraku profile,
David's portrait/Holopix, and the final field frame. The independent longplay
confirms the manager menu, blank phone pages, and apartment prompt. Payment and
post-attack permutations remain host-only; the ROM handlers switch portraits
and prompt roots but do not establish an attacker spawn or combat-world result.
The City Inn price is fixed at 30 nuyen and only its affordability outcome
depends on the wallet/story gate; this does not generalize to profile-derived
stats or Off/Def bars.

## Next audit focus

Continue the user-requested UI/video audit work with the class/race and
numeric-display permutations. The profile audit files named above already
catalog the 13 starter/recruited profiles, source race maxima, skill bounds and
costs, combat-bar factors, ammo state, and Magic defense party composition.
Review their source and test coverage before adding another slice, then deepen
Off/Def checks over archetype/race, weapon and armor changes, posture, spells,
and party conditions. Keep every numeric value tied to its ROM-backed inputs
and state transition; compare against independent frames only when the same
profile/action state is visible there. Continue preserving unrelated worktree
changes and keep implementation, internal capture, and independent visual
evidence separate in the records.

## 2026-10-23 profile-aware Off/Def screen continuation

The command-screen audit now extends the existing ROM arithmetic matrix to
the visible field path. A new Qt CTest loads the three Joshua starter records
and all ten named recruited templates, checks each race/class identity, then
opens command page 2 and drives the live posture focus with controller edges.
For each of the thirteen records it checks all five ROM-expected Atk/Def
values, exact named bar assets at the source positions, sound `0x25` on each
posture adjustment, both endpoint clamps, exact Down redraw, and clean Start
return. Only the fixture's starting posture is normalized to Full Defense.

The QA writes 65 full 320x224 host frames per build; Debug and Release both
pass and all frame hashes match. I visually inspected Samurai, Troll, Mage,
Shaman, and posture endpoints. Independent section-06 frame `002550` was
reviewed again and supports the Gator Shaman's neutral Mana Zap display and
nearby bar rows only; it does not independently validate all class/race
presets or posture values. Keep the combat-bar hill partial.

Next continue the total numeric-display audit through actual item/equipment
selection and stat-upgrade presentation: check that record mutations refresh
the same bars and that every class/race Attributes/Skills grid shows its own
ROM values, restrictions, caps, and preview costs. For every number, record
the source field, formula or lookup, input action that changes it, and the
screen update. Do not infer a race/class branch when the ROM formula only
consumes that profile's changed stats, skills, gear, or limits. Keep internal
Qt captures separate from independent Genesis comparison.

## 2026-10-23 class/race Attributes/Skills screen continuation

A second Qt permutation now carries all 13 source profiles through the actual
Start-menu Attributes/Skills view. It confirms each profile's race/class
header, then walks all 20 cursor cells with controller inputs. Across those
states the screen's numeric glyphs and labels are compared against the
corresponding raw profile bytes, while typed cell offsets and the class-only
Sorcery restriction are checked separately. A leaves the child without
changing stats, skills, or Karma; Start then restores the exact field frame.
The test checks 260 profile/cell combinations and writes one full 320x224 BMP
per profile; Debug and Release pass and all thirteen image hashes match.
Screens were inspected for Samurai, Rat Shaman, Bear Shaman, Elf Mage, and
Troll Samurai. There is no independent Genesis capture of this page, so these
screens establish internal presentation against ROM records rather than
independent visual fidelity.

The existing source test still covers the five race maximum rows, every skill
rank boundary and cost branch, view-only versus upgrade modes, and confirmed
Karma/stat writes. The physical equipment-selection audit is now covered for
all 13 source profiles; continue with race/class upgrade-limit and preview-cost
states on the separate Hotel route. Keep each display number connected to its
source bytes, formula or table, user action, and resulting pixels. Avoid
extending a numeric range from the broken implementation or treating Qt
captures as Genesis references.

## 2026-10-23 physical Equip to Off/Def audit

The all-profile combat-bar QA now performs a real command-menu Equip action
for weapon 1 on each of the three Joshua records and ten named recruited
records. It uses Left/A/A/Right/Right, confirms the active record's equipped
slot and item code, then checks the displayed command average, the +0x81/+0x82
attack skill pair, Body, flag-bit-5 versus item-0x27 priority, the first
cyberware bonus bit, and both named bar strips. The race/class preset fields
and all other profile data remain in the record. It also captures the changed
field HUD and checks that a second command-menu open/Start return reproduces
the post-equipment field frame exactly.

Debug and Release CTests pass. Each run writes 65 posture frames, 13 post-equip
bar frames, and 26 pre/post field frames; all 104 BMP hashes match across
configurations. The inspected Troll Samurai frames show the expected field
HUD transition after equipping. These are host captures; independent Genesis
evidence remains limited to the Gator Shaman Mana Zap frame. The field-action
baseline artifact previously called `06_combat_bars_after_equip.bmp` was
actually captured after restoring the source record; it is now accurately
named `06_combat_bars_after_fixture_restore.bmp`.

Next audit Hotel upgrade previews across source race/class profiles: enumerate
which stats are selectable, race-specific maxima, class-gated skills, current
Karma and exact preview costs; test min/max boundaries, insufficient Karma,
Cancel/No/Yes, and the record/Karma/pixel changes after a confirmed purchase.
Keep this separate from the command-menu read-only Attributes/Skills page and
keep missing independent Genesis screens clearly marked.

## 2026-10-23 Hotel Karma profile permutation continuation

The Hotel upgrade audit now physically enters the child for each of the three
Joshua presets and ten named recruit records. The QA compares all 0x100 bytes
of every named record asset with ROM `0x7291A..0x7351A`, reads the five-by-six
race ceiling table directly from `0xC22A6`, and independently calculates every
selected preview from its record bytes and traced source formula. Across all
thirteen records it walks all twenty cells (260 full-frame profile/cursor
states), checks selected numeric detail including the special Essence and
Magic sources, and verifies the current +0x8D Karma glyph.

Additional physical states cover all thirty race/attribute ceilings, Mage and
Shaman Sorcery rank 12, rank-12 Firearms for all four classes, Samurai/Decker
Sorcery rejection, a zero attribute increasing to one for cost one, and the
zero-Firearms/zero-Pistols reduced-cost path. Each profile attempts a Body
increase with one less than its ROM-derived cost, opens the exact-funds
default-Yes prompt with B, selects No and declines with B, then reopens with A
and confirms with A. The test checks Body, Karma, updated preview or maximum
message, and Start return for every profile. It resumes the ordinary Hotel
checkout branch after restoring its baseline record.

Debug and Release CTests pass for the original Hotel path and the profile
permutation. The profile run writes 398 full-screen BMPs per configuration;
all relative filenames and SHA-256 hashes match. I reviewed the normal
Joshua-Samurai preview, Mage Only response, Troll-Samurai cost, a Human
Samurai reaching the maximum, and the zero-rank reduced-cost frame at native
320x224. The error message shares y=176..184 with selected-description text in
these host captures. No matching independent Genesis Hotel Karma page occurs
in the supplied longplay, so keep the visual hill partial.

## 2026-10-23 all-profile physical armor permutation

The all-profile combat-bar QA now sweeps all seven armor items through the
13 source runner profiles (91 profile/item cases). It verifies source profile
bytes against ROM, physical item-cell selection, D4D50 `+0x25/+0x26` detail
rows, equipped `+0x76` summary and marker, `+0x6E` phase, the source
clear-then-equip/repeat-unequip route, and unchanged formula-derived Atk/Def
values and named bar pixels. It captures selected, clear, equipped-command,
equipped-field, and unequipped-field screens, plus the source no-armor baseline.

During QA, the source record's item-2 ammo value of 32 initially failed a
state-preservation check because `0x36FC` correctly clamps it to capacity 12
on the first HUD refresh. The test now reads the capacity from D4540 `+0x24`,
asserts that refresh result, and uses the refreshed value for subsequent
armor-state comparisons. The armor action itself leaves the ammo value,
weapon, stats, skills, and unrelated inventory unchanged. The equipped armor
index is not an input to the displayed Atk/Def formula: the command bars use
the `0x55E62` action modifier path, while combat hit resolution uses target
`+0x76` at `0x55948`.

Debug and Release CTests pass. Each armor subdirectory contains 539 BMPs; the
full audit output contains 643 images per configuration, all with matching
relative filenames and SHA-256 hashes. I inspected selected armor, cleared
replacement, equipped command page, and field return frames. Independent
longplay frame 002560 supports the selected Armor Clothing layout and lower
Wpns/Guns panel, but no exact replacement/equip transition or all-profile
visual sequence is present. The inventory and combat-bar hills remain
partial. Details are in
`visual-audits/armor-profile-permutation-audit-2026-10-23.md`.

## Next audit focus after armor

Continue with physical weapon selection across the thirteen source profiles.
Trace every weapon code and D4540 capacity/detail row, including how the active
magazine is normalized in `+0x4E`, how the next weapon receives that percentage,
and how the field HUD clamps and displays the resulting count. Exercise
selection, Equip/unequip/switch, repeated input, command Atk/Def refresh, Start
return, and inventory-empty cases. Keep the shared reserve clips in `+0x4F`
separate from per-weapon `+0x4E` rounds, and compare full-screen changes with
the independent video only where the same weapon and state are visible.


## 2026-10-23 field inventory detail-value phase audit

The selected detail number at `0x12A8C` now uses the 40-row `0x6AC46` table
and the actual number glyph upload from `0x334C` (`0x6A5BE` -> VRAM `0xDAC0`).
The profile combat-bar QA checks 43 occupied numeric-detail states from all 13
source runner records, phase 0..37; Debug and Release frame hashes match.
Independent section 06/07 frames show Medkit uses 06, 05, 04, 03, 02, and 01
across First Aid, target, result, and Inventory states; section 07 frame 000030
shows the result at 01 and 000120 returns to field. Host first-aid captures
match the `detail-value-6` and `detail-value-5` assets. Focused Debug and Release
CTests passed for First Aid and profile permutations. The inventory hill remains
partial because broad independent weapon/gear and ammo-transfer frames are
absent. Full findings are in
`visual-audits/field-item-detail-value-phase-audit-2026-10-23.md`. The retired
`detail-value-40..255.bmp` files still exist on disk but are not listed or loaded.

The next goal at that time was physical weapon-selection across all 13 source
profiles; that audit is now complete below. ROM trace starts at
`0x12148`/`0x12168`: selected item code comes from
`+0x66+slot`; weapon toggles flow through `0x121E0`, `0x125FA`, and `0x36FC`.
`0x125FA` stores ceil(`rounds*100 / D4540[weapon].+0x24`) in `+0x4E`; a new
weapon gets round-half-up(`percent*capacity / 100`). Shared reserve clips remain
`+0x4F`. Existing focused unit and field-action cases cover capacities 6, 11,
12, and 50, while source HUD QA covers all 12 D4540 weapon capacities. The
physical profile CTest currently tests a controlled weapon-1 action for each
profile; it does not yet sweep each profile's actual starter weapon and all 12
selected target capacities through the controller. That is the next audit gap.
The 13 source profiles carry starter weapon codes 1..11 plus one unarmed
Human-Samurai-1, and most have +0x4E=32 which is clamped by the field refresh
to their own D4540 capacity.


## 2026-10-23 all-profile weapon selection and ammo audit

Extended `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`
with ROM-derived D4540 capacities, a 29,952-state old-round/old-weapon/new-weapon
formula matrix across all 13 source profiles, and physical controller routes for
actual starter weapons plus distinct target codes 1..12. The physical route
checks switch, HUD counter glyph, code-dependent Atk/Def skill band, Start
return, same-cell unequip and re-equip, and unchanged shared +0x4F clip reserve.
Debug and Release CTests pass; each has 117 full-screen weapon frames and 1,872
manifest rows, with matching filenames and hashes. Source frames around section
06/07 show neighboring inventory/HUD states only, not the weapon switch itself;
keep the hill partial. Full source and visual analysis is in
`visual-audits/field-weapon-selection-profile-permutation-audit-2026-10-23.md`.

The test uncovered and now handles the source Human Samurai-1 unarmed sentinel
`+0x56=0x7F`, `+0x57=0`, `+0x4E=0`, `+0x4F=0`. Weapon-code combat bars use
three source skill bands (`0x12C74`): codes 1..7 use `+0x81/+0x82`, 8..10 use
`+0x81/+0x83`, and 11..12 use `+0x81/+0x84`. The QA formula checks the bar
values and named assets for all 13 source profiles and all three bands.

The section 06/07 independent-longplay scan is now complete. It found visible
Armor Clothing, Stim Patch/Medkit details, First Aid target/result pages, and a
return to field, but no weapon selection, clip reload, or post-switch Atk/Def
screen. The ROM trace and internal full-frame QA remain regression evidence;
the weapon visual claim remains partial.

## 2026-10-23 section 06/07 inventory count and First Aid boundary follow-up

Reopened full-resolution captures around the item changes: section 06 frames
002563 (Stim Patch uses 04), 002571/002580 (Medkit uses 06), 002610 (target
selection at 05), 002640 (Medkit used at 04), 002655 (03), 002685 (target
selection at 02), and 002699 (Inventory at 01); section 07 frame 000030 shows
the result at 01 and 000120 returns to field. The sample frames do not log every
input and do not show the final zero-count inventory cell. The longer sampled
sequence is recorded in
`visual-audits/field-item-detail-value-phase-audit-2026-10-23.md`.

Raw ROM path: selected item dispatch at `0x1243C`; Medkit effect/fully-treated
guard at `0x1253E`; successful treatment decrements the selected `+0x6E` phase
at `0x124DC`; phase zero clears the slot through `0x1275C`; the detail renderer
`0x12A8C` reads that slot phase. Item code `0x13` is routed to automatic use by
the item classifier, while First Aid codes are `0x10..0x12`. A helper audit
found C++ accepted `0x13` as a physical patch, despite the Lua rule and menu
classifier; the C++ guard and unit fixture now reject it. This was a helper/test
contract defect; the physical controller selection already filtered the code.

Focused `shadowrun_ghidra_first_aid_conversion` and
`shadowrun_ghidra_conversion_qt_field_first_aid_qa` pass in Debug and Release.
The unit case checks phase 2 -> 1 -> clear and fully-treated no-consumption; the
full-screen controller route still covers one use only. The Debug and Release
Qt QA runner targets were rebuilt. The weapon-selection profile permutation
audit is internally covered across all 13 ROM profiles, but independent
weapon-switch/reload imagery remains absent.

## 2026-10-23 ROM-backed First Aid profile formula audit

The Medkit formula is now documented against raw ROM `0x1253E` and
`0x0DBA`. It takes the acting runner's Biotech `+0x89`; the selected target's
physical `+0x4C` selects difficulty 8/6/4, target Body `+0x77 / 3` reduces it
to a floor of 2, and target mental `+0x9C` affects the fully-treated guard and
the treatment result but not difficulty. Zero successes still apply one point;
successful effects use the two meter writers, clear status bits 6/4, set bit 0,
and install `0xB45C`. The target's race-specific Body ceiling comes from
`0xC22A6`.

`shadowrun_ghidra_first_aid_conversion` now reads all thirteen source records
from the US ROM and sweeps all user/target profile pairs, every Body value up
to the target race ceiling, every physical/mental meter pair, and all legal
success counts. It checks 436,320 treatment outcomes and 1,313 fully-treated
no-roll/no-mutation cases. Debug and Release pass. The existing Lua route test
and Qt field First Aid QA also pass in both configurations; their presentation
and control evidence remains one visible Gator/Shaman longplay sequence and
one Samurai host route, not thirteen independent screenshots.

The audit exposed an architecture boundary in the evidence records: the Lua
`menu/screens/item_action.lua` First Aid path and the Qt
`FieldSceneRuntime` C++ controller/helper are parallel implementations. The
Qt field call at `field_scene_runtime.cpp:12024-12037` passes active runner
`skills[8]` and target meters/Body into the C++ helper. The new matrix validates
that helper, not Lua execution. HILLS, screen-system audit, and function-status
notes were corrected to keep those paths distinct. Consolidating First Aid
under one Lua-owned authoritative state remains open; do this before claiming
the Qt First Aid hill architecturally defended. Keep the visual hill partial:
the longplay contains no profile-wide or outcome-number presentation proof.

Audit note:
`decompilation/docs/ghidra/visual-audits/first-aid-medkit-profile-formula-audit-2026-10-23.md`.

## 2026-10-23 First Aid fully-treated controller permutation follow-up

Extended `shadowrun_ghidra_conversion_qt_field_first_aid_qa` so each of the
13 raw-ROM race/class profiles takes the physical controller path through a
successful Medkit use, result acknowledgement, Inventory return, then a second
Medkit attempt against a target with both meters at 10. The second route
reseats the cell through directional input because the source redraw suppresses
the inventory selector after nested return. It asserts the source
`items.text.option.first_aid_not_injured` message, zero six-sided dice samples,
unchanged meters/status/continuation/item/phase, retained displayed use count
01, and a clean return. The test seeds a one-use Medkit for this second branch
after the successful-use phase may have depleted its stack.

The Troll Samurai case uses legal race maximum Body 11, physical meter 8, and
the source difficulty floor 2. Five profile captures per record (65 total)
match by relative filename and SHA-256 across Debug/Release. The full-target
frame shows the split source message `This person is` / ` not injured!` and
the unchanged `01` use count. Debug/Release CTests pass for the Lua menu route,
Qt First Aid controller route, and 436,320-case ROM-profile formula matrix
(1,313 fully-treated no-roll helper cases).

These host captures verify the Qt controller path, not independent Genesis
screenshots. The supplied section 06/07 longplay still only supports the
Joshua Gator/Shaman successful Medkit sequence; it does not show this no-injury
message or all-profile presentation. Keep the First Aid/inventory hills
partial: Lua `item_action.lua` and Qt `FieldSceneRuntime` still hold parallel
First Aid state paths, source RNG startup parity is open, and route-specific
Windows audio loopback remains unrecorded. Updated status is in `HILLS.md`,
`decompiled-function-status.tsv`, `screen-system-audit.md`, the Qt visual
audit index, and the linked First Aid formula audit.

## 2026-10-23 First Aid patch profile permutation follow-up

Raw ROM caller `0x11740` reads the selected code at `0x1243C`; Stim Patch
`0x11` branches at `0x1247E`, Trauma Patch `0x12` branches at `0x1244C` /
`0x12466`, both converge at `0x124AC`, and phase `+0x6E` decrements at
`0x124AE`. Stim skips at target mental `+0x9C == 10` or writes +2 via
`0x37EC`; its conditional status-bit update checks physical `+0x4C`. Trauma
skips at physical `+0x4C == 10` or writes +2 via `0x3674`; its status update
checks mental `+0x9C`. A successful stack reaching zero clears via `0x1275C`.
Neither patch branch rolls dice; both share source message
`items.text.option.first_aid_slap_patch_used` on success and
`items.text.option.first_aid_not_injured` on the corresponding guard.
Ghidra `fn_00011740.cpp` is only the cross-check.

`shadowrun_ghidra_conversion_qt_field_first_aid_qa` now drives those success
and guard actions for every one of the 13 raw-ROM runner profiles: 52
controller actions, with source item slots/phases preserved when present and
controlled patches seeded in the Medkit cell otherwise. The cases cover the
zero/nonzero opposite-meter status paths and verify zero random-service events,
typed meter/status/continuation/phase state, displayed result and detail,
selector/HUD, and Inventory return. Section-06 frame `002563` independently
shows Joshua selecting Stim Patch at uses `04`; its placement/label supports
the inventory UI, while the Gator starter template is uses `06` and therefore
is not a numeric-state pixel match. No supplied Genesis capture shows either
patch effect or no-injury branch.

Debug/Release focused CTests pass for the Lua menu route, Qt First Aid physical
route, and ROM-backed formula matrix. The Medkit manifest has 13 rows, the
patch manifest has 52, and all 143 full-screen profile captures match across
configurations by filename and SHA-256. The full First Aid controller test
and audit notes are linked from `HILLS.md`,
`visual-audits/first-aid-patch-profile-permutation-audit-2026-10-23.md`, and
`visual-audits/first-aid-medkit-profile-formula-audit-2026-10-23.md`. Keep the
hill partial: Qt C++ and Lua First Aid state ownership is still duplicated,
the longplay has no patch effect/guard sequence, and exact route Windows audio
loopback is unrecorded.


## 2026-10-23 later-longplay Magic Defense search

Continued the independent visual check for the non-magic Magical Defense page.
Sampled 10,096 frames at one-second intervals across longplay sections 08-41
(source time 0720-10815 seconds). The green-pixel title locator used the host
Magical Defense title crop at `(120,48)` only to rank candidates; its strongest
exact-position match scored 0.5395. Opened top candidate frames in sections 09,
11-14, 20, 24, 26-27, and 33. They show Matrix/databank menus or NPC/run dialogue,
not the two-strip Defense page. This is bounded negative evidence: the locator
is not OCR and the one-second sampling cannot rule out a brief or differently
rendered page. No independent fallback capture was identified; keep the Magic
hill partial. Details are in `visual-audits/magic-spell-action-permutation-
audit-2026-10-23.md` and the Magic rows of `HILLS.md`,
`screen-system-audit.md`, and `visual-analysis/qt-visual-audit.tsv`.


## 2026-10-23 Pocket Secretary Current Run input permutation

Raw ROM `0xF312` waits on `FFDD22 & 0xFF00`, polling once per VBlank and
advancing E660 before the wait check. Ghidra `fn_0000F312.cpp` agrees. The
active Lua Current Run no-run route uses the shared `MESSAGE` screen with
`Pad.EDGES`; Qt paints the text and bezel from the Lua snapshot. The physical
`shadowrun_ghidra_conversion_qt_field_secretary_qa` route now covers a 30-frame
zero-edge hold and every nonzero controller byte `0x01..0xFF`. Each edge
returns to Notebook selection 0 and clears stale response text. The Debug and
Release manifests both have 256 rows and matching SHA-256
`72F60D8BB7C181A304F0AA10B5A272A556CBC824D38EDA47A8F962997CBD750E`; both
focused CTests pass. The host no-run, held, and return full-screen BMPs were
inspected. A bounded one-second text-template scan of all 10,815 longplay
frames in sections 01-41 found no matching response; top candidates were other
NPC dialogue. Keep the Notebook hill partial because positive current-run
details, restored-save prerequisites, and independent Genesis Notebook frames
remain open. Full trace and evidence: `visual-audits/pocket-secretary-current-
run-no-run-input-audit-2026-10-23.md`.

## 2026-10-23 Pocket Secretary active Current Run dispatch trace

Raw ROM `0x1E72C` dispatches the active Current Run detail page through
`0xF3DFA[FFFBED]`; canonical run types 0-6 map to the handlers recorded in
`visual-audits/pocket-secretary-current-run-no-run-input-audit-2026-10-23.md`.
The common builder at `0x1EE64` clears the inner panel and writes the context
label, run-type label, and `FFFBF4` value. ROM `0x1E69C` selects one context
offer row, sets progress `FFFBF3`, computes FFFBF4 from the base payment plus five times a roll bounded by
`scale * first_runner_negotiation + 1`, then calls the
initializer table at `0xF3DC6`. Shared payout at `0x1F00A` multiplies the
Ghoul Bounty amount by the live `FFFBF0` kill count; Matrix type 6 uses
`FFFBF0` as its target-system ID.

The Lua `run_registry.active_offer` already retains source initializer,
context, canonical type, payment, progress delta, and type-specific state.
`0x1E72C` is now indexed in `decompiled-function-status.tsv` as partial. The
positive page remains an explicit boundary: helper text operands are mapped
per type in the audit, but source initializer bounds, restored-save
prerequisites, and actual screen rendering are not fully validated.

The follow-on context audit found a Ghoul offer text mismatch. Raw `0x1E744`
sets area 2 in context 0 and rolls an area below 4 in context 2; the current
Lua offer already stored that result as `offer.area_id`, but its text always
used area 2. `run_ghoul_bounty.lua` now renders the stored area and rejects an
invalid index. The `shadowrun_ghidra_run_actor_defeat` test retains context-0
Redmond coverage and drives Mortimer/context 2 through its eight-row offer,
Negotiation roll, and all four area rolls, asserting each matching label.
Debug and Release CTest both pass. This is offer-text logic coverage, not a visual Current Run
capture. Preserve the earlier no-run input QA results and keep the Pocket
Secretary hill partial.


## 2026-10-23 Negotiation payment profile audit

Raw ROM `0x1E69C` reads the first live runner's Negotiation byte at
`FF018C` (`+0x8C`), computes `bound = scale * skill + 1`, and displays
`base_payment + 5 * roll`. The Qt service reads the first party record's
`RunnerSkillIndex::negotiation`; the skill conversion maps index 11 to
`+0x8C`. There is no direct class/race branch in the formula; the thirteen
ROM presets vary through their Negotiation ranks (0-4), while the Attributes /
Skills screen permits 0-12.

Added `gameplay/tests_lua/run_offer_payment_profile_tests.lua` to the Lua suite.
It drives the actual `run_support.negotiate_payment` against all 40 Johnson
offer rows, every skill rank 0-12, and each roll endpoint (1,040 cases). The
maximum amount is 6,450 nuyen across raw presets (base 6,350, scale 5 at
rank 4) and 6,700 at rank 12 (base 6,100, scale 10). ROM common builder
`0x1EE64` sends the amount to grouped decimal renderer `0xC216` at `$C322`,
which inserts a separator after every three digits; the maximum displays as
`6,700`. The test ran under Lua 5.4 and passed, but does not execute the ROM
glyph formatter or prove rendered pixels. Source trace and caveat are in
`visual-audits/run-contract-success-paths.md`; HILLS row 773 now lists the
check.

The positive Current Run page remains an unresolved boundary: the menu script
returns `CURRENT_RUN_UNRESOLVED` for an active offer, the Qt Notebook adapter
currently only recognizes the no-run wait, and save restoration does not
publish `FFFBEC` run state. Continue by tracing the positive branch's raw ROM
input loop, source field requirements, and seven handler presentations before
connecting typed Lua data to its renderer. Do not infer a restored-save UI from
the offer-construction test.


## 2026-10-23 Positive Current Run action path trace

Direct Capstone disassembly of the US ROM confirms `FFFBEC` is the signed
active context word: a negative value takes the no-run message, while 0-4 are
valid offer contexts. It is not a Boolean. `0xF236` dispatches the selected
Notebook row; active `0xF2E2` calls `0x1E72C`, whose type-specific handler
uses common builder `0x1EE64`, then the page enters shared wait `0xF312`. Any
new high-byte controller edge dismisses the page; return goes back through
`0xF236`, which redraws the Notebook at row 0. The positive page is a single
detail view with no additional selection/scroll action. Existing full-mask QA
covers only the no-run branch.

The positive Current Run permutation matrix still needs the 13 source
initializer IDs (canonical types 0-6), route/client/description variants,
Matrix's conditional system-location line, a no-edge hold, all 255 input masks
for each active detail path, clean return/reopen, and restored-save invariants.
The source field map and action-loop trace are recorded in
`visual-audits/pocket-secretary-current-run-no-run-input-audit-2026-10-23.md`
and status rows `0xF236`, `0xF2E2`, `0x1E72C`, and `0xF312`.


## 2026-10-23 Payment display and longplay follow-up

Raw ROM disassembly and Ghidra `fn_0000C216.cpp` agree that `$C322` uses the
grouped decimal glyph formatter, which writes least-significant-first and
inserts a separator after every three digits. Corrected the earlier mistaken
"four-digit field" wording in the test/docs. Added `6,450` and `6,700`
bitmap-font and glyph-index cases to
`ghidra_conversion/tests/rendering/decimal_glyph_conversion_test.cpp`;
`shadowrun_ghidra_decimal_glyph` passes in Debug and Release using
`ghidra-conversion-debug-all` / `ghidra-conversion-release-all` and
`build/conversion/root-workbench/{debug,release}`. The Lua 1,040-case payment
test also passes directly and appears in `run_lua_suites.sh`.

I reviewed the prebuilt contact sheets for all 41 current longplay sections
(5-second anchors in early sections, 10-second anchors in sections 08-41). No
sampled frame shows the positive Current Run detail page. This is bounded
negative evidence, not proof that the page never appears; an independent
positive-page screenshot remains unresolved.

The final full Lua suite run exits 0. `submenu_tests` passes 228/228,
`equiv_units` reports 17,100 calls with zero mismatches, and
`equiv_heartbeat` reports 6,000 calls with zero mismatches after the focused
fixture repairs recorded below. The payment-profile suite passes.

Focused `git diff --check` on the changed tracked audit/test paths passes. Whole
worktree `git diff --check` reports trailing spaces in the already-dirty
generated `docs/ghidra/rom-conversion-audit.tsv`; preserve that file and do not
normalize it as part of this slice.


## 2026-10-23 Independent New Game payment visual anchor

Inspected the supplied 320x224 longplay frame at
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_03_jump_house_stoker_job_0118-0238/frame_003300.png`:
the Jump House offer displays base 40 and negotiated 55. Raw context-zero row 4
at `0xF3EBC` is Package Delivery with base 40 and scale 1. The raw offer path
`0x2148A -> 0x1E69C` reads first-runner Negotiation from `FF018C`, stores
`FFF0C0=40`, and computes `FFFBF4=55`; `0x21526/0x2153C` pass those values to
`0xC216`. Same-session section-06 frame `002550` shows Joshua Human Shaman;
the source Joshua Gator Shaman preset is Human/Shaman with Negotiation 3, so
the bound is 4 and amount 55 implies roll 3. The Lua test now names that exact
row/profile/roll within its 1,040-case matrix. C++ Debug/Release grouped-glyph
tests cover 40, 55, 6,450, and 6,700. This independently checks the Johnson
offer amount against source and profile state, not the Notebook Current Run
page, whose positive detail screenshot remains unavailable.


## 2026-10-23 Matrix Pull Plug permutation and visual follow-up

Direct Capstone disassembly of the US ROM at `0x18450..0x185E2` confirms the
Yes/logout path: `0x185CE` writes `FFF0DC = FF`, calls the five-step
`0x18776` fade, then `0x26A00`. The latter plays `0x29` on entry and performs
the `14 + 6 + 11 + 6 + 6 + 11` VBlank display sequence. This is 10 fade frames
plus 54 display frames, returning at VBlank 64. The Lua route already used
that source clock; `submenu_tests.lua` had incorrectly expected the screen to
leave on the confirmation frame.

Updated `gameplay/tests_lua/submenu_tests.lua` to follow the complete action
sequence. It checks default-No dismissal; living Willpower failure returning
to the deck after `TN >> 1` physical damage; resisted Black IC with a
successful Body roll; Body failure subtracting 3 stun with the minimum of 1;
and lethal shock. The confirmations use A, B, and C. Each logout route checks
no sound during VBlanks 1-9, sound `0x29` on VBlank 10, transition input being
ignored through black VBlank 63, then Command return and its following `0x25`
refresh sound on VBlank 64. The focused Lua test passes 228/228.

Inspected the existing Debug native frames beside the independent Genesis
Section 15 captures. Full cross, collapsed cross, center dot, sparkle,
returning dot, and black frame match the sequence at `005420`, `005427`,
`005430`, `005435`, `005440`, and `005445`. The existing phase-mask audit
records IoU values 0.998/0.992/1.000/0.957/1.000/1.000. No C++ or Qt test was
run in this slice; the Lua suite was rerun after the test correction. Updated
`visual-audits/matrix-traversal-viewport.md` to remove its stale statement that
the transition art was unported. This follow-up's first full Lua run still had
the Salish and graph-fixture failures described below; both were then corrected.


## 2026-10-24 Lua fixture repairs and full-suite pass

The initial `salish_lodge_tests.lua` failure came from calling
`salish_red_buffalo.present_indexed_title_service` without installing the
`indexed_title_service` speaker/profile context returned by
`Building.resolve_field_location_route`. After supplying that returned
`salish.red_buffalo` / `red_buffalo_woman` context, the no-gift More path reached
the actual shared graph and exposed a second fixture omission: no graph,
condition, cinematic, menu, speaker, or script-text catalogs were loaded. The
test now loads the real Harlequin Main Conversation JSON and its catalogs,
dispatches the actual choice/page adapters, and asserts the final
`renraku_update` uses Red Buffalo's speaker and text profile. Direct
`salish_lodge_tests.lua` passes. Production content/runtime was not changed.

The `dialogue_graph_tests.lua` sample cinematic had become invalid as the
catalog schema gained required `assetAlias` and story-page `textLayout` fields;
its test host also lacked `present_story_page`. The fixture now supplies a
valid minimal alias/layout and verifies the cinematic asset alias, text key,
and layout reaching the presenter. Direct `dialogue_graph_tests.lua` passes.

The first rerun after those repairs isolated the only remaining failure to
`equiv_heartbeat`. Its first-difference diagnostic showed the actor-state
comparison was failing solely on the split runtime's root-level cached
`source_record_slot`; the pre-split reference consumes that context but does
not retain it on the actor root. The new slot cache is used by the split
heartbeat's typed actor-slot validation. The parity helper now omits only that
root field from serialized actor-state comparison while retaining exact
comparisons for contexts, return values, nested state, and service traces. The
focused `equiv_heartbeat.lua 200 1 30` run reports 6,000 calls, 2,492
non-nil results, 23 matching errors from both implementations, and zero
mismatches. This synthetic differential is an internal parity check, not
independent ROM-fidelity evidence.

The final full suite was run under MSYS Bash with `PATH` including
`C:\msys64\ucrt64\bin` and `C:\msys64\usr\bin`, and
`LUA_BIN=C:/msys64/ucrt64/bin/lua.exe`; `run_lua_suites.sh` exits 0. The
modified test/audit paths pass `git diff --check`. No C++/Qt test or build was
run in this continuation.

Changed paths in this continuation: `gameplay/tests_lua/submenu_tests.lua`,
`gameplay/tests_lua/salish_lodge_tests.lua`,
`gameplay/tests_lua/dialogue_graph_tests.lua`,
`gameplay/tests_lua/equiv_heartbeat.lua`, and
`docs/ghidra/visual-audits/matrix-traversal-viewport.md`, plus this
continuation handoff. The detailed task records and prior dirty/untracked
preservation notes remain in this handoff; do not stage unrelated worktree
changes.

## 2026-10-24 Sinsearach hostile-root physical route

The previous gate audit's only local QA gap was the hostile guard root's lack of
an ordinary-input full-frame capture. Direct Capstone disassembly of the US
ROM at `0x1FE38..0x1FE84` confirms bit 18 clear branches to prompt `0x11B5A0`;
if bit 18 is set, Group Item bit 5 can also select that prompt. With both
hostile conditions clear, bit 20 selects the friendly root `0x11B5BC` or the
Frosty root `0x11B5D8`. The hostile menu uses the same Questions / Elders /
Later labels, but has its own guard text.

Added `hostile-questions` to the existing physical choice-permutation QA and
registered `shadowrun_ghidra_conversion_qt_field_sinsearach_gate_hostile_root_physical_qa`.
It starts at Scene 9, clears story bit 18, leaves Group Item bit 5 clear, walks
Up into record 230/event 0, asserts `Hold it, round-ear!` and the ordered
choice labels, then exercises the Questions path, both question topics, the
question loop, and the Elders/busy fallback to command `0x39`. Its primary
`actual.bmp` is the hostile root. The test writes 11 full-frame captures;
Debug/Release hashes match for all eleven, including `actual.bmp` SHA-256
`a561adfab39e6cba1da23cfa01f1d75773a8eacafde6b4d5d1c6ec9267acd945`. I
inspected the full 320x224 root at native resolution. No independent Genesis
encounter frame has been identified, so the gate hill remains partial.

Builds:
- `cmake --build build/conversion/qt/debug --target shadowrun_ghidra_conversion_qt_qa`
- `cmake --build build/conversion/qt/release --target shadowrun_ghidra_conversion_qt_qa`

Focused Debug and Release CTest each pass all four physical choice routes:
`questions`, `hostile_root`, `frosty_elders`, and `frosty_later`.
`HILLS.md`, `decompiled-function-status.tsv`, and the gate choice audit now
record this gap as closed while retaining the independent visual limitation.

Changed paths in this slice: `ghidra_conversion/qt_runtime/CMakeLists.txt`,
`ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp`,
`docs/ghidra/HILLS.md`, `docs/ghidra/decompiled-function-status.tsv`,
`docs/ghidra/visual-audits/sinsearach-gate-choice-permutation-audit-2026-10-23.md`,
and this handoff. No production dialogue logic changed. Preserve all unrelated
dirty paths and do not stage files.
## Adjacent numeric-UI spot check and next source boundary

I reopened the independent Section 06 Genesis capture
`long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/frame_002550.png`
beside the Debug `field-spell-combat-bar-qa/actual.bmp`. The Human Shaman / Mana
Zap state, Damage and Power labels, Neutral posture pane, equipment rows, and
Atk/Def strip placement agree. The compressed 30-fps capture supports only
nearest-row corroboration for the bars; source formulas and live runner fields
remain authoritative for exact Attack 6 / row 5 and Defense 5 / row 4. No new
bar defect was found.

Next bounded source boundary is the positive Pocket Secretary Current Run
page: ROM `0xF2E2`/`0x1E72C` and seven canonical handlers are traced, and the
accepted Lua offer retains context, source initializer, type-specific routes,
clients/variants, objective data, and negotiated payment. The page is still an
explicit `CURRENT_RUN_UNRESOLVED` boundary; do not invent its strings or
formatting. Recover/extract the ROM text tables and exact dynamic line layout
before wiring a typed presenter. Current no-run QA does not cover this page.

## 2026-10-24 Current Run source text and Courier wrap audit

Continued ROM-first positive Current Run work. Raw US ROM traces now record
common builder `0x1EE64`, static labels `0xF3DA3`, context/type tables
`0xF1D8C`/`0xF1DEE`, saved amount `FFFBF4`, and output slots `C208`, `C21A`,
`C29A`, and `C322`. The type-specific detail text is assembled through
`FFB740` and helper `0xC168`; it wraps at 26 columns and preserves the
`0x80` row-break controls copied by `0xC134`. The audit now records per-type
input fields, conditional Matrix location text, Ghoul payment/count behavior,
and the shared handler fragments. Positive Current Run presenter and detail
page input remain unimplemented.

Independent Section 03 captures at global times 223s and 228s corroborate the
Courier offer's explicit blank row and a context-zero `$40` base to `$55`
negotiated result. Section 04 Bodyguard captures corroborate shared route/client
phrases. These are offer-dialogue helper checks, not Current Run page frames.
No positive Current Run page was seen in the sparse 146-frame early-longplay
sample; this does not rule out an unsampled visit.

Fixed Courier offer phrase boundaries in
`decompilation/gameplay/scripts/run_package_delivery.lua`: it now preserves
the source two-space phrase separators and only the two explicit row breaks
before “Take it to”. Added route coverage in
`decompilation/gameplay/tests_lua/run_target_event_tests.lua` for all 12,090
legal source route combinations across five contexts, including context-zero
same-location rejection. Focused Lua test passes:
`lua decompilation/gameplay/tests_lua/run_target_event_tests.lua`.
`git diff --check` passes for the two modified Lua paths.

Changed this slice: the two Lua paths above, the Current Run visual audit, and
this handoff. Preserve all other preexisting dirty/untracked paths. Do not mark
the Notebook hill defended; positive typed presentation, source initializer
and restored-save prerequisites, positive all-mask/return/reopen QA, and an
independent Genesis positive-page capture remain open. Next, inspect the active
Lua offer schema and menu route snapshot/presenter boundary before implementing
a typed Current Run view; keep offer body helpers and positive-page display
claims distinct.

## 2026-10-24 Pocket Secretary positive Current Run handler QA

This section supersedes the preceding next-step note about implementing the
typed positive page. The Pocket Secretary hill remains partial.

### Completed

- `run_support.current_run_view()` now resolves the accepted active offer's
  source initializer to one of seven ROM detail types and emits a typed view
  with the source Johnson/type labels, saved negotiated payment, and exact
  handler-specific body text. The wrapper follows `0xC168`'s 27-cell scan;
  soft wraps preserve the shared stream while `0x80` control breaks preserve
  explicit blank rows.
- The Lua menu opens `NOTEBOOK_CURRENT_RUN` (screen 41) on A and returns to
  Notebook row 0 on any new controller edge. The typed runtime bridge carries
  the view to Qt. Qt paints the ROM-derived header/body positions while Lua
  owns the menu wait and E660 cadence.
- Field QA found that the tree-screen predicate did not recognize
  `NOTEBOOK_CURRENT_RUN`, so the overlay closed after a successful route
  snapshot. The predicate now retains the Notebook shell for screen 41.
- The Debug/Release field QA seeds one valid source offer row for each of the
  seven canonical handlers, captures each full-screen page and its return,
  and compares every header, numeric, and body glyph against the extracted
  field font at the traced pixel coordinates. Together the seven frames cover
  all five Johnson contexts; Recovery uses initializer 9 and its two-site
  target pool. The Ghoul frame checks the low payment edge: base `$10`,
  Negotiation 0, bound 1, roll 0. The Matrix frame uses source Expert
  initializer 8/context 4 and verifies the grouped `$6,700` amount from base
  `$6,100`, Negotiation 12, bound 121, roll 120, plus the conditional
  Aztechnology objective line. Each type checks a clean row-0 Notebook redraw.
  Lua content QA covers all 13 source initializer IDs,
  all 24 Matrix objectives, 12,090 Courier/Bodyguard route pairs, 145,080
  Bodyguard route/client combinations, all four gang-description variants,
  and recovery/recruitment target pools.
- The positive input case uses the Section 03 context-0 Courier route from
  Ares Weapon Emporium `(2,11)` to Jackal's Lantern `(2,5)`, with Joshua Gator
  Shaman Negotiation 3 and the source row's base `$40`, bound 4, roll 3, and
  saved `$55`. QA holds the page for 30 frames, confirms only the lower bezel
  changes, and tests all masks `0x01..0xFF` with reopen and clean return. Both
  configurations produce a 257-line manifest with SHA-256
  `72F60D8BB7C181A304F0AA10B5A272A556CBC824D38EDA47A8F962997CBD750E`.

### Evidence and limits

- Builds from `decompilation/ghidra_conversion/`:
  `cmake --build build/conversion/qt/debug --target shadowrun_ghidra_conversion_qt_qa`
  and the corresponding `release` command.
- Debug/Release CTest passes:
  `ctest --preset ghidra-conversion-qt-debug-tests -R "shadowrun_ghidra_gameplay_script_menu_route|shadowrun_ghidra_conversion_qt_field_secretary_qa" --output-on-failure`
  and the same command with `ghidra-conversion-qt-release-tests`.
- Lua passes:
  `lua decompilation/gameplay/tests_lua/run_current_run_view_tests.lua` and
  `lua decompilation/gameplay/tests_lua/run_offer_payment_profile_tests.lua`
  and `lua decompilation/gameplay/tests_lua/run_registry_save_validation_tests.lua`.
- The full Lua suite and the 6,000-case heartbeat check pass through the
  repository wrapper. On this host, run it from PowerShell with MSYS
  utilities on PATH:
  $env:PATH = 'C:\msys64\usr\bin;C:\msys64\ucrt64\bin;' + $env:PATH; &
  C:\msys64\usr\bin\bash.exe decompilation/gameplay/tests_lua/run_lua_suites.sh.
  The Windows bash alias fails with a package-identity error.
- Debug/Release `shadowrun_ghidra_field_dialogue_runtime` now passes the live
  Mr. Gunderson interaction through the Section 03 Courier offer, Negotiation
  rank 3/bound 4/roll 3, `$40` base/`$55` accepted payment, and response return.
  Its former test expected the ROM `||` paragraph break to put the package and
  destination in separate indexed text segments; the actual blank-segment
  control break keeps the package phrase with the origin paragraph and the
  destination/payment phrase with the second paragraph. The assertions now
  check the ROM-derived body segments, not renderer behavior.
- Debug/Release CTests also pass for gameplay script runtime QA, run actor
  defeat, menu route, field dialogue QA, and field dialogue runtime.
- `run_registry.validate_saved_state()` now rejects source-backed state whose
  Johnson context, initializer alias, canonical run ID/type, progression delta,
  or saved payment does not join to a matching ROM `F3EBC` row. A new Lua suite
  covers all 40 rows x Negotiation ranks 0..12 x both roll endpoints (1,040
  states), malformed context/payment edges, and an accepted Courier save /
  restore at `$40` -> `$55`. This covers the tuple and numeric invariants; it
  does not prove the interactive Johnson-to-save route for every offer row or
  handler type.
- Focused `git diff --check` passes for the Current Run code, QA, and audit
  paths.
- All seven type `actual.bmp` hashes match between Debug and Release. They are
  under `build/conversion/qt/{debug,release}/qt_runtime/field-secretary-qa/notebook-records/current-run/type-{0..6}/actual.bmp`.
  Identical Debug/Release type-page SHA-256 values: type 0 `201F30EDFEF6061C26800F6DBD44ED81ABC984481B7622C08CE765FC5A595B53`;
  type 1 `37914549F3BE3723360422C8EF697FD94AF62B6E6312E75DF93210D7807DCA05`; type 2 `D330DA2958CB7BECA0B1A337E5A172CE53A7C558D678EFBDF53CEE6F8FC094FC`;
  type 3 `031F87C3FDA0C15AB09984D0424A35CF85131EAE9FAEE52E84C14C03CF8AC2FC`; type 4 `B8A9B830C14D0534F442A24BA467FE260BB5A274D07094CDDCED74311DD33632`;
  type 5 `12CD9F169DAB5E8E1EDF7FC8F18EC5398ED548921E26CC5F363778D1F9460117`; type 6 `53F7A0B20DF1E29B21D3CB8E8DBE16F3EBC344612D2D7AF1343A5BA13E0D177B`.
  The positive Courier page is also at
  `.../notebook-records/current-run/positive/actual.bmp`. These are host
  regression captures, not independent Genesis visual proof.
- The 257-line input manifest is
  `build/conversion/qt/{debug,release}/qt_runtime/field-secretary-qa/notebook-current-run-positive-input-masks.tsv`,
  SHA-256 `72F60D8BB7C181A304F0AA10B5A272A556CBC824D38EDA47A8F962997CBD750E`.
- Section 03 frames `003150` and `003300` independently corroborate the Courier
  offer text and `$40` to `$55` negotiation result. They do not show the
  positive Notebook detail page. No positive-page Genesis screenshot has been
  found.
- The field QA injects accepted-record state through its QA seed. It does not
  drive the Johnson acceptance/negotiation action into every active record.
  The standalone registry test now verifies exact source-row and negotiated
  payment invariants, and the field dialogue integration test drives the
  source-matched Courier acceptance path. Acceptance-to-save coverage for all
  source rows/types and a positive-page Genesis capture remain open. Keep
  status `partial`.

### Changed paths in this slice

`decompilation/gameplay/scripts/run_support.lua`,
`decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua`,
`decompilation/gameplay/scripts/enums.lua`,
`decompilation/gameplay/scripts/menu/field_init.lua`,
`decompilation/gameplay/tests_lua/run_current_run_view_tests.lua`,
`decompilation/gameplay/tests_lua/run_registry_save_validation_tests.lua`,
`decompilation/gameplay/scripts/run_registry.lua`,
`decompilation/gameplay/tests_lua/run_lua_suites.sh`,
`decompilation/ghidra_conversion/include/shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp`,
`decompilation/ghidra_conversion/include/shadowrun_ghidra/scripting/game_script_runtime.hpp`,
`decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp`,
`decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.hpp`,
`decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`,
`decompilation/ghidra_conversion/qt_runtime/src/srx_refactor/enums.hpp`,
`decompilation/ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp`,
`decompilation/ghidra_conversion/qt_runtime/tests/dialogue/field_dialogue_runtime_test.cpp`,
`decompilation/ghidra_conversion/tests/gameplay_script_menu_route_conversion_test.cpp`,
`decompilation/docs/ghidra/visual-audits/pocket-secretary-current-run-no-run-input-audit-2026-10-23.md`,
`decompilation/docs/ghidra/HILLS.md`,
`decompilation/docs/ghidra/decompiled-function-status.tsv`,
`decompilation/docs/ghidra/visual-analysis/qt-visual-audit.tsv`, and this
handoff. Preserve all other pre-existing dirty and untracked paths; do not
stage files.
