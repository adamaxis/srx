# Field random-event selector

## Source trace

The US ROM routine at `0x59986` selects conditional events for the current
field context. It makes two calls to the source bounded-random routine at
`0x0D3A` with bound 5, so each draw returns 0 through 4. Their sum selects one
of nine columns. The current field context indexes the 56-byte group map at
`0x3312`; each group contributes nine cells from the enable table at
`0x1D70E4` and nine pointers from `0x1D7174`.

When the enable byte is nonzero, the selector dereferences that cell's pointer
and returns the selected handler address in A6. The trigger at `0x1E166`
starts that handler as a world activity. When the enable byte is zero, the
selector returns zero and the trigger takes `0x1E216`'s dynamic world-effect
sequence path instead.

The new-event attempt itself is gated: the field tick counter must satisfy
`FFDF88 & 0x3FF == 10`, the source random value must pass its current threshold,
and the active effect/entity and world-activity guards in `0x1E166` must allow
it. An already active sequence is forwarded rather than replaced. Therefore,
the nine rows shown per scene are possible table outcomes, not an event
guaranteed on every visit or movement step.

The 26 UI event handlers targeted by enabled table cells supply prompt records to
`0x1F246` and `0x1F3C4`. The extractor records each handler's opening panel,
visible choices, scene IDs, selector sums, and valid roll pairs. The **Field
events** tab now shows all enabled outcomes and all dynamic actor-template
coverage, then links confirmed choice paths that create actors. These rows are
eligible outcomes; the ROM's field tick, random threshold, active-effect, and
world-activity gates still decide whether a selector attempt runs.

The context byte is the current scene ID, not a free-standing event category:
the field-scene loader copies `FFDDC4` to `FFFBF8` at `0x2F32`, `0x3074`, and
`0x3084`; `0x59986` then maps that value through `0x3312`. Several scenes share
one context group and therefore the same nine outcome cells. The viewer groups
the repeated scene contexts and summarizes known choice-dependent actor paths
beside the complete selector tables below.

## ROM encounter evidence (partial)

Scene IDs below are ROM field-scene IDs. The tables cover the field random-event selector, its zero-cell dynamic actor templates, and the choice continuations traced so far. A listed actor is not a permanent resident or guaranteed on every visit. This is not yet a complete map-population or pursuit roster: direct map triggers, state-driven police response, timed actor spawning, and other scripted events are separate source paths.

### Choice-dependent actor paths traced in the ROM

