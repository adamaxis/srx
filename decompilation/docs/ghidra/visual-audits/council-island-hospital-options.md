# Council Island Hospital Options

## Scope

This audit covers the scene-4 hospital field-service prompt and the separate
Hospital Options screen. The ROM does not send event 4 to a hospital interior:
the field event opens the shared service dialog, and the default service branch
can enter a full-screen menu on the same field map.

## Source and transitions

- Scene 4 mode/event `4/4` is the hospital service route at ROM `0x1FA02`.
  The alternate-result byte selects package handoff, package payment, or the
  default service prompt. The default prompt offers Services and Leave.
- ROM `0xC928` builds Hospital Options, then waits for VBlank and input while
  advancing the E660 grid effect. Its three cursor rows dispatch Medical
  Attention (`0xCA2E`), Cyberware (`0xCA16`), and Exit (`0xCE7C`).
- Medical Attention enters the Lua treatment task for `0xCA2E`. Its choices
  depend on the active runner's class, Body, Willpower, Physical, and Stun
  values. Recovery rows are 100%, 90%, 70%, and 40%, with the ROM's cumulative
  prices; an unaffordable choice leaves the runner and wallet unchanged. Fully
  recovered runners receive the ROM's no-treatment-needed message. A runner
  with Physical 9, or Physical 10 with Stun below 10, sees the source's empty
  treatment view.
- The host displays the selected Lua state and applies the typed party/wallet
  transaction. The ROM's post-treatment `0x38FA` refresh is represented by the
  normal field record/HUD refresh path; exact transition timing remains open.
  Cyberware enters the existing typed Cyberware screen.
- Exit opens `0xCE7C`'s Yes/No prompt with No selected. No returns to Hospital
  Options; Yes completes the hospital task and resumes the field door-return
  route. Start does not accept the prompt. In Hospital Options, B calls field
  action `0x391A` and returns to the same loop at `0xC982`; Lua preserves the
  selected row across the typed host runner-selection service. Start is ignored
  by that loop at `0xC9FE`.

The Lua task owns choices, cursor movement, and transitions in
`gameplay/scripts/council_island_hospital.lua`. The Qt field runtime presents
the resulting typed screen with the extracted field UI resources; it does not
own a second menu state machine. Text strings are named ROM-extracted assets.

## Verification

The scene-4 entity link in `gameplay/world/entities/council_island_hospital/interactions.json` connects record 93 to `council_island_hospital.present_field_exit_prompt`. The alternate-result byte selects package handoff/payment versus the default Services/Leave prompt. Event-table row 4 remains 0x0D provenance; the physical-entry QA confirms the tested default route stays on Scene 4. The bits-23/24 branch at 0x1F830 belongs to a separate mode-exit policy and does not gate this event-4 route.

Debug and Release CTests cover
`shadowrun_ghidra_conversion_qt_field_hospital_options_qa`,
`shadowrun_ghidra_conversion_qt_field_hospital_leave_confirmation_qa`,
`shadowrun_ghidra_conversion_qt_field_hospital_medical_attention_qa`,
`shadowrun_ghidra_conversion_qt_field_hospital_physical_entry_qa`,
`shadowrun_ghidra_field_dialogue_runtime`, and
`shadowrun_gameplay_script_runtime_qa`. The Qt cases write complete 320x224
frames to `qt_runtime/field-hospital-qa/options/actual.bmp`,
`qt_runtime/field-hospital-qa/leave-confirmation/actual.bmp`, and
`qt_runtime/field-hospital-qa/medical-attention/actual.bmp`. The Medical
Attention frame was opened at original resolution; the full lower bezel is
present. Lua route tests cover each treatment row, its charge and recovery,
insufficient funds, fully recovered runners, and the empty-row case.
The physical-entry test also writes `qt_runtime/field-hospital-physical-entry-qa/spawn.bmp`,
`doorway-contact.bmp`, `actual.bmp`, and `hospital-options.bmp`, plus the No-default leave prompt, Start no-op, Options after No, Yes-selected prompt, immediate Yes handoff, and completed field return; it also writes route.tsv. Ordinary Up
reaches scene-4 record 93/event 4 and opens the complete nurse Services/Leave
prompt; A opens Hospital Options on the same map in field mode 4. The complete
320x224 Debug and Release prompt/options, leave-confirmation, and final field-return captures match by SHA-256 and were
inspected at original resolution. The options QA sends Start, Down, then B:
Start leaves the screen open, B dispatches the field action, and the menu
returns with the Cyberware row still selected. It captures the full
`options/runner-action-return.bmp` frame in both builds; the frames match.

The shared select_field_exit_prompt runtime now snapshots linked return anchors before starting event-route prompts. This closes the prior gap where the hospital prompt opened without a saved same-map door return. The route records entry `(27346,14428)`, return `(27346,14778)`, and 131 frames.

These captures are internal presentation checks. No independent Genesis
hospital frame was identified in the supplied longplay, so the exact hospital
pixels and treatment-screen placement are not independently verified. The
shared field runner-selection renderer's source target/fade/animation tail and
exact post-treatment transition timing remain open. Keep this hill partial.
