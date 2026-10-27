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

The threshold comes from scene-profile word 41 (`FFDE18`, with the profile
beginning at `FFDDC6`). The extracted Scene 2 descriptor stores `0x6000` in
that word. `0x0D44` advances the shared source seed by `seed * 0x20E5 + 1`
modulo 16 bits; `0x0D3A` returns the high word of `seed * bound`. A Scene 2
sum-3 test therefore has to pass the chance draw first, then preserve the two
bound-5 selector draws and the per-actor source draws that seed initial
behavior delays.

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

### Field alarm state [C]

The field tick at `0x1DE7C` calls the alarm routine at `0x1DFC4` after the
scene-21-through-43 map-effect service. The alarm routine first decrements an
active timer at `FFE11E`; a signed negative result enters clear helper `0x1E09A`.
It then checks the security-terminal disable byte `FFE170`, requires
`(FFDF88 & 0x3F) == 0x0D`, and accepts only a `0x0D44` random result below
`0x8000`. The player's `FFFC04` position is converted to tile coordinates by
shifting each axis right four bits. The scene profile pointer at `FFDE1E`
contains the alarm rectangle list at `+0x0C`; each rectangle is four words
`x_min, x_max, y_min, y_max`, terminated by a negative `x_min`. A hit uses
exclusive lower and inclusive upper bounds on both axes.

The rectangle and profile pointer values are extracted from the US ROM into
`assets/field/field-alarm-profiles.tsv` and `field-alarm-regions.tsv`; their
record addresses remain in the rows as provenance. The extraction source is
`tools/rom_research/profile_field_world_effect_regions.py` [C]. The runtime
uses those typed scene rectangles directly. A hit in scene `0x14` schedules
the existing `0x1DBAA` timeout path and does not raise the alarm. Other scenes
require field mode `2`; they load timer `0x960` even if the alarm was already
active. A newly raised alarm sets `FFE16F`, starts sound `0x30`, clears bit 0
of each of the seven actor records' `+0x9E` flags, and requests the scene
profile's `+0x10` reinforcement list through `0x1E0DC`. The shared clear path
clears `FFE16F`, promotes `+0x9E` bit 1 to bit 2 for those records, and stops
sound `0x30`. A successful projected-terrain mark at `0x167D8` clears both
`FFE16F` and `FFE170` without taking the alarm-clear side effects.

The separate encounter selector called from `0x1E166` also changes while the
alarm is active: it bypasses the `FFDF84` gate and uses the table at `0x1E20A`
instead of `0x59986` [C]. The typed encounter outcome resolves that fixed list
from Lua content. Since `0x1E1E0` clears `D0`, the list uses the screen-edge
candidate branch at `0x1E302`; the map-derived `0x1E40C` branch is reserved for
calls that enter `0x1E216` with nonzero `D0`.

### Shared dice-pool check (`0x0DBA`)

The ROM helper at `0x0DBA` takes a dice-pool count in `D6` and a target number
in `D4`. It rolls that many six-sided dice; each 6 adds six and rerolls, then
the first non-6 face is added. A die counts as a success when its total reaches
the target. `FFF0DD` receives the number of successful dice. Thus a caller's
constant such as 5 or 7 is a target, not the number of dice rolled.

When `D6` starts at zero, the source loop still rolls one die: it clamps the
decremented loop counter to zero before its `DBF` loop.

The companion party helper at `0x23750` returns the number of active runner
records in `D0` and the average active-runner `+0x78` value in `D6`. Some event
handlers pass that average to `0x0DBA` as the dice pool; it is not a count of
the active runners. Other handlers load a runner field or highest party value
directly into `D6`. The individual traces below identify both inputs where
they are recoverable.

## ROM encounter evidence (partial)

Scene IDs below are ROM field-scene IDs. The tables cover the field random-event selector, its zero-cell dynamic actor templates, and the choice continuations traced so far. A listed actor is not a permanent resident or guaranteed on every visit. This is not yet a complete map-population or pursuit roster: direct map triggers, state-driven police response, timed actor spawning, and other scripted events are separate source paths.

### Choice-dependent actor paths traced in the ROM

