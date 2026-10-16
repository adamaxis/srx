# Matrix random alerts

## ROM trace

- `0x173F2` initializes `FFF0A0` from field alarm `FFE16F`. `0x17466` copies that initial alert value to `FFE189`, the last-alert sound latch.
- Entering `0x54662` sets `FFF0A6` to `-1` (overload armed) and clears the action-attempt counter `FFF0A8`. The initial node view is built at `0x546CC`.
- Failed action handling at `0x54924` increments `FFF0A8` after the failure message is dismissed. Successful dispatch increments it at `0x54962` after the selected node action returns.
- Each rebuilt node view calls `0x54A8E`, which consumes `rand(10)` even when the attempt counter is zero. If the roll is less than `FFF0A8`, it clears the counter and calls `0x1989A` to raise the alert by one.
- `0x1989A` keeps alert values 0 and 1, clamps lower results to no alert, and clamps higher results to active alert. If `FFE189` differs from the new status, it plays `0x54` for none, `0x53` for passive, or `0x52` for active. An armed increase above active enters overload and returns before the sound-latch check, so that path plays no alert sound.
- The ordinary notice is text stream `0x1669E8`; the route waits 25 VBlanks, waits for a controller event, and restores the node view. Overload runs `0x18776` first: five palette steps separated by two VBlanks (10 frames). It then follows the 54-frame timing of `0x26A00`, displays `0x166A75`, waits 30 VBlanks and an input, and returns through the node-action exit branch. The host currently fades the whole panel to black and holds it through those 54 frames; the source's transfer artwork is still missing.

## Host route

Lua owns the attempt count, alert value, notice text, and frame waits. The Qt host forwards the Matrix alert roll to the same shared source RNG used by other game calls. Its purpose-specific callback lets the route test set deterministic alert rolls without changing the RNG values used by unrelated menu operations.

The route now observes the source order and timing: 10 fade frames, 54 display-transition frames, the overload message, its 30-frame minimum, then the any-input wait and Matrix exit. Its full-screen fade clears the old node actions and selector before the message. The original `0x26A00` graphics are not yet ported; the 54-frame interval is rendered as black, so visual fidelity for that animation remains open. The current debug Matrix route is not the ROM's live Matrix actor/IC loop.

## QA

Debug route and Lua-rule CTests cover initial field-alarm state, a rand(10) draw on every rebuilt node view, passive and active alerts, their sound IDs, text, and 25-frame waits. The overload route checks that no alert SFX is added, each fade step follows two VBlanks, the 54-frame transition interval precedes the message, and the 30-frame wait/input gate follows it. `system-overload-fade-start.bmp`, `system-overload-fade-black.bmp`, and `system-overload-message.bmp` were reviewed as full 320x224 frames; the old actions and selector disappear under a full black frame, and the message fully replaces the prior panel. These are host-rendered regression captures, not independent Genesis visual references.