| Event path | Eligible scenes and selector sums | What the choice does |
|---|---|---|
| Sudden disturbance, handler `0x20EFA` | Scenes `0-3` and `5`; sums `0-3` and `7` | The MORE choice reaches the shared stage gate. Below stage 2, this route returns explicit Renraku attack text through a 20-entry resource selection; above that gate it selects the Strike Team group size. |
| Sudden disturbance, handler `0x20F42` | Scenes `0-1` and `3`; sums `0`, `3`, and `5` | Shares the same urban follow-up as handler `0x20EFA`. |
| Sudden disturbance, handler `0x2115C` | Scenes `6-9` and `44-50`; sum `0` | The MORE choice reaches the shared stage gate. At stages `0-1` or with the event-state bit set, it selects from the wilderness pack table below; at stages `2-8` with the gate clear, it selects Strike Team groups. |
| Lone Star patrol, handler `0x20F12` | Scenes `0` and `3`; sums `3` and `4` | Attack queues the list at `0x1D6FDC` (`lone11`, `lone12`, `lone detectiv`). Run away may queue `0x1D6FE4` or `0x1D6FE0`, according to the thresholded roll result. |
| Injured-person event, handler `0x20F72` | Scenes `0`, `1`, and `3`; sums `5` and `7` | The ordinary injured-man Attack choice can queue one Lone Star detective at `0x1D6FE4`. A separate pale-figure variant can reveal a vampire; its sequence starts at `0x1D6D8A`, after the Hell Hound pointer in the shared list. |
| Alley grenade seller, handler `0x2112C` | Scenes `0`, `2`, `3`, and `5`; sums `5-7` | Attack rolls `0-59` try the one-bodyguard list at `0x1D7054`; a successful start leads to a gun battle. Rolls `60-99`, or failure to start that actor, reveal undercover Lone Star. Buying the grenades has its own 60/40 split: rolls `0-59` continue through funds/slot checks; `60-99` expose the undercover agent. |
| Company Man confrontation, handler `0x20F5A` | Scenes `3` and `5`; sums `1` and `4` | One continuation says the man remains irate and a Lone Star patrol arrives. The response text then says the officers turn their attention to the player. |
| Flustered-woman event, handler `0x20F8A` | Scenes `0-1`; sums `3-4` | The MORE continuation divides into suspected-harm and apparent-law-official versions. The law-official version can reveal an undercover Lone Star agent; its Attack/Comply/Run choices can end in a gun battle, questioning/release, fine, arrest/confiscation, escape, or being overtaken and shot. See the terminal branch table below. |
| Haggard-man event, handler `0x20FA2` | Scenes `2` and `5`; sums `1-2` | The doctor offers 50 nuyen to recapture his escaped patient. Help and Threaten resolve without combat; Attack or Threaten can call a patrol, and one alternate Attack branch starts a fight with the doctor's Gator Shaman/bodyguards. See the terminal branch table below. |
| Lone Star patrol, handler `0x2106A` | Scenes `41-43`; sum `5` | The follow-up arrest prompt's Attack choice reaches `0x2466C`. The first three named entries at `0x1D6FEC` are `lone11`, `lone12`, and `lonemage`; the trailing control word is still unresolved. |
| Lone Star dynamic templates | Scene `1` (sum `2`) and scenes `41-43` (sum `8`) | The zero-cell dynamic sequences include `lone11`, `lone13`, and `lonemage`. |
| Disturbance Strike Team escalation | Scenes `0-3`, `5-9`, and `44-50` | The MORE continuations check `FFFBFB` and bit 8 of `FFFC22`. With the bit clear, stages `2-3` select two Strike Team templates (`0x1D6FD0`), `4-6` select four (`0x1D6FC8`), and `7-8` select five (`0x1D6FC4`). Stages `0-1` or a set bit take the low-stage branches described in the next rows; the urban route still returns explicit Renraku attack text. |
| Wilderness disturbance packs, handler `0x2115C` | Scenes `6-9` and `44-50` | At stages `0-1` or with bit 8 set, the ROM rolls 0-99: 30% queues three Hell Hounds, 30% two Wendigos, 20% two Bandersnatches, 5% a Rat Shaman and two Indian warriors, 10% an Elven mage and two Elven warriors, and 5% takes the no-list branch. At stages `2-8` with the bit clear, it uses the Strike Team groups above. |
| Explicit Renraku Strike Team prompt, handler `0x210CC` | Scenes `35-40`; sum `1` | The MORE choice dispatches to `0x24918`, which queues two Strike Team templates from `0x1D6FD0`. This is the selector path whose text explicitly names Renraku. |
| Renraku Security prompt, handler `0x210E4` | Scenes `35-40`; sum `4` | Its follow-up dispatches to `0x24936` and queues one corporate-guard template from `0x1D705C`. Keep it separate from the Strike Team event. |
| Scene-47 mode-`0x13` lists (Cave 4 guide cross-reference) | Scene `47`; lists `0x1E5C4` and `0x1E628` | One list has one Strike Team, two bodyguards, and a pedestrian; the other has three Strike Teams and a corporate guard. These ROM lists match Mike's Cave 4 map note about a Renraku strike team; the cave name is guide cross-reference evidence, not a ROM label. They are separate from the random disturbance and the scene-35-40 selector outcomes. |

The Strike Team counter is advanced by dispatch command `0x2C` at `0x21BE4`.
That handler increments the byte at `FFFBFB`, caps it at `8` through
`0x1F1E0`, and queues two Strike Team templates from `0x1D6FD0`. Its linked
prompt text names Mako Sochou. This is a story-driven source for the same
counter used by later disturbance choices; the containing map context is not
yet joined to a scene ID in the event index.

