# Employee Recruitment generated-offer text audit (2026-10-28)

## Scope and result

Audited the generated Johnson offer page for every normal Employee Recruitment
target tuple and every client-name selection. The page previously displayed a
period and comma together after the area, such as `in Downtown Seattle., Jorge
Kish`. ROM handler `0x1E91A` replaces the suffix's final period with a comma
before appending the selected client. The Lua offer presenter now produces
`in Downtown Seattle, Jorge Kish`, matching the source transformation.

The Pocket Secretary Current Run text builder already applied this
period-to-comma rule. This fix aligns the generated offer presenter with that
builder and the shared ROM text path; it does not complete the Current Run
screen or the Recruitment hill.

## ROM and runtime data path

- ROM `0xF1E58` contains the per-area suffix strings. The strings end in a
  period, including `in Downtown Seattle.` and `in the Penumbra District.`.
- ROM `0xF1F1A` indexes the per-area location-name lists. The initializer-5
  target pool at `0xF3E6E` selects `(0,5)`, `(0,4)`, `(3,2)`, or `(3,3)`.
  Employee Recruitment initializer 11 uses the two-site pool at `0xF3E76`,
  `(1,2)` or `(3,6)`; initializer 12 uses the fixed `(5,6)` Renraku tuple.
- The type-5 offer helper at `0x1E91A` combines `0xF39BE`, the selected target
  name and area suffix, the client indexed from `0xF35A6`, `0xF39CD`, and the
  fixed follow-up appended at `0x1E9AC` from `0xF39EC`. It changes the suffix
  terminator to a comma before inserting the employee name.
- Runtime flow is `RunRegistry.present_offer_for_context()` to
  `run_employee_recruitment.present_offer()` and `game.ui.say_text()`. The
  accepted typed offer retains the selected `route.target` and `employee_name`.
  `run_support.current_run_view()` formats the related Current Run body
  separately and already replaces the terminal period.

## Target and presentation permutations

All seven tuples below were captured through their normal Recruitment offer
route. The second axis is the source table of twelve employee names at
`0xF35A6`; the pure-Lua test exercises every tuple/name combination (84 text
streams) and checks the exact forced-break markers and punctuation.

| Initializer | Area/location | ROM-backed location label | Area suffix |
|---:|---:|---|---|
| 5 | `(0,5)` | Fuchi's local office | Downtown Seattle |
| 5 | `(0,4)` | Mitsuhama Computer Technologies | Downtown Seattle |
| 5 | `(3,2)` | Fuchi Industrial Electronics | Penumbra District |
| 5 | `(3,3)` | Mitsuhama's local office | Penumbra District |
| 11 | `(1,2)` | Ares' Corporate Offices | Puyallup Barrens |
| 11 | `(3,6)` | Lone Star Security | Penumbra District |
| 12 | `(5,6)` | the Renraku Offices | Renraku Arcology |

The seven Debug and Release Qt route captures were inspected. The location,
area, employee name, and corrected comma render in the source text panel; the
long Fuchi and Mitsuhama labels remain within the panel with the source blank
row before the employer statement. These BMPs are internal Qt runtime
captures, not independent Genesis images. This audit did not identify or use
an independent Genesis capture of the Employee Recruitment offer page.

## Validation

- `lua.exe decompilation/gameplay/tests_lua/run_target_event_tests.lua` passes
  with the 84-case Recruitment text matrix and existing field target-event
  checks.
- `lua.exe decompilation/gameplay/tests_lua/run_current_run_view_tests.lua`
  passes; it covers 13 initializers and the existing Current Run text paths.
- Debug and Release each pass all seven
  `shadowrun_ghidra_conversion_qt_field_employee_recruitment_*_pedestrian_qa`
  route CTests.
- For each route, the generated `.bmp` and `.tsv` filename sets and SHA-256
  hashes match between Debug and Release.

The broader independent visual comparison, full offer/payment/action
permutations, and Recruitment completion path remain open.
