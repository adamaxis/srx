# Shadowrun handoff

## Current scope

The active implementation target is **the repository root**, the typed C++/Qt project built from the Ghidra analysis. The older native-port source is archived under `archive/legacy_native_port/`; do not continue its implementation.

The requested feature area is still **partial**: creature AI, dynamic/random encounters, and B-button target selection need more ROM tracing and integration. The executable should not be described as a complete playable game.

## Work completed

Two focused changes were committed:

- **`bfa717b47b Wire posture-based world actor route bands`**
- **`5cd78e095a Verify all posture route bands for creature AI`**

The first change traced the ROM’s posture-dependent route-distance bands and connected the relevant value to the Lua actor behavior context. Previously, the Lua route logic expected a context field that the host never supplied. World actors now pass their posture into behavior and rendering contexts. The second change expanded the Lua test to exercise all nine posture values across the three ROM-derived bands.

The source trace used the table at ROM `0x13E58`; the relevant routine indexes it using the actor record’s `+0x4D` field. The table’s lower/upper pairs are:

- Postures 0–2: `0x0640..0x0780`
- Postures 3–5: `0x0460..0x05A0`
- Postures 6–8: `0x01E0..0x03C0`

The test uses a generic kind-8 Weenr profile and keeps the actor ID’s backing string alive for the context’s `string_view`. It exercises the route-enabled flag combination (`0x2D`) and all nine posture values.

### Important limitation

This does **not** establish that Scene 2’s production Weenrs use this route branch. Their source flags are `0x25`, which do not enable the `+0x10.3` branch. The added test proves the branch’s posture bands in isolation; it does not prove a Scene 2 behavior or visual fix.

The changes and supporting evidence are documented in [actor-ai-heartbeat-source-audit.md](../ghidra/actor-ai-heartbeat-source-audit.md), [field-random-encounter-creature-targeting.md](../ghidra/visual-audits/field-random-encounter-creature-targeting.md), [HILLS.md](../ghidra/HILLS.md), and [decompiled-function-status.tsv](../ghidra/decompiled-function-status.tsv).

Relevant implementation and test files are [the actor heartbeat](../../gameplay/scripts/actor/heartbeat.lua), [the actor module map](../ghidra/lua-actor-ai-module-boundaries.md), [field_scene_runtime.cpp](../../qt_runtime/src/field/field_scene_runtime.cpp), and [game_script_runtime_test.cpp](../../qt_runtime/tests/scripting/game_script_runtime_test.cpp).

## Verification performed

- The focused `shadowrun_gameplay_script_runtime_qa` test passed in **Debug** and **Release** after the posture-band coverage was added.
- In the preceding pass, the selected script-runtime and field-random-encounter spawn QA tests passed in both configurations.
- I inspected the internal field encounter captures: 19 BMPs per configuration, with matching Debug/Release hashes. The frames were reviewed at original resolution. The actors were often outside the viewport, so these captures are useful for checking internal rendering consistency and regressions, not for proving Genesis fidelity.
- Earlier Hell Hound close-effect captures were also reviewed. Those likewise remain internal evidence.

No feature work was left half-edited after the two commits; the subsequent exploration was read-only.

## What I was investigating when I stopped

I was looking at the source-derived dynamic actor catalog and how its profiles reach the Lua behavior path. The table [dynamic-world-activity-actors.tsv](../../assets/field/entities/dynamic-world-activity-actors.tsv) has **731 rows and 19 unique profile/action template combinations**. It includes 12 spell records across four spell templates:

- `lonemage_k0a_spell01` — scenes 1, 41, 42, 43
- `rat_shaman_k0b_spell05` — scenes 1, 5
- `elven_mage_k06_spell09` — scene 3
- `gator_shaman_k09_spell02` — scenes 3, 4

There are also seven unique item templates in the catalog. The existing source data and Lua path support generic profile resolution for dynamic actors; some creature-specific behavior branches already exist, including Hound, Gargoyle, and Wendigo paths.

I also corrected a stale status claim in the source documentation: the linked `0x55C76` combat-reaction path is not wholly disconnected. Lua calls `combat.resolve_reaction`, returns a typed meter update, and Qt applies it to the live party/world-actor record. That is still only **partial** support; it does not finish the actor action families or their complete lifecycle.

## Remaining work and suggested order

1. **Trace spell and item action dispatch in the ROM.** Start with the four spell templates above and the actor action fields around `+0x56/+0x57`. Determine the actual dispatch, cadence, target/effect behavior, and whether each path calls the common reaction routine or another effect path. Then implement the behavior in Lua through the existing typed host boundary. Keep spells out of the ammo path, as requested.
2. **Exercise the real actor profiles through behavior tests.** Use the source catalog to test the 19 profile/action combinations, particularly spell users. Tests should demonstrate behavior and decisions, not just parse metadata.
3. **Continue the actor service and lifecycle trace.** The full source actor-pool/service behavior, auxiliary child allocation, post-down handling, recovery/removal, and subsequent actions still need to be mapped. A downed record is currently left active and stationary, while B-button target cycling reserves it.
4. **Finish target selection from ROM evidence.** Trace B-button eligibility, cycling, and target-state transitions, then connect that behavior to the Lua-authoritative actor state and Qt input forwarding.
5. **Verify encounter behavior end to end.** Establish when and where actors spawn, how they approach, when they attack, and what ends the encounter. Prior service-pass traces did not establish arrival: actors remained outside the viewport, and there is no independent Genesis encounter trace in the current evidence.
6. **Review complete scene captures after meaningful changes.** Inspect the whole frame for compositing or unrelated regressions. Internal dumps are useful regression artifacts, but they do not independently prove that the behavior matches the Genesis game.

## Workspace state

At handoff, the latest two commits are the posture-route changes above. The worktree also contains these uncommitted paths, which were not part of those commits; inspect and preserve them before changing anything:

- Modified: `docs/ghidra/visual-audits/world-activity-security-terminal-floor.md`
- Modified: `gameplay/world/entities/ares_weapon_emporium/entity.tsv`
- Untracked: `tools/rom_research/longplay_contact_sheets.py`

Builds and CTest should use the active project’s presets at the repository root. Continue tracing the ROM first, then express the behavior as typed native state and Lua content; do not add an emulator-shaped runtime. Independent Genesis evidence is required before claiming that a visual or behavioral defect is resolved.
