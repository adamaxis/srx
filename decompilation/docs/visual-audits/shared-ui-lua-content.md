# Shared UI Lua content review

Scope: explicit Lua text keys for the shared Yes/No confirmation choices and
the shared field-stat captions. This is content wiring only. Native presenters
still own layout, focus, selection, timing, sound requests, and effects.

## Nine-question visual review

1. **Does it look correct?** The full-screen inventory discard/reload and Ares
   shop confirmation captures show readable text and the cursor on the selected
   row. The command overview shows the field-stat captions in their existing
   positions. The Hotel pages use a black base and the source 28x21 grid; the
   small marks near the lower edge are from its random-grid effect, not stale
   buttons.
2. **Does the look match behavioral state?** Lua returns stable `yes` / `no`
   IDs and explicit label keys in that order. Existing native state still
   selects the initial row and changes it on input. Script QA checks the exact
   ID/key/value tuples for both confirmation rows and all eleven field metrics.
3. **Are the visuals correct?** This change does not move, recolor, or redraw
   controls. The field text catalog resolves the keys to the imported ROM text;
   script and catalog QA check exact values. This verifies the binding, not
   independent Genesis pixel fidelity.
4. **Should it animate?** The labels are static. Confirmation selection changes
   only through the native input path. Hotel's separate random-grid animation
   advances once per VBlank while its menu is open; the Hotel Karma child pauses
   that parent animation.
5. **Is there sound?** The label lookup adds no sound. Native menu movement and
   action requests remain mapped to events `0x25` and `0x24` (2.134 s and
   2.484 s decoded resources, respectively). Exact-route Windows loopback is
   still not recorded.
6. **Are the rest of the screens clean?** Full-screen captures include the
   inventory shell, command HUD, Hotel pages, and Ares shop. Hotel painting
   begins with a full 320x224 black clear before drawing its page and grid;
   returned child pages are checked against their parent composition. No old
   button row is visible beneath these tested pages.
7. **Do multiple frames support the interpretation?** Debug and Release
   produce matching filenames and SHA-256 hashes for 55 field-action frames,
   252 Hotel frames, 21 Ares shop frames, and the command overview frame (329
   full-screen captures total). All route contact sheets were reviewed, with
   native-size checks of representative confirmation, Hotel, shop, and command
   frames. The supplied longplay does not provide an exact independent capture
   of these confirmation sequences, so it is not used as a visual golden.
8. **Does the new UI replace the old UI cleanly?** This Lua-only binding adds
   no controls. The Hotel presenter clears the full screen before each page,
   and the tested inventory/shop child routes return without their controls
   left under the parent page.
9. **Was the complete affected interval reviewed?** The full Debug/Release QA
   routes were rerun for script runtime, field action, command, Hotel, Ares
   shop, and field text catalog. Each passed 6/6 in both configurations. Every
   emitted full-screen route frame was reviewed in contact sheets; the
   representative states listed above were also opened at native size.

## Limits

The independent visual source remains the supplied longplay, and it contains no
matching confirmation sequence for this slice. These checks establish stable
script content and internal route consistency; they do not claim independent
Genesis visual verification. The exact-route speaker output also remains
unrecorded.
