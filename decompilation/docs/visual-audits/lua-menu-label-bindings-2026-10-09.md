# Lua menu-label binding review

Scope: move the remaining Pocket Secretary and command-overview display keys
into Lua, separate ID-only arrays from rendered menus, and remove C++ label-key
inference. The game renderer, cursor state, item/runner values, and native
actions are unchanged. Dialogue speaker portraits and New Game archetype
portraits are already selected by Lua; this slice does not change extracted
portrait pixels.

## Visual verification questions

1. **Does it look correct?** I reviewed four contact sheets covering all 235
   unique Debug images among the 370 fresh Debug/Release screenshot paths, then
   opened the command-overview runner summary and two Notebook pages at native
   320x224 resolution. Text stays within its existing wells, the selected
   command row and Notebook cursor remain visible, and adjacent field/shop/
   dialogue regions are intact. This script binding does not change portrait
   pixels. The previously reported Stoker extraction defect was corrected by
   the shared sprite-repeat decoder fix (`7a8dfd3ab8`); the current 56x72
   portrait matches the independent section-03 frames 000082, 000301, and
   000601 at its in-panel position, with face-crop RGB MAE 8.914, 8.574, and
   8.564 respectively. This closes the Stoker extraction symptom only; it
   does not establish fidelity for other head assets.
2. **Does the look reflect behavioral state?** Lua returns ordered IDs and
   explicit label keys for rendered menus and text catalogs. Native typed
   state still selects the runner, menu row, and Notebook page. Pure ID lists
   such as inventory actions and the motel quest hook use `string_list`; the
   runtime now rejects an ID-only list passed as a rendered menu.
3. **Are the visuals correct?** No position, color, image, or redraw code
   changed. The imported ROM text values are resolved from the Lua key arrays;
   the script-runtime and field-menu-catalog checks validate IDs and keys.
   These are internal Qt captures, not an independent Genesis pixel golden.
4. **Is it animated, and should it be?** Labels and portraits are static. The
   source random-grid effect remains active in Pocket Secretary and Hotel
   menus: ROM routine `0xE660` advances it once per VBlank. Across the four
   Notebook frames that differ between Debug and Release, pixel diffs are
   confined to the lower-bezel range `x=40..231, y=200..215`; text, selector,
   panel, and surrounding frame are identical.
5. **Is there sound, and how long does it play?** This data-binding change
   adds no sound request and changes no sound duration. Existing menu entry
   events remain on their native paths; the Lua label lookup does not retrigger
   them. Exact-route Windows loopback is not recorded by this review.
6. **Are the rest of the screen and previous controls correct?** Full-screen
   route captures include field, command overview, Notebook, Hotel, dialogue,
   clips, cyberware, and Ares shop states. The Hotel presenter clears all
   320x224 pixels before drawing its black page and grid. No old button rows or
   doubled panels appeared under the tested child and return screens. Hotel's
   black-base presentation still lacks an independent Genesis capture.
7. **Do multiple frames support the interpretation?** The 370 current paths
   match between Debug and Release. Four Notebook pairs differ only in the
   source random-grid cells described above; the other 366 path pairs are
   pixel-identical. All 235 unique Debug images were reviewed in contact
   sheets, and the four differing Release images were opened at native size.
   This supports internal consistency, not independent game fidelity.
8. **Does each new page replace the old one cleanly?** No controls were added.
   Pocket Secretary children redraw the inner grid, Hotel starts each page
   from a full-screen clear, and the return screenshots contain no leftover
   dialogue, menu, or cursor layer.
9. **Was the complete affected interval checked?** The focused script,
   catalog, menu, dialogue, hotel, shop, clips, cyberware, and command-overview
   QA set passed 25/25 tests in both Debug and Release. The screenshot review
   includes complete frames and transition neighbors rather than a crop of the
   edited text area.

## Content and presentation boundaries

`dialogue.lua` already selects a speaker's portrait alias, and the shared
dialogue presenter loads that extracted asset. `startup.lua` already maps the
three New Game archetype IDs to their portraits. The archetype selection screen
does not show stat rows; starting attributes remain in the selected ROM-derived
runner record. In-game runner summary labels (race, class, posture, and stat
captions) come through Lua, while the displayed values remain native state.

There is no independent longplay capture of these exact Pocket Secretary,
Hotel, or Ares UI routes in the available reference set. The screenshot evidence
therefore establishes source-bound content, correct native state projection,
clean composition, and Debug/Release agreement only.
