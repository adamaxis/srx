# Field menu visual regression contract

`shadowrun_menu_visual_regression_test` is the pixel-level visual gate for the
field command screen. It is intentionally independent of the current BMP
diagnostic outputs: each expected region is selected from the extracted
Genesis resource or from the selected `0x7291A`/`0x72A1A`/`0x72B1A` runner
record.

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

When a source resource or record mapping changes, the test should fail with
the affected region and mismatch count. Do not weaken it to a single
"some pixel exists" assertion or approve a new menu screenshot without
checking the corresponding source writer and extracted asset.
