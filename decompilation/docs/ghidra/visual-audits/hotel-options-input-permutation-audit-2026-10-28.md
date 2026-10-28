# Hotel Options input permutation audit

Date: 2026-10-28

## ROM path

The menu entry at `0xCF38` clears the entity-status marker, resets display
buffers, prepares the Hotel palette/grid/font, services party status once,
then draws the title and cursor at the first row. Each loop at `0xCF82..0xCF94`
waits for VBlank, latches and samples controller edges, and advances the random
grid effect before checking input.

ROM `0xCF98..0xCFC2` checks Up (bit 8) before Down (bit 9), and applies at most
one wrapped cursor step. It then checks A (bit 14) at `0xCFC6`; a fresh A edge
dispatches the currently selected row through the pointer table at `0xC14C8`
(`0xD024` for Karma Screen, `0xDA6A` for Check Out). Direction is processed
before A, so a same-sample direction+A selects the moved row. The A branch is
before the C/action test at `0xCFEA..0xCFF4`, so A+C dispatches the selected
option and skips the action service. With no A edge, C invokes `0x391A` and
keeps the menu open. The loop has no B (bit 12) or Start (bit 15) branch; those
edges do not move the cursor, open a child, or exit this menu. The source's
Up-before-Down rule matters for the single-step transition: when both are set,
exactly one cursor step occurs.

Before that loop, `0xCF38` services party status and enters the shared wait at
`0xF312`. That wait advances the lower display effect, reads `FFDD22`, masks
to the high byte, and repeats while no fresh controller edge is present. Thus
any nonzero high-byte edge acknowledges the status page; held-button bits in
the low byte alone do not. The capture stays in Scene 2/event 2 through this
page, but its unknown ROM/emulator identity makes it corroborating evidence,
not an independent Genesis reference.

## Exhaustive transition coverage

`shadowrun_ghidra_hotel_options_input_permutation` checks all 256 combinations
of Up, Down, Left, Right, A, B, C, and Start from both cursor rows (512
row/mask states), including simultaneous direction and confirmation/action
edges. It separately exercises all 256 high-byte edge masks on the preceding
Hotel status page: mask zero remains on status, every nonzero mask opens
options at Karma Screen, and low-byte held buttons without a new edge do not
advance. This checks the status transition mapping, not the wait's VBlank or
bezel cadence. It also runs the options loop through B/Left/Right/Start-only,
Down+C, and Up+Down+A+B+C+Left+Right+Start samples. This confirms
B/Left/Right/Start are ignored in the parent options loop, C invokes the
mode-action callback after movement, and A dispatches the row selected by that
same sample before C can run. The existing
`shadowrun_ghidra_conversion_qt_field_hotel_qa` supplies the full-screen route
coverage for Karma, Check Out, No/Yes, and clean field return.

## Visual cross-check

The independent Section-11 frame
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_11_source_1620-1920/frame_002580.png`
shows the `Hotel Options` title, `Karma Screen` selected above `Check Out`,
and the menu frame at 320x224. The host capture
`build/conversion/qt/release/qt_runtime/field-hotel-qa/options/actual.bmp`
shows the same title, row order, and selected row. The runner portrait and
equipment HUD differ with the seeded QA party, so this is a menu-layout/state
comparison rather than a whole-frame pixel match. The Grok-supplied
`e8_karma_spend` capture also holds this option state with no intervening
direction input; its ROM/emulator identity is unknown, so it is corroborative
capture evidence only.

## Limits

The input permutation test validates source-ordered menu transitions. It does
not prove the status wait's VBlank cadence, random bezel sequence, or Genesis
timing for every input mask. No independent longplay segment shows the B, C,
simultaneous-input, Karma
return, or checkout permutations. The Hotel hill remains partial. This audit
does not replace the source/architecture review of the existing C++ Hotel
controller described in the project instructions.
