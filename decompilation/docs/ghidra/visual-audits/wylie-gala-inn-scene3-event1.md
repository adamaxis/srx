# Wylie's Gala Inn, scene 3/event 1

## ROM route

Scene 3 record 99 at `0x18ABB4` is Wylie's Gala Inn. Its mode-3 event value
is 1. The ordinary scene event table's row 1 at `0x18A6FC` contains scene
`0x0D`, but this is not the normal destination: the event handler at `0x1F77C`
opens root `0xF4A18`, sets the shared room charge to 50 nuyen, then returns
`D6=-1` from the field prompt. The host must keep the player on scene 3 while
the prompt runs.

Command `0xEA` at `0x249FA` checks `FF018F` bit 2. Set selects the free-stay
message at `0x114EF5`; clear selects the 50-nuyen offer at `0x114E66`. The
shared payment command `0x06` charges the current amount, selects paid or
insufficient-funds feedback, and enters Hotel Options through `0x1F816` only
after a successful payment or free stay. The signed run-result branches select
the package text roots at `0xF49E0` and `0xF4A50`.

The link profile is `gameplay/world/entities/penumbra_site_09_wylies_gala_inn/links.tsv`.
`wylie_gala_inn.lua` owns the prompt choices and results; the generic typed
building service performs the wallet transaction, and the existing Hotel
runtime presents status and options.

## Runtime evidence

`shadowrun_ghidra_conversion_qt_field_wylie_gala_inn_qa` uses ordinary Up input
from south of record 99. It verifies the source collision record and event,
opens the two-choice inn prompt without leaving scene 3, selects the 50-nuyen
room, verifies the wallet goes from 50 to 0, acknowledges the paid message,
and captures the Hotel status page followed by the selected Hotel Options
page. The images are full 320x224 captures:

- Release: `build/conversion/qt/release/qt_runtime/field-wylie-gala-inn-qa/`
- Debug: `build/conversion/qt/debug/qt_runtime/field-wylie-gala-inn-qa/`

The Lua suite also checks Leave, insufficient funds, free stay from runner
status bit 2, unknown restored status, and both signed run-result text branches.
The paid physical route and the shared Hotel menu are internal runtime
regression evidence. No matching independent Genesis capture of this inn
interaction has been identified, so the visual hill remains partial.

## Open coverage

The other scene-3 building services and their story/run-gated branches remain
separate routes. This QA does not establish that all buildings have complete
entry transitions, nor does it cover every wallet, restored-save, or run-result
state through physical Qt input.
