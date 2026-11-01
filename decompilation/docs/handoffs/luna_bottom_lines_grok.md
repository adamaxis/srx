# Shadowrun (Genesis) PC port: handoff and standing instructions for Luna

You are taking over ongoing work on the PC port of the Sega Genesis game *Shadowrun*.
Read this whole file before touching anything. The "Bottom lines" section is not optional.

---

## 0. Bottom lines (non-negotiable)

1. **ROM is the source of truth.** Every behavioural claim, fix, or test expectation must cite ROM
   evidence (address range, table address, disassembly line) or emulator evidence (recording + frame
   number, pixel coordinates). Tag findings **[C]** confirmed (you read the code/data or saw the frame)
   or **[H]** hypothesis. Never invent addresses, frame numbers, values, or names. If you don't know,
   say so and list what would settle it.
2. **Never weaken a test to make it pass.** Do not loosen tolerances, widen ranges, drop assertions,
   shorten loops, add waits that hide behaviour, or switch exact comparisons to fuzzy ones. A test
   expectation may change ONLY with ROM/emulator evidence cited in a comment next to the change.
   (This has been the #1 failure mode in past runs.)
3. **Headless only.** Always run with `QT_QPA_PLATFORM=offscreen`, `SRX_HEADLESS=1` and dummy/null
   audio. Never open a window, never play audio, never grab the user's screen.
4. **One branch: `port-fixes`** on github.com/adamaxis/srx. Never push to `sr-refactor` or `main`.
   Never force-push, rebase pushed commits, or delete remote branches. Old branches (core-fixes-1,
   combat-fixes-1, menus-2) still exist remotely; leave them alone unless the user says otherwise.
5. **Push only after a full green QA run** (`/workspace/qa.sh` prints `QA_GREEN`). A red push happened
   once (ffe1e13b, fixed by 589a5257); don't repeat it. Focused test runs are not enough to push.
6. **Never print, echo, or log the GitHub token.** It lives in the environment variable
   `SRX_GH_TOKEN`. Push only through a credential helper:
   ```
   H='!f(){ echo username=x-access-token; echo "password=$SRX_GH_TOKEN"; }; f'
   git -c credential.helper= -c credential.helper="$H" push https://github.com/adamaxis/srx port-fixes:refs/heads/port-fixes
   ```
   Verify with `git ls-remote origin port-fixes`.
7. **Don't overwrite git-tracked assets.** Never run the asset extractor straight into
   `decompilation/assets`. If that happens: `git checkout -- decompilation/assets`.
8. **Don't revert intentional deviations** (section 3).
9. **Small, verifiable steps.** One backlog item per commit when possible. Keep a running log at
   `/workspace/shadowrun/out/progress_notes.md` (timestamp, item, state, next step) so an
   interrupted run can pick up from there. Stash unfinished work with a descriptive name rather than
   leaving it dirty or pushing it.
10. **Report honestly.** Say what was tested and what wasn't (e.g. "passes headless QA, not
    playtested, needs Windows retest"). "Matches ROM" requires addresses.

---

## 1. Project layout

- Repo: `adamaxis/srx`. Integration worktree: **`/workspace/srx_int`** (branch `port-fixes`).
  Remote HEAD at handoff: **`fe9f16db`** (249/249 ctest green).
- Upstream `sr-refactor` is at 35c9d022; the user integrates from `port-fixes` themselves.
- Old worktrees `/workspace/srx_core_wt`, `srx_combat_wt` and `srx_menus_wt` are retired; don't use them.
- Architecture:
  - **Lua holds gameplay and engine core logic** (`decompilation/gameplay/scripts/*.lua`, tests in
    `decompilation/gameplay/tests_lua/`).
  - **C++ is only the skeleton** (storage, RNG, collision, rendering, bindings) under
    `decompilation/ghidra_conversion/`.
  - New logic goes in Lua unless it's truly skeleton work.
- Assets: ROM assets are extracted offline into resource files, with palettes pre-baked as images and
  audio pre-rendered.
  - The CMake option `SRX_AUTO_EXTRACT_ASSETS` (on by default) runs the extractor at build time into
    the build directory. `cmake/srx_extract_assets.cmake` copies only untracked files in.
  - ROM default path: `SHADOWRUN_US_ROM` = `../input/shadowrun_genesis_us.bin`.
- Docs: `docs/ghidra/` (e.g. `visual-audits/field-enemy-stacking-trace.md`,
  `field-barrier-presentation.md`, `visual-audits/player-hit-spark.md`).

## 2. Build and QA

- Full QA: **`/workspace/qa.sh`**.
  - It is fail-fast, takes about 12–14 minutes, and prints `QA_GREEN` only on full success.
  - It covers the Qt and non-Qt builds, ctest (249 tests) and the Lua suites.
  - `matrix_traversal_entry_qa` is skipped by design, because its reference frames aren't in git.
- The user builds on **Windows (MinGW/MSYS2 + Qt)**:
  ```
  cmake -S decompilation/ghidra_conversion -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSHADOWRUN_GHIDRA_QT_FRONTEND=ON
  cmake --build build
  ```
  This environment is Linux and can't test Windows. Keep code portable (no POSIX-only calls in
  runtime code, mind path separators, case-sensitive includes). For Windows-only breakage, ask the
  user for their error output.
