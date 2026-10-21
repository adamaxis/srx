# Puyallup Ork Gang contract entry

**Status:** partial. The ordinary building route and the exact active-run
diversion are physically covered. The later gang encounter result and reward
settlement remain open.

## ROM route

Puyallup Scene 1 records 76 and 77 (`0x17D9FC` / `0x17DA06`) are distinct
collision rectangles with raw value `0x00C9`, event 9. Event-table row 9 at
`0x17D629` contains `0x0D`, so the default route selects field Scene 13.
The guide and contract-location index cross-reference the site as the Ork Gang
Hideout; the name is not stored in the collision row.

Before the normal location route resumes, ROM `0x1F086` compares the current
area/event (`FFFBF8` / `FFFBF9`) with the active gang-contract target
(`FFFBEE` / `FFFBF`). A matching target for Puyallup area 1/event 9 selects
actor list `0x1D6D4A` and resource profile `0x122054`, and opens the passage
from text `0xFE0DE` using descriptor `0xFDE26`. The queued actor descriptors
then override the generic Scene-13 table result and prepare Scene 15. The Lua
`run_gang_contract` module contains the target/event mapping and passage task;
the Ork Gang entity link retains the normal event-table route and return policy.

## Physical QA

The named Debug/Release CTest uses ordinary Up movement and a seeded run-state
fixture to exercise three source cases:

| Source case | Route result |
| --- | --- |
| Record 76, active target area 1/event 9 | Entry passage, then Scene 15 with four actors |
| Record 77, active target area 2/event 9 | No passage; row 9 loads Scene 13 |
| Record 76, no active contract | No passage; row 9 loads Scene 13 |

The matching route reached event 9 on frame 5, opened the passage on frame 22,
and reached Scene 15 after dismissal with four actors published. Each fallback
reached Scene 13 on frame 22 with no dialogue and zero funds. The test writes
approach, contact, entry-passage, destination, and route-log artifacts under
`qt_runtime/{debug,release}/field-puyallup-ork-gang-contract-entry-qa/`.
All 13 files match across configurations by SHA-256. Full 320x224 contact,
dialogue, and destination images were inspected at original resolution.

These captures validate the host route and layout only; no independent Genesis
frame for this exact Puyallup contract branch has been identified. The
record-77 exact-match branch, combat result, run completion, and reward remain
unverified.
