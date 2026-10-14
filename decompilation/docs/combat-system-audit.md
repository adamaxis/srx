# Combat system audit

Date: 2026-10-14  
Scope: active `decompilation/ghidra_conversion/` project, especially the Qt field runtime and Lua gameplay scripts.

## Assessment

Combat is not absent. The active project has a connected firearm slice: a player can fire, a projectile can travel through the field collision service, a ROM-profiled Scene 2 encounter can be hit, the target meter can change, and the creature can enter its hurt animation. Target selection and several creature actions also have live paths.

It is still a partial conversion, not a complete combat system. The most serious gap is that the player magic page can select and equip a spell, but field combat input only enables firearm actions. The player spell therefore never reaches the Lua spell resolver. Melee/unarmed and consumable combat actions are also not connected through the player field-action route. Even the firearm slice has unresolved source timing tables, a source-order difference in when its hit roll consumes random values, incomplete target/status and post-hit behavior, and an ownership split in which C++ still owns important player combat timing and pose state.

The facts below distinguish code paths that exist from behavior whose Genesis fidelity is still unproven. Existing CTest and QA source was inspected but not executed for this audit. No combat code was changed.

## ROM combat lifecycle

The source path is a set of linked services, not one damage function:

| Stage | ROM path | Recovered responsibility |
|---|---|---|
| Actor service and animation | `0xA8EA`, `0x4618`, `0x5FDC`, `0x600E` | Behavior/action service and sprite animation run in distinct passes over actor records. The animation cadence comes from record counters and the selected finite or looping list. |
| Player input and target | `0xACAE`, `0x1DC4C`, `0x1438C`, `0x14446`, `0x14492` | A held-A action is gated by the active record and its action delay. B selection scans a mode-dependent target pool, retains a still-valid target, or draws a replacement. |
| Firearm action | `0x1514E`, `0x38AC`, `0x14FF8`, `0x15278`, `0x152FA` | The source consumes ammunition before allocating the projectile records. It initializes a moving primary and a stationary finite visual record. Codes 8–10 use the three-round, three-ray branch; other firearm codes use one round for one action. |
| Projectile movement and collision | `0x14E0A`, `0x14F50`, `0x15ABA`, `0x14F86` | The primary makes four ordered movement/recovery probes per behavior pass, scans active records for contact, and has separate terrain, retry, stopped, and burst-follow-up continuations. |
| Firearm hit resolution | `0x55640`, `0x55948`, `0x559BC`, then `0xAFF2` | The source calculates and stores an attack result, resolves the target roll and item effect, applies armor/defense and record-specific meter changes, then services the damaged actor's follow-up state. |
| Creature action | `0x13D0C`, `0x14C7C`, `0xB1DC`, `0x55C76` | Creature policy refreshes its target and route on its source cadence, accepts/rejects an attack by range and actor state, then runs a multi-stage action/reaction calculation. |
| Spell action | `0x14D6A`, `0x1033E`, `0x15830`, `0x55B78`/`0x559BC` | A negative equipped-slot value selects the spell table. Spell trials and delayed effect records use a separate path from firearm ammunition. Spells do not consume firearm ammo. |
| Non-firearm item action | `0x15646`, `0x36FC` | Nonnegative action codes 13–15 enter an effect-record and inventory-phase path; they do not use the firearm spawn/ammo routine. |

The US ROM is the primary source for these paths. The disassembly and exported Ghidra functions are cross-checks. The source-function tracker still marks the important projectile, spawn, collision, damage, and creature-action entries partial; see `docs/decompiled-function-status.tsv` rows 169, 172–173, 176–177, 180, 880, 887–888, and 890.

## What currently connects in the Qt game

### Player firearms

`FieldSceneRuntime` passes held A, action readiness, and a firearm-availability fact into the player Lua heartbeat. Lua returns `combat_action_requested`. The C++ `service_player_weapon_action()` then validates the record again, handles empty-magazine reload, calls the typed spawn-cycle helper, commits ammunition, allocates source slots 9–14, and starts the attack pose. The projectile's subsequent movement and hit policy are in `actor_ai.lua` and `combat.lua`; C++ supplies terrain/collision facts and applies the returned typed meter update.

The catalog has typed rows for the 12 firearms, including magazine capacity and the ROM-derived item-effect/difficulty fields. The source's three-ray grouping is by action codes 8–10, not by a unique animation for each gun. The extracted projectile lists are shared directional families selected by action class and heading; the sound event is selected by action code and item status. The ROM does not appear to supply a unique projectile clip for each firearm.

There is an integrated hit QA route beyond the player-action fixture: it creates the ROM-profiled Scene 2 Weenr encounter, lets actor 0 approach naturally, selects it with B, then fires a Warhawk. The QA checks a live projectile collision, world-meter change, C072 hurt animation, and its cue. It pins the PRNG at the shot edge and sets the runner's weapon/ammo/skill values for a repeatable case. That is good evidence that this specific vertical slice is connected; it does not establish ordinary unmodified play across weapons, actors, maps, or random states. The separate player-action fixture has no live actor records.

