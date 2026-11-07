# Shadowrun local Luna: execution rules

## Scope and startup
- Work on the Genesis Shadowrun PC port in this checkout. Local Luna CLI only;
  never launch or resume Cursor cloud agents.
- Before writing, check branch/HEAD, dirty files, stashes, active local Luna
  processes and their worktrees. Coordinate ownership; never start a duplicate
  worker or overwrite another worker's edits. A dirty/detached checkout is not
  permission to reset, switch, stash, apply an old patch, or discard work.
- `port-fixes` is the sole integration branch. Never use `main`, `sr-refactor`
  or retired worktrees. Preserve existing WIP and stashes. Under the user's standing
  authorization, finished independently verified slices are committed and pushed
  to port-fixes promptly, without bundling unfinished work. A slice must pass its
  acceptance and neighboring tests and introduce no regressions. Unrelated failures
  reproduced on its exact parent do not block it; disclose them and full-suite status.
  Fetch/reconcile upstream before work
  and recheck immediately before a fast-forward-safe push; do not overwrite active
  workers or dirty work. Destructive changes still require explicit approval.
  Never force-push, rewrite pushed history, delete remote branches, or expose tokens.
- Follow the user's latest priority and delegated scope. Known current blockers
  come first. Alternate bug fixes and architecture cleanup one-for-one when that
  standing queue applies; do not substitute cleanup for an urgent user defect.

## One concrete item per work cycle
1. **Discover:** verify the highest-priority current failure from the user's queue,
   tests or playtesting. Start with the explicit assignment. Consult only relevant
   entries in `decompilation/docs/handoffs/playtest_2026-11-01.md` and
   `decompilation/docs/handoffs/grok_progress_notes.md`. Old notes/status counts
   are leads, not facts. If no known failure remains within scope, timebox a narrow
   ROM-versus-port gap analysis and choose one demonstrable missing behavior.
   Do not turn discovery into a documentation audit or invent new scope.
2. **Plan briefly:** state the symptom, reproduction, suspected ownership and
   acceptance checks in a few lines; then investigate and implement. A plan,
   inventory or rewritten handoff is not an implementation deliverable.
3. **Reproduce and establish evidence:** exercise the production lifecycle that
   exhibits the defect. Add/register a regression that fails for the right reason
   on the exact baseline and passes with the fix when applicable. Do not manually
   inject the very flag or result whose production wiring the test should detect.
   Verify ROM inputs, arithmetic, control flow, callers and side effects. Use
   controlled seeds for combat/formula checks, not full-route RNG/frame lockstep,
   seed padding, compensating draws or longplay-identical random rolls.
4. **Develop:** fix the existing generic mechanism with the smallest coherent
   change. Use Lua for gameplay and thin C++ host services. Preserve alternate
   callers and transition order; avoid duplicated frameworks, per-scene patches,
   hardcoded compensation and extra clamps unsupported by source evidence.
5. **Test iteratively:** run focused tests after meaningful changes, then relevant
   neighboring paths. Inspect actual runtime output; for visual changes view the
   port captures against independent ROM/emulator/longplay evidence where available.
   A renderer comparing its own assets is not independent proof. Never weaken
   assertions, tolerances or input loops, increase timeouts to hide failure, or
   change expectations without ROM/emulator evidence beside the change.
6. **Review and complete:** inspect the diff, test edits and affected callers.
   Provide reproducible parent-fails/fix-passes evidence where applicable. The
   executor independently checks Luna's claims and runs full integration QA; Luna's
   READY alone is not approval. Distinguish implemented, tested, integrated and
   pushed. Report commands actually run, results, exact commit/worktree state and
   remaining blockers; never claim completion from planned or stale test results.

Bound investigation: checkpoint by 45 minutes; narrow a dead end rather than
repeating probes or rewriting reports. At 90 minutes, stop that item, preserve
its exact WIP/evidence and state the blocker plus the next discriminating check.
Select another independent item only if already authorized and it cannot collide
with preserved work; otherwise stop. Every cycle ends in verified code/test
progress or an evidence-backed blocker, not a documentation-only substitute.

## Documentation budget
Code and verified results are the default deliverable. Documentation-only work
requires an explicit docs request or strictly necessary, narrowly scoped setup.
Use the existing progress notes above as the one authoritative task record;
update only on material state change (selected, verified, blocked or completed).
Keep the entry short: item, evidence/result, WIP location and next step. Keep raw
logs/captures as artifacts, not new handoff narratives. Do not repeatedly reread
or rewrite project history, duplicate status across handoffs/HILLS/coverage/audits,
create a report hierarchy, or disguise docs-only commits as implementation.
Update source comments or coverage metadata only when required by the actual fix.
Historical guides and handoffs are evidence/reference, not a second task queue
or permission to push. Do not require a missing handoff before doing scoped work.

## Architecture and source rules
- Active host: `decompilation/ghidra_conversion/`; authoritative gameplay/content:
  `decompilation/gameplay/scripts/` and `decompilation/gameplay/world/`.
  Lua owns AI, movement policy, animation cadence, combat, events, quests,
  dialogue and menus. C++ supplies typed facts, collision queries, rendering,
  audio, input and persistence; no parallel gameplay/menu state machine.
- Use `E.*` from `decompilation/gameplay/scripts/enums.lua`; do not hand-edit
  generated enum headers. Reuse shared dialogue, effects and animation mechanisms.
- US ROM: `decompilation/input/shadowrun_genesis_us.bin`; cross-check Ghidra and
  `decompilation/export/disassembly.s`. Tag behavioral findings `[C]` only with
  inspected code/data or viewed independent evidence; otherwise `[H]`. Cite
  addresses or recording/frame/pixel evidence. Never invent source facts.
- Recover behavior into native typed systems, not a 68000 interpreter, runtime
  ROM/bytecode decoder, VDP simulation or copied disassembly. Offline extraction
  may produce named host assets; do not migrate retired native-port code.
- Preserve `kDebugMenuAlwaysUnlockedForTesting = true`, `exit_bearing`/outward
  building exits, approximate ordinary fades, and variable-sized choice lists
  with source-compatible behavior. Keep runtime code portable to Windows/Qt.

## Build and QA safety
- All automated runtime/test/probe runs are headless:
  `QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy ALSOFT_DRIVERS=null`.
  Never open game windows, play audio or capture the user's screen.
- On this box source `/workspace/tcenv.sh`; existing builds are `/workspace/b2`
  (Qt) and `/workspace/b2n` (non-Qt). Configure `SRX_AUTO_EXTRACT_ASSETS=OFF`.
  Never extract into tracked `decompilation/assets/`, overwrite tracked assets,
  or hand-add generated assets. Do not run an extractor for an ordinary fix.
- Register tests with CMake/CTest or the existing Lua runner. Run broader QA,
  normally `/workspace/qa.sh` (both builds, CTest and Lua), to detect regressions;
  report its actual status. A focused pass is not a claim of full QA_GREEN.
- Land each completed slice after independent review, production acceptance and
  neighboring checks, with no new failures. Unrelated failures may remain only
  with current exact-parent reproduction and explicit disclosure; never waive a
  new failure, weaken expectations, or call an incomplete branch complete.
  Do not silently exclude tests, including socket failures. Reconcile fresh remote
  port-fixes without discarding active WIP; conflicts block landing that slice.
- For instruction-only edits, validate instruction consistency, referenced paths,
  actual diff and preservation of WIP; gameplay builds/full QA are unnecessary.