### Terminal branches traced so far

The prompt command values dispatch through the ROM table at `0x12CCB4`. The
extractor records each opening choice's command and target. The companion
`assets/field/world-activity-terminal-outcomes.tsv` records the continuations
that have been followed through their terminal dialogue, funds changes, and
actor-list handoffs. The map viewer shows these rows beside the per-scene event
prompt. A dispatch target by itself is not treated as a terminal outcome.

**Flustered woman (`0x20F8A`, scenes 0-1, sums 3-4).** MORE enters `0x23AA8`,
which selects either a suspected-harm prompt or an apparent-law-official
prompt. In the suspected-harm route, Attack/intervention reaches `0x23B0C`:
when the FFF0DD check is below 2 and the actor sequence starts, Eye Fiver 2
and two Eye Fiver 1 actors are queued and the text hands off to a fight. If
that condition fails, the thug runs and the player gets 100 nuyen. Walking
away ends with three men abducting the woman. In the apparent-official route,
Attack reveals the undercover agent when FFF0DD is below 10; otherwise the man
runs and the woman thanks the player. Questioning reveals the undercover
prompt only when FFF0DD is zero; otherwise the man exposes himself as Lone Star
undercover, abducts the woman, and leaves. Walking away also ends in the
abduction.

At the undercover prompt, Attack dispatches to `0x23412` and queues the
undercover officer plus two Lone Star actors from `0x1D6FDC`, starting combat.
Comply enters stateful suspect-ID and weapon-permit checks. Their traced ends
include release, accepted permit, a 500-nuyen on-sight fine, or arrest with
weapon confiscation. Run can outrun the patrol, be overtaken and fired on, or
outrun some officers while others catch up and open fire. Those outcomes are
selected by the source's state and random branches, not by the map selector.

**Escaped patient (`0x20FA2`, scenes 2 and 5, sums 1-2).** MORE enters
`0x23BB4`, whose state/random branches select the doctor's response and expose
Attack, Threaten, and Help choices. Help restrains the patient and pays 50
nuyen. Threaten can make asylum staff show credentials, take their patient,
and pay 50 nuyen. Attack has a 50/50 source roll: one half reaches the same
paid asylum resolution; the other half calls Lone Star and shows a patrol
choice prompt after identifying the doctor as a ward director and the patient
as his escapee. Threaten can also call the patrol; its other roll branch pays
50 nuyen and sends the doctor and assistants away. In the alternate Attack
prompt, source results below 1 queue a Gator Shaman and two bodyguards, a
result of 1 queues two bodyguards, and results above 1 pay 100 nuyen and end
the encounter without combat.

The doctor event's patrol prompt uses the shared Lone Star Attack/Comply/Run
dispatchers. Attack starts the patrol fight; Comply can resolve through the
same permit, identity, fine, release, and warrant checks; Run can escape or
be caught and fired upon. This is distinct from the undercover-woman prompt:
the doctor calls a uniformed patrol, while the woman branch reveals an agent
already posing as a law official.

The outcome table includes those two encounter graphs plus the direct patrol,
Company Man, injured-person, alley-seller, watcher, and scene-41-to-43 Lone
Star paths. Rows marked `partial` name a specific follow-up still open; a
handler with some traced rows is not automatically treated as complete.

**Phantasmal watcher (`0x20F2A`, scenes 0, 3, and 5; sums 1-2 and 8).** The
opening prompt at `0x120F00` links the two responses directly to data nodes:
refusing reaches `0x120F18`, where the player is rebuked and the watcher
vanishes; accepting reaches `0x120F20`, where the watcher leads the player to
an old man being beaten by Eye-Fivers. Dispatch `0x9A` (`0x23812`) loads the
player field at `FF0177`, runs the threshold-6 trial counter, then branches on
`FFF0DD`. Result 0 selects four attackers from `0x1D6DAE` (two Eye Fiver 2,
two Eye Fiver 1); result 1 selects two Eye Fiver 1 from `0x1D6DB6`. Both
successful sequence starts show the gangers turning on the player and link to
the message at `0x120F5C`, which says the watcher waves thanks and fades as the
gangers come at the player. The source branch that actually resumes or exits
that combat, and any post-fight reward path, is still partial. Results above 1,
or failure to start the selected sequence, instead add 200 nuyen and show the
mage's thanks and payment at `0x120F64`/`0x11DAF8`; that no-combat branch is
traced to its end. The encounter does not introduce Lone Star.

