# Vid-Phone physical entry through the Scene-0 terminal

**Status:** partial. The normal Scene-0 event-31 collision now reaches the
ordinary Lua field terminal, Vid-Phone contacts, Captain Quinton's first-call
panel, the terminal redraw, and a completed Scene-0 return fade. Debug and
Release have a named physical-input CTest and full-frame captures. No matching
independent Genesis call frame has been identified.

## ROM route

- Scene 0 record 62 is at `0x0D584A`. Its `0x00DF` collision rectangle is
  `x=0x0180..0x019F`, `y=0x0060..0x007F`, grid cell `(12,3)`.
- Event 31 bypasses the map event table and reaches the ordinary terminal
  dispatcher at `0x0DB26`. The Lua field menu offers Call Taxi, Vid-Phone,
  Cyberspace, and Exit; its selected option determines the next action.
- Fresh New Game initializes the contact flags to `0x1C7F` at the existing
  `0x59C48` path. Restored saves still leave the flags unresolved until a save
  adapter supplies them. The QA combines a positional restore checkpoint with
  this known New Game value through a test-only fixture.
- Captain Quinton is the first available contact in that fixture. The contact
  record and Lua content resolve his name, portrait, and first-call text. The
  first-call panel maps Services to A and Goodbye to B; it is a button-choice
  panel, not a cursor list.

## Physical QA and captures

`shadowrun_ghidra_conversion_qt_field_vid_phone_physical_entry_qa` starts at
`(0x1800,0x0A00)`, directly south of the source rectangle, and uses ordinary
Up input to trigger it. The test selects Vid-Phone and Captain Quinton, checks
the full-screen source text and portrait, chooses Goodbye with B, confirms the
terminal image is restored, selects Exit, and waits for the 60-frame host
modal-return transition to finish.

The Debug capture set is under
`build/conversion/qt/debug/qt_runtime/field-vid-phone-physical-entry-qa/`.
`actual.bmp` is the 320x224 first-call panel; `field-terminal.bmp` shows the
four-choice menu, `terminal-restored.bmp` shows its clean redraw, and
`field-return.bmp` shows the full Scene-0 field and HUD after the fade. These
are runtime regression captures, not independent Genesis visual evidence.
The seven BMPs in this capture set are byte-identical between Debug and
Release by SHA-256.

## Remaining work

- Match a Vid-Phone call against an independent Genesis capture.
- Exercise each contact-specific service tree from a physical field route.
- Complete the source-backed branches and transactions that remain open in
  the Vid-Phone contact audit.
