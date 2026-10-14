# Pocket Secretary Selection and Cancel Audio Audit

Date: 2026-10-07
ROM: US Genesis Shadowrun
Scope: Notebook C action, Tips C action, Notebook/Tips movement and return

## ROM trace and native route

The Notebook loop at `0xF236` services the current page and dispatches the
selected row. C is a separate mode-action branch: it calls `0x391A`, then
branches back to the Notebook loop with the current selection intact. The
shared mode-action routine requests audio event `0x25`. Qt's typed Notebook
state now handles C explicitly, calls the existing mode-action callback, and
returns `notebook_mode_action`; `FieldSceneRuntime` retains the page and
selection while issuing the event.

Event `0x25` maps to `audio.effects.resource.037`. The extracted clip lasts
2.134 seconds including its release tail. The shared Debug/Release audio test
checks the resource mapping, decoded playback, default-output `PlayingState`,
and a 2.05–2.20 second playback interval. The field route QA checks the event
request and same-page return. Route-specific Windows loopback has not been
captured, so these checks prove the selected event/resource and Qt output
playback path, not the audible waveform at the user's speakers.

The source Notebook A row dispatch, Up/Down cursor movement, and B/Start
return paths do not request audio in `0xF236`; a selected child handler may
have its own behavior. On Tips, `0xF9A4` also routes C through `0x391A` and
returns to Tips. Tips Left/Right movement and A/B/Start returns make no sound
request. The parent command overview's event `0x24` occurs once when A opens
Pocket Secretary; it is a separate 2.484-second page-entry cue, not a cursor or
cancel sound. Root Up/Down movement has no event, as recorded in the
[root navigation audit](pocket-secretary-navigation-audio.md).

The Debug and Release field-secretary CTests produce full-screen frames under
`qt_runtime/field-secretary-qa/`, including `notebook-menu.bmp`,
`notebook-mode-action.bmp`, `notebook-selection-1.bmp`,
`notebook-tips-actual.bmp`, `notebook-tips-page-2.bmp`, and the return frames.
Route assertions check event `0x25`, page-open state, selected row, and the
Notebook image region after C. The C action does not replace the screen; Tips
entry and return use the source-derived panel clear before redrawing content.

## Eight-question review

1. **Does it look correct?** The Release `notebook-menu.bmp` and
   `notebook-mode-action.bmp` are identical at 320x224, as the source says C
   resumes the same Notebook screen. The selected `Current Run` row stays
   selected. This confirms internal Qt continuity; the supplied Genesis
   longplay has no matching Pocket Secretary visit.
2. **Does the look reflect the behavioral state?** Yes for this transition.
   `PocketSecretaryMenuState::notebook.open` remains true and selection stays
   at zero. The route QA checks both state values and that exactly event
   `0x25` was requested. The sound itself has no visible selection effect.
3. **Are the visuals correct?** The `0xF236` C branch does not draw another
   page or move the cursor; the two full-screen Qt captures are pixel-identical.
   This supports same-screen continuity, not exact Genesis pixel fidelity.
   Independent Pocket Secretary frames remain unavailable.
4. **Is it animated, and should it be?** The active Notebook loop advances
   the shared `0xE660` lower-bezel effect once per VBlank. C does not start a
   separate animation or alter the menu. The effect is confined to its twelve
   source-derived lower-bezel destinations; independent Genesis cadence and
   appearance remain unverified.
5. **Is there a sound, what is it, and how long does it play?** Notebook C
   calls `0x391A`, which requests event `0x25`; Qt maps it to
   `audio.effects.resource.037`, 2.134 seconds including the release tail.
   Shared audio QA verifies decoded playback, default-output state, and
   duration. The C route asserts the event, but its speaker loopback waveform
   has not been recorded. Notebook row movement and A/B/Start loop branches
   are source-silent. Tips uses the same C event and silent movement/return
   branches. Event `0x24` is the separate 2.484-second Pocket Secretary entry
   cue.
6. **Are other areas of the screen also correct?** I reviewed the entire
   320x224 before/after frames, including title, divider, all Notebook rows,
   selector, outer panel, lower-bezel animation, and surrounding field HUD.
   The C route leaves the page unchanged. Debug/Release differences in the
   mode-action capture are confined to the animated lower bezel at y=200..215.
7. **Do multiple frames support this interpretation?** The Notebook entry,
   C-action, selected-row, Tips entry/page/hold, and return captures cover
   multiple route states. The C before/after pair supports the same-page
   interpretation, while the other frames show the surrounding source-driven
   route. None is an independent Genesis capture; the visual-fidelity claim
   remains open.
8. **Were old buttons or graphics left beneath a new page?** No page swap
   occurs on Notebook C, so no replacement panel is drawn over stale controls.
   The route keeps the same typed page and selector. When opening Tips, Qt
   clears the child panel before drawing its text and counter; existing route
   QA checks that parent buttons/selector do not survive under Tips and that
   Tips content is cleared on return.

**Disposition:** This closes the missing Qt Notebook C-to-event-`0x25` route
for the implemented Notebook path. `SOUND-01` remains partial: other child
page action handlers, route-specific Windows loopback, and independent
Genesis Pocket Secretary frames are open. The root navigation report is a
non-defect under the ROM and remains tracked separately as `SOUND-02`.
