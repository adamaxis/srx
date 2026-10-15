# Notebook Mr. Johnsons Page Audit

Date: 2026-10-07
ROM: US Genesis Shadowrun
Scope: Notebook route `0xF416`, visible Johnson list, detail wait, C action, and return

## ROM trace and Lua route

Raw US-ROM bytes at `0xF416` are the primary source. The handler clears and
rebuilds the child panel, writes the `Mr. Johnsons` title from `0xC56AE`, and
draws its separator from `0xC56D6`. FFFC27 bits 0 through 4 gate the five
entries in this source order:

| FFFC27 bit | Name text | Detail text |
|---:|---:|---:|
| 0 | `0xC5728` Mr. Gunderson | `0xC5774` |
| 1 | `0xC5755` Julius Strouther | `0xC58D5` |
| 2 | `0xC5766` Mortimer Reed | `0xC593F` |
| 3 | `0xC5736` Vigore & Jarl | `0xC57D2` |
| 4 | `0xC5744` Caleb Brightmore | `0xC584E` |

The list cursor follows the visible rows and wraps at both ends. A or B calls
the lower-panel grid rebuild at `0xF404`, presents the selected source detail,
and waits through `0xF312`; the next controller input rebuilds the selected
list row. C calls shared mode action `0x391A`, requests event `0x25`, and
resumes the same list selection. Start returns to the Notebook. A/B detail
selection, list movement, and Start do not make a sound request in this handler.

The host imports the five names and details as keyed entries in
`assets/dialogue/script-text.lua`; doubled `0x80` line breaks are preserved.
Lua stores the visible source indices, selection, and detail-wait state and
owns the list input and transitions. `FieldSceneRuntime` supplies typed
FFFC27/story facts, then projects the Lua route snapshot into the existing Qt
painter state. The Qt projection redraws the lower panel grid at the detail
transition before placing text, then redraws the list and selector on return.
It keeps the title, separator, and shared frame. The Lua wait advances the
shared E660 lower-bezel effect once per VBlank.

The Stoker Jump House response at `0x213E2` sets FFFC27 bit 0 and is covered by
the referral route QA. This audit's all-five-entry pass uses the explicitly
diagnostic FFFC27 value `0x1F`; writers for bits 1 through 4 and loaded-save
state remain unresolved. That diagnostic state exercises the five page paths,
not a claim that ordinary play currently unlocks every name.

## Nine-question review

1. **Does it look correct? — Unknown against Genesis.** No matching independent
   Pocket Secretary visit was found in the supplied longplay. I reviewed full
   320x224 Debug and Release list, detail, and return frames against the ROM
   text/order and known menu layout; these are internal Qt captures, not a
   Genesis visual golden.
2. **Does the look match behavioral state? — Yes for tested state.** The
   typed visible-entry list follows FFFC27 bits 0..4 in ROM order. The selected
   row's cursor remains at that row while A/B displays its detail, and the
   next wait input returns to the same selection. C leaves the list and
   selection in place while requesting exactly event `0x25`. QA checks all
   five source indices and each selection. The five-entry screenshot route
   uses the diagnostic all-bits value described above.
3. **Are the visuals themselves correct? — Partially verified.** The imported
   text matches the five ROM strings, including source blank lines; title,
   divider, 16-pixel row spacing, detail origin, named grid asset, and cursor
   placement are source-derived. Full-screen dumps show every detail fitting
   without clipping. Independent Genesis palette, exact pixel placement, and
   wording appearance remain unverified because there is no matching capture.
4. **Should anything animate? — Yes, the shared lower-bezel effect continues.**
   The `0xF312` wait services E660 once per VBlank. The Mr. Johnson detail
   handler does not start a separate animation; E660 continues during the
   detail wait. The QA stores every full-screen VBlank frame from wait frames
   00 through 30, checks that the title/detail/selection region stays fixed,
   and checks that lower-bezel pixels change. Exact live-game random seed and
   visual cadence are not independently established.
5. **Does it have sound? — C only in this handler.** C calls `0x391A`, which
   requests event `0x25`, mapped to `audio.effects.resource.037`; the extracted
   duration is 2.134 seconds including its release tail. Shared Debug/Release
   audio QA checks decode, configured-output playback state, and duration.
   This route test checks the event request, but Windows loopback for this
   route is not captured. A/B, cursor movement, and Start have no sound call
   traced in `0xF416`/`0xF312`.
6. **Are the other visible areas correct? — Yes in the reviewed Qt captures.**
   The entire 320x224 image was reviewed: title, separator, all five names,
   selected marker, detail text, panel frame, animated lower bezel, and the
   surrounding screen. No unexpected overlay was visible.
7. **Do multiple frames support the interpretation? — Yes internally; no
   independent game sample.** QA opens every detail with alternating A/B,
   dismisses each through the wait, checks all five list selections, exercises
   both cursor-wrap ends, calls C, and returns to Notebook with Start. Debug
   and Release contain the corresponding full-screen states. The supplied
   longplay has no matching interface visit.
8. **Does the new UI replace the old UI cleanly? — Yes in tested states.**
   Each detail capture has no visible Johnson list rows or selector beneath
   the prose. On return, the full Notebook interior matches the list state
   captured immediately before opening that detail. Start returns to the
   parent Notebook row with no detail text left behind. The outer frame,
   title, and divider remain intentionally shared.
9. **Was the complete affected time window reviewed? — Yes for the route
   states and the 30-VBlank animated hold.** The QA traverses Pocket Secretary
   entry, Notebook, Mr. Johnsons list, C, all five details and dismissals,
   list wrap, and Start return. It saves the complete screen for each state
   and every update frame of the 30-frame detail hold. The parent/field route
   is covered by the existing full-screen field-secretary QA. No continuous
   independent Genesis capture exists for this screen.

## Evidence and disposition

- **Code changed:** Lua-owned child-page state and input, typed source-fact
  bindings, route-to-painter projection, full-panel replacement/return
  drawing, and field-secretary QA captures.
- **Behavior implemented:** FFFC27-filtered list, A/B detail wait, C event
  `0x25`, cursor wrapping, and Start return.
- **Internally checked:** Debug/Release CTests
  `shadowrun_ghidra_gameplay_script_menu_route`,
  `shadowrun_ghidra_conversion_qt_field_secretary_qa`,
  `shadowrun_ghidra_conversion_qt_field_dialogue_referral_followup_qa`, and
  `shadowrun_ghidra_field_menu_text_catalog`. Full-screen output is under
  `qt_runtime/field-secretary-qa/`, including `notebook-mr-johnsons-list.bmp`,
  `notebook-mr-johnsons-detail-0.bmp` through `-4.bmp`, detail-return frames,
  `notebook-mr-johnsons-detail-wait-frame-00.bmp` through `-30.bmp`, and
  `notebook-mr-johnsons-return.bmp`.
- **Independently visually checked:** no matching Genesis Pocket Secretary
  frame was found in the supplied longplay. ROM text and routing are verified;
  external visual fidelity is not.
- **Defect status:** the missing native child route is implemented, but this
  page remains partial until non-diagnostic story/save flags and an independent
  game reference are available. Do not call the page Genesis-visually resolved.
