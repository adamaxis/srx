# Mike's RPG Center: Shadowrun reference notes

Research pass: 2026-10-10  
Reference: [Mike's RPG Center — Shadowrun](https://mikesrpgcenter.com/shadowrun/index.html)  
Evidence class: secondary guide/reference. The ROM remains authoritative for implemented behavior.

## How to use these notes

This page indexes the Shadowrun material linked from Mike's RPG Center: the
manual, walkthrough, item and character catalogs, six exterior-area maps,
eight wilderness/cave maps, and fourteen Matrix maps. Its game pages are
credited 2000–2011 and do not identify a ROM revision. Treat descriptions,
prices, names, and map labels as navigation clues. Before making a runtime
rule, confirm its conditions, state flags, coordinates, message, and route in
the US Genesis ROM; use the Ghidra export only as a cross-check.

The tables below summarize the useful findings rather than reproduce the
site. The site notes negotiation-zero prices for the catalog lists. Those
prices are reference values, not necessarily final transaction prices under
the player's current Negotiation skill.

## Parent-page inventory

| Page | Useful coverage |
|---|---|
| [Armor](https://mikesrpgcenter.com/shadowrun/armor.html) | Weapon and gun defense values for seven armor pieces; Lined Duster conceals illegal weapons. Prices are negotiation 0. |
| [Contacts](https://mikesrpgcenter.com/shadowrun/contacts.html) | Contact acquisition and offers: discounts, equipment, information, access, travel, and Lone Star record changes. Useful for quest prerequisites and commerce cross-checks. |
| [Cyberdecks](https://mikesrpgcenter.com/shadowrun/cyberdecks.html) | Six deck templates with Hardening, Response, MPCP, memory, storage, load, and source/price. Allegiance Alpha is placed with Michael's belongings. |
| [Cyberware](https://mikesrpgcenter.com/shadowrun/cyberware.html) | Eight implant names, Essence costs, effects, and clinic-specific prices. Prices differ substantially by district. |
| [Items](https://mikesrpgcenter.com/shadowrun/items.html) | General items, spell locks, foci, quest/group items, and availability. Includes the Rat's Nest Landmark Map prerequisite clue. |
| [Johnsons](https://mikesrpgcenter.com/shadowrun/johnsons.html) | Five Johnson entries, rough job role, and meeting place. Useful as a dialogue/quest contact cross-check. |
| [Manual (27-page PDF)](https://mikesrpgcenter.com/manuals/genesis/Shadowrun.pdf) | Officially linked Genesis manual. It is a separate document from this walkthrough/reference index; verify interface instructions against the actual ROM and captures. |
| [Maps index](https://mikesrpgcenter.com/shadowrun/maps.html) | Names and links for all exterior, wilderness/cave, and Matrix maps listed below. |
| [NPCs](https://mikesrpgcenter.com/shadowrun/npcs.html) | Additional named-runner list and Stark recovery chain. This page is not in the main Shadowrun index navigation. |
| [Programs](https://mikesrpgcenter.com/shadowrun/programs.html) | Matrix program roles and where to buy/find them; includes program decay/repair descriptions. |
| [Shadowrunners](https://mikesrpgcenter.com/shadowrun/shadowrunners.html) | Ten recruitable runners with archetype and location; Stark has a multi-step rescue/healing/recruitment dependency. |
| [Spells](https://mikesrpgcenter.com/shadowrun/spells.html) | Fourteen spell names, broad effects, and merchant locations. Spell level, target rules, resource cost, and timing still need ROM derivation. |
| [Walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html) | Story progression, prerequisites, optional detours, quest items, and a five-corporation Chernobyl passcode chain. |
| [Weapons](https://mikesrpgcenter.com/shadowrun/weapons.html) | Fifteen gun/grenade templates with damage, power label, ammo count, and availability; five accessories and restrictions. It does not describe the ROM's firing cadence or exact clip compatibility. |

### Catalog data worth cross-checking in ROM

- **Weapons:** Light pistols: American L36 (1 damage, Light, 11), Model 101T
  (1, Light, 12), Security 500 (1, Light, 25), Streetline Special (1, Low,
  6). Heavy pistols: Warhawk (3, Brutal, 6), Max-Power (3, High, 10),
  Predator (3, High, 15). SMGs: Mach 22 (3, Light, 24), AK-97 (3, Light,
  30), HK227-S (3, Medium, 28). Shotguns: Roomsweeper (6, High, 8),
  Allegiance (6, Brutal, 6). Grenades: Concussion (3, Brutal, 6), Scatter
  (3, Light, 6), Frag (6, Medium High, 6). The page's `Ammo` column is not
  enough to infer single/semi/automatic policy, attack speed, clip object
  identity, or whether one button edge consumes one or multiple rounds.
- **Accessories:** Smartgun System needs Smart Goggles or Smartlink to have an
  effect; Laser Sight is listed as incompatible with smartlink; Silencer is
  for pistols and Sound Suppressor for SMGs. The guide says unsilenced gunfire
  inside a building raises an alert. Verify the actual ROM alert path before
  adding behavior.
- **Armor:** Vest (1/2), Armor Clothing (2/1), Vest with Plates (3/4), Jacket
  (3/5), Lined Duster (2/3), Light Combat (4/6), Heavy Combat (6/8), where
  the page labels the columns Wpns/Guns. Do not silently swap these into
  runtime attack/defense fields without tracing how the ROM uses them.
- **Cyberdecks:** Allegiance Alpha (0 hardening, 0 response, MPCP 3, 30/120
  memory, 100/250 storage, 10/30 load), PCD-500 (1/0/4, 50/160, 100/325,
  20/40), Fuchi Cyber-5 (2/1/6, 100/240, 500/500, 20/60), SEGA CTY-360
  (3/1/8, 200/320, 500/650, 50/80), Fuchi Cyber-7 (4/2/10, 300/400,
  1000/1000, 50/100), Excalibur (5/3/12, 500/500, 1000/1000, 100/120).
  Paired figures are current/maximum according to the page's column labels.
- **Cyberware:** Datajack (0.2 Essence; deck access), CyberEyes (0.2;
  counters invisibility and aids combat), Hand Razors (0.1), Spurs (0.2),
  Smartlink (0.5), Muscle Replacement (1.0; Strength and Quickness), Dermal
  Plating (0.5; armor), Wired Reflexes (2.0; combat speed). The five clinic
  price schedules differ by area; use the linked page if validating a
  specific merchant.
- **General and quest items:** Electronic Kit; Maglock Passkeys ratings 2–5;
  Medkit; Smart Goggles; Stim Patch; Trauma Patch. Story items include Eye-
  Fiver Ear Cuff, Gargoyle Horn, two gifts for the Sinsearach trial,
  Halloweener Patch, Hell Hound Pelt, Indian Lands Visa, Landmark Map, Map
  Overlay, Ork Armband, Prototype Cyber-heart, Security Pass, and Weapon
  Permit. Spell locks and spell foci have separate rating rows and sellers.
- **Magic list:** Barrier, Confusion, Flame Bolt, Flame Dart, Heal Wounds,
  Hellblast, Invisibility, Mana Blast, Mana Storm, Mana Zap, Rockskin, Sleep,
  Stink, Super Barrier. The spells page describes effects and sellers; the
  items page separately covers focus/lock equipment. Spell resource usage is
  a gameplay rule to derive from ROM, not the guide's text.
- **Matrix programs:** Analyze, Attack, Deception, Degrade, Medic, Mirrors,
  Rebound, plus map merchant listings such as Slow, Shield, Smoke, Sleaze,
  and Relocate. The map listings show which shops offer these systems but
  are not a complete program behavior specification.

## Story outline and explicit dependencies

The walkthrough is organized as Redmond Barrens, Seattle, David Owlfeather,
The Ritual, The Sinsearach Elves, Renraku, Vigore, Ellisia's Tomb, and the
optional Chernobyl side quest. Its useful high-level dependency chain is:

1. Stoker's Coffin Motel: the guide says Michael's belongings are held for
   250 nuyen; Gunderson is the early job source. The listed contents include
   a letter, Allegiance Alpha, wristband, two holopix, and 500 nuyen.
2. Seattle/Puyallup information leads through Tabatha Shade, Boris, Caleb
   Brightmore, Frosty, and the Salish-Shidhe lead.
3. Reach David Owlfeather by a valid travel path; free Aragorn from Mitsuhama
   before Owlfeather identifies Spirit Eyes.
4. Spirit Eyes' three-part ritual uses a Gargoyle Horn, Hell Hound Pelt, and
   living Feathered Serpent scale. The site describes encounter/drop and
   travel alternatives; check each against ROM flags and drop tables.
5. Sinsearach trials: package/gift exchange, rescue Ilene Two Fists from
   Hollywood Correctional (level-5 maglock at the cell), then a verbal trial.
   Completing them reveals Frosty/Harlequin progression.
6. Renraku investigation: Club Penumbra has a reputation/entry gate and a
   Mako encounter; Matchstick/Ito yields a passcode; Renraku's sixth-floor
   safe contains Map Overlay and triggers an alarm.
7. Vigore progression: after returning to Harlequin, Gunderson identifies
   Rat's Nest; the Landmark Map is a separate item with explicit prerequisite
   clues below.
8. Ellisia's Tomb is the final route; the guide describes a Thon fight and
   a Feathered Serpent intervention.
9. Optional Chernobyl chain: collect five datastore fragments from Ares,
   Aztechnology, Fuchi, Mitsuhama, and Renraku, then use `CHERNOBYL` at UCAS
   Federal Government; the guide says the System Files node removes the
   virus and grants 50,000 nuyen.

The page has spelling/name variants (for example, Tabatha/Tabitha, Mako
Sochou/Sachou, and several misspellings in descriptions). Do not use guide
spelling to rename ROM-owned entities or dialogue.

## Exterior, wilderness, and cave map notes

| Map | Labeled locations and useful content |
|---|---|
| [Redmond Barrens](https://mikesrpgcenter.com/shadowrun/maps/redmond_barrens.html) | 1 Jump House/Gunderson; 2 Jackal's Lantern/Ricky; 3 Boris' Greenhouse; 4 ghoul warehouse; 5 Hollywood Correctional; 6 Stoker's Coffin Motel; 7 Ares Weapon Emporium; 8 Little Chiba's Chop Shop; 9 Rat's Nest; 10 Shiawase Atomics; 11 Halloweeners/Ratspike. Its numbered map image places Rat's Nest in the southwest, separate from Stoker's Motel. |
| [Downtown Seattle](https://mikesrpgcenter.com/shadowrun/maps/downtown.html) | Links to Penumbra/Renraku; Space Needle/Trent; Gates Undersound; Fuchi and Mitsuhama; Matchstick/Julius Strouther and Icarus Descending/Caleb Brightmore with entry gates; Roscoe buys data files; Seattle General Hospital. |
| [Penumbra District](https://mikesrpgcenter.com/shadowrun/maps/penumbra.html) | Link to Downtown; Fuchi, Mitsuhama, Lone Star, Big Rhino/Mortimer Reed, Wylie's Gala Inn, Eye-Fivers/Mauler, Club Penumbra. Guide gives entry requirements/contact offers. |
| [Puyallup Barrens](https://mikesrpgcenter.com/shadowrun/maps/puyallup.html) | Tarislar apartments/inn; Crime Mall; Underground 93; Ares; Riannon's Magic Shop; Wanderer/Vigore and Jarl; Dr. Bob's clinic; Orc hideout/Roadrash. The page enumerates weapon, armor, deck/program, spell, focus, and cyberware offers. |
| [Council Island](https://mikesrpgcenter.com/shadowrun/maps/council_island.html) | Passport Lodge with security check; Friendship Restaurant/Walking Bear; inn; Medicine Lodge and Spirit Eyes; Orc Embassy; hospital. Useful for travel, magic, and medical routes. |
| [Renraku Arcology](https://mikesrpgcenter.com/shadowrun/maps/renraku_arcology.html) | Downtown link; Renraku Computer Systems; Weapon's World; Wire-Masters; Merlin's Lore; Microtronics; Frag Grenade/Phantom. Merchant stock spans weapons, cyberware, magic, decks, programs, and deck upgrades. |
| [Wilderness Area 1](https://mikesrpgcenter.com/shadowrun/maps/wild1.html) | Arrival point, David Owlfeather, Red Buffalo Woman, entrances to Caves 1/2, one cave with no listed point of interest. |
| [Wilderness Area 2](https://mikesrpgcenter.com/shadowrun/maps/wild2.html) | Two connections to Cave 1, a no-interest cave, and a Cave 3 link. |
| [Wilderness Area 3](https://mikesrpgcenter.com/shadowrun/maps/wild3.html) | Cave 2/Cave 3 connections and Stark. |
| [Wilderness Area 4](https://mikesrpgcenter.com/shadowrun/maps/wild4.html) | Cave 3/Cave 4 links, Sinsearach, and a no-interest cave. |
| [Cave 1](https://mikesrpgcenter.com/shadowrun/maps/cave1.html) | Wilderness 1 and two links to Wilderness 2. |
| [Cave 2](https://mikesrpgcenter.com/shadowrun/maps/cave2.html) | Wilderness 1 and Wilderness 3 links. |
| [Cave 3](https://mikesrpgcenter.com/shadowrun/maps/cave3.html) | Links to Wilderness 2, 3, and 4. |
| [Cave 4](https://mikesrpgcenter.com/shadowrun/maps/cave4.html) | Wilderness 4, Renraku strike team, Feathered Serpent. |

## Matrix map index

The [Matrix map index](https://mikesrpgcenter.com/shadowrun/maps.html) lists
fourteen nodes. The individual maps label datastore/puzzle categories, not
world-map door destinations. Use the labels as a Matrix quest checklist, and
confirm access order, menu text, result flags, and alert effects from the ROM.

| Map | Guide labels / quest clue |
|---|---|
| [Ares](https://mikesrpgcenter.com/shadowrun/maps/ares.html) | File categories plus security control, maglocks, cameras, automated equipment, HVAC, and elevator; Chernobyl fragment `(H,R)` and an Ilene clue. |
| [Aztechnology](https://mikesrpgcenter.com/shadowrun/maps/aztechnology.html) | Data, project, competition, marketing, system, security, alert, elevator, equipment, camera; Chernobyl fragment `(B)`. |
| [City Hall](https://mikesrpgcenter.com/shadowrun/maps/cityhall.html) | Management, outdated/project/finance/security/confidential data and control/equipment nodes. |
| [Club Penumbra](https://mikesrpgcenter.com/shadowrun/maps/clubpenumbra.html) | System, finance, outdated, security, automated-equipment, camera nodes. |
| [Fuchi](https://mikesrpgcenter.com/shadowrun/maps/fuchi.html) | Security-file clue about prototype cybernetic heart; Chernobyl fragment `(N,L)`; management clue; maglock, elevator, camera and alarm nodes. |
| [Gates Undersound](https://mikesrpgcenter.com/shadowrun/maps/gatesundersound.html) | File categories, elevator/equipment/maglock/camera/underwater-equipment nodes. Exterior map identifies it as a hotel. |
| [Hollywood Correctional Facility](https://mikesrpgcenter.com/shadowrun/maps/hlywdcorrfac.html) | Prisoner Files clue for Ilene Two Fists; security, HVAC, and maglock nodes. |
| [Ito's System](https://mikesrpgcenter.com/shadowrun/maps/itossystem.html) | Renraku strike-team/Mako clue, a map clue, Vigore/Thon clue, and Tomb of Ellisia clue. |
| [Lone Star](https://mikesrpgcenter.com/shadowrun/maps/lonestar.html) | Vigore and cybernetic-heart clues; case/legal/prisoner/security data plus alert, elevator, maglock, camera, and equipment controls. |
| [Mitsuhama](https://mikesrpgcenter.com/shadowrun/maps/mitsuhama.html) | AmerIndian prisoner clue; Chernobyl fragment `(C,O)`; project/security/management and control nodes. |
| [Renraku](https://mikesrpgcenter.com/shadowrun/maps/renraku.html) | Mako, Ito, Map Overlay/Ellisia, Frosty, and Joshua clues; Chernobyl fragment `(E,Y)`; system, camera, maglock, HVAC, alert nodes. |
| [Seattle General Hospital](https://mikesrpgcenter.com/shadowrun/maps/seattlegenhosp.html) | Outdated, system, management, and automated-equipment nodes. |
| [UCAS Federal Government](https://mikesrpgcenter.com/shadowrun/maps/ucasfedgov.html) | Chernobyl-virus clue and system/prisoner/finance/management/security data with maglock, camera, HVAC, satellite, and alert controls. |
| [Unlisted](https://mikesrpgcenter.com/shadowrun/maps/unlisted.html) | A collage of unnamed nodes with different datastore/control layouts. Preserve them as unlisted until ROM/map evidence identifies each system. |

## Official Genesis manual cross-check

The linked [official Genesis manual](https://mikesrpgcenter.com/manuals/genesis/Shadowrun.pdf)
is a 27-page scanned PDF, so its text is not searchable in the original file.
These notes come from OCR of the page images; OCR misspellings and collapsed
column layouts are not treated as game facts.
The manual describes intended controls and player-facing rules. It does not
establish frame cadence, exact formulas, hidden prerequisites, or the ROM's
implementation details.

### Field controls and character screens

- Overhead field controls: D-pad moves; A fires the equipped gun; B targets an
  enemy; C switches team members; Start opens Character Info. In Character
  Info, A/B select and Start exits; C switches team members. Cybercombat uses
  A/B/C to launch a program in offensive/neutral/defensive posture.
- The field HUD's portrait panel shows each active teammate, equipped firearm
  or spell, remaining ammunition when holding a weapon, physical health, and
  mental health. The target marker color communicates the target's condition;
  targeting makes the selected character face the enemy.
- Character Info exposes Clips, Attribute/Skills, Cyberware, Magic, and Pocket
  Secretary. The manual explicitly distinguishes mage/shaman Magic Selection
  from non-mage physical/mental defenses against magic. It says posture affects
  attack and defense bars; magic users also see spell drain change with force
  and posture. This supports the project's separate mage/non-mage panels but
  does not identify the ROM variables or exact bar conversion.
- The inventory holds up to eight items. Selecting a weapon changes the attack
  bar; the item description appears beside its icon. Equip/Unequip, Trade,
  Discard, and Cancel are the item actions. The manual describes Clips as a
  screen for manual reload and team clip distribution, while field combat can
  reload automatically. Save/Load is available outside and not in combat.
- Posture controls the balance between offense and defense, changing those
  bars; spell drain may also change. For computer-controlled characters,
  defensive posture hangs back and may heal allies; offensive posture closes
  distance to deal damage. This is a behavior clue to verify against actor AI,
  not authorization to implement AI decisions in C++.
- Gun handling description: semi-automatic light pistols have high fire rate
  and short range; heavy pistols trade some rate for a bigger hit and better
  range; shotguns fire slowly because of the pump; full-auto SMGs fire three
  rounds at one close target or spread hits among distant targets. Movement,
  moving targets, long range, and invisible targets make shots less likely to
  hit; standing still and firing less can improve aim. Grenades are delayed,
  area-effect throws with variable landing accuracy. This explains why one
  input edge must not become an arbitrary burst, but exact shot cadence and
  ammo decrements still require source tracing.
- All weapons use the same ammo. The manual describes six uses per Medkit and
  six uses per slap patch; Stim heals mental damage and Trauma heals physical
  damage by 20%. Passkeys open locks at or below rating; an Electronics Kit
  reduces difficulty but can fail and raise an alarm. Use these as checks on
  item templates and actions, with the ROM deciding actual charges/formulas.
- Magic: spells use Force, can cause mental drain, and may be resisted by
  Willpower; Fetishes absorb drain and crumble. Targets must be in line of
  sight. Combat spells ignore armor, with Flame resisted by Body and Mana/Sleep
  by Willpower; Hellblast, Mana Storm, and Sleep are area spells. Heal Wounds
  repairs physical damage only. Illusions have limited duration; invisibility
  is less effective against CyberEyes and certain creatures. Foci can increase
  spell success without the drain of increasing spell Force; a Power Focus
  raises Force for all spells. These labels do not specify the ROM's targeting
  checks, tick counts, drain formula, or spell-ammo presentation.

### Locations, services, and quest flow

- The manual calls out distinct service classes: bars/clubs, weapon/magic/deck
  shops, hotels, hospitals, corporations, abandoned buildings, and wall
  cyberterminals. Hotel stays can restore physical and mental health and open
  karma spending; body affects healing rate. Hospitals handle severe wounds
  and team knockouts. Cyberterminals provide taxi, vid-phone, and Matrix
  options. This is a useful checklist for the shared location-service script,
  not proof that every marker opens a new interior map.
- Johnson jobs are recorded in the Notebook; another job is gated until the
  existing run is completed, canceled, or its required carried item is lost.
  Contacts offer information, services, equipment, and further contacts.
  Runners can have good/bad/neutral attitudes and hire conditions affected by
  mission outcomes, dismissal, abandonment, and team healing.
- Corporation interiors have three to six floors according to the manual.
  Alarms lock front doors, deploy guards, disable elevators, and make Matrix
  systems active-alert; the manual lists waiting, terminals, and the Matrix as
  ways to address the alarm. This is a broad system cross-check only; each
  building's actual floors, devices, and transition targets must be profiled
  from its ROM records.
- The manual's maps section starts at printed page 38. The scanned map labels
  complement the site's six map pages; use ROM scene/record data to correlate
  entrances and exits. The map illustrations do not show event activation
  conditions or prove whether a doorway requires collision, interaction, or a
  scripted story trigger.

### Matrix controls and mechanics

- Matrix entry needs a Deck, Datajack, and functioning MPCP; field terminals
  offer the access point. The deck interface separates the main view, right
  node/IC information, left program/alert HUD, and persona condition.
- Programs have load time based on size and compete for active memory. A second
  program pauses while another runs. A/B/C launch with offensive/neutral/
  defensive posture. The system map can retreat to the previous node before
  combat; jack-out may be blocked by Black IC.
- IC types have different roles: Access/Barrier block routes; Killer/Blaster
  attack the persona; Trace IC can dump or burn the deck; Relocate may cancel a
  trace. Alert packets can raise system alert if they reach the edge. These
  descriptions are navigation cues for the ROM's typed node/IC logic, not a
  substitute for its tables, outcomes, or timing.

### Manual-to-ROM discrepancy log

Keep discrepancies visible instead of silently reconciling them. The manual
calls out details such as “semi-auto” pistols and three-round SMG bursts, while
the game's own input/cadence implementation still has to be proven from the
ROM service loop and actor records. The Mike's RPG Center walkthrough is
shortened and omits the Rat's Nest shelter search narrative present in the ROM
text table. Conversely, the manual's generic business descriptions do not
name scene-specific service routes. The US ROM is the deciding source for all
runtime branches; guide and manual prose are labeled external corroboration.

## Redmond ROM coordinate cross-check: Jump House and Rat's Nest

The Mike's RPG Center Redmond map image is 479-by-326 pixels; the extracted
scene-2 background is 1920-by-1344 pixels. A proportional overlay (about four
background pixels per site-map pixel) places these ROM collision centers near
the site's numbered markers:

| Site map marker | ROM crosswalk candidate | Event and source geometry |
|---|---|---|
| 1 Jump House | Scene 2, record 85 at `0x1730FC` | Event 4, grid cell `(14,4)`, bounds `x=0x01C0..0x01DF`, `y=0x0080..0x009F`; event-table row 4 selects scene `0x0D` (13). This is the strongest Jump House entry candidate. |
| 6 Stoker's Coffin Motel | Scene 2, record 91 at `0x173138` | Event 2, grid cell `(30,20)`, bounds `x=0x03C0..0x03DF`, `y=0x0280..0x029F`; opening story route overrides the generic row. |
| 7 Ares Weapon Emporium | Scene 2, record 92 at `0x173142` | Event 11, grid cell `(38,17)`, bounds `x=0x04C0..0x04DF`, `y=0x0220..0x023F`; service route overrides the generic row. |
| 8 Little Chiba's Chop Shop | Scene 2, record 93 at `0x17314C` | Event 1, grid cell `(55,17)`, bounds `x=0x06E0..0x06FF`, `y=0x0220..0x023F`; table row selects resource-activity scene `0x1A` (26). |
| 9 Rat's Nest | Scene 2, record 97 at `0x173174` | Event 7, grid cell `(13,34)`, bounds `x=0x01A0..0x01BF`, `y=0x0440..0x045F`; special story service runs before the generic row. |

This supports identifying scene 2 as Redmond Barrens and record 97 as the
Rat's Nest approach. It also supports record 85 as the Jump House route into
scene 13. These are secondary-source name crosswalks, not names stored in the
ROM; the approximate overlay is not exact world-to-image calibration.

Scene 2 record 96 is another event-7 rectangle at cell `(23,37)` with bounds
`x=0x02E0..0x02FF`, `y=0x04A0..0x04BF`. The site overlay places it away from
marker 9, but the ROM routes both event-7 records through the same indexed
service. Keep the physical record distinction visible; do not silently label
both rectangles Rat's Nest.

For record 97, the ROM collision dispatcher publishes event 7 and reads
scene-2 event-table row 7 (`0x172CE3`), which is `0x1A`; the ROM classifies
that value as resource-activity scene 26. The guide says the Landmark Map
search follows Harlequin's request and asking Gunderson about Vigore. ROM
cross-check confirms the menu's conditional branches: the mode-command menu
builder at `0x22F90` includes “What do you know about a Johnson named
Vigore?” (`0x1045B0`) when `FFFC1E` bit 26 is set, and also includes a distinct
non-prerequisite response when that bit is clear and `FFFC27` bit 3 is set.
Selecting the bit-26 option dispatches action `0xE7` to `0x249A4`, which sets
`FFFC34` bit 13. The writers of the relevant world flag are now identified:
handlers `0x22018` and `0x22072` branch on `FFFC22` dialogue-state bits and
can set `FFFC1E` bit 26; handler `0x22072` also has a separate bit-27 / service
bit-14 outcome. These writers are traced in the ROM, but their originating
Harlequin choice and the live menu-to-action connection have not yet been
reconstructed in the Qt game.

The indexed service at `0x545B6` later requires scene 2, event 7, alternate
result 0, `FFFC34` bit 13 set, and consumed/one-shot bit 10 clear. It consumes
the prerequisite, marks bit 10 and the Landmark Map group item, awards +2
Karma to each runner record, and displays ROM text records `0x117193`,
`0x117266`, and `0x117314`. Its source `D6=-1` result bypasses the generic
table destination, so this search does not load scene 26. The service's Lua
pages and host effects are connected, but current QA seeds the prerequisite;
the complete lead-to-search route remains partial.

The Jump House path is now profiled as two separate behaviors. Scene-2 record
85 (`0x1730FC`, event 4) selects scene 13 through row 4 at `0x172CE0`. Before
the scene transition, indexed title service 22's branch at `0x20484..0x204C6`
selects descriptor `0x110D1E`, whose first text record is `0x116C12`: the
location description seen in longplay frame 001200. After the scene loads,
scene-13 event-30 records 11/12 (`0x1B3CF8`, `0x1B3D02`) lead through
`0xB7B8 -> 0x20990`. That scene branch selects row 0 of `0x12CC50`, profile
`0x121E80`, and the first Gunderson A/B/C prompt at `0x105114`. The 56x72 ROM
profile sprite at `0x121E80` matches the portrait in longplay frame 001500.
The site map names marker 1 Jump House and Mr. Gunderson; the walkthrough says
the player speaks to him in a booth. These external names now align with ROM
scene/event data and independent frames. The entry page uses a different
56x72 portrait at `0x121EBC`; the full Qt frame matches frame 001200. The live
scene-2/event-4 route now opens that Lua entry page and resumes scene 13 after
A. Scene-13/event-30 is bound to the Lua Johnson interaction, including the
initial root, information menu, dynamic question selector, Johnson answer,
question follow-up, and state-gated Vigore/Errascoe responses. The question
screens were compared with the numbered longplay frames; site names remain a
cross-check, not trigger or dialogue authority. Its ordinary-movement trigger
still lacks end-to-end QA, and the work/contact transaction branches remain
partial.

Scene 13 records 9/10 (`0x1B3CE4`, `0x1B3CEE`) are separate event-0 rectangles
at cells `(2,7)` and `(3,7)`. Their exact ROM exit/action semantics and return
destination are still unresolved. The site does not document a Rat's Nest
interior map, and it does not specify trigger mechanics for either location.
See the [Jump House entry and Gunderson visual audit](visual-audits/jump-house-entry-and-gunderson.md)
for the source trace and the 30-fps frame sequence.

The site labels Stoker's Motel as Redmond location 6 and Rat's Nest as
location 9. This supports keeping their events and dialogue separate; the
site does not say the motel has a dedicated interior map.

### Current project follow-up

- Add a source-derived Redmond location enum/crosswalk keyed by scene and
  trigger record (or another ROM-stable identity), rather than using scene ID
  26 alone as a building name. Several scene-2 table rows point into the same
  resource-activity class.
- Finish the Redmond lead chain by connecting the ROM-traced `FFFC1E` bit-26
  writers and Harlequin choices to the live Johnson-menu option at `0x1045B0`
  and action `0xE7`. The downstream event-7 pages and award run after the
  prerequisite is present; current QA seeds that prerequisite.
- Finish the source-profiled Jump House script: ordinary-movement QA for
  scene-13/event-30; full question/contact/work flows and their state/wallet
  effects; entry and work audio/timing; and the unknown event-0 exit route.
  The first work contract composes location/message records around `0xF2196`,
  `0xF21DD`, and `0xF23B2..0xF241F`; its formatted salary prompt is at
  `0x1060E0` or `0x106171` according to the source branch.
- Trace scene-13 event-0 records 9/10 before implementing the exit route and
  its return destination.
- Profile the other scene-2 event rectangles against the same map image and
  label only correspondences supported by both geometry and ROM paths.
- The site provides no button/menu layout or animation timing evidence. The
  section-03 longplay independently shows the entry narration, A/B/C menu,
  portrait, and page order; use ROM draw/cadence traces for exact animation and
  timing claims.

## Implementation cautions from this pass

- This site is especially useful for human-readable location labels and
  cross-area quest dependencies, which the numeric map-event tables do not
  encode in clear text.
- It does not define generic entry/exit behavior. A map marker is not proof
  that the ROM fires an event on entry rather than on collision, interaction,
  time, or a story condition.
- Do not spawn NPCs or bodies from walkthrough/map illustrations. The pages
  identify named contacts and static locations; runtime actor creation and
  persistence still need ROM actor-record/service evidence.
- Do not copy prose into Lua as unconditional dialogue. Dialogue, portrait,
  offered choices, item transfer, repeat visit, and exit state should be
  driven by the speaker/entity state and ROM-derived quest flags.
- Item descriptions are useful labels, not complete item templates. Price,
  icon, item code, stack rules, clip type/capacity, equip state, and combat
  behavior should come from the ROM's item/weapon records and UI routines.

## Expanded site comb: exact map labels and actionable cross-checks

This pass inspected the individual map pages as well as the category index,
catalog pages, walkthrough, and linked manual. The site has 6 city-area maps,
4 wilderness maps, 4 cave maps, and 14 Matrix layouts. The lists below keep
the site's numbered labels intact so they can be crosswalked against ROM map
records later. The site does not provide ROM scene IDs or coordinates.

### City-area maps and site-listed service gates

| Map | Numbered location labels and noteworthy details from the page |
|---|---|
| [Redmond Barrens](https://www.mikesrpgcenter.com/shadowrun/maps/redmond_barrens.html) | 1 Jump House: Gunderson, Wilma Temmenhoff contact (1,000), Ilene after rescue; 2 Jackal's Lantern: Ricky; 3 Boris' Greenhouse (100 to hear story; forged Salish-Shidhe passport 1,000; Rating-2 passkey 10,000; Mach 22 1,200); 4 ghoul warehouse; 5 Hollywood Correctional; 6 Stoker's (rest 20); 7 Ares Weapon Emporium; 8 Little Chiba's (medical/cyberware); 9 Rat's Nest; 10 Shiawase Atomics; 11 Halloweeners/Ratspike (1,500 or reputation-adjusted to talk; Halloweener Patch 3,000; Agira contact 10,000). |
| [Downtown Seattle](https://www.mikesrpgcenter.com/shadowrun/maps/downtown.html) | 1 Penumbra connection; 2 Renraku connection; 3 Space Needle/Trent and free Prof. Jefferson contact; 4 Gates Undersound (rest 75); 5 Fuchi; 6 Mitsuhama; 7 Matchstick (150 nuyen or reputation 6), Rianna, Julius Strouther, Quinton Jaymes contact (1,000); 8 Icarus Descending (reputation 6), Caleb Brightmore, Kipp David and Sergeant Chillicut contacts (2,000 each); 9 Roscoe buys data files; 10-11 abandoned buildings; 12 Seattle General Hospital (medical/cyberware). |
| [Penumbra District](https://www.mikesrpgcenter.com/shadowrun/maps/penumbra.html) | 1 Downtown connection; 2 Fuchi; 3 Mitsuhama; 4 abandoned building; 5 Lone Star; 6 Big Rhino/Mortimer Reed, Alesandro Hobbs contact (1,500), Winston Marrs; 7-8 abandoned buildings; 9 Wylie's Gala Inn (rest 50); 10 Eye-Fivers/Mauler (2,000 or reputation 6 to speak; Ear Cuff 4,000; Faradouchi contact 10,000); 11 Club Penumbra (reputation 4 and 75 nuyen), Freya. |
| [Renraku Arcology](https://www.mikesrpgcenter.com/shadowrun/maps/renraku_arcology.html) | 1 Downtown connection; 2 Renraku Computer Systems; 3 Weapon's World; 4 Wire-Masters; 5 Merlin's Lore; 6 Microtronics; 7 The Frag Grenade (entry 25), Phantom. Pages enumerate the stock of those stores, including weapons/clip, armor/medical items, cyberware, spells/foci, programs, decks, and deck upgrades. |
| [Puyallup Barrens](https://www.mikesrpgcenter.com/shadowrun/maps/puyallup.html) | 1 abandoned building; 2 Tarislar Garden Apartments; 3 Tarislar City Inn (rest 30); 4 Crime Mall (weapons, armor, clips, passkeys, electronics kit, accessories, programs, decks, upgrades); 5 Underground 93 (100 nuyen or reputation 6); 6 Ares Macrotechnology; 7 Riannon's Magic Shop; 8 The Wanderer/Vigore and Jarl, Max contact (750), Peter Uvehr; 9 Dr. Bob's Quickstitch Clinic; 10 and 12 abandoned buildings; 11 Orc Gang/Roadrash (750 or reputation 5; Ork Armband 2,000; Henry J. Culver contact). |
| [Council Island](https://www.mikesrpgcenter.com/shadowrun/maps/council_island.html) | 1 Passport Lodge (500 if security check passes); 2 Friendship Restaurant/Walking Bear; 3 inn (rest 40); 4 Medicine Lodge (magic/foci); 5 Orc Embassy; 6 hospital (medical/cyberware); 7 Spirit Eyes. |

The guide and its own location pages separate several easily conflated facts:
Gunderson is in Redmond's Jump House and is the lead to the Rat's Nest, while
the Johnsons catalog places Vigore and Jarl in Puyallup's Wanderer. The
walkthrough says Vigore is absent from the Rat's Nest and the player finds the
Landmark Map there. This is an external-site cross-reference, not proof that
the game spawns or moves those people in those places at every quest state.
The site calls the Redmond location 6 Stoker's and location 9 the Rat's Nest;
it gives no Rat's Nest interior map. Do not create an interior or place a
scenery corpse from these listings.

### Wilderness/cave route graph as labeled by the site

| Map | Numbered labels / links |
|---|---|
| [Wilderness 1](https://www.mikesrpgcenter.com/shadowrun/maps/wild1.html) | 1 arrival; 2 David Owlfeather; 3 Red Buffalo Woman; 4 Cave 1; 5 Cave 2; 6 cave with no listed point of interest. |
| [Wilderness 2](https://www.mikesrpgcenter.com/shadowrun/maps/wild2.html) | 1 and 2 are separate Cave 1 connections; 3 cave with no listed point of interest; 4 Cave 3. |
| [Wilderness 3](https://www.mikesrpgcenter.com/shadowrun/maps/wild3.html) | 1 Cave 2; 2 Cave 3; 3 Stark. |
| [Wilderness 4](https://www.mikesrpgcenter.com/shadowrun/maps/wild4.html) | 1 Cave 3; 2 Cave 4; 3 Sinsearach; 4 cave with no listed point of interest. |
| [Cave 1](https://www.mikesrpgcenter.com/shadowrun/maps/cave1.html) | 1 Wilderness 1; 2 and 3 are separate Wilderness 2 connections. |
| [Cave 2](https://www.mikesrpgcenter.com/shadowrun/maps/cave2.html) | 1 Wilderness 1; 2 Wilderness 3. |
| [Cave 3](https://www.mikesrpgcenter.com/shadowrun/maps/cave3.html) | 1 Wilderness 2; 2 Wilderness 3; 3 Wilderness 4. |
| [Cave 4](https://www.mikesrpgcenter.com/shadowrun/maps/cave4.html) | 1 Wilderness 4; 2 Renraku strike team; 3 Feathered Serpent. |

The diagram establishes the site's intended connectivity and named markers,
but its arrows/labels are not enough to establish transition trigger geometry,
one-way state, collision bounds, encounter placement, or story gating. The ROM
map/door records must settle those details.

### Full Matrix page inventory

These are the numbered nodes as the site's pages label them. Datastore names
are useful quest-search hints; the page doesn't specify the ROM's option text,
passcode checks, alert-update order, or what a successful search changes.

| Matrix page | Numbered node labels and page callouts |
|---|---|
| [Ares](https://www.mikesrpgcenter.com/shadowrun/maps/ares.html) | 1 Outdated Files; 2 Project Files; 3 System Files; 4 Management Files; 5 Security Files; 6 Legal Files; 7 Confidential Data (Chernobyl letters H/R; Ilene tip); 8 Maglocks; 9 Security Control; 10 Cameras; 11-12 Automated Equipment; 13 HVAC System; 14 Elevator Control; 15 Alert Control. |
| [Aztechnology](https://www.mikesrpgcenter.com/shadowrun/maps/aztechnology.html) | 1 Confidential Data (Chernobyl B); 2 Project Files; 3 Competition; 4 Marketing Data; 5 Project Files; 6-7 System Files; 8 Outdated Files; 9 Security Files; 10 Project Files; 11 Alert Control; 12 Elevator Control; 13-14 Automated Equipment; 15 Alert Control; 16 Cameras. |
| [City Hall](https://www.mikesrpgcenter.com/shadowrun/maps/cityhall.html) | 1 Management Files; 2 Outdated Files; 3 Project Files; 4 Financial Data; 5 Security Files; 6 Confidential Data; 7 HVAC Systems; 8 Automated Equipment; 9 Satellite Feed; 10 Cameras; 11 Maglocks; 12 Alert Control; 13 Cameras. |
| [Club Penumbra](https://www.mikesrpgcenter.com/shadowrun/maps/clubpenumbra.html) | 1 System Files; 2 Financial Data; 3 Outdated Files; 4 System Files; 5 Security Files; 6 Automated Equipment; 7 Cameras. |
| [Fuchi](https://www.mikesrpgcenter.com/shadowrun/maps/fuchi.html) | 1 Outdated Files; 2 System Files; 3 Security Files (prototype cybernetic clue and Chernobyl N/L); 4 Sinsense Files; 5 Financial Data; 6 Management Files (cybernetic-heart clue); 7 HVAC Systems; 8 Maglocks; 9 Satellite Feed; 10 Alert Control; 11 Cameras; 12 Elevator Control. |
| [Gates Undersound](https://www.mikesrpgcenter.com/shadowrun/maps/gatesundersound.html) | 1 System Files; 2 Outdated Files; 3 Security Files; 4 Management Files; 5 Financial Data; 6 Elevator Control; 7 Automated Equipment; 8 Maglocks; 9 Cameras; 10 Underwater Equipment. |
| [Hollywood Correctional](https://www.mikesrpgcenter.com/shadowrun/maps/hlywdcorrfac.html) | 1 Outdated Files; 2 System Files; 3 Security Files; 4 Financial Data; 5 Management Files; 6 Prisoner Files (Ilene Two Fists tip); 7 HVAC Systems; 8 Maglocks. |
| [Ito's System](https://www.mikesrpgcenter.com/shadowrun/maps/itossystem.html) | 1 System Files; 2 Management Files (Renraku strike-team tip); 3 Financial Data (map tip); 4 Confidential Data (Vigore/Thon tip); 5 Project Files (Tomb of Ellisia tip). |
| [Lone Star](https://www.mikesrpgcenter.com/shadowrun/maps/lonestar.html) | 1 Case Files; 2 System Files; 3 Outdated Files; 4 Legal Files; 5 Prisoner Files (Vigore tip); 6 Security Files; 7 Management Files; 8 Financial Data (cybernetic-heart tip); 9 Automated Equipment; 10 Elevator Control; 11 Automated Equipment; 12 Alert Control; 13 Maglocks; 14 Alert Control; 15 Cameras. |
| [Mitsuhama](https://www.mikesrpgcenter.com/shadowrun/maps/mitsuhama.html) | 1 Outdated Files; 2 Security Files (AmerIndian prisoner tip); 3 Management Files; 4 Project Files; 5 Competition; 6 System Files; 7 Financial Data; 8 Confidential Data (Chernobyl C/O); 9 Maglocks; 10 Cameras; 11 Elevator Control; 12 Alert Control; 13 Automated Equipment. |
| [Renraku](https://www.mikesrpgcenter.com/shadowrun/maps/renraku.html) | 1 Financial Data; 2 Management Files (Mako tip); 3 Outdated Files; 4 Confidential Data (Chernobyl E/Y and Ito tip); 5 Project Files (Ellisia map tip); 6 Competition; 7 System Files; 8 Project Files; 9 Security Files (Frosty and Joshua tips); 10 HVAC Systems; 11 Cameras; 12 Satellite Feed; 13 Maglocks; 14 Alert Control. |
| [Seattle General Hospital](https://www.mikesrpgcenter.com/shadowrun/maps/seattlegenhosp.html) | 1 Outdated Files; 2 System Files; 3 Management Files; 4 Automated Equipment. |
| [UCAS Federal Government](https://www.mikesrpgcenter.com/shadowrun/maps/ucasfedgov.html) | 1 Outdated Files; 2 Project Files; 3 System Files (Chernobyl virus); 4 Prisoner Files; 5 Financial Data; 6 Management Files; 7 Security Files; 8 Maglocks; 9 Alert Control; 10 Maglocks; 11 Satellite Feed; 12 HVAC Systems; 13 Cameras; 14 Alert Control. |
| [Unlisted](https://www.mikesrpgcenter.com/shadowrun/maps/unlisted.html) | Multiple unnamed layouts/collage groups. Across them the page labels management/system/project/security/financial/outdated/legal/prisoner/simsense files, dead projects, HVAC, cameras, automated equipment, security control, maglocks, and alert control. Keep unnamed rather than assigning a corporation from visual resemblance. |

### NPC and catalog cross-check additions

- The separate [NPCs page](https://www.mikesrpgcenter.com/shadowrun/npcs.html)
  lists nine runners: Peter Uvehr, Phantom, Rianna, Ricky, Trent, Ilene Two
  Fists, Stark, Walking Bear, Winston Marrs. The [Shadowrunners page](https://www.mikesrpgcenter.com/shadowrun/shadowrunners.html)
  also lists Freya, for ten total; the discrepancy suggests the NPC page may be
  incomplete/stale, so do not use it as the full recruitment roster.
- The NPC page adds Stark's route: wilderness/cave find -> Seattle General
  Hospital diagnosis -> Fuchi third-floor prototype cyberheart -> recovery ->
  optional recruitment at Underground 93. The map/walkthrough pages should be
  cross-checked for the exact transitions and state gates.
- The [Johnsons page](https://mikesrpgcenter.com/shadowrun/johnsons.html)
  lists Gunderson (small-time/courier), Julius Strouther (mid-level),
  Mortimer Reed (mid-level), Vigore and Jarl (mid-level acquisitions), and
  Caleb Brightmore (upper-level Matrix runs). The separate map/walkthrough
  pages don't always refer to the same Johnson as a physical occupant: the
  guide says Gunderson directs the player to Rat's Nest, while Vigore and Jarl
  are listed at The Wanderer in Puyallup. Treat “points to a location” and
  “is physically present there” as different facts.
- Contact-page spellings vary: `Alesandro`/`Alessandro`, `Allesandro`, and
  Quinton `Jaymes`/`James`; the contact list also describes Agira as available
  from Ratspike and Max as available from Vigore in Puyallup. Use ROM strings
  and contact indices for canonical identity, not normalization by guess.
- Catalog exactness caveats: the site calls the initial deck `Allegience
  Alpha`, labels Mach 22 `SMG` on Redmond and `MSG` on Puyallup, and the
  Redmond map alone gives Ratspike's reputation-adjusted 1,500-nuyen talk
  threshold. Keep the source wording in notes, then resolve spelling/type
  questions against ROM item/contact data.

### Site-wide source coverage and use limits

The site's main index covers Armor, Contacts, Cyberdecks, Cyberware, Items,
Johnsons, Manual, Programs, Shadowrunners, Spells, Walkthrough, Weapons, six
city-area maps, four wilderness maps, four cave maps, and fourteen Matrix maps.
The separate NPC list is additional. Printable variants and thumbnails are
presentation duplicates rather than separate game-behavior sources. For
implementation research, the walkthrough is a quest dependency checklist;
catalog pages are a naming/stat/price cross-check; city maps are the best
human-readable location index; wilderness/cave pages give a rough edge graph;
Matrix pages identify node categories; and the scanned manual describes
player-facing rules. None provides the ROM scene IDs, event order, exact
coordinates, hidden state bits, actor heartbeat cadence, hit formulas, or UI
pixel/timing goldens.

#### Full Shadowrun-site reread: 2026-10-10

I reopened the Shadowrun landing page and checked every linked HTML guide and
map page listed above, plus the separate NPC page. The coverage is 12 catalog/
walkthrough pages (including the map index), 6 city maps, 4 wilderness maps,
4 cave maps, 14 Matrix maps, the landing page, and the separate NPC list. The
linked Genesis manual PDF timed out in the current browser fetch; its existing
manual notes above are based on the earlier page-image/OCR pass and should not
be described as a fresh PDF inspection. The catalog pages consistently label
prices as Negotiation 0 where stated. Spelling and map-label contradictions
remain recorded rather than silently normalized.

Current issue cross-checks from the reread:

- The [Redmond map](https://mikesrpgcenter.com/shadowrun/maps/redmond_barrens.html)
  labels Stoker's Motel as marker 6 and Rat's Nest as marker 9, in separate
  parts of the map. The [Vigore walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html)
  says Gunderson directs Joshua to the Rat's Nest after Harlequin's request;
  Vigore has already left, and the Landmark Map becomes the quest target only
  after showing Harlequin the first map and speaking to Gunderson about Vigore.
  This supports the map-name/quest crosswalk, but neither source gives the
  entrance collision bounds, entry dialogue text, exit destination, or ROM
  state condition.
- The [Puyallup map](https://mikesrpgcenter.com/shadowrun/maps/puyallup.html)
  labels marker 2 as Tarislar Garden Apartments and marker 3 as Tarislar City
  Inn (rest for 30 nuyen). The [walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html)
  places the apartment visit after the Sinsearach trials and Frosty's
  introduction. These names corroborate ROM text labels, but the map does not
  establish that apartments and inn share a scene or entrance behavior.
- The Redmond and Puyallup maps list different shop inventories, clip prices,
  cyberware, and access requirements; use them to form a completeness checklist
  for the shop catalog, not as transaction rules. The weapons and item pages
  describe effect categories (including gun silencers, SMG suppressors,
  smartgun prerequisites, passkey ratings, and spell foci) but do not supply
  the firing cadence, alert update order, success formula, or selection pixels.
- The fourteen Matrix maps include repeated node classes and page-specific
  clue callouts. `Unlisted` contains several separate unnamed map diagrams;
  preserve those as unassigned until ROM evidence identifies them. The map
  labels are navigation aids, not proof that a node uses a particular ROM
  entry or result handler.