- New tests must be wired into CMake/ctest, or they silently don't run. This happened once with
  `entity_projection_conversion_test`.
- Optional VDP mirror: `SRX_VDP_MODEL=1` (opt-in). R6/R7 latch the scroll and sprite table one frame
  behind. Not the default; don't make it default without evidence.

## 3. Intentional choices: do NOT "fix" these

- **The player faces outward (away from the door) when leaving a building.** This is a deliberate
  deviation from the ROM. It's implemented for all 59 exits.
- `kDebugMenuAlwaysUnlockedForTesting = true` stays as it is.
- **Ordinary fades** only need to be roughly right (similar length and behaviour). Frame-exact
  accuracy is required only for real animated sequences (cutscenes, the Matrix unplug, etc.).
- **Dialogue and menus:** replicate the original 3-choice behaviour exactly, but without hard-coding
  3. The user wants more than 3 choices later.

## 4. Code conventions

- Use enums, not raw hex: `E.ActorFlag.PEDESTRIAN`, `E.*` in `scripts/enums.lua`. A ROM address
  can go in a comment; magic numbers don't belong in logic.
- Prefer abstraction and data-driven code over per-scene or per-actor special cases. Read the ROM
  table; don't hard-code a scene number or profile name. (A C++ check on the profile name
  `pedestrian_k0b_none00` was just removed for this reason.)
- Combine related ROM tables into one class or module (e.g. weapon stats together with clip sizes).
- Decode item and stat values from the ROM, not from `items.lua` or other assumptions.
- Pseudocode and specs use the same split: Lua for logic, C++ for the skeleton.

## 5. Known failure modes from past runs (watch for these)

1. **Weakening QA to get green.** Past runs loosened the world-activity camera comparison, weakened a
   table-collision QA, and shortened an input loop from N to 16 ticks while adding countdown waits.
   All were rejected. If a test fails, find out why, using the ROM.
2. **Dropping an existing special path while adding a new one.** The #9/#10 change first deleted
   the spawned-encounter enemy path, and runtime checks still rejected the new hurt/down states.
   Before finishing, search for every caller and every state check affected.
3. **Wrong rounding.** The ROM truncates (`asr`/`lsr`, `andi.w #-0x80`, etc.), so don't round. A
   rounded heading caused the backwards walk animation (#13/#14). Copy the exact arithmetic.
4. **Applying a fix too broadly.** A give-screen "hold" patch (0028) based on one recording applied
   everywhere. Scope fixes to the exact ROM condition (it was the weapon/non-weapon picture change,
   ROM 0x12A54).
5. **Applying state at the wrong point in a transition.** The #5 entry placement first ran at the
   wrong step and got overwritten by the default spawn. Trace the ROM order (selection at 0x169E6
   via FFE114, applied at 0x2B78) and match it.
6. **Mistaking a side effect for a bug.** The world-activity camera "drift" was really the ROM's
   scene redraw rounding the camera down to 0x80 (0x2BE2/0x2BEA). Check for an existing ROM mechanism
   before adding a "hold" or "lock".
7. **Calling something ROM behaviour too early.** The user disputed several items and was right
   about #3 (the Gunderson exit). When a playtest report conflicts with your reading, look for more
   ROM paths (dynamic spawns, story-flag branches, alternate tables) and check against the longplay
   before closing the item.
   - Open now: the user says **pedestrians walk on almost every map**, but the default lists only
     have them in 7 scenes. There is almost certainly another spawn path.
8. **Extra clamps and "safety" code that change behaviour.** An extra clamp (241db169) made dead
   bodies slide at the map edge. Don't add clamps the ROM doesn't have.
9. **Off-palette colours.** Genesis output uses only palette colours. Blending font edges or the
   sidebar with the background (Cyberware rows) produced impossible colours. Check that pixels are
   on the palette.
10. **Mistaking capture-setup differences for port bugs.** Emulator recordings may have a different
    party or portraits than the port's test setup (sidebar slots 2–3, the give-screen recipient
    portrait or name x). Fix the setup or note it; don't change the port to match a bad setup.
11. **Tests not wired into ctest** (section 2), and **claiming green from focused runs**: only
    `QA_GREEN` from `/workspace/qa.sh` counts.
12. **Going past the time box.** Full QA takes about 13 minutes. Plan around it, and stash rather
    than push half-done work.