**Company Man (`0x20F5A`, scenes 3 and 5).** Attack can make him flee or make
him draw a gun; Talk and Walk Away each have state/roll branches that can bring
in a patrol. The sequence `0x1D7074` starts one actor whose template label is
`pedestrian`; the ROM does not explicitly identify that actor as the Company
Man. Branches that start it return the text-only gun-draw node at `0x120FE0`.
Its `0xFFFFFFFF` marker means the adjacent strings are not Attack/Talk/Run
choices on that node. Separate branches go directly to the actual patrol menu
at `0x120FE8`: Talk's threshold result below 1, or Walk Away's 80-99 roll.
That menu visibly offers Attack, Talk, and Run with commands `0x95`, `0x96`,
and `0x99`, which route into the shared police choice handlers already traced.
Here Talk (`0x96`) uses the common comply branch, while Run (`0x99`) uses the
common pursuit branch. The menu path is therefore resolved; the exact actor
identity and combat exit after the gun-draw sequence remain partial.

**Injured person (`0x20F72`, scenes 0, 1, and 3).** The ordinary branch offers
Attack, First Aid, or Walk Away. Successful Attack starts the one-detective
sequence at `0x1D6FE4` and raises FFFBFA by two, capped at eight; if the
sequence cannot start, the handler returns to the pale-figure text. First Aid
adds 150 nuyen, while Walk Away passes the moaning figure. On the separate
low-roll pale-figure branch, the source selects text from FFF0DD; First Aid
can start the vampire template at `0x1D6D8A`. That address starts at the
vampire record, not the preceding Hell Hound record in the shared sequence.
The successful vampire-combat handoff and its post-combat exit remain partial.

**Alley grenade seller (`0x2112C`, scenes 0, 2, 3, and 5).** Attack rolls
`0-59` try one bodyguard from template `0x1D6532` via list `0x1D7054`; when
that sequence starts, the seller draws a heavy pistol and combat begins. On
rolls `60-99`, or if the bodyguard cannot be queued, the seller disappears and
an undercover Lone Star patrol arrests the player unless they choose Attack,
Comply, or Run. Buying six grenades uses a separate roll: `0-59` proceeds to
inventory and funds checks, while `60-99` triggers the undercover arrest
prompt. A successful sale costs 180 nuyen and assigns six Frag Grenades; no
free slot or insufficient funds returns the seller's dismissal text. Walking
away leaves the seller without starting combat or buying the item.

**Scene-41-to-43 patrol (`0x2106A`).** A clear FFFC22 bit 10, or a low roll
with the bit set, opens an arrest prompt. With the bit set, rolls `15-49`
wave the player past and rolls `50-99` call dispatch to verify the ID. The
arrest prompt has Attack and Comply, not Run. Its Attack outcome can end in a
fatal warning or queue the first three named templates (`lone11`, `lone12`,
`lonemage`) from `0x1D6FEC`. The list has a trailing control word not yet
classified; the rejected-ID Talk choice and the post-arrest continuation at
`0x12140C` also remain partial.

The per-scene viewer now shows each indexed handler, roll sums, visible opening
choices, dispatches, traced branch outcomes, and trace status. Handlers with no
terminal rows stay pending; handlers with a `partial` row are called out as
partial rather than appearing complete.

### Lone Star coverage by named urban field map (dialogue selector only)