| Event path | Eligible scenes and selector sums | What the choice does |
|---|---|---|
| Sudden disturbance, handler `0x20EFA` | Scenes `0-3` and `5`; sums `0-3` and `7` | MORE checks `FFFBFB` and `FFFC22` bit 8. With stage `0-1` or the bit set, it selects one of 20 resource bundles but returns the holocam/photo text, with no Renraku attack. With stage `2-8` and the bit clear, it attempts a 2/4/5-person Strike Team encounter; a failed sequence start falls back to the photo text. |
| Sudden disturbance, handler `0x20F42` | Scenes `0-1` and `3`; sums `0`, `3`, and `5` | MORE checks `FF018F` bits 1 and 2. The first set bit takes priority: bit 1 attempts four Mafia Eye-Fivers, bit 2 attempts four `weenr2` actors; neither bit gives only the generic gang-color text. A failed actor sequence falls back to the photo text. This handler does not use the Renraku or Lone Star follow-up. |
| Sudden disturbance, handler `0x2115C` | Scenes `6-9` and `44-50`; sum `0` | MORE checks `FFFBFB` and `FFFC22` bit 8. Stages `0-1` or a set bit choose a wilderness pack from six weighted outcomes; stages `2-8` with the bit clear attempt a 2/4/5-person Strike Team encounter. |
| Lone Star patrol, handler `0x20F12` | Scenes `0` and `3`; sums `3` and `4` | Attack queues the list at `0x1D6FDC` (`lone11`, `lone12`, `lone detectiv`). Run away may queue `0x1D6FE4` or `0x1D6FE0`, according to the thresholded roll result. |
| Injured-person event, handler `0x20F72` | Scenes `0`, `1`, and `3`; sums `5` and `7` | The ordinary injured-man Attack choice can queue one Lone Star detective at `0x1D6FE4`. A separate pale-figure variant can reveal a vampire; its sequence starts at `0x1D6D8A`, after the Hell Hound pointer in the shared list. |
| Open security panel, handler `0x20FBA` | Scenes `21-25` sums `1, 7`; `26-29` sum `2`; `30-34` sum `0` | Re-wiring rolls the party's highest Electronics field (`+0x8A`) as the dice pool against target 5. Zero successes trip the alarm and process the scene's map-effect list (2-5 corporate guards in these scenes); more than one disables the current world-effect service through `FFE170`. Walking away chains into the scientist prompt. |
| Scientist, handler `0x20FD4` | Scenes `21-25` and `30-34`; sums `4` and `6` | Asking for information rolls slot 0's source field `+0x7A` as the dice pool against target 5. Both result branches return refusal/suspicion text; walking away returns a leave-behind and floor-text sequence. Neither command directly queues an actor. |
| Hidden-camera event, handler `0x21004` | Scene `20` sum `0`; scenes `26-29` sum `0`; scenes `35-40` sum `7`; scenes `41-43` sum `1` | MORE uses the active party's average `+0x78` as the dice pool; its target is 4, raised to 5 or 6 by the active support-slot flags. Zero successes show the alarm text; scene 20 only schedules its special timeout, while scenes 26-43 process their map-specific corporate-guard lists. Any success gives the nerves text. |
| Management terminal, handler `0x2109A` | Scenes `21-25` sum `5`; `26-29` sum `4`; `30-34` sum `3`; `35-40` sum `0` | Hack rolls the Computer field (`+0x88`) as the dice pool against target 5: zero successes sound the alarm and process the scene's corporate-guard list; one finds nothing; two or more compose a floor-labeled database clue. Walking away has no follow-up effect. |
| Company Man confrontation, handler `0x210B4` | Scenes `21-25` sum `8`; `26-29` sum `1`; `30-34` sum `2`; `35-40` sums `2` and `5` | Attack rolls the party's average `+0x78` as a dice pool against target 7; 0 or 1 successes raise the alarm, while more than 1 adds one Karma to each runner. Insult rolls slot 0's `+0x7A` against target 7: zero calls security, one makes him laugh, and more than one returns a floor-labeled clue. Walk away is text-only. |
| Custodian / missing-pass check, handler `0x2101E` | Scene `20`; sum `1` | Asking for information uses a 0-99 roll, the ID-badge flag (`FFFC22` bit 10), and two story bits (`FFFC1E` bits 18/19) to select a clue, suspicion, or evasive response. Walking away gives a 25% “Where’s your pass?” branch only when the badge bit is clear; its MORE command enters the shared corporate-guard ID handler `0x242FA`. No direct Lone Star roster is queued. |
| Alley grenade seller, handler `0x2112C` | Scenes `0`, `2`, `3`, and `5`; sums `5-7` | Attack rolls `0-59` try the one-bodyguard list at `0x1D7054`; a successful start leads to a gun battle. Rolls `60-99`, or failure to start that actor, reveal undercover Lone Star. Buying the grenades has its own 60/40 split: rolls `0-59` continue through funds/slot checks; `60-99` expose the undercover agent. |
| Company Man confrontation, handler `0x20F5A` | Scenes `3` and `5`; sums `1` and `4` | One continuation says the man remains irate and a Lone Star patrol arrives. The response text then says the officers turn their attention to the player. |
| Flustered-woman event, handler `0x20F8A` | Scenes `0-1`; sums `3-4` | The MORE continuation divides into suspected-harm and apparent-law-official versions. The law-official version can reveal an undercover Lone Star agent; its Attack/Comply/Run choices can end in a gun battle, questioning/release, fine, arrest/confiscation, escape, or being overtaken and shot. See the terminal branch table below. |
| Haggard-man event, handler `0x20FA2` | Scenes `2` and `5`; sums `1-2` | The doctor offers 50 nuyen to recapture his escaped patient. Help and Threaten resolve without combat; Attack or Threaten can call a patrol, and one alternate Attack branch starts a fight with the doctor's Gator Shaman/bodyguards. See the terminal branch table below. |
| Security-guard stop, handler `0x20FEC` | Scenes `20`, `26-34`, and `41-43`; sums `4-5` and `7` | Without the badge flag, Attack starts a corporate-guard encounter and Talk branches on `FF017A`; the talk result can trigger a dispatch check or continue to a security-mage prompt. |
| Lone Star patrol, handler `0x2106A` | Scenes `41-43`; sum `5` | `0x24622` checks badge bit 10 and can waive the player through, verify ID, or open arrest. Attack rolls at `0x2466C`; a zero result shows the fatal-warning prompt, while nonzero passes list `0x1D6FEC` to the spawner and returns combat dialogue. Comply reaches settlement `0x235AA`. |
| Concealed plate / laser to Lone Star | Plate handler `0x21036`: scenes `20-34`, `41-43`; sums `2`, `5-6`. Laser handler `0x21050`: scenes `35-40`; sum `8`. Direct patrol handler `0x2106A`: scenes `41-43`; sum `5`. | The plate response can continue to the laser check. Both laser outcomes contain patrol command `0xD9` -> `0x24622`, the same badge/dispatch/arrest decision used by the direct patrol event. See the source trace below. |
| Lone Star dynamic templates | Scene `1` (sum `2`) and scenes `41-43` (sum `8`) | The zero-cell dynamic sequences include `lone11`, `lone13`, and `lonemage`. |
| Disturbance Strike Team escalation | Scenes `0-3`, `5-9`, and `44-50` | The MORE continuations check `FFFBFB` and bit 8 of `FFFC22`. With the bit clear, stages `2-3` select two Strike Team templates (`0x1D6FD0`), `4-6` select four (`0x1D6FC8`), and `7-8` select five (`0x1D6FC4`). Successful starts return the explicit Renraku attack text; a failed start falls back to the holocam/photo text. Stages `0-1` or a set bit take each handler's separate low-stage route. |
| Wilderness disturbance packs, handler `0x2115C` | Scenes `6-9` and `44-50` | At stages `0-1` or with bit 8 set, the ROM rolls 0-99: 30% three Hell Hounds, 30% two Wendigos, 20% two Bandersnatches, 5% a Gator Shaman plus two Indian warriors, 5% a Rat Shaman plus two Indian warriors, and 10% an Elven mage plus two Elven warriors. There is no no-list result in this branch. At stages `2-8` with the bit clear, it uses the Strike Team groups above. |
| Explicit Renraku Strike Team prompt, handler `0x210CC` | Scenes `35-40`; sum `1` | The MORE choice dispatches to `0x24918`, which queues two Strike Team templates from `0x1D6FD0`. This is the selector path whose text explicitly names Renraku. |
| Renraku Security prompt, handler `0x210E4` | Scenes `35-40`; sum `4` | Its follow-up dispatches to `0x24936` and queues one corporate-guard template from `0x1D705C`. Keep it separate from the Strike Team event. |
| Scene-47 mode-`0x13` lists (Cave 4 guide cross-reference) | Scene `47`; lists `0x1E5C4` and `0x1E628` | One list has one Strike Team, two bodyguards, and a pedestrian; the other has three Strike Teams and a corporate guard. These ROM lists match Mike's Cave 4 map note about a Renraku strike team; the cave name is guide cross-reference evidence, not a ROM label. They are separate from the random disturbance and the scene-35-40 selector outcomes. |

A separate source path also selects list `0x1E628`. Prompt command `0x60`
dispatches through table `0x12CCB4` at `0x1F57C`; its entry at `0x12CE34`
points to raw-ROM code `0x225D2`, which is absent from the exported
disassembly. It debits 75 nuyen when available, then its Mako-activity gates
require FFFC1E bit 10
set, bit 11 clear, and either FFFC30 bit 21 or FFFC38 bit 3. It writes
`0x1E628` to `FFE120`, selects resource profile `0x122000`, uses stream
`0xF4CBC`, then follows descriptor `0xF4CD0` before the field transition.
When those story gates pass and the debit succeeds, `0x21A4E` leaves D1
negative; `0x225D2` records `FFF0D6=-1`, queues the list, and clears D6.
Shared exit helper `0x1FED2`
preserves that clear D6, so finalizer `0x1FD80` allows Scene-3/event-0 row 0
to continue to field scene 17. `0x02900` calls `0x04388`, which runs the
common `0x043B6` world-frame service. It drains `FFE120` without checking the
scene and installs actor-list outcome callback `0x53FFC`. After the four
`FFE177`-counted actor behavior transitions, the callback marks Mako defeated.
The list's extracted Scene-47 table context does not override the traced
Scene-17 route. The menu node at `0x111316` contains
command `0x60` in its first choice slot at `0x111326`. Adjacent code at
`0x225CA` loads `0x111316` into A0 and returns. The title stream `0x114A88` is
the 75-nuyen entry-fee prompt, with first option `0x103A7A` (“Easy money,
chummer. I'll pay”) and second option `0x104FEB` (“I'll talk to you later,
chum”). The command-0x60 caller is traced to the Club Penumbra
Scene-3/event-0 doorman branch in finalizer `0x1FD80`. The extractor indexes
the same list under the Scene-47 activity-table context, but the source exit
sequence continues to Scene 17 and the common consumer has no scene transition.
The Club dialogue graph models the entry checks and both Mako introduction
pages. Its Lua action queues the extracted list before the pages; after the
source route resumes to Scene 17, the typed field path resolves and publishes
the four rows from the Scene-47 table context. The host counts four eligible
Lua behavior-completion edges and resets actor service links at zero. Dispatch
through FFE104 now invokes the typed Lua result graph; Qt route QA remains open. This is not a
random-event selector result.

The linked ROM records confirm this is a Mako story route. Entry descriptor
`0xF4CBC` points to text `0xF9718` (“Once inside, you inquire about Mako” and
the bartender points him out); its following descriptor `0xF4CD0` points to
`0xF9788`, where Mako notices someone behind the player. Actor-list outcome
callback
`0x53FFC` runs after the four `FFE177`-counted actor behavior transitions. It
adds 2 heat capped at 8, adds 2 to each runner's Karma source byte, sets
FFFC1E bit 11, clears FFFC30 bits 20/21 and sets bit 24, clears FFFC38 bit
17, increments the Strike Team counter `FFFBFB` up to 8, and opens descriptor
`0xF4CA8` / text `0xF98FA` about Mako lying unconscious. The callback trigger
does not test a kill flag or wait directly for the `FFDF86` counter armed by
`0x1DBAA`. The encounter gate's FFFC1E bit 10 is the
Michael alias learned through Boris command `0x2B`; its FFFC30 bit 21
alternative is set by the Big Rhino Mako-information action. Big Rhino's
answer says Mako has been spending time at Club Penumbra, consistent with the
doorman caller now identified at Scene 3/event 0. This clue is one of the
handler's two alternative Mako-information gates; the destination is no
longer inferred from that clue alone. See the [Boris
Greenhouse audit](visual-audits/boris-greenhouse-scene2-event6.md), [Big Rhino
audit](visual-audits/big-rhino-scene11.md), and [Club Penumbra audit](visual-audits/club-penumbra-scene3-event0-and-scene17-event30.md).

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