### Creature AI and reactions

Lua owns parts of the dynamic-creature target refresh, movement decisions, accepted close-range action, action animation, and next-heartbeat hurt/down reaction. The host exposes current map collision and actor facts. B target selection is also sent through Lua; a later firearm shot uses the selected actor's current position when the selection is still valid.

The coverage is selective. The target-pool scan and automatic target-service path are not fully moved or implemented, the full actor-record lifecycle is incomplete, and the encounter catalog does not exercise every actor action family. The live player-to-creature firearm hit route covers a source-spawned test encounter, while the general NPC item/spell/effect routes remain partial.

### Magic and item actions

The magic menu has typed spell definitions and selection/force UI. Finishing selection stores a negative equipped slot and the selected spell code on the runner, which is the right kind of state for the ROM's overlapping spell/item code spaces. The current field input gate, however, declares an action available only when the equipped action classifies as `weapon_ammo`. The Lua player heartbeat consequently cannot request a spell action. If a request somehow reaches `service_player_weapon_action()`, it explicitly returns for `magic_spell` before casting. The active Lua calls to `resolve_spell_action()` are in the dynamic encounter actor behavior path, not a player field-cast path.

There is partial NPC spell/effect infrastructure and named effect presentation for a subset of spell codes. It does not complete all 13 spell effects, and it does not make the player's selected spell castable. Non-firearm effect items 13–15 have source classification and some action policy, but the connected player route does not execute the ROM `0x15646`/`0x36FC` effect path. No player melee/unarmed action route is apparent in the current field gate either.

## Confirmed gaps and high-risk areas

### 1. Player spell selection ends before a cast

This is a functional disconnect, not a visual uncertainty. In `field_scene_runtime.cpp`, `combat_action_available` requires `FieldCombatActionKind::weapon_ammo` (`~8037–8055`). `service_player_weapon_action()` returns immediately for `magic_spell` (`~8356–8384`). The spell menu only records the selected spell/force and returns to the map. In Lua, `resolve_spell_action()` is reached from the dynamic actor spell branch (`actor_ai.lua` around 2672), with no corresponding player action dispatch. The code correctly avoids spending ammo on a spell, but currently does so by doing nothing.

### 2. Player combat policy and animation timing still live in C++

The current architecture requires Lua to own combat policy, movement/animation state, and cadence. The Qt host derives the firearm gate, advances `runner.animation_action_gate`, chooses/executes reload and ammo handling, and starts/clears the player attack pose. Lua only reports the held-A request and renders the supplied attack mode. This duplicates the combat state boundary and makes the runtime harder to extend consistently to spells, melee, and items.

The item rows also contain `attack_delay_scale` and `reload_delay_scale` labels such as `half`, `normal`, and `double`, consumed by the C++ delay service. The ROM audit explicitly says the `0xCF3D4` and `0xCF610` weapon/timing semantics still need typed extraction (`docs/rom-native-audit.md`, table “Data still to pull into typed native data”). Until those tables are traced and bound to the weapon templates, per-weapon attack/reload speeds must be treated as unverified. Current QA's cooldown measurements describe the current host implementation; they do not prove Genesis timing.

### 3. Projectile attack roll is consumed later than in the ROM

ROM `0x14FF8` resolves the attack/action pair while initializing the projectile record. The current host publishes the child record first and asks Lua to compute its attack seed on that projectile's first behavior heartbeat. The implementation comments and status row 880 explicitly describe this ordering. Because the resolver consumes the shared source random stream, this can shift later random results as well as delay the stored attack result. The follow-up rays do reroll through Lua, but the primary ordering still needs to be restored to source order.

### 4. Action-profile data is unresolved at the combatant boundary

`script_combatant_facts()` sets `linked_action_profile_index` to `0xFFFF` because the live `+0xD8` record link and linked `+0xA0` selector are not populated. `dynamic_status_modifier()` needs a valid profile index when its flags request the primary/secondary profile rolls; it then returns `nil` without one. This blocks those source modifiers instead of silently fabricating a profile. Status rows 880 and 887 call linked-profile validation an open issue.

### 5. The NPC-to-world-target meter commit appears to use the wrong identity

Lua's combat result carries both `source_record_slot` and `record_type` as separate values. The script bridge validates both separately. In the dynamic actor action commit in `field_scene_runtime.cpp` around lines 3097–3155, the party branch uses the selected candidate's slot, but the world-target branch searches for an actor whose `source_record_slot == update.record_type`. Those fields do not represent the same identity in the Lua result. That can reject or misapply an NPC action against another world actor when its slot differs from its record type. The end-to-end Warhawk QA uses the projectile update path, which has a separate correctly linked commit helper, so it does not cover this NPC-to-world-target branch.