| Scene | Named map | ROM selector paths linked to Lone Star | What is confirmed |
|---:|---|---|---|
| 0 | Downtown Seattle | `0x20F12`, `0x20F72`, `0x20F8A`, `0x2112C` | Direct patrol stop; injured-person Attack; undercover-woman branch; alley-seller Attack or purchase branch. |
| 1 | Puyallup Barrens | `0x20F72`, `0x20F8A` | Injured-person Attack and undercover-woman branch. |
| 2 | Redmond Barrens | `0x20FA2`, `0x2112C` | Doctor may call a uniformed patrol; alley seller may expose an undercover patrol. |
| 3 | Penumbra District | `0x20F12`, `0x20F5A`, `0x20F72`, `0x2112C` | Direct patrol stop; Company Man may bring patrol attention; injured-person Attack; alley-seller branches. |
| 4 | Council Island | None named in the enabled dialogue-event cells | This only says the selector table has no Lone Star-named path here; it does not rule out a separate pursuit system. |
| 5 | Renraku Arcology | `0x20F5A`, `0x20FA2`, `0x2112C` | Company Man, doctor, and alley-seller choices can reach Lone Star. |

This confirms Lone Star-linked random-event paths in five of the six named
urban field maps (scenes 0-5, excluding Council Island). Most are conditional
branches inside other encounters, not an unconditional patrol spawn. It
supports the reported broad urban coverage; it does not establish that wanted
pursuit can spawn on every one of the 56 scene contexts. Dynamic Lone Star
templates are separately listed for scene 1 and scenes 41-43. The general
map-wide pursuit route and its wanted-state conditions remain untraced.

There is also a separate player legal-state path to resolve. The third 32-entry Tips pointer table at `0xC2816` maps global entries 65-96; entries 70-73 point to text starts `0xC4390`, `0xC4465`, `0xC451E`, and `0xC45E1`. Entry 70 contains the wanted-for-questioning notice (including `0xC442C`); entry 71 is addressed to detectives and describes the suspect as armed and dangerous; entry 72 is addressed to hunting parties and calls the player a fugitive; entry 73 is a Lone Star Records prisoner-transfer notice about Vigore. Since the selector tests each table's local zero-based entry index, these four entries correspond to `FFFC38` bits 5-8; the wanted/fugitive notices are bits 5-7. These are Notebook display-eligibility bits; the source trace has not shown them to be the wanted-state values consumed by field pursuit. ROM contact text at `0xC63CF` and `0xC651C` offers to delete the Lone Star record, while `0x10E3C1` and `0x10F009` describe a Weapon Permit and patrols flagging the player down. Those strings establish that the record/permit mechanics exist, but they do not establish the writer/checker for the first three tip bits or the map-wide patrol spawn condition. The general patrol pursuit route remains untraced; this known-selector profile therefore cannot say which other maps support it or how often it occurs.

A separate nearby-effect path runs through `0xAB02 -> 0x16B98`. It can place
one of ten `FF164C` records near the transitioning actor with a weighted
interaction code; `0x1DF34` later checks a strict `+/-0x100` radius and sends
that code to the reward handler `0x133A6`. The `0x5A` and `0x5B` weighted
sentinels select an equipped-item result or clear the record. This is a
proximity-effect/reward route, not evidence of a general Lone Star patrol
spawn. The ordinary weighted interaction codes still need their player-facing
effects mapped, and the wanted-state map-wide patrol route remains unresolved.
Keep this path separate from the random dialogue and dynamic actor tables
below.

### Broad dynamic creature coverage

The zero-cell dynamic sequence table shows creatures across many field contexts. Hell Hounds are eligible on 26 scene IDs: Downtown Seattle (0), Wilderness Areas 1-4 (6-9), Hollywood Correctional Facility (20), contexts 26-29, and contexts 35-50. Across the 33 scene/sum outcomes containing Hell Hounds, the selected sequence lists 1-4 Hound actors. This generic dynamic sequence is separate from the wilderness disturbance's low-stage pack of exactly three Hounds. Wendigos and Bandersnatches cover 1, 4, 6-9, and 44-50; Gargoyles cover 4, 6-9, and 44-55. These are temporary screen-edge event sequences with terrain-tested placement, not permanent map residents.

### Later-game Strike Team coverage

