# Matrix random alerts

## ROM trace

- `0x173F2` initializes `FFF0A0` from field alarm `FFE16F`. `0x17466` copies that initial alert value to `FFE189`, the last-alert sound latch.
- Entering `0x54662` sets `FFF0A6` to `-1` (overload armed) and clears the action-attempt counter `FFF0A8`. The initial node view is built at `0x546CC`.
- Failed action handling at `0x54924` increments `FFF0A8` after the failure message is dismissed. Successful dispatch increments it at `0x54962` after the selected node action returns.
- Each rebuilt node view calls `0x54A8E`, which consumes `rand(10)` even when the attempt counter is zero. If the roll is less than `FFF0A8`, it clears the counter and calls `0x1989A` to raise the alert by one.
- `0x1989A` keeps alert values 0 and 1, clamps lower results to no alert, and clamps higher results to active alert. If `FFE189` differs from the new status, it plays `0x54` for none, `0x53` for passive, or `0x52` for active. An armed increase above active enters overload.
- The ordinary notice is text stream `0x1669E8`; the route waits 25 VBlanks, waits for a controller event, and restores the node view. Overload fades through `0x18776`, transitions through `0x26A00`, displays `0x166A75`, waits 30 VBlanks and an input, then returns through the node-action exit branch.

## Host route

Lua owns the attempt count, alert value, notice text, and frame waits. The Qt host forwards the Matrix alert roll to the same shared source RNG used by other game calls. Its purpose-specific callback lets the route test set deterministic alert rolls without changing the RNG values used by unrelated menu operations.

The route closes the Matrix menu after the overload message wait. The native fade and full `0x26A00` display restore are still open work. The current debug Matrix route is not the ROM's live Matrix actor/IC loop.

## QA

Debug route and Lua-rule CTests cover initial field-alarm state, a rand(10) draw on every rebuilt node view, passive and active alerts, their sound IDs, text, and 25-frame waits. `system-alert-message.bmp` was reviewed as a full host frame: notice rows are separated and no node-action buttons remain underneath. This is a host-rendered sanity capture, not an independent Genesis visual reference.
