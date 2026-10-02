# Qt conversion visual-audit method

This is the working procedure for deciding whether a converted screen is
faithful enough to advance. It applies to the entire Qt conversion, not only
to the defects that have already been reported.

## Evidence order

Every audit item has three separate evidence channels:

1. **ROM behavior.** Identify the entry point, input/cursor state, records or
   tables read, draw calls/layer order, timing counters, and exit transition.
   The ROM is authoritative. Ghidra output is a cross-check and a place to
   annotate recovered calls; it is not a runtime specification.
2. **Independent visual reference.** Use the hand-inspected frames from
   `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`.
   Record the section, frame/range, visible selection, shell, moving layers,
   transparency, and transition state. The uploader's red opening mark is
   excluded as a game asset.
3. **Native output.** Run the deterministic Qt dump for the exact route and
   state, inspect the 320x224 image by hand, and record the artifact path.
   Internal same-source compositors are regression checks only: they can catch
   a renderer changing its own extracted assets, but they cannot prove that
   the extraction or interpretation matches Genesis.

An item is not source-verified because a unit test passes, because a named
asset exists, or because a screenshot contains some expected pixels.

## Required audit passes

Work through these passes in order. A later pass cannot hide a failure in an
earlier one.

### 1. Resource and route inventory

For each screen or shared mechanism, identify the direct assets, typed source
records, aliases, and native entry point. Reject a missing resource as a
conversion gap first; do not invent a replacement routine before rechecking
the ROM/Ghidra call path. Keep extraction sources and runtime source files
separate.

### 2. Shared shell and layer pass

Before judging text or icons, verify the frame's ownership and order:

- opaque device/menu/HUD shell survives every overlay;
- map/playfield remains visible only where the source leaves it visible;
- color-zero sprite pixels are transparent only for sprite families that the
  source writes transparently;
- plane-sized extracted overlays do not use their black canvas to erase an
  already-installed host layer;
- field HUD chrome remains opaque while the playfield remains underneath it;
- nested rows replace the correct source row instead of being drawn over a
  five-button row with a four-button row;
- vblank/clear/fade operations affect the intended surface and not the
  persistent bezel or the next scene.

The executable QA paths enforce representative shell invariants for startup,
normal field HUD, and command menus. Those invariants are deliberately small
layer checks, not a self-generated full-frame golden.

### 3. State-space pass

For every interactable, exercise the complete state sequence, recording the
typed state before and after each input:

```text
initial -> select each entry -> move across every boundary -> reselect
        -> unselect/cancel -> reopen -> confirm/deny -> return to parent
```

Repeat this for each source archetype where the records differ. For menus,
include empty and occupied cells, equipped and unequipped items, unavailable
actions, posture/status variants, and all nested submenus. A default Samurai
record is not evidence for Decker or Gator/Shaman.

### 4. Math and projection pass

For every displayed value, change or use the typed input that drives it and
check both the source-derived result and its visual projection. This includes
attack/defense/status bars, ammo rounds and clips, equipment borders, item
icons, cursor position, health/mental values, damage/fade weights, and
quantity/transfer arithmetic. Test boundaries and rejected operations, not
only a normal successful action.

### 5. Timing and transition pass

Capture the first frame, stable frame, transition frame, and return frame for
each animated route. Check source counters, sprite entry/exit, palette/fade
behavior, vblank clearing, scroll/camera placement, and the destination state.
Intro scenes are composed layers and choreographed sprites; the FFmpeg dump is
an analysis reference only and must never become a runtime bitmap flipbook.

### 6. Independent visual review

Inspect the native artifact at source size against the corresponding longplay
frames. Review the whole frame, not just the changed rectangle. Record:

- screen identity and visible choice;
- shell and boundary geometry;
- text baseline and clipping;
- sprite count, source-local bounds, alpha, and draw order;
- animation/fade phase and persistence across frames;
- camera/map/HUD relationship;
- state-specific math and selected/unselected treatment.

If the longplay is ambiguous, mark the item `needs_independent_capture`; do
not promote it to `defended` from an internal compositor.

## Status rules

The ledger in `qt-visual-audit.tsv` is the source of truth for this process.

- `pending`: ROM entry or route is known, but native reconstruction or review
  has not been completed.
- `partial`: one subpath is native, but the shared mechanism or state matrix
  is incomplete.
- `failed`: the independent review or an invariant exposed a mismatch.
- `needs_independent_capture`: source behavior is understood, but the supplied
  longplay does not establish the disputed detail.
- `defended`: ROM entry/cursor/draw/exit trace, typed native state, regression
  test, CLI image dump, and independent visual review all exist.

Only `defended` items may be used as evidence that a larger architecture hill
is complete. Updating a status requires adding the artifact command and the
reason for the decision in the ledger. Do not delete a failed observation;
resolve it with a new artifact and a new review entry.

## Current audit commands

These commands write verification artifacts outside `assets/`:

```text
shadowrun_ghidra_conversion_qt.exe --startup-qa <audit-dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-dump <file> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-hud-qa <audit-dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-command-dump <file> --field-archetype <samurai|decker|gator> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-action-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-clips-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-dialogue-dump <prompt> <file> --assets <assets>
```

The `--field-archetype` switch selects an existing startup record; it does
not create a second game state machine. It exists so visual review can prove
that the command/HUD projection follows the selected source record.
`--field-hud-qa` renders all three archetypes, verifies each frame against
its own extracted face family and HUD chrome, and fails if two archetypes
produce identical frames.

## What this prevents

This method prevents a narrow fix from being called complete when the actual
problem is shared: an opaque overlay erasing a shell, a menu tested only once,
a four-item submenu drawn over five parent buttons, a Decker/Gator record
silently rendered as Samurai, a sprite family merged into a background, a
fade affecting the wrong layer, or a runtime path that has no source-derived
record to explain it. Such a finding goes back to conversion analysis before
new behavior is invented.
