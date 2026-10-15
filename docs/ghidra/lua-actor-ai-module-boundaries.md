# Lua actor AI module boundaries

`gameplay/scripts/actor_ai.lua` is the stable module loaded by
`GameScriptRuntime::actor_heartbeat`. It returns `require("actor.init")`, so
the host still calls `ActorAI.heartbeat(context)` and Lua remains the only
owner of per-actor behavior and presentation state.

`actor/init.lua` loads the modules in dependency order and returns the heartbeat
API. Their responsibilities are:

| Module | Responsibility |
| --- | --- |
| `actor/tables.lua` | Actor behavior constants and policy profiles; validates and links each profile to `data.actor_profiles`. |
| `actor/targeting.lua` | Profile validation, target eligibility and selection, source heading rules, and initial actor state. |
| `actor/anim.lua` | Entry, hit, walk, action, projectile, and player animation state and cadence. |
| `actor/actions.lua` | Action delays, target markers, item/spell dispatch, and pending effect state. |
| `actor/movement.lua` | Lua movement decisions, projectile progression, collision-result handling, and position updates. |
| `actor/heartbeat.lua` | `ActorAI.heartbeat(context)` orchestration and access to `game.actors[actor_id]`. |
| `actor/registry.lua` | Shared names for cross-module functions and state, resolved through wrappers at call time. |

Lua selects movement, animation, targeting, and action state. The C++ host
continues to provide typed collision and audio services, then applies the
returned state to the field model. The split does not change actor-service or
render-pass scheduling. Those ROM paths and their limits are recorded in the
[actor heartbeat source audit](actor-ai-heartbeat-source-audit.md).

The split was compared with the catalog-linked monolithic script before the
entry point was switched: 240,000 heartbeat calls and 17,100 per-function calls
produced zero state, return-value, context, or service-log mismatches. Eight
deliberate behavior mutations were all detected. The separate Qt
`shadowrun_gameplay_script_runtime_qa` exercises the public entry point and
continues to cover the source-backed actor action and animation paths.
