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
| Injured-person event, handler `0x20F72` | Scenes `0`, `1`, and `3`; sums `5` and `7` | The follow-up Attack choice reaches `0x23A3A` and queues the one-template Lone Star detective list at `0x1D6FE4`; the accompanying ROM text says Lone Star rounds the corner. |
| Alley grenade seller, handler `0x2112C` | Scenes `0`, `2`, `3`, and `5`; sums `5-7` | The Attack choice reaches `0x24C4C`. Its 60-percent branch queues two templates from `0x1D7054`, both named `bodyguard`; the follow-up text identifies the encounter as an undercover Lone Star patrol. Preserve the source template identity when showing these actors. |
| Company Man confrontation, handler `0x20F5A` | Scenes `3` and `5`; sums `1` and `4` | One continuation says the man remains irate and a Lone Star patrol arrives. The response text then says the officers turn their attention to the player. |
| Flustered-woman event, handler `0x20F8A` | Scenes `0-1`; sums `3-4` | Its continuation exposes a Lone Star undercover officer and can bring in backup. |
| Haggard-man event, handler `0x20FA2` | Scenes `2` and `5`; sums `1-2` | A doctor calls a Lone Star patrol; a follow-up says the officers take interest in the player. |
| Lone Star patrol, handler `0x2106A` | Scenes `41-43`; sum `5` | The follow-up arrest prompt's Attack choice reaches `0x2466C` and queues the list at `0x1D6FEC`, containing `lone11`, `lone12`, and `lonemage`. |
| Lone Star dynamic templates | Scene `1` (sum `2`) and scenes `41-43` (sum `8`) | The zero-cell dynamic sequences include `lone11`, `lone13`, and `lonemage`. |
| Disturbance Strike Team escalation | Scenes `0-3`, `5-9`, and `44-50` | The MORE continuations check `FFFBFB` and bit 8 of `FFFC22`. With the bit clear, stages `2-3` select two Strike Team templates (`0x1D6FD0`), `4-6` select four (`0x1D6FC8`), and `7-8` select five (`0x1D6FC4`). Stages `0-1` or a set bit take the low-stage branches described in the next rows; the urban route still returns explicit Renraku attack text. |
| Wilderness disturbance packs, handler `0x2115C` | Scenes `6-9` and `44-50` | At stages `0-1` or with bit 8 set, the ROM rolls 0-99: 30% queues three Hell Hounds, 30% two Wendigos, 20% two Bandersnatches, 5% a Rat Shaman and two Indian warriors, 10% an Elven mage and two Elven warriors, and 5% takes the no-list branch. At stages `2-8` with the bit clear, it uses the Strike Team groups above. |
| Explicit Renraku Strike Team prompt, handler `0x210CC` | Scenes `35-40`; sum `1` | The MORE choice dispatches to `0x24918`, which queues two Strike Team templates from `0x1D6FD0`. This is the selector path whose text explicitly names Renraku. |
| Renraku Security prompt, handler `0x210E4` | Scenes `35-40`; sum `4` | Its follow-up dispatches to `0x24936` and queues one corporate-guard template from `0x1D705C`. Keep it separate from the Strike Team event. |
| Scene-47 mode-`0x13` lists | Scene `47`; lists `0x1E5C4` and `0x1E628` | One list has one Strike Team, two bodyguards, and a pedestrian; the other has three Strike Teams and a corporate guard. They are separate from the random disturbance and the scene-35-40 selector outcomes. |

The Strike Team counter is advanced by dispatch command `0x2C` at `0x21BE4`.
That handler increments the byte at `FFFBFB`, caps it at `8` through
`0x1F1E0`, and queues two Strike Team templates from `0x1D6FD0`. Its linked
prompt text names Mako Sochou. This is a story-driven source for the same
counter used by later disturbance choices; the containing map context is not
yet joined to a scene ID in the event index.

The traced Lone Star selector and linked choice paths reach five of the six ROM-named city-area maps: Downtown Seattle (scene 0), Puyallup Barrens (1), Redmond Barrens (2), Penumbra District (3), and Renraku Arcology (5). Scene 4, Council Island, has no Lone Star handler in this random selector. The selector paths also include contexts 41-43. This confirms broad coverage across the city maps in the current selector audit.

ROM Notebook tips 71-73 include dispatch messages ranging from wanted for questioning to armed and dangerous to fugitive language addressed to hunting parties. The source text is at 0xC442C, 0xC44DE, and 0xC451E. This story-state evidence may broaden police coverage further; the code that checks the wanted progression and selects map-wide patrol actors still needs to be traced.

A separate actor-transition path, 0xAB02 -> 0x16B98, performs weighted scene-entity selection under record-kind and world-mode gates. Its weighted result codes are not yet classified into event actors and ordinary entity actions, so it remains outside this encounter list. Keep this path open while tracing the user's reported broad Lone Star coverage and other map-wide spawns.

### Broad dynamic creature coverage

The zero-cell dynamic sequence table already shows creatures across many field contexts. Hell Hounds are eligible on 26 scene IDs: Downtown Seattle (0), Wilderness Areas 1-4 (6-9), Hollywood Correctional Facility (20), and additional contexts 26-29 and 35-50. Wendigos and Bandersnatches cover 1, 4, 6-9, and 44-50; Gargoyles cover 4, 6-9, and 44-55. These are temporary screen-edge event sequences with terrain-tested placement. The complete per-template scene and sum coverage follows below.

### Later-game Strike Team coverage

The disturbance handlers at 0x20EFA, 0x20F42, and 0x2115C make the same FFFBFB progression counter available across scenes 0-3, 5-9, and 44-50 (16 scene IDs). On the ten named city and wilderness area maps, this covers nine: Downtown Seattle (0), Puyallup Barrens (1), Redmond Barrens (2), Penumbra District (3), Renraku Arcology (5), and Wilderness Areas 1-4 (6-9). Scene names 0-5 come from ROM contract profiles; the Wilderness labels for 6-9 are guide cross-references in gameplay/world/areas/map-guide-labels.tsv. Council Island (4) has no disturbance handler. Contexts 44-50 are additional scene IDs whose map names are not established here.

With bit 8 of FFFC22 clear, stages 2-3 select two Strike Team actors, 4-6 select four, and 7-8 select five. The story choice at 0x21BE4 advances this shared counter and queues two Strike Team actors. The low-stage disturbance routes differ: the urban continuation returns explicit Renraku attack text, while the wilderness continuation can choose the creature packs listed above. This is the broad, later-game Strike Team route represented by the current disturbance tables. The wanted-state patrol logic and other map-event spawns remain separate paths to profile.

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