**Shared map-effect transition (`0x1E04A` / `0x1E0DC`).** In scene 20,
`0x1E04A` only calls `0x1DBAA` to schedule the world-activity timeout. In
other scenes it proceeds only in world mode 2 (`FFDF40 == 2`): it arms
`FFE11E` with `0x960`, then checks the per-activity latch `FFE16F`. If the
latch is clear, it sets it, requests audio event `0x30`, clears pending actor
effect flags, and calls `0x1E0DC`. The audio event's human-readable sound name
is not established by this trace. A later call while the latch is set does not
spawn the list again.

`0x3074` loads the current scene profile from the pointer table at `0x3232`
into `FFDDC6`; the profile longword at offset `+0x58` is copied to `FFDE1E`.
`0x1E0DC` follows that pointer to its `+0x10` effect-list pointer. Each
nonzero 8-byte list entry supplies a tagged template pointer and a location
record pointer. After removing the tag bit, every entry in scenes 21-43 uses
the ROM template at `0x1D6A1C`, named `corp guard`. For a visible entity,
`0x1E0DC` projects it and copies the location record's X/Y words into the
entity. It creates an entity; it does not itself call the combat-start
routine. If entity or effect-slot allocation fails, the list stops at that
entry, so the following counts are the map list's maximum entries, not a
guaranteed number on screen.

| Scene IDs | Effect-list pointer | Corp-guard entries |
|---|---:|---:|
| `21` | `0x1D7460` | 2 |
| `22`, `24` | `0x1D7AE6` | 3 |
| `23` | `0x1D821A` | 2 |
| `25` | `0x1D8AA0` | 2 |
| `26` | `0x1D9868` | 3 |
| `27` | `0x1D9F4E` | 3 |
| `28-29` | `0x1DA71E` | 4 |
| `30` | `0x1DAF34` | 5 |
| `31`, `34` | `0x1DB518` | 3 |
| `32-33` | `0x1DBB10` | 4 |
| `35` | `0x1DC2C4` | 3 |
| `36-37` | `0x1DCA24` | 4 |
| `38` | `0x1DD17C` | 2 |
| `39` | `0x1DD8E2` | 4 |
| `40` | `0x1DDD96` | 2 |
| `41` | `0x1DE2B0` | 4 |
| `42-43` | `0x1DE8BA` | 4 |

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

**Secretary / building badge check (`0x21082`, scenes 21-29 and 41-43; sums
2-3 and 8).** The opening dialogue at `0x121414` reaches command `0xDC` and
handler `0x246AA`. It checks `FFFC22` bit 10, which this dialogue identifies
as the ID-badge condition. With that bit set, it returns the apology and
choices at `0x121428`: ask for information (`0xDE` -> `0x24724`) or walk away
(`0x121448`). The information action uses byte `FF017A` as dice-pool count
against target 6 (the byte's stat name is not established here). Results above 1
compose 'You manage to make her laugh...' with the current floor string
selected by `FFE12C`, then return the shared dialogue data at `0x11CD92`;
result 1 returns the follow-up chain beginning at `0x121440`; result 0 writes
the failed-charm text and returns `0x11CD92`. On that last branch, a separate
0-99 draw at `0x247A2` controls whether the same failure text is copied a
second time (threshold `0x46`), not whether an alert is raised. The exact
selection behavior beyond the shared `0x11CD92` node and the full linked
follow-up from `0x121440` remain partial.

When the badge bit is clear, `0x23750` averages field `+0x78` over active
runner records into dice-pool count `D6`; `0x246AA` checks that pool against
target 6. Result
0 enters `0x1E04A` and returns the secretary-flees text at `0x121460`. Result
1 returns the scream/subdue response at `0x121478`, whose command `0xDD`
reaches `0x246FC`: a 0-99 draw below 40 enters `0x1E04A` and reports that the
building goes on alert; 40-99 reports that no alert comes and the secretary
flees. Results above 1 return `0x12148C`, where the text says she is subdued
and locked in a closet, followed by floor-specific response data. In world
mode 2, `0x1E04A` arms the `0x960` transition timer, latches the effect, and
starts the map's world-effect processing; it does not directly name or queue
a Lone Star roster. This handler and its two random checks do not read the
heat byte `FFFBFA`. Treat this as a secretary/building-security event, not as
evidence of a general Lone Star spawn. The later shared floor-choice action
and map-effect actor results are not yet joined.

**Open security panel (`0x20FBA`, scenes 21-34).** The prompt at `0x1211BC`
offers Re-wire (`0xC9` -> `0x2416A`) or Walk away (`0x1211EC`). The re-wire
service (`0x24252`) selects the highest Electronics value among active runner
slots using their `+0x8A` fields, then calls `0x0DBA` with that value as the
dice-pool count and target 5. The resulting success count is in `FFF0DD`.

A zero result returns the crossed-wires/building-alarm text at `0x1211D4` and
calls `0x1E04A` to start the map's world-effect transition. The handler names
no fixed guard list of its own. In scenes 21-34, the shared transition
processes the scene-specific list above, which contains two to five corporate
guards; the call does not itself start combat. Result 1 returns the 'tougher
than it looked' response at `0x1211DC` without a direct state write. A result above 1 returns the camera
feed-disconnected response at `0x1211E4` and sets `FFE170`. The active-effect
service (`0x163DA`) and world-effect scheduler (`0x1DFC4`) both check this flag;
the world-activity initializer (`0x167AE`) clears it. Thus the ROM records a
local security-system outcome, not a fixed Lone Star spawn.

Walking away returns `0x1211EC`, whose first text is 'You leave them behind'
and whose following prompt data is the scientist interaction at `0x20FD4`.
That event's standalone prompt is `0x1211F4`; asking for information (`0xCB`
-> `0x2429E`) uses slot 0's `+0x7A` field as the dice-pool count against target 5.
Result 0 returns
`0x12120C` (the scientist looks concerned and disappears behind a secure door,
then refuses); results 1 or higher return the refusal at `0x121214`. Walking
away returns `0x12121C`, beginning with the leave-behind text and the floor
string sequence. These routines do not write funds or queue actors. The linked
floor/security continuation after these returned text records remains partial.

**Management terminal (`0x2109A`, scenes 21-40).** The selector chooses this
event at sum 5 in scenes 21-25, sum 4 in scenes 26-29, sum 3 in scenes 30-34,
and sum 0 in scenes 35-40. The prompt record at `0x1214AC` offers Hack the
database (`0xDF` -> `0x247D0`) or returns the walk-away record at `0x1214D4`
(`0x11D2A8`).

`0x247D0` reads the runner's Computer skill byte at `FF0188` as the dice-pool
count and calls `0x0DBA` against target 5. With `FFF0DD == 0`, it calls
`0x1E04A` and returns the alarm message at `0x1214C4`. The shared transition
therefore adds the current
scene's corporate-guard map-effect list when its world-mode and latch checks
allow it; the maximum list sizes are two to five across scenes 21-40. This is
not a direct combat handoff, and the actor AI/response after insertion remains
to be traced. With exactly one success, it returns the no-value message at
`0x1214CC`. With more than one, it copies the database-search line and a
first-through-sixth-floor text variant to `FFB740`, then returns shared text
node `0x11CD92`. `FFE12C` indexes that floor-label table; this branch alone
does not prove that the selected label is the runner's actual current floor.
The terminal handler does not directly write funds or story flags. Walking
away returns its text record without a state change.

**Company Man confrontation (`0x210B4`, scenes 21-40).** The selector enables
this handler at sum 8 in scenes 21-25, sum 1 in scenes 26-29, sum 2 in scenes
30-34, and sums 2 and 5 in scenes 35-40. Prompt record `0x1214F4` uses
profile/resource pointer `0x121E2C` and offers Attack (`0xE0` -> `0x24846`),
Insult (`0xE1` -> `0x2489A`), or Walk away (`0x121538`).

