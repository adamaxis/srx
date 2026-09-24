# Field menu visual regression contract

`shadowrun_menu_visual_regression_test` compares the native command-screen
framebuffer against a **same-source compositor** of the individually named
extracted assets. That catches renderer bugs. It does **not** defend the hill against
Genesis: a bad extraction matches itself. Treat it as `same-source` until
`assets/visual-goldens/` holds integer-scale 320x224 captures. Headless dumps
(`shadowrun_visual_dump`) and leftover-clear tests are the visual CLI.

Each exercised case writes three real framebuffer images to
`build-<config>/visual-regression/`:

- `<case>-actual.bmp`: the pixels produced by the native C++ renderer;
- `<case>-expected.bmp`: the pixels composed from extracted Genesis assets and
  source runner records; and
- `<case>-diff.bmp`: black matching pixels and red mismatches.

`manifest.tsv` in the same directory records the state that produced each
frame: stage, archetype, menu selection/row, active item, equipped weapon and
armor, posture/status indexes, combat-bar indexes, inventory item codes, and
the three corresponding image filenames. A failed test leaves these artifacts
in place, so a failure is inspectable from the rendered image and its state,
not just from a boolean assertion.

The test covers:

- the exact `0x10F50` background plus `0x116C4` inventory-box composition;
- all three one-time Joshua archetype records, including their portraits,
  class labels, active item details, and item-to-icon mappings;
- both source icon palette variants when a record selects one;
- all five `0x11454` middle-button focus blocks at their source coordinates;
- the `0xD29F6` inventory cursor in every one of the eight cells;
- the `0x11624` equipment borders independently of the inventory cursor; and
- the source command font at the runner, status, item, and combat-label
  destinations.

The test also exercises every occupied inventory slot in each startup
archetype, so item detail art and the source-selected item data are covered by
full-frame comparisons rather than isolated pixel-presence checks.

The ROM listing/bytecode call paths remain the behavioral evidence:
`0x10F50`, `0x110BC`, `0x11136`, `0x111E8`, `0x11454`, `0x11592`,
`0x11624`, and `0x12A8C`. The gameplay capture is the visual reference for
screen identity, layout, focus treatment, and the distinction between
selection cursor and equipment glow; it is not used as a lossy browser-scaled
pixel baseline.

Run it explicitly with:

```text
cmake --build --preset debug-tests --target shadowrun_menu_visual_regression_test
ctest --preset debug --output-on-failure -R shadowrun_menu_visual_regression
```

When a source resource or record mapping changes, the test fails with the
affected case, mismatch count, and artifact paths. Do not weaken it to a
single "some pixel exists" assertion or approve a new menu screenshot without
checking the corresponding source writer and extracted asset.
