# Matrix datastore Transfer Data

**Status:** partial. The ordinary search, download prompt, storage checks,
run-objective branches, and source paths of the special story-file handler
whose inputs are available in the field runtime are connected to the Lua
node-action route. Other special-handler paths remain gated until their
source facts are available. No independent Genesis capture of this screen has
been identified.

## ROM trace

Primary source: `input/shadowrun_genesis_us.bin`.

- Node selection reaches Transfer Data at `0x54C5E` after the shared action
  check at `0x548A0`.
- If `FFF0E3 < 0` and the current node class matches `FFFBF2`, the run-objective
  branch skips the search. A negative `FFFBF6` uploads immediately and
  completes the run; otherwise the run's file ID and size go directly to the
  preview, and the stored file gets the objective marker `-1`.
- The ordinary path prints `0x16674D`, adds four dots at 20-frame intervals,
  then waits 9 more frames. It checks the special story-file handler at
  `0x55058`, then compares `(Int + Computer + Sensor) / 3` with
  `rand(10) + 1`. Failure prints `0x1667A5`.
- A successful ordinary search picks `node class * 5 + rand(5)` as file ID and
  `(rand(5) + 2) * 10` as size. The 70 names come from the ROM table at
  `0xE3A85`, with `0x14` bytes per entry.
- The prompt at `0x1667CE` defaults to Yes. Up/Down switches the selection;
  A confirms; the ROM cursor helper has no sound call. No prints
  `0x1667BA`; any input edge dismisses result messages.
- Yes checks free storage with `0x19558`, then searches the five file slots.
  The insufficient-storage message uses `0xE383D` and `0x166812`; a full file
  list uses `0x16681B`. Ordinary downloads use the value table at `0x16668A`
  and the node-colour multiplier (Red uses 5).
- The special handler at `0x55058` selects story data by current system and
  node class. Its flag words are Tips & Clues page 3 (FFFC38), Tips & Clues
  page 2 (FFFC34), and world progress (FFFC1E); it writes the changed flag
  word before showing text at `0x166968`. System 21 class 3 can also read the
  Group Items word (FFFC22). System 16 class 1 adds 50,000 to the active wallet
  after setting its source flag. The field host passes these facts through
  the typed Matrix world-state transaction. The host reads FFFC34/FFFC38 from
  the actual three-word Notebook tip state, FFFC1E from world-progress state,
  and FFFC27's known low bits from the Mr. Johnsons Notebook state. Lua gets
  the active system ID from its Matrix session. Unrestored tip/Johnson facts
  remain absent.
- The adapter currently evaluates only paths whose required facts are
  available: systems/classes 7/10, 16/1, 17/0 and 9, 18/2, 3, 9 and 10,
  19/0 and 12, 20/10, 21/2, 3 and 10, 22/2 and 4, 23/9 and 12, and 24/10.
  System 18 uses only FFFC34. System 16 class 1 requires the live wallet
  because it grants 50,000 nuyen; System 21 class 3 requires FFFC22. Systems
  19 class 0 and 23 class 12 run only on their FFFC27 bit-3 branch; the
  alternate FFFC26 branch remains gated. Paths reading FFFBFA, FFFBFB, or
  additional FFFC22 bits also remain gated, including System 21 class 4 and
  System 23 class 13.
- The dot loop and Yes/No branch contain no explicit sound call. A shared
  action/input sound outside those branches has not been independently
  established.

## Seven-question visual review

1. **Does it look correct?** The Qt route captures show the search message,
   filename/meter preview, Yes/No choices, and result messages in separate
   rows. The preview is readable and the selector is a slim yellow mark.
   These are internal host captures, not proof of Genesis visual equivalence.
2. **Does the look reflect the behavior?** The prompt starts on Yes, Up/Down
   changes the highlighted choice, A accepts, and B leaves the prompt
   unchanged. The route keeps Transfer Data selected beneath its temporary
   prompt cursor, so dismissing a result returns to the same datastore action.
3. **Are the visuals source-backed?** Text addresses, filename IDs, dot
   cadence, and prompt behavior are traced to the ROM. The native panel draws
   the decoded text from those facts. Exact original screen coordinates,
   colors, and pixel shape remain unverified without an independent capture.
4. **Is it animated, and when?** Only the search dots animate here: one dot
   appears every 20 source frames, through four dots, followed by a 9-frame
   pause. The filename prompt and result messages are static.
5. **Is there sound?** No sound call was found in the dot loop or prompt
   cursor branch. No sound duration is assigned to those steps. Any common
   action-gate sound remains unverified.
6. **Are neighboring controls clean?** The Qt route asks for a full redraw at
   each search/prompt/message transition. QA paints a prompt over a previous
   prompt image and compares the result to a fresh message render; the Yes/No
   row must be gone. This checks the panel replacement path, not unrelated
   Matrix game graphics.
7. **Do multiple frames support the reading?** Debug and Release route runs
   capture the first dot, full prompt, cancellation, ordinary completion,
   failed detection, insufficient storage, objective preview/completion, and
   immediate upload. These frames corroborate internal state transitions;
   they are not multiple independent examples from the original game.

## Verification

`shadowrun_ghidra_gameplay_script_menu_route` exercises the search timing,
ordinary success/failure, prompt selection, storage rejection, objective file,
immediate upload, and return to the same node. `shadowrun_ghidra_matrix_lua_rules`
checks the complete 70-name catalog, source RNG order, storage/full-slot
messages, and the translated special-handler rule branch. Both tests pass in
Debug and Release. The Qt route executable writes the following 320x224
captures under `build/conversion/qt/{debug,release}/qt_runtime/`
`matrix-node-action-route-qa/`:

- `transfer-search-first-dot.bmp`
- `actual.bmp` (ordinary filename and Yes/No prompt)
- `transfer-download-cancelled.bmp`
- `transfer-download-complete.bmp`
- `transfer-no-result.bmp`
- `transfer-no-storage.bmp`
- `transfer-objective-preview.bmp`
- `transfer-objective-complete.bmp`
- `transfer-upload-complete.bmp`

The system-map route writes `actual.bmp` in the sibling
`matrix-map-route-qa/` directory.

The captures were reviewed as full frames. The system-map capture shows the
six fixed ROM labels at separate 32-pixel rows. These captures establish host
redraw and state placement only; independent Genesis frames are still needed
for visual fidelity claims. Special-handler paths outside the supported fact
set remain incomplete.