The disturbance handlers at `0x20EFA`, `0x20F42`, and `0x2115C` make the same FFFBFB progression counter available across scenes 0-3, 5-9, and 44-50 (16 scene IDs). Within the ROM field-area/exploration set 0-9 and 44-50 (17 scene IDs), this route is enabled on 16; Council Island (scene 4) is the exception. Scenes 44-50 are selectable field-map contexts, not merely scripted activities. Scene 47 is cross-referenced as Cave 4 because its separate mode-`0x13` lists contain Renraku Strike Teams; scenes 44-46 and 48-50 still need their own map-name crosswalks. With FFFC22 bit 8 clear, stages 2-3 select two Strike Team actors, stages 4-6 select four, and stages 7-8 select five. This later-stage branch is broader than the explicit Renraku-building prompts in scenes 35-40.

The story choice at `0x21BE4` advances this shared counter and queues two Strike Team actors. The low-stage disturbance routes differ: the urban continuation returns explicit Renraku attack text, while the wilderness continuation can choose the creature packs listed above. The wanted-state patrol logic remains a separate, untraced path.

### Complete enabled dialogue-event selector coverage

Each row is one of the 26 enabled ROM handlers. Scene ranges are context IDs;
sums identify the enabled cells that return that handler.

| Event text | Scenes | Sums | Handler |
|---|---|---|---|
| There is a sudden disturbance up ahead. | `0-3`, `5` | `0-3`, `7` | `0x20EFA` |
| Crowds part as you are approached by a Lone Star patrol. | `0`, `3` | `3-4` | `0x20F12` |
| The shadowy image of a watcher materializes in front of you. | `0`, `3`, `5` | `1-2`, `8` | `0x20F2A` |
| There is a sudden disturbance up ahead. | `0-1`, `3` | `0`, `3`, `5` | `0x20F42` |
| A Company Man storms up to you. | `3`, `5` | `1`, `4` | `0x20F5A` |
| Ahead of you is a man lying on the street. | `0-1`, `3` | `5`, `7` | `0x20F72` |
| A flustered woman passes by you. | `0-1` | `3-4` | `0x20F8A` |
| A haggard man collapses at your feet. | `2`, `5` | `1-2` | `0x20FA2` |
| You notice an open security panel. | `21-34` | `0-2`, `7` | `0x20FBA` |
| A scientist walks by, deep in thought. | `21-25`, `30-34` | `4`, `6` | `0x20FD4` |
| A secuity guard spots you. | `20`, `26-34`, `41-43` | `4-5`, `7` | `0x20FEC` |
| A shiver runs down your spine. | `20`, `26-29`, `35-43` | `0-1`, `7` | `0x21004` |
| An old custodian wiping the floor squints up at you. | `20` | `1` | `0x2101E` |
| You look down and notice you are about to step on a concealed security plate. | `20-34`, `41-43` | `2`, `5-6` | `0x21036` |
| With no warning, a low-level laser detection field activates. | `35-40` | `8` | `0x21050` |
| A Lone Star patrol spots you. | `41-43` | `5` | `0x2106A` |
| Someone taps you on the shoulder. | `21-29`, `41-43` | `2-3`, `8` | `0x21082` |
| You find a terminal logged into the management database. | `21-40` | `0`, `3-5` | `0x2109A` |
| A Company Man walks up and flicks a toothpick at you. | `21-40` | `1-2`, `5`, `8` | `0x210B4` |
| You stumble upon a Renraku Strike Team. | `35-40` | `1` | `0x210CC` |
| Renraku Security spots you as they round a corner. | `35-40` | `4` | `0x210E4` |
| A security mage spots you. | `20-43` | `2-3`, `6-8` | `0x210FC` |
| The hair on the back of your neck rises. | `6-9`, `44-50` | `2`, `4`, `6`, `8` | `0x21114` |
| As you walk by, a man calls to you from a dark alleyway. | `0`, `2-3`, `5` | `5-7` | `0x2112C` |
| A group of young AmerIndian boys are scouring the terrain. | `6-9` | `2` | `0x21144` |
| There is a sudden disturbance up ahead. | `6-9`, `44-50` | `0` | `0x2115C` |

### Complete dynamic actor-template coverage