### 6. Player damage has no matching reaction state

`combat.lua` queues `incoming_hit` only for targets with `record_type >= 3` (world actors). `begin_received_hit_reaction()` is serviced only in the dynamic-world-encounter branch of `actor_ai.lua`. Party records receive meter updates, but the active player heartbeat has no corresponding hurt/down transition. Thus the current creature-action path can change a player meter without a connected player damage pose/audio/recovery lifecycle. This is separate from the well-covered creature hurt reaction after a player projectile.

### 7. Hit follow-through and record lifecycle remain partial

The Lua hit resolver applies the item effect, a typed armor modifier, defense, and a meter update. The ROM's shared hit tail also has linked-record/status behavior, global counters, and post-hit `0xAFF2` state changes. The status table marks death/recovery, linked status profiles, complete counter branches, target recovery/removal, and auxiliary child allocation open. A meter reaching zero is not equivalent to a completed actor death/removal flow.

### 8. Input classes are not unified

The field's player action service accepts firearms and handles their ammo/reload path. Spells are gated out. Codes 13–15 take a different ROM effect-record route, which is not called by this player service. The architecture currently has no single Lua action dispatcher that handles firearm, spell, item effect, and unarmed actions with explicit source rules. This makes a working firearm action easy to mistake for a complete combat system.

### 9. Fidelity evidence is narrower than the code coverage

Existing QA checks many deterministic internal transitions and full-screen frames. The integrated random-encounter hit test is valuable, but it controls the encounter, runner stats, weapon, ammo, and random seed. The longplay provides some combat and target-marker references but does not identify the exact action codes or isolate matching firearm/spell outcomes. Audio asset durations and event requests are measured internally; independent Genesis audio timing is still open. No claim that every weapon, spell, hit reaction, or combat animation is visually/audio verified is supported by the current evidence.

## Evidence and coverage already present

- `gameplay/scripts/combat.lua`: shared dice, spell, creature-reaction, firearm attack-seed, and firearm-hit resolvers.
- `gameplay/scripts/actor_ai.lua`: player held-A request, target selection, projectile heartbeat, dynamic creature action, and world-actor damage reactions.
- `gameplay/scripts/items.lua`: item catalog and current weapon templates.
- `qt_runtime/src/field/field_scene_runtime.cpp`: player action gate and host spawn/ammo service, target/collision queries, typed meter commits, and effect rendering.
- `qt_runtime/tests/scripting/game_script_runtime_test.cpp`: deterministic resolver and actor-heartbeat cases, including projectile hit and damage-result cases.
- `qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp`, `run_field_random_encounter_spawn_qa()`: ROM-profiled encounter approach, B selection, one-shot Warhawk hit, hurt reaction, and separate three-ray burst route.
- `docs/decompiled-function-status.tsv`: source-function completion states. The combat-related entries remain partial.
- `docs/visual-audits/weapon-spawn-ammo-consumption.md`, `docs/visual-audits/field-random-encounter-creature-targeting.md`, and `docs/actor-ai-heartbeat-source-audit.md`: source traces and current evidence limits.

## Recommended repair order

1. **Finish the ROM combat input/action matrix.** Trace the player-side call paths for firearm, spell, item effect, and unarmed actions, including each gate, target rule, delay, resource use, effect-record allocation, and return state. Use that matrix as the contract for Lua dispatch.
2. **Extract the remaining weapon/timing tables.** Resolve `0xCF3D4` and `0xCF610` into typed weapon-template fields, then remove or correct any hand-authored cadence labels that do not match source data.
3. **Move player action state to Lua.** Lua should decide which action is requested, own its delay/cadence and animation state, and return a typed host request for allocation/audio. The host should validate and perform the narrow resource operation without keeping a second action timer/pose machine.
4. **Connect player spells and effects end to end.** Make selected spells cast through the player heartbeat and the source spell gates, without touching firearm ammo. Add the distinct item-effect path for codes 13–15. Keep each supported action tied to its actual ROM table row and effect profile.
5. **Restore source-order combat rolls and finish target linking.** Resolve the primary attack result during spawn initialization, carry the live linked-profile selector into Lua, and fix the NPC-to-world-target identity comparison.
6. **Complete the damage lifecycle for both sides.** Connect party hurt/down/disabled behavior and sound, world actor `0xAFF2` follow-through, meter-zero state, removal/recovery, and source global counters before expanding encounter volume.
7. **Expand controlled cases, then use independent references.** Cover every firearm class, player spells, item effects, NPC weapon/spell actions, party and world targets, zero-meter transitions, and simultaneous actors. Compare full-screen Genesis frames/audio where the longplay or a fresh capture provides a matching event; report internal QA and independent fidelity evidence separately.

Until these steps are complete, the accurate description is: **a partially connected firearm/creature combat slice with a real Scene 2 hit route, plus incomplete spell, item, player-damage, and actor-lifecycle paths.**
