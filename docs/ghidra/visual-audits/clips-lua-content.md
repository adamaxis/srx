# Clips Lua Content Review

Date: 2026-10-14
ROM: US Genesis Shadowrun
Screen: field Clips menu, entry `0x11B58`

## Source and ownership

ROM entry `0x11B58` installs the top-level selector row from `0xCD33E` and
loads its labels from `0xD1DDC`. The nested Trade entry at `0x11E7E` installs
`0xCD37A` and reads the four nested labels from `0xD1E24`. The menu's prompt,
status, and result text uses the source strings cataloged by the field menu
audit. The offline import keeps the strings in
`assets/dialogue/script-text.lua`.

`gameplay/scripts/menu/screens/clips.lua` owns selector,
reload-confirmation, recipient, quantity, transfer, distribution, and
result-wait state. `gameplay/scripts/menu/rules/clips.lua` contains the
source-derived arithmetic. The `GameScriptRuntime` host bridge exposes three
typed runner records, the active slot, equipped-weapon magazine capacity, and
an ammo-record write. `FieldSceneRuntime` mirrors Lua's route snapshot into
the Clips presenter; its legacy C++ action path is used only when the Lua
route is inactive. The shared wait screen is
`gameplay/scripts/menu/screens/message.lua`. Source addresses and state
transitions are recorded in the
[Clips source audit](../screen-system-audit.md#clips-0x11b58-0x12062).

ROM entry `0x11B58` gates actions through `0x12062`. Its signed-byte test
routes zero and negative reserves through the `0x11C9C` Function Aborted exit;
the negative-byte path also clears the reserve. Reload with remaining rounds
opens the default-No prompt; only A confirms, and No returns silently. Successful reload
consumes one shared clip, adds `0x26` rounds, and clamps to the equipped
weapon's `D4540 + 0x24` capacity. Trade filters the active roster using the
source/remainder bits at `+0x10`; the source runner's clip reserve is debited
and the chosen recipient's reserve is credited. Distribution counts active
runners and accumulates the reserve with source byte arithmetic. If the byte
total wraps to zero it leaves all records untouched; otherwise the ROM writes
the quotient to all three records and adds the remainder to the record marked
by status bit 1. The byte-wrap and inactive-record effects are covered by the
Lua route test.

## Review checklist

1. **Does it look correct?** All 26 fresh Debug full-screen BMPs were reviewed
   at 320x224. The headings, counters, prompt rows, selectors, trade recipient,
   and result text stay within the Clips panel. Two named aliases duplicate the
   initial Clips and Trade states. The supplied longplay has no matching Clips
   route, so these captures do not independently establish Genesis pixel
   fidelity.
2. **Does the look reflect behavioral state?** The route drives each of the
   four top-level selections, selector wrap, Cancel result, nested Trade
   selection and transfer, reload No/Yes, no-recipient and zero/signed-negative-reserve gates,
   and return. Lua route tests check selected state, record writes, byte wrap,
   signed-negative reserve clearing, and stable IDs; menu-catalog QA checks
   each key resolves to ROM text.
3. **Are the visuals correct?** Root and nested button rows use separate
   extracted ROM assets (`0xCD33E` and `0xCD37A`); reload confirmation uses
   `0xCD3B6`. Full-screen captures show the selected focus blocks and text in
   the intended panel. These host captures use imported source assets and do
   not independently prove Genesis pixel fidelity.
4. **Is it animated?** The ROM samples the root, prompt, and Trade loops after
   VBlank. No periodic menu effect or animation call is present; these screens
   change on input and their content remains static between inputs.
5. **Is there a sound, and how long does it play?** The source requests event
   `0x25` on selector movement and `0x24` on action/confirmation. The mapped
   resources have 2.134 s and 2.484 s file durations including release tails.
   The exact Clips route's audible playback length has not been measured by
   loopback.
6. **Are the rest of the screen areas correct?** Review covered the entire
   scene in every checkpoint, including runner and party portraits, inventory,
   equipment, posture/status bars, bottom action row, and right HUD. Trade,
   confirmation, and message screens do not overwrite those regions.
7. **Do multiple frames support the interpretation?** The captures cover all
   four top-level rows, nested Trade movement/quantity/action/result and
   Ricky/Winston recipient cycling with wrap, no-recipient and both reserve
   abort cases, both reload choices, acknowledgement, and parent return.
   Debug and Release emit the same 26
   filenames with matching SHA-256 hashes. No matching independent
   gameplay sequence is present in the supplied video.
8. **Were new controls drawn over old controls?** The root, nested Trade, and
   reload-confirmation screens use separate extracted rows. The message painter
   clears its center title and button strip. Acknowledging the Cancel message
   restores the parent command image pixel-for-pixel; no stale Clips row or
   duplicate control remains under the current controls.
9. **Was the complete affected sequence reviewed?** All current full-screen
   checkpoints were reviewed across the full capture set. The matching Release
   files have the same names and SHA-256 hashes. The QA dump clears prior BMPs in its
   dedicated output directory before writing, so old states cannot enter the
   review set.

## Verification and disposition

Debug and Release gameplay-script route CTests pass (1/1 each); Qt Clips,
field-menu-catalog, and Clips-runtime CTests pass (3/3 each). Each clean Qt
route dump emits 24 distinct states plus two aliases (26 BMPs); Debug and
Release filenames and SHA-256 hashes match. The selector, reload prompt, trade,
ammo writes, distribution, and result wait now run through Lua. Independent
Genesis visual coverage, loaded-save field composition, and route audio
loopback remain open.
