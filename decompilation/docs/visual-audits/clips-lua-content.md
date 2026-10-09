# Clips Lua Content Review

Date: 2026-10-09
ROM: US Genesis Shadowrun
Screen: field Clips menu, entry `0x11B58`

## Source and ownership

ROM entry `0x11B58` installs the top-level selector row from `0xCD33E` and
loads its labels from `0xD1DDC`. The nested Trade entry at `0x11E7E` installs
`0xCD37A` and reads the four nested labels from `0xD1E24`. The menu's prompt,
status, and result text uses the source strings cataloged by the field menu
audit. The offline import keeps the strings in
`assets/dialogue/script-text.lua`.

`gameplay/scripts/clips.lua` returns stable IDs paired with explicit ordered
keys for both selector rows and all 21 panel/message rows. The shared Lua text
catalog supplies their ROM copy. `GameScriptRuntime` validates the returned
key count and identifiers; `FieldClipsMenuRuntime` remains authoritative for
focus, action flow, ammunition, and runner-record changes. The ROM traces for
selector IDs, ammunition fields, and state transitions are recorded in the
[Clips source audit](../screen-system-audit.md#clips-0x11b58-0x12062).

## Review checklist

1. **Does it look correct?** All 17 Debug full-screen checkpoints were
   inspected at 320x224. The headings, counters, prompt rows, selectors, and
   result text stay within the Clips panel. The supplied longplay has no
   matching Clips route, so these captures do not independently establish
   Genesis pixel fidelity.
2. **Does the look reflect behavioral state?** The route drives each of the
   four top-level selections, selector wrap, nested Trade selection, reload
   No/Yes, and return. Runtime tests check the selected row and the Lua ID/key
   pairs; menu-catalog QA checks each key resolves to its expected ROM text.
3. **Are the visuals correct?** Root and nested button rows use separate
   extracted ROM assets (`0xCD33E` and `0xCD37A`); reload confirmation uses
   `0xCD3B6`. The full-screen captures show the matching focus treatment and
   imported text. No independent Genesis Clips selector or reload frame was
   found, so the external visual comparison remains open.
4. **Is it animated?** The root and Trade loops wait for VBlank and sample
   input. Their source path does not call the field menu's random-grid effect;
   selection changes on input, with no separate menu animation cadence.
5. **Is there a sound, and how long does it play?** The source requests event
   `0x25` on selector movement and `0x24` on action/confirmation. Their mapped
   clips are 2.134 s and 2.484 s including release tails. Route QA checks the
   event IDs; Windows speaker loopback for the Clips route remains unrecorded.
6. **Are the rest of the screen areas correct?** Review covered the entire
   scene in every checkpoint, including Joshua and party portraits, inventory,
   equipment, status bars, bottom action row, and right HUD. Reload and Trade
   text do not cover those regions.
7. **Do multiple frames support the interpretation?** The 17 captures cover
   each top-level row, nested Trade, both reload choices, result, and parent
   return. The same internal states were emitted by Debug and Release and all
   corresponding image hashes match. No matching independent gameplay
   sequence is present in the supplied video.
8. **Were new controls drawn over old controls?** The root, nested Trade, and
   reload-confirmation screens use their separate extracted rows. Returning
   with A/B restores the parent command/menu image pixel-for-pixel; no stale
   inventory row or duplicate selector remains under the current controls.
9. **Was the complete affected sequence reviewed?** All 17 full-screen Debug
   checkpoints were inspected individually. The matching Release files have
   the same names and SHA-256 hashes. The content-key migration changes no
   layout or state behavior.

## Verification and disposition

Debug and Release gameplay-script, Clips-route, and field-menu-catalog CTests
pass (3/3 per configuration). Each route emits 17 full-screen BMPs; Debug and
Release filename sets and SHA-256 hashes match. The Lua selectors and their
21 text rows now carry explicit keys rather than relying on C++ prefix-based
key construction. Independent Genesis visual coverage and route audio
loopback remain open.
