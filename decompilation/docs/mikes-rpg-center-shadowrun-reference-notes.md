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

## Redmond ROM coordinate cross-check: Rat's Nest lead

The Redmond map is a 480-by-320 image. A rough proportional overlay of the
scene-2 field coordinates places four independent collision landmarks near
the map's labels: Stoker's Motel (scene-2 record 91, `0x173138`, bounds
`0x03C0..0x03DF` by `0x0280..0x029F`), Ares (record 92, `0x173142`,
`0x04C0..0x04DF` by `0x0220..0x023F`), Little Chiba (record 93, `0x17314C`,
`0x06E0..0x06FF` by `0x0220..0x023F`), and Rat's Nest (record 97, `0x173174`,
`0x01A0..0x01BF` by `0x0440..0x045F`). This supports identifying scene 2 as
Redmond Barrens and the event-1 and event-7 rectangles as the Little Chiba
and Rat's Nest approaches, respectively. The proportional overlay is a
secondary-source name crosswalk, not a ROM-stored map label or a substitute
for exact world-to-image calibration.

For record 97, the ROM's collision dispatcher publishes event 7 and reads
scene-2 event-table row 7 (`0x172CE3`), which is `0x1A`; the ROM classifies
that value as resource-activity scene 26. The guide independently says
Rat's Nest is in the southwest and its Landmark Map availability requires
showing the first map piece to Harlequin and asking Gunderson about Vigore.
ROM cross-check now confirms the dialogue half of this gate: the mode-command
menu builder at `0x22F90` adds “What do you know about a Johnson named
Vigore?” (`0x1045B0`) only when `FFFC1E` bit 26 is set. Selecting that option
dispatches command `0xE7` to `0x249A4`, which sets `FFFC34` bit 13 and presents
the Johnson response. The indexed service at `0x545B6` later requires scene 2,
event 7, alternate result 0, `FFFC34` bit 13 set, and its consumed/one-shot
bit 10 clear. It clears the prerequisite and several service bits, marks bit
10, then displays the ROM's Rat's Nest search sequence. This makes record 97
the strongest identified Rat's Nest event candidate and confirms a quest
precondition; the producer of `FFFC1E` bit 26, exact selected resource, item
grant, and return route still need tracing. Record 96 is a second event-7
rectangle, so it receives the same ROM service; its relationship to the guide
marker is not independently resolved by geometry.

The site labels Stoker's Motel as Redmond location 6 and Rat's Nest as
location 9. This supports keeping their events and dialogue separate; the
site does not say the motel has a dedicated interior map.

### Current project follow-up

- Add a source-derived Redmond location enum/crosswalk keyed by scene and
  trigger record (or another ROM-stable identity), rather than using scene ID
  26 alone as a building name. Several scene-2 table rows point into the same
  resource-activity class.
- Trace the producer of `FFFC1E` bit 26, scene-26 resource selection, the
  map-item grant, and exit/return state. The guide's Harlequin/Gunderson
  prerequisite now has a ROM-correlated Johnson-menu gate (`FFFC1E` bit 26 ->
  command `0xE7` -> `FFFC34` bit 13 -> scene-2/event-7 service), but bit 26's
  producer remains unresolved.
- Profile the other scene-2 event rectangles against the same map image and
  label only correspondences supported by both geometry and ROM paths.
- The site provides no button/menu layout or animation timing evidence. Use
  independent Genesis frames or ROM draw/cadence traces for those claims.

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
