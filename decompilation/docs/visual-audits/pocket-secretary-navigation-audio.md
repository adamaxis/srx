# Pocket Secretary Root Navigation Audio Audit

Date: 2026-10-07  
ROM: US Genesis Shadowrun  
Screen: Pocket Secretary root menu, entry `0xC2A6`

## ROM trace

`0xC2A6` draws the five-row parent page. Its loop at `0xC2D4` waits for
VBlank, latches and samples the controller, and advances the random grid
effect through `0xE660`. Up and Down are tested at `0xC2EA` and `0xC2FC`;
accepted movement sets the cursor delta and calls `0xC376` at `0xC328`, then
returns directly to the loop. `0xC376` writes the old cursor cell, wraps the
typed row index, and writes the new cursor cell. This path contains no sound
request or sound-service call. A dispatches the selected handler at `0xC33A`,
C calls the mode-action handler at `0xC35E`, and Start returns at `0xC374`.

The parent command overview separately requests event `0x24` when A opens
Pocket Secretary. Its extracted `audio.menu.command-024` clip lasts 2.484
seconds including its release tail. That page-entry cue is not retriggered by
root cursor movement. Movement therefore has no playback duration to measure.

The Qt `apply_pocket_secretary_input` path returns `selection_moved` without
calling its audio callback; `FieldSceneRuntime` binds event `0x25` only to the
Pocket Secretary C/mode-action callback. The field-secretary QA now asserts
the single `0x24` entry request, clears the event log, then checks four
accepted Down moves and the fifth move's wrap to row 0 with no audio events.
Its named root frames are `actual.bmp`, `selection_1.bmp` through
`selection_4.bmp`, and `wrap_to_0.bmp`.

## Full-screen frame review

I reviewed all six root frames in both Debug and Release at 320x224. Between
builds, every pixel above the lower bezel is identical; the only differences
are the random `0xE660` lower-bezel cells at y=200..215. Between successive
root selections, pixel differences are confined to the previous/current
selector cells and the lower-bezel effect; the labels, title, separator,
outer frame, and field HUD remain unchanged. The wrap frame returns the
selector to row 0. This confirms clean native redraw behavior for this route,
but is not an independent Genesis comparison.

## Eight-question review

1. **Does it look correct?** The full-screen root frames show the five source
   labels inside the Pocket Secretary panel and the selector at the current
   row. I reviewed the entry frame, every row transition, and the wrap frame
   in the Debug and Release output. This is an internal Qt review; the supplied
   Genesis longplay contains no matching Pocket Secretary visit.
2. **Does the look reflect the behavioral state?** Yes in the native route:
   `PocketSecretaryMenuState::selection` advances from 0 through 4 and wraps
   to 0, and the cursor moves with that typed state. QA checks the selection
   value and both the previous and new cursor cells for every transition.
3. **Are the visuals correct?** The root uses the source option order at
   `0xC1418`; `0xC2A6` writes the text and `0xC376` places the selector at the
   selected row. The five labels and cursor locations are checked in complete
   320x224 frames. Without an independent Genesis frame of this menu, the
   exact external appearance remains unverified.
4. **Is it animated, and should it be?** The root loop calls `0xE660` once
   per VBlank while the page is active, so the two-cell random lower-bezel
   effect advances every menu frame. Cursor movement itself is immediate and
   has no separate animation. The current root QA checks cursor transitions;
   the shared E660 route checks its source cadence and lower-bezel bounds.
5. **Is there a sound, what is it, and how long does it play?** No sound is
   requested for Up/Down. The parent A-open event `0x24` is a separate
   2.484-second clip including release tail. Root navigation has no playback
   duration because it does not request audio. Debug/Release route QA checks
   one entry event and no additional event through all four moves and wrap;
   speaker loopback for this menu is not independently measured.
6. **Are the other screen areas correct too?** Review used full 320x224
   frames, including the complete outer panel, title, separator, all rows,
   lower bezel, and surrounding field HUD. Pixel assertions verify every row
   label and that the selector leaves the previous row as it moves.
7. **Do multiple frames support the interpretation?** Yes, multiple native
   captures show each intermediate selection and wrap. No independent
   Genesis Pocket Secretary frame is present in the supplied longplay, so the
   repeated captures support only the source-derived state and internal
   projection, not external pixel fidelity.
8. **Were old buttons or graphics left under the new state?** No page change
   occurs during root navigation; the full parent page is repainted from the
   typed selection. QA verifies that the prior selector cell changes and the
   new selector cell appears, with no duplicate cursor left behind. Child
   entry/return clearing is covered by separate route checks.

**Disposition:** `SOUND-02` is closed as a non-defect under the ROM source.
The screenshot evidence does not close the independent visual-fidelity item
for the Pocket Secretary hill. `SOUND-01` selection/cancel paths remain open.