Attack calls `0x23750`, then uses the average active-runner `+0x78` value as
the `0x0DBA` dice pool against target 7. Zero successes call `0x1E04A` and
return `0x121510` (As you pull your guns, the Company Man flees and raises
the alarm.); exactly one success also calls `0x1E04A` and returns `0x121518`
(You wound him as he escapes down the hall. The alarm soon comes to life..).
More than one success increments `+0x8D` on all three runner records (Karma)
and returns the death text at `0x121520`/`0x11FF24`.

Insult rolls slot 0's `+0x7A` field as the dice pool against target 7. Zero
successes call `0x1E04A` and return the security-called text at `0x121528`;
exactly one returns the laugh-and-leave text at `0x121530`; more than one
stages the stares-you-down line plus a first-through-sixth-floor response
selected from the table at `0x121540` using `FFE12C`, then returns shared text
node `0x11CD92`. Walk away returns `0x121538`/`0x12013E` (He saunters away.)
with no direct state write identified. For alarm branches in scenes 21-40,
the shared map-effect service may process that scene's list of two to five
corporate guards when its mode and latch checks pass; it does not start
combat or name a Lone Star roster.

**Hidden-camera check (`0x21004`, scenes 20, 26-29, 35-43).** The MORE command
(`0xD4` -> `0x244D2`) calls `0x23750`, which returns active-runner count in
`D0` and average active-runner `+0x78` in `D6`. That average is the
`0x0DBA` dice-pool count. The handler sets target `D4` to 4, then adds 1 for
each active support slot (`FF0210` and `FF0310`, bit 0), giving target 4, 5,
or 6 for one, two, or three active runners. Its entry stub also clears
`FFF10C` and writes `0xFF` to `FFF0B5`; their meanings are not established here.

If `FFF0DD` is zero, `0x244D2` calls `0x1E04A` and returns `0x121320`: the
alarm sounds and the player recognizes the hidden camera. Any nonzero result
returns `0x121328`: nothing happens and the player suspects nerves. This event
does not write the heat byte `FFFBFA` or name a Lone Star roster. On scene 20,
the alarm follows the special timeout-only path. On scenes 26-43, the shared
transition processes the scene-specific corporate-guard list above when its
world-mode and latch checks pass. It does not itself start combat.

**Custodian / missing-pass check (`0x2101E`, scene 20, sum 1).** The entry
stub pairs prompt record `0x121330` with profile `0x121E5C`. That profile is a
56x72 dialogue portrait; the ROM also selects the same profile for a separate
scene-19 old-man conversation through `0x20990`/`0x20C78`. The shared profile
does not establish that the two events are the same character or that either
portrait is a map actor. The custodian prompt offers “I'm looking for some
information” (`0xD5` -> `0x2451C`) and “Walk away” (`0xD6` -> `0x2457C`).

Asking for information draws 0-99. `FFFC22` bit 10 is the ID-badge flag
identified by the adjacent security dialogue. With the flag set, rolls 0-4
return `0x121358` (“Why do you want to know?” followed by the suspicious
walk-away text); rolls 5-24 check the story bits; rolls 25-99 return the
evasive response at `0x121348`. With the flag clear, rolls 5-24 return the
suspicious response, rolls 0-4 check the story bits, and rolls 25-99 return
the evasive response. In the story-bit check, `FFFC1E` bit 18 must be set and
bit 19 clear to return `0x121350`, which tells the player that Ilene Two Fists
is in a northern cell. If that condition fails, the result is the evasive
response at `0x121348`. These response records have no choice commands, and
the two handlers do not directly change story flags, funds, or actor lists.