## 6. Evidence sources

- ROM: `../input/shadowrun_genesis_us.bin`; disassembly `disassembly.s` (addresses are cited as
  `0xNNNN` or `disassembly.s:line`).
- Longplay reference frames (e.g. `min_002/f_0540`, `min_044` for the Gunderson exit) and emulator
  recordings under `au/` (e.g. `au/e8_karma_spend`).
- Menu capture comparisons have used frame offset +193 and LAT=-1. Earlier outputs were in `/tmp/mc`,
  which may be gone.
- Prefer scripted screening (pixel diffs, contact sheets, writing large output to files) over
  inspecting frames one by one.

## 7. Status of the playtest backlog

The full list with a status column is in `/workspace/shadowrun/out/playtest_2026-10-25.md`; keep it
updated. Summary at handoff:

**Fixed and pushed:**
- #3: Gunderson and Wanderer exits return to the source scene.
- #5: office entry placement.
- #9/#10: placed actors are combat targets; edge culling matches ROM 0x5A62.
- #13/#14: walk direction truncation.
- #18: music restarts on a same-scene return.
- #19: pedestrians, data-driven (partial, see open items).
- #24: safes, ROM 0x540D6/0x541B2.
- #26: dialogue bottom border.
- #32: Sega shock pixel test (it already matched the ROM).
- #35: audio queue (needs a Windows listen test).
- Give screen: hold scope, "????" line, black posture-plate fill, cursor hidden for one frame.
- Cyberware: palette-correct bottom rows.
- Alarm cleared on scene exit and death; hospital door.

**Judged ROM-correct (with addresses in the backlog):**
- #11: damage, ROM 0x55948.
- #12: enemy speed tables.
- #15: muzzle flash tables. The look wasn't confirmed against a frame.
- #16: hit flash.
- #20: dialogue waits for a button.
- #22: Rat's Nest text is story-gated.
- #29: HUD shows loaded rounds.
- #30: Def ignores armor; armor is applied in combat at 0x55E92.
- #33: rotor.
- #34: landing.

**Waiting on the user's repro or screenshot:**
- #21: Ricky.
- #23: which door.
- #25: half-drawn tiles.
- #27: picker or call screen.
- #28: Magic Defense.
- #31: posture outline.
- #7: buildings can't be entered.
- #17: retest on a fresh Windows build.

## 8. Open work, in priority order

1. **Pedestrians on almost every map** (the user's correction).
   - Current state: `/workspace/shadowrun/out/ped_scenes.md`. Default lists have pedestrians only in
     scenes 0x00–0x05 and 0x14.
   - Find the other spawn mechanisms: periodic or random street spawners, scene-entry scripts, and
     the single-walker branch at 0xA800 for scenes 0x15–0x2B. That branch's gates are FFFBED==5,
     FFDF40==2, FFE12C==FFF0C8 and FFFC3C==0; what FFFBED means is [H].
   - Check against longplay frames from several maps.
   - Implement it data-driven in Lua and extend `field_pedestrian_scenes_qa`.
   - Also give corp guards a gameplay check: default-list actors with route data can now
     route-walk.
2. **Hotel door not entered in the Karma replay.**
   - The emulator fades into the hotel around frame 8690 of `au/e8_karma_spend`; the port's runner
     reaches the door but never enters.
   - This may be the same bug as the user's #7. Trace the door trigger and the input timing.
3. **Karma screen compare** once (2) works, or seed a state at the hotel desk.
4. **Deck screens** compared against the longplay.
5. **Blink and sound timing audit** for the menus: blink rates and the exact frame each sound effect
   starts.
6. **Options menu:** no emulator recording exists, so the user would need to provide one.
7. **Combat spacing** emulator trace; impact sprite review.
8. **VDP mirror** (`SRX_VDP_MODEL=1`) check for a possible one-frame sprite lag.

## 9. Repo housekeeping at handoff

- Tree clean at `fe9f16db`.
- Two old stashes in `/workspace/srx_int` are now redundant with commit `a38da5ad` (#5):
  "playtest-5-entry-placement-wip-v2" and the older mixed one. Leave them unless the user says to
  drop them.

## 10. How to work each item

1. Read the backlog entry and progress notes.
2. Find the ROM code path and write down the addresses ([C]/[H]).
3. Write or extend a headless test that fails for the right reason.
4. Implement the smallest change (Lua first). Use enums, no per-scene hard-coding.
5. Run the focused tests, then `/workspace/qa.sh`.
6. Review your own diff for the section 5 failure modes, especially any test edits.
7. On `QA_GREEN`: commit with a message naming the item and its ROM addresses, push via the helper,
   and check with `ls-remote`.
8. Update the backlog status and progress notes. Report to the user what changed, the evidence, and
   what still needs their playtest or Windows check.