These are the actor templates in the zero-cell dynamic world-effect sequences.
The selector's runtime placement routine chooses open screen-edge positions;
these are not permanent map residents or fixed map coordinates.

| ROM template | Scene contexts | Random sums |
|---|---|---|
| Bandersnatch | `1`, `4`, `6-9`, `44-50` | `0-1`, `3`, `7-8` |
| elven mage | `3` | `0` |
| eye5r 1 | `0`, `5`, `10-19` | `0-8` |
| eye5r 2 | `0`, `5` | `6`, `8` |
| Gargoyle | `4`, `6-9`, `44-55` | `1`, `4`, `6-8` |
| gator shaman | `3-4` | `3-6` |
| ghoul1 | `51-55` | `0`, `2-5`, `7-8` |
| guard1 | `20-25`, `30-34` | `0-1`, `3`, `7-8` |
| Hell Hound | `0`, `6-9`, `20`, `26-29`, `35-50` | `0-1`, `3`, `5-6` |
| indian warior | `1` | `6` |
| lone11 | `1` | `2` |
| lone13 | `41-43` | `8` |
| lonemage | `1`, `41-43` | `2`, `8` |
| ork ganger | `1`, `10-19` | `0-8` |
| rat shaman | `1`, `5` | `6`, `8` |
| vampire | `0`, `51-55` | `0`, `7-8` |
| weenr1 | `2`, `10-19` | `0-8` |
| weenr2 | `2` | `8` |
| Wendigo | `1`, `4`, `6-9`, `44-50` | `1-2`, `5-8` |

The actor names above are read from the ROM templates, retaining source
spellings such as `indian warior`. Display labels may be normalized in the
viewer, but source identity and scene coverage remain tied to the template row.

## Map-placement boundary

Enabled table cells identify dialogue/event handlers, not fixed actor origins.
The zero cells reach a separate dynamic actor/effect candidate builder. The
actor types are fixed by their selected sequence, but `0x1E302` chooses open
screen-edge coordinates from the current viewport. It tests the ROM's terrain
tile grid before accepting candidates. These are
temporary event actors, not permanent residents, and the selector does not
provide one fixed coordinate to draw on a static map. The viewer therefore
shows their source actor art in the Field events panel and leaves the map
overlay to records with actual source coordinates. Fixed default actors,
gated pedestrian coordinates, and the three explicit scene-47 actor lists
remain separate source records.

## Sprite review

The Field events panel uses the template's direction-0 idle image as a type
preview; it does not use that thumbnail as a placed map actor. I reviewed the
13 new kind/palette image sets and their eight direction outputs. One source
frame needs a separate visual follow-up: Wendigo kind 14, palette line 1,
direction 4 selects descriptor `0xC103A` and produces only a 9-by-2-pixel
nontransparent crop from its 16-by-8-pixel tile grid. The source descriptor and
frame data are recorded in `assets/catalog.tsv`; this is not hidden by the
Field events thumbnail, which shows the direction-0 portrait. Do not treat the
direction-4 image as visually verified for gameplay until it is compared with
an independent game capture or the actor's complete render path is traced.

## Extracted coverage

`decompilation/tools/extract_world_activity_outcomes.py` creates
`decompilation/assets/field/world-activity-outcomes.tsv` from the US ROM. The
table has 504 rows: nine sums for each of 56 contexts. It records all 246
enabled scene/sum cells, their selected handler, prompt text and choices, plus
the 258 cells that use the dynamic sequence path. The graphics importer also
creates `field/entities/dynamic-world-activity-actors.tsv` and the actor-type
sprites referenced there. It contains 731 scene/sum/actor rows covering all
258 dynamic outcomes, 19 named templates, and 13 entity-kind/palette pairs.
This companion table preserves the selected ROM sequence and its ordered
template overrides; placement remains dynamic. The pointer and mask offsets
are retained as provenance; the viewer presents readable actor names and art
without requiring the user to interpret those offsets.

Regenerate or compare the source table with:

```powershell
python decompilation/tools/extract_world_activity_outcomes.py
python decompilation/tools/extract_world_activity_outcomes.py --check
```