Walking away returns the “He continues mopping as you walk on” response at
`0x121368` whenever the ID-badge bit is set. With the bit clear, `0x2457C`
draws 0-99: 0-74 returns the same mopping response; 75-99 returns
`0x121370` (“Hey, where's your pass?”). Its MORE command (`0xCC`) reaches the
shared ID handler `0x242FA`, already traced under the security-guard stop.
That follow-up is corporate security: its Attack/Talk and ID/dispatch paths
can attempt a corporate guard or continue to the security-mage prompt; they
do not name Lone Star. The custodian commands themselves contain no direct
story-flag, funds, or actor-list write. The adjacent concealed-plate prompt at `0x121384`
is a separate selector entry; adjacency in the prompt data does not prove that
the custodian event automatically continues into it.

**Security-guard stop (`0x20FEC`, scenes 20, 26-34, and 41-43).** The opening
record `0x121238` asks for an ID and dispatches command `0xCC` to `0x242FA`.
With FFFC22 bit 10 clear, the record at `0x121278` offers Attack and Talk.
Attack (`0xD2` -> `0x24402`) attempts one corporate guard from list
`0x1D705C` (template `0x1D6A1C`) and returns the weapon-draw text at
`0x121290`; the combat exit remains partial. Talk (`0xD0` -> `0x24432`) runs
uses `FF017A` as the dice pool against target 6: result 0 attempts that guard and starts the
world-effect transition before returning the lame-story attack text at
`0x121298`; result 1 returns `0x1212A0`, where the guard becomes suspicious and
checks with dispatch; results above 1 return `0x1212B4`, where the guard
accepts the story but the dialogue continues into a separate security-mage
check (`0xCD` -> `0x24332`).

With FFFC22 bit 10 set, `0x242FA` rolls 0-99: below 20 returns the
ID-verification record `0x121254`, while 20-99 returns the show-ID/wave-by
record `0x12124C`. The dispatch handler `0x24392` rolls 0-99; 0-39 attempts
one corporate guard, starts the world-effect transition, and returns the
negative-dispatch text at `0x121270`; 40-99 returns the verified/wave-by text
at `0x121268`. These are corporate-security branches. The separate
concealed-plate/laser entries below are what connect this wider scene group to
the Lone Star patrol handler.

**Security-mage check (`0x210FC`, scenes 20-43).** The exact selector cells
are scene 20/sum 2; scenes 21-25/sum 6; 26-29/sum 7; 30-34/sum 8; 35-40/sum
3; and 41-43/sums 3 and 7. Its prompt at `0x1212BC` says 'A security mage
spots you. ID please.' and sends MORE (`0xCD`) to `0x24332`. The security
guard Talk-success continuation at `0x20FEC` reaches this same command.

`0x24332` reads `FFFC22` bit 10. Clear returns the no-badge page `0x1212D0`,
with Attack and Talk choices. Set rolls `FF017C` as the dice pool against
target 5: result
2 or higher opens the ID/dispatch page at `0x12124C`; below 2 attempts the
effect list at `0x1D7064`. Its first template is the ROM's corp mage at
`0x1D6A66`. A failed sequence start returns to the ID page; a successful start
calls `0x1E04A` and returns the 'deadlier spell' text at `0x1212E8`.

Without a badge, Attack (`0xD3` -> `0x2441A`) attempts that corp-mage list and
returns the weapon-draw text at `0x121290`; the routine does not check the
sequence-start result. Talk (`0xD1` -> `0x24482`) rolls `FF017A` as the
dice pool against target 6.
Only result 1 reaches the 'misplaced ID' dispatch page at `0x1212F0`; result 0
and results above 1 attempt the corp-mage list, call `0x1E04A`, and return the
attack text at `0x121298`. The unusual single-result success is what the ROM
encodes.

The ID page's MORE command (`0xCE` -> `0x24392`) rolls 0-99: 0-39 attempts
one corp guard from `0x1D705C` (template `0x1D6A1C`), calls `0x1E04A`, and
returns negative-dispatch/alert text at `0x121270`; 40-99 returns the verified
ID/wave-by text at `0x121268`. The 'misplaced ID' page's MORE command
(`0xCF` -> `0x243CA`) uses the same 40/60 split, but its 0-39 result attempts
the corp-mage list at `0x1D7064` before returning the same `0x121270` text.
That dispatch text is reused across the guard and mage paths. These routines
do not read the heat byte `FFFBFA` or name a Lone Star roster; the alert and
actor sequence are corporate-security outcomes. The mage's successful combat
exit and the post-alert actor result remain partial.

**Concealed-plate / laser patrol chain (`0x21036`, `0x21050`, `0x2106A`).**
These are sibling selector entries, not a direct branch from the preceding
`0x20FEC` security-guard dialogue. The concealed-plate entry `0x21036` is
enabled on scenes `20-34` and `41-43`, using scene-specific sums from `2`, `5`,
and `6`. Its `0x245AA` check uses the average active-runner `+0x78` value
from `0x23750` as the dice pool. The target starts at 4 and increases by one
for each active support slot (`FF0210` and `FF0310`, bit 0). Result zero calls
`0x1E04A` and returns the alarm text at `0x121398`; a nonzero result returns
the successful jump text at `0x1213A0`. Both response records carry the
continuation to command `0xD8` / `0x245F4`.

The low-level laser entry `0x21050` is enabled on scenes `35-40` at sum `8`
and enters the same `0x245F4` check. It uses the average active-runner `+0x78`
value from `0x23750` as the dice pool against target 8. Result zero calls
`0x1E04A` and returns
`0x1213BC`, where the player says a beam broke their leg; a nonzero result
returns `0x1213C4`, where the player freezes and steps over the beams. Both
continuations reach command `0xD9` / `0x24622`, which displays the Lone Star
patrol encounter. This is a conditional patrol path enabled by the plate and
laser event cells across scenes `20-40`, plus scenes `41-43`; the direct
patrol entry `0x2106A` is separately enabled on scenes `41-43` at sum `5`.

At `0x24622`, a clear badge bit 10 opens the arrest prompt. With the bit set,
a 0-99 roll below 15 also arrests the player, 15-49 waves them past, and
50-99 calls dispatch; dispatch rejects the ID below 40 and verifies it at
40-99. Thus the text “Lone Star patrol spots you” does not mean combat always
starts. Scene `20` is guide-cross-referenced to Hollywood Correctional
Facility; this trace does not yet establish names for the remaining scene
IDs. The three entries are present in the ROM event-cell tables, and the
branch behavior comes from their linked handlers and dialogue records.

**Renraku team/security prompts (`0x210CC`, `0x210E4`, scenes 35-40).** The
team prompt is selected at sum `1`; MORE command `0xE2` calls `0x24918`, which
attempts the list at `0x1D6FD0` (two Strike Team templates at `0x1D6AFA`),
starts the world-effect transition, and returns response data at `0x12156C`.
The security prompt is selected at sum `4`; MORE command `0xE3` calls
`0x24936`, which attempts one corporate guard from `0x1D705C`, starts the same
transition, and returns that same `0x12156C` data. The shared response begins
“There he is, that's Joshua!! Open fire!” and then contains the Renraku
Security line and another `0xE3` continuation. This ROM data is cross-linked;
the exact end of the repeated/shared dialogue and combat exit remain partial.

**Urban disturbance / Renraku (`0x20EFA`, scenes 0-3 and 5).** MORE command
`0x93` reaches `0x2333A`. If `FFFBFB <= 1` or bit 8 of `FFFC22` is set, the
ROM chooses one resource bundle from the 20-entry table at `0x12D09C` and
returns the holocam text at `0x11CF74`: someone takes the player's picture and
disappears. This branch does not start a Renraku fight. With `FFFBFB` at 2-8
and the bit clear, stages 2-3 attempt two Strike Team actors (`0x1D6FD0`),
stages 4-6 attempt four (`0x1D6FC8`), and stages 7-8 attempt five
(`0x1D6FC4`). A successful start returns the explicit attack text at
`0x11CFDD`; if the sequence cannot start, control falls back to the holocam
text. The post-combat exit is still partial.

**Faction disturbance (`0x20F42`, scenes 0-1 and 3).** MORE command `0x9B`
reaches `0x23876`. If `FF018F` bit 1 is set, the ROM attempts four Mafia
Eye-Fivers from `0x1D7010` and selects the Eye-Fiver text at `0x11D098`. Only
if bit 1 is clear does it test bit 2; when set, it attempts four `weenr2`
actors from `0x1D7038` and selects the Halloweeners text at `0x11D019`. If
neither bit is set, it returns the generic veteran-runners/gang-colors text at
`0x11D128` without queuing attackers. If either selected sequence cannot
start, it falls back to the same randomized-resource holocam event used by
`0x20EFA`. This is a separate faction encounter, not the Renraku branch and
not a Lone Star encounter. The source test uses the player-record byte at
`FF018F`; the meaning and writers for bits 1-2 are still untraced. The combat
exit remains partial.

**Wilderness disturbance (`0x2115C`, scenes 6-9 and 44-50).** MORE command
`0x94` reaches `0x233A6`. With `FFFBFB` at 2-8 and bit 8 of `FFFC22` clear, it
uses the same 2/4/5 Strike Team escalation as `0x20EFA`; a failed start
returns to the holocam text. At stages 0-1 or with the bit set, `0x24AA8`
rolls 0-99 and chooses: 0-29, three Hell Hounds (`0x1D6F50`); 30-59, two
Wendigos (`0x1D6F68`); 60-79, two Bandersnatches (`0x1D707C`); 80-89, the
AmerIndian hunting-party text followed by a bound-2 split between a Gator
Shaman plus two Indian warriors (`0x1D70AC`) and a Rat Shaman plus two Indian
warriors (`0x1D70BC`); or 90-99, an Elven mage plus two Elven warriors
(`0x1D70CC`). Each of the two hunting-party actor lists therefore has a 5%
chance. The previous summary's 5% no-list branch was incorrect. Combat exit
and post-fight outcomes remain partial.

**Being stalked (`0x21114`, scenes 6-9 and 44-50).** The enabled sums are
4 and 6 in scenes 6-9, and 2 and 8 in scenes 44-50. The prompt at `0x121588`
offers Attack (`0xED`), Duck for cover (`0xEE`), and Wait (`0xEF`). This is a
separate event graph from the wilderness disturbance above; it does not check
the `FFFBFB` progression stage or bit 8 of `FFFC22`.

Attack enters `0x24AA8` and rolls 0-99: 0-29 attempts three Hell Hounds from
`0x1D6F50`; 30-59 attempts two Wendigos from `0x1D6F68`; 60-79 attempts two
Bandersnatches from `0x1D707C`; 80-89 selects one of two hunting parties with
a bound-2 roll (Gator Shaman plus two Indian warriors at `0x1D70AC`, or Rat
Shaman plus two Indian warriors at `0x1D70BC`); 90-99 attempts an Elven mage
and two Elven warriors from `0x1D70CC`. The text nodes are `0x1215DC` through
`0x1215FC`. These starts hand off toward combat; the combat exits are not
joined yet.

Duck enters `0x24B5A`, makes a separate 0-99 roll, and chooses target 8, 6,
6, 5, or 5 for the same five bands. The `0x0DBA` dice-pool count is the
average `+0x78` value across active runner records (`0x23750`). A zero result
in `FFF0DD` falls through to the Attack pack roller.
A nonzero result returns the matching avoidance node: Hell Hounds lose the
player's scent (`0x1215A4`), a Wendigo passes (`0x1215AC`), a Bandersnatch
fails to notice the player (`0x1215B4`), the AmerIndian hunting party passes
(`0x1215BC`), or the elves continue past (`0x1215C4`). The `80-89` band uses
resource descriptor `0x121E08`; `90-99` uses `0x122024`.

Wait enters `0x24BF0`: rolls 0-49 use the same Attack pack roller; 50-74 show
the guide prompt at `0x1216E0`; 75-99 call `0x59BCC` and return the healer
text at `0x1215CC`. The guide offers the AmerIndian Village for 75 nuyen and
the Sinsearach for 200. The payment handlers subtract from `FFFBFE`; if funds
are sufficient, the 75-nuyen route initializes scene 6 and the 200-nuyen route
initializes scene 9. Insufficient funds and either decline route return the
shaman's refusal text without changing destination. The healing helper clears
status bits 6 and 4 on each of three runner records, calls the two source
record services with value 10, then writes `0x0A` to each record's `+0x4C`
and `+0x9C` timer fields. Their gameplay meanings are not inferred here.

A neighboring text node at `0x1215D4` describes an AmerIndian hunting party
signaling the player to stay quiet. The traced choices above do not explicitly
branch to that node; its connection to the shared result consumer remains
unresolved, so it is not counted as a confirmed outcome. The outcome table
includes the urban and wilderness disturbance branches, this stalker event,
and the direct patrol, Company Man, injured-person, alley-seller, watcher,
and scene-41-to-43 Lone Star paths. Rows marked `partial` name a specific
follow-up still open; a handler with some traced rows is not automatically
treated as complete.

**Arrowhead search (`0x21144`, scenes 6-9; sum 2).** The prompt at
`0x121688` says the AmerIndian boys are looking for an arrowhead and offers
Attack, Help them, or Walk away. Attack enters `0x24CF4`, which attempts the
list at `0x1D70C0`: two Indian Warrior actors using template `0x1D6C22`. The
returned text says the boys flee and nearby warriors notice the player. This
routine does not check the sequence-start result, so whether the encounter
actually starts remains conditional; its combat exit is also not traced.

Help enters `0x24D0C`. It selects the highest `+0x7B` value among active
runner records as the dice-pool count and calls `0x0DBA` against target 4.
`FFF0DD=0` returns the failure text at `0x1216AC` and makes no direct nuyen
change. A result of 1, or a result above 1 outside scene 9, adds 50 nuyen to
`FFFBFE` and returns the arrowhead/reward text at `0x1216B4`. For a result
above 1 when the current scene byte `FFFBF8` is 9, the ROM instead adds 100
nuyen and returns the dragon-scale story at `0x1216BC`; its MORE link continues
to the automatic sale text at `0x1216D0`. This branch does not call an item
inventory service or write an item record. Walk away returns the text-only
page `0x1216D8` (?You leave them behind.?) without a direct state change.

**Phantasmal watcher (`0x20F2A`, scenes 0, 3, and 5; sums 1-2 and 8).** The
opening prompt at `0x120F00` links the two responses directly to data nodes:
refusing reaches `0x120F18`, where the player is rebuked and the watcher
vanishes; accepting reaches `0x120F20`, where the watcher leads the player to
an old man being beaten by Eye-Fivers. Dispatch `0x9A` (`0x23812`) loads the
player field at `FF0177` as the dice-pool count against target 6, then branches on
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
fatal warning or pass list `0x1D6FEC` to the effect spawner; the first three
template entries are `lone11`, `lone12`, and `lonemage`. The raw list then contains nonzero longwords
`0x00000002` and `0x9D020000`, followed by the null at `0x1D7000`. The iterator
in `0x1E1FC` treats each nonzero longword as a candidate template address and
stops on a null entry or actor-allocation failure; it does not parse this tail
as metadata. The tail's intended meaning and resulting actor state remain
unresolved. If dispatch rejects the ID, record `0x121270` is a text-only
negative-dispatch page: its second text pointer is `0xFFFFFFFF`, which makes
the common prompt loop wait for input and return without presenting choices
(`0x1F3C4` -> `0x1F412` -> `0x1F592`). The nearby `0x104EF8` string and Attack
label are in the overlapping record at `0x121278`; that is the separate
no-badge corporate-guard prompt returned by `0x242FA`, not the dispatch-reject
page. Its Talk command `0xD0` reaches `0x24432` and is traced in the security-
guard section above. The `0x1D6FEC` actor-list tail remains unresolved.

Comply (`0xDA` -> `0x24662`) sets response record `0x12140C` and branches to
the shared arrest settlement at `0x235AA`. That routine visits all three
runner records, clears item codes `8`, `9`, `10`, `12`, `13`, `20`, `49`, and
`50` from each record's eight item slots, clears heat `FFFBFA`, and computes a
base fine of `1000 * (former heat + 2)`.
The current Lua catalog in `gameplay/scripts/items.lua` maps those codes to
`ak_97`, `hk227_s`, `mach_22`, `roomsweeper`, `frag`, `maglock_passkey`,
`light_combat_armor`, and `heavy_combat_armor`; this set includes gear beyond
firearms.
If the runner cannot afford it, the ROM repeatedly halves the fine until it
fits the available nuyen, then deducts it. The returned record starts with the
text at `0x11D79E` (`You are escorted to Lone Star's main office...`) and its
MORE command (`0xDC` -> `0x246AA`) continues into the separately profiled
secretary/badge-check event (`0x21082`). The handoff is confirmed; the
secretary event's later floor/state continuation remains partial.

After the inventory, heat, and fine changes, settlement `0x235AA` has a second
conditional state handoff. If `FFFC1E` bit 0 is set, it enters the scene-mode
initializer through `0x2AFA` with scene value `3` and position-record pointer
`FFE114=0x2B14`. The field-entry loader consumes that record: camera origin
`(0x6D80, 0x0000)` and three runner positions `(0x7600, 0x0800)`,
`(0x7700, 0x0780)`, and `(0x7500, 0x0780)`. This trace establishes a transfer
to field scene `3`, but not its guide-map name. It then checks the active
`FFFBED` selector and `FFFC1C` bit 0. Selector
`1` clears the high effect slots through `0x1DED8`; selectors `2` and `4` clear
`FFFC22` bit 9. Each matching selector branch resets `FFFBEC` and `FFFC1C`
through `0x1E68C`, prepares context-dependent text using `0xF359E` (selector
`1`) or `0xF2518` (selectors `2`/`4`), and returns shared node `0x11CD92`.
If the matching bit is clear, or the selector is another value, that extra
cleanup/text branch is skipped. The selected scene's map identity and the
downstream shared text consumer still need their own source trace.

The per-scene viewer now shows each indexed handler, roll sums, visible opening
choices, dispatches, traced branch outcomes, and trace status. Handlers with no
terminal rows stay pending; handlers with a `partial` row are called out as
partial rather than appearing complete.

### Lone Star coverage by field context

| Scene | Named map | ROM selector paths linked to Lone Star | What is confirmed |
|---:|---|---|---|
| 0 | Downtown Seattle | `0x20F12`, `0x20F72`, `0x20F8A`, `0x2112C` | Direct patrol stop; injured-person Attack; undercover-woman branch; alley-seller Attack or purchase branch. |
| 1 | Puyallup Barrens | `0x20F72`, `0x20F8A` | Injured-person Attack and undercover-woman branch. |
| 2 | Redmond Barrens | `0x20FA2`, `0x2112C` | Doctor may call a uniformed patrol; alley seller may expose an undercover patrol. |
| 3 | Penumbra District | `0x20F12`, `0x20F5A`, `0x20F72`, `0x2112C` | Direct patrol stop; Company Man may bring patrol attention; injured-person Attack; alley-seller branches. |
| 4 | Council Island | None named in the enabled dialogue-event cells | This only says the selector table has no Lone Star-named path here; it does not rule out a separate pursuit system. |
| 5 | Renraku Arcology | `0x20F5A`, `0x20FA2`, `0x2112C` | Company Man, doctor, and alley-seller choices can reach Lone Star. |

The six named city contexts alone do not describe the full footprint. The
separate plate/laser chain at `0x21036` and `0x21050` reaches the Lone Star
patrol prompt at `0x24622` from selector cells across scenes `20-43`: the plate
event is enabled on `20-34` and `41-43`, and the laser event is enabled on
`35-40` (it is also a continuation of the plate event). The direct patrol cell
`0x2106A` is enabled on `41-43`. These are conditional event chains with a
badge/dispatch/arrest decision, not a patrol actor automatically spawned on
map entry.

The known route crosswalk anchors several elevator-connected scene groups to
named locations. These names combine ROM-backed area routes with the guide's
map markers; the elevator child lists then connect the sibling scene IDs:

| ROM scenes | Named route anchor | Evidence boundary |
|---|---|---|
| `20` | Hollywood Correctional Facility | Redmond records 89/90 conditionally select scene 20 at marker 5. |
| `21-24` | Fuchi Industrial Electronics | Penumbra record 97 selects scene 21 at marker 2; the ROM elevator list connects 21-24. No floor numbering is established. |
| `26-28` | Unassigned resource-activity group | Redmond routes can select scene 26, but the ROM and map guide do not tie this numeric group to one named interior. Scene 29 is a separate event-29 selector trigger, not a child-list member. |
| `30-33` | Mitsuhama Computer Technologies | Penumbra record 95 selects scene 30 at marker 3; the ROM elevator list connects 30-33. No floor numbering is established. |
| `35-39` | Renraku site | Renraku Arcology record 46 selects scene 35 at marker 2; the guide calls the site Renraku Computer Systems and ROM contract text says Renraku Offices. No floor numbering is established. |
| `41-42` | Lone Star Security | Penumbra record 96 selects scene 41 at marker 5; the ROM elevator list connects 41-42. No floor numbering is established. |

Scenes `25`, `29`, `34`, `40`, and `43` have event-29 elevator selectors but
are not members of the child lists above. Their own place names are not
established. Redmond's scene-2 routes also send several distinct markers to
numeric scene `26`; that is not evidence of one named office interior or a
reason to label Redmond as having an office building. Scene-specific names
are now available in `gameplay/world/areas/map-guide-labels.tsv`; keep every
unanchored route numeric. The traced footprint therefore includes Lone Star
events on the Penumbra route and the named multi-floor group, alongside the
conditional scene-20-to-43 security chains. It is broader than the six outdoor
city maps, without proving a roaming patrol spawn on every map.

Across all 56 field contexts, the extracted zero-cell dynamic actor table
separately contains `lone11` and `lonemage` at scene 1/sum 2, and `lonemage`
plus `lone13` at scenes 41-43/sum 8. Its other dynamic actor lists do not
contain a `lone*` template. This describes only the zero-cell actor lists;
choice-driven encounter lists and the plate/laser continuation above are
separate routes and must not be omitted from a map event summary.

### Coordinate-triggered map-effect response

The ROM also has a separate region-triggered world-effect path. This is not the
random dialogue selector above. `0x03074` copies 0x60 bytes from the selected
scene profile; for scenes `21-43`, the dword at profile offset `+0x58` points to
an effect profile. That profile points to a rectangle list at `+0x0C` and an
actor list at `+0x10`. The normalized source rows are in
[`field-world-effect-profiles.tsv`](field-world-effect-profiles.tsv),
[`field-world-effect-regions.tsv`](field-world-effect-regions.tsv), and
[`field-world-effect-actors.tsv`](field-world-effect-actors.tsv), generated by
`decompilation/ghidra_conversion/tools/rom_research/profile_field_world_effect_regions.py`.

`0x1DD94` calls `0x1DFC4` during the active field-world pass. The coordinate
check runs when `FFDF88 & 0x3F == 0x0D`; the PRNG value must be at most `0x7FFF`
for the rectangle list to be checked. The player coordinates are read from
12.4 world-position fields and shifted right four bits. A rectangle matches
when `x_min < player_x <= x_max` and `y_min < player_y <= y_max`. On scenes
other than `20`, mode `2` sets the `0x960`-tick timer; if the effect latch is
clear, the routine latches the transition, requests its sound, and passes the
profile actor list to `0x1E0DC`. Scene 20 uses its special timeout path and has
no profile actor list in this table.

Every template in the scene `21-43` effect actor lists is the ROM template
`0x1D6A1C` (corp guard). The high bit on a list template marks a group-start
record; subsequent untagged records are linked to that actor by the spawn
routine. These are conditional security-response actors, not permanent map
residents and not Lone Star, Hell Hounds, or Renraku Strike Teams. Lists in
scenes `39`, `40`, and `43` have no coordinate rectangles, although their
profile actor lists can still be used by an explicit alarm transition.

That explicit route is `0x1E04A`: security-event handlers call it to start the
same scene profile actor list without requiring the player to enter one of the
coordinate rectangles. `0x1E0DC` initializes the listed actors and effect
slots; it does not itself start combat. This explains map-specific corporate
guard responses but does not establish a global wanted-state patrol system.
The time-driven `0x1E166` screen-edge sequence selector is a third, distinct
path; its creature and actor candidates are summarized below.

### Selector boundary for global wanted-state pursuit

The ROM field tick at `0x1E166` applies its actor/effect guards, checks the
`FFDF88 & 0x3FF == 10` cadence and source random threshold, then calls
`0x59986`. That selector draws twice with bound 5, adds the results, maps the
current scene byte `FFFBF8` through `0x3312`, and indexes the enable/pointer
tables at `0x1D70E4` and `0x1D7174`. The selector itself does not read the
`FFFBFA` counter or the `FFFC22` state word. Its scene tables do enable the
conditional patrol paths described above, including the broad `20-43`
plate/laser range. The distinction is that the event cells make those
encounters possible; they do not implement a single global wanted flag that
automatically inserts the same patrol into every map.

The separate heat byte `FFFBFA` is encounter state, not the selector's
map-wide spawn gate. The indexed report at
`archive/retired_analysis/analysis/direct_references.tsv` lists 29 references, but a raw
US ROM operand sweep found 38 executable direct-address sites. The nine missed
sites lie in two timeout callbacks and a status/file-state branch that were
not reached by the analyzer's seeded control-flow walk:

| ROM instructions | Recovered path | Heat effect | What this does and does not establish |
|---|---|---|---|
| `0x53FFC`, `0x54002`, `0x5400E` | Actor-list outcome callback installed by `process_pending_entity_template_lists` at `0x43B6` when `FFE120 == 0x1E628`; dispatched after the four `FFE177`-counted actor behavior transitions | Add 2, clamp to 8 | The outcome changes heat, Karma, and story flags; it does not insert a field patrol. `0x1DBAA` separately arms `FFDF86` (`0x78`, or `0x258` when `FFE162` is nonzero). |
| `0x54286`, `0x5428C`, `0x54298` | Actor-list outcome callback installed by the same dispatcher when `FFE120 == 0x1E5C4`; dispatched after its four `FFE177`-counted actor behavior transitions | Add 2, clamp to 8 | A second actor-list outcome changes heat and progress state; it does not insert a field patrol. `0x1DBAA` separately arms `FFDF86` (`0x78`, or `0x258` when `FFE162` is nonzero). |
| `0x55238`, `0x55242`, `0x5524E` | Status/file-state branch for `FFFBBC == 0x17` and `D0 == 0x0D` | Read heat tiers | With nonzero heat, set exactly one of `FFFC38` bits 6, 7, or 8 for heat `1-2`, `3-7`, or `8`; clear the other two and continue to the notebook-file result path. This updates available Notebook notices, not map actors. |

The other source writes include the shared patrol Attack handler `0x23412`
(+2, capped at 8, only when the patrol list starts), the Run handler `0x23782`
(+1, capped at 8), the scene-41-to-43 Attack handler `0x23A3A` (+2, capped at
8 when its list starts), and combat action code near `0x55AFA` for actor
records whose `+0x9B` byte is `0x13`. Compliance and follow-up handlers can
decrement heat; payment/service handlers can clear it. Arrest settlement at
`0x235AA` copies then clears the heat byte, removes source item codes
`8`, `9`, `10`, `12`, `13`, `20`, `49`, and `50` from the three runner
records, computes a first fine of 1,000 nuyen
times `(heat + 2)`, repeatedly halves that amount until it fits the available
funds, and deducts it. The selector at `0x59986` and field tick at `0x1E166`
do not directly read the heat byte.

The scene-41-to-43 patrol handler separately reads `FFFC22` bit 10 before its
ID-check choices. Its failure text says the officer did not recognize the
player's ID badge. ROM references also show bit 11 being set by a separate
permit/service path. These credential checks change the patrol encounter's
outcome; neither the field selector nor these handlers use them as a global
map-wide patrol-spawn gate. The heat-dependent status/file-state branch above
does write Notebook notice eligibility bits, which the Notebook renderer
reads; that connection is now traced and remains separate from field actor
selection. It does not establish that a global wanted value causes roaming
patrols. The ROM still contains the conditional map-event patrol routes
established above, while any separate map-wide pursuit path remains
unresolved.

The third 32-entry Tips pointer table at `0xC2816` maps global entries 65-96;
entries 70-73 point to `0xC4390`, `0xC4465`, `0xC451E`, and `0xC45E1`.
Entry 70 is the wanted-for-questioning notice (including `0xC442C`); entry 71
is addressed to detectives and says the suspect is armed and dangerous; entry
72 is addressed to hunting parties and calls the player a fugitive; entry 73
is a Lone Star Records prisoner-transfer notice about Vigore. The Notebook
renderer at `0xFA40` tests each table-local entry bit in `FFFC38` and includes
only marked rows. A raw ROM sweep found 26 absolute `FFFC38` operands, while
the direct-reference index lists only eight; the additional instructions
include this Notebook file-state path. The branch beginning at `0x551C0` sets
bit 5 at `0x551FC` for the `FFFBBC == 0x14`, `D0 == 0x0A` path when `FFFC1E`
bit 3 is clear. The `FFFBBC == 0x17`, `D0 == 0x0D` path marks bit 6 for heat
1-2, bit 7 for heat 3-7, and bit 8 for heat 8. These bits control which
notices are available in the Notebook. The route writes the updated bitfield
back at `0x55578` and uses text at `0x166968` ("an interesting file that you
download to your notebook"). The field selector does not consume these bits
as patrol-spawn state.
ROM contact text at `0xC63CF` and `0xC651C` offers to delete the Lone Star
record, while `0x10E3C1` and `0x10F009` describe a Weapon Permit and patrols
flagging the player down. Those strings establish the service mechanics, not
a global map-wide patrol spawn. The scene-name crosswalk is partial: several
corporate groups are anchored by route and elevator evidence, while scenes
25-29, 34, 40, and 43 still lack independent names for every context.

A separate nearby-effect path runs through `0xAB02 -> 0x16B98`. It can place
one of ten `FF164C` records near the transitioning actor with a weighted
interaction code; `0x1DF34` later checks a strict `+/-0x100` radius and sends
that code to the reward handler `0x133A6`. The `0x5A` and `0x5B` weighted
sentinels select an equipped-item result or clear the record. This is a
proximity-effect/reward route, not evidence of a general Lone Star patrol
spawn. The ordinary weighted interaction codes still need their player-facing
effects mapped, and the separate global wanted-state patrol route remains unresolved.
Keep this path separate from the random dialogue and dynamic actor tables
below.

### Broad dynamic creature coverage

The zero-cell dynamic sequence table shows creatures across many field contexts. Hell Hounds are eligible on 26 scene IDs: Downtown Seattle (0), Wilderness Areas 1-4 (6-9), Hollywood Correctional Facility (20), contexts 26-29, and contexts 35-50. Across the 33 scene/sum outcomes containing Hell Hounds, the selected sequence lists 1-4 Hound actors. This generic dynamic sequence is separate from the wilderness disturbance's low-stage pack of exactly three Hounds. Wendigos and Bandersnatches cover 1, 4, 6-9, and 44-50; Gargoyles cover 4, 6-9, and 44-55. These are temporary screen-edge event sequences with terrain-tested placement, not permanent map residents.

### Later-game Strike Team coverage

The disturbance handlers at `0x20EFA`, `0x20F42`, and `0x2115C` make the same FFFBFB progression counter available across scenes 0-3, 5-9, and 44-50 (16 scene IDs). Within the ROM field-area/exploration set 0-9 and 44-50 (17 scene IDs), this route is enabled on 16; Council Island (scene 4) is the exception. Scenes 44-50 are selectable field-map contexts, not merely scripted activities. Scene 47 is cross-referenced as Cave 4 because its separate mode-`0x13` lists contain Renraku Strike Teams; scenes 44-46 and 48-50 still need their own map-name crosswalks. With FFFC22 bit 8 clear, stages 2-3 select two Strike Team actors, stages 4-6 select four, and stages 7-8 select five. This later-stage branch is broader than the explicit Renraku-building prompts in scenes 35-40.

The story choice at `0x21BE4` advances this shared counter and queues two Strike Team actors. The low-stage disturbance routes differ: the urban continuation returns explicit Renraku attack text, while the wilderness continuation can choose the creature packs listed above. The conditional event patrols above are independent of the still-untraced global wanted-state patrol logic.

The typed dialogue action applies the capped counter increment when Boris's Brother? choice selects command `0x2C`. Raw ROM code shows that `0x21BE4` passes `D0=1` to `0x1E1FC`: `0x1E216` then calls `0x1E40C`, which builds ten possible positions from two camera-relative map anchors and keeps only candidates whose `0x1E41E` terrain lookup is zero. This is the map-derived path; `0x1E302` is the separate screen-edge path selected by D0=0. In `0x1E40C`, the first anchor is camera origin plus `0x0A00` X and `0x0700` Y, offset toward the relevant map half by `0x0500`, then aligned to the `0x0200` grid with a `0x0100` center offset. It tests five positions along Y and five along X at offsets `0`, `+0x0200`, `+0x0400`, `-0x0200`, and `-0x0400`. The typed host models both placement policies. The extracted named list `0x1D6FD0` contains template pointer `0x001D6AFA` twice, then a null sentinel at `0x1D6FD8`; `0x1D6AFA` matches the extracted Strike Team profile in pending list `0x1E628`. Lua resolves these two rows outside the random selector, and the field host uses the map-derived candidates when the story command queues that list. Command `0x2C` returns prompt group `0xF546C`: text `0xF95E2` presents the Strike Team message, followed by the shared MORE caption at `0xFE5BF`. The graph now presents that page and continues in place. Combat behavior and the post-combat return remain incomplete; the later event-list placement coordinates are not substitutes for the generated map positions.

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
| You notice an open security panel. | `21-25`; `26-29`; `30-34` | `1, 7`; `2`; `0` (paired by scene range) | `0x20FBA` |
| A scientist walks by, deep in thought. | `21-25`, `30-34` | `4`, `6` | `0x20FD4` |
| A security guard spots you. | `20`, `26-34`, `41-43` | `4-5`, `7` | `0x20FEC` |
| A shiver runs down your spine. | `20`; `26-29`; `35-40`; `41-43` | `0`; `0`; `7`; `1` (paired by scene range) | `0x21004` |
| An old custodian wiping the floor squints up at you. | `20` | `1` | `0x2101E` |
| You look down and notice you are about to step on a concealed security plate. | `20-34`, `41-43` | `2`, `5-6` | `0x21036` |
| With no warning, a low-level laser detection field activates. | `35-40` | `8` | `0x21050` |
| A Lone Star patrol spots you. | `41-43` | `5` | `0x2106A` |
| Someone taps you on the shoulder. | `21-29`, `41-43` | `2-3`, `8` | `0x21082` |
| You find a terminal logged into the management database. | `21-40` | `0`, `3-5` | `0x2109A` |
| A Company Man walks up and flicks a toothpick at you. | `21-40` | `1-2`, `5`, `8` | `0x210B4` |
| You stumble upon a Renraku Strike Team. | `35-40` | `1` | `0x210CC` |
| Renraku Security spots you as they round a corner. | `35-40` | `4` | `0x210E4` |
| A security mage spots you. | `20`; `21-25`; `26-29`; `30-34`; `35-40`; `41-43` | `2`; `6`; `7`; `8`; `3`; `3, 7` (paired by scene range) | `0x210FC` |
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

`decompilation/ghidra_conversion/tools/rom_research/extract_world_activity_outcomes.py` creates
`assets/field/world-activity-outcomes.tsv` from the US ROM. The
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
python decompilation/ghidra_conversion/tools/rom_research/extract_world_activity_outcomes.py
python decompilation/ghidra_conversion/tools/rom_research/extract_world_activity_outcomes.py --check
```

## Native runtime status (partial)

The Qt field loop supplies typed state to Lua's `field_encounters` module,
which owns the ROM cadence/eligibility and probability decisions. Qt advances
the source PRNG only after Lua reports that blockers and the `0x3FF` cadence
allow a probability draw. Lua then decides whether to miss, forward an active
sequence, or select a scene outcome; the host supplies the two bounded draws
only for the selector branch. Zero-cell results spawn the listed actors
through the source edge-placement and entity/effect slot rules, then call Lua
for behavior and render heartbeats. Dynamic actor entry art is extracted by
entity kind and direction; entry delay and finite C0B2 frame count are passed
as typed values. Lua links targets using the ROM candidate pools and source
PRNG. The B-button target selector is connected to live actor records; its
`+0x11.4` selected latch remains separate from the `+0x10.4` action-candidate
reservation. Enabled dialogue handlers remain a dispatch boundary.

The creature entry branch also returns the `0xB05E` cue selected by entity
kind. For the encounter kinds, those requests select table-2 slots 46, 47, 86,
88, and 90. The audio manifest omits these verified silent/no-op slots, so the
source request is preserved without inventing an audible effect or duration.

This does not complete random-event gameplay. Enabled dialogue handlers,
creature item/spell actions, attack effects, hit/damage/death, removal and
queued activity continuations remain open. Ordered template overrides are
retained; only the `+0x11` render/selection byte is currently projected into
typed runtime state. The Release Qt executable builds, but this slice has no
new scene dump or QA run and remains partial.
