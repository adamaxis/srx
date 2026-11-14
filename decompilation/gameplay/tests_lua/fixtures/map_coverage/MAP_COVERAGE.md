# ROM-to-runtime map coverage

Results come from `baseline_results.tsv`. Categories 7–9 are static presence checks only. Most MISSING/WRONG rows are known harness gaps (approach side, elevator selector, cave room seeding, taxi region mapping, exit-offset expectation), not confirmed port bugs; see the mapharness briefs.

## Totals by category and status

| Category | PASS | WRONG | MISSING | PRESENT-UNTESTED | Total |
|---|---:|---:|---:|---:|---:|
| edge | 13 | 0 | 1 | 0 | 14 |
| door | 6 | 3 | 30 | 0 | 39 |
| bexit | 0 | 10 | 88 | 0 | 98 |
| elevator | 0 | 0 | 176 | 0 | 176 |
| cave | 0 | 0 | 48 | 0 | 48 |
| travel | 8 | 0 | 180 | 0 | 188 |
| spawn | 0 | 0 | 0 | 1112 | 1112 |
| revent | 0 | 0 | 0 | 504 | 504 |
| trigger | 0 | 0 | 0 | 868 | 868 |
| **Total** | **27** | **13** | **523** | **2484** | **3047** |

## Per-scene status

| Scene | Name | PASS | WRONG | MISSING | PRESENT-UNTESTED |
|---:|---|---:|---:|---:|---:|
| 00 | Scene 00 | 6 | 0 | 25 | 23 |
| 01 | Scene 01 | 7 | 1 | 15 | 36 |
| 02 | Scene 02 | 3 | 2 | 17 | 43 |
| 03 | Scene 03 | 6 | 0 | 23 | 26 |
| 04 | Scene 04 | 0 | 0 | 12 | 29 |
| 05 | Scene 05 | 5 | 0 | 14 | 33 |
| 06 | Scene 06 | 0 | 0 | 13 | 28 |
| 07 | Scene 07 | 0 | 0 | 3 | 27 |
| 08 | Scene 08 | 0 | 0 | 4 | 27 |
| 09 | Scene 09 | 0 | 0 | 3 | 27 |
| 0A | Scene 0A | 0 | 0 | 4 | 44 |
| 0B | Scene 0B | 0 | 0 | 2 | 44 |
| 0C | Scene 0C | 0 | 0 | 2 | 42 |
| 0D | Scene 0D | 0 | 0 | 16 | 44 |
| 0E | Scene 0E | 0 | 0 | 0 | 43 |
| 0F | Scene 0F | 0 | 2 | 0 | 43 |
| 10 | Scene 10 | 0 | 0 | 2 | 43 |
| 11 | Scene 11 | 0 | 0 | 0 | 43 |
| 12 | Scene 12 | 0 | 0 | 2 | 43 |
| 13 | Scene 13 | 0 | 2 | 0 | 43 |
| 14 | Scene 14 | 0 | 0 | 0 | 50 |
| 15 | Scene 15 | 0 | 0 | 26 | 52 |
| 16 | Scene 16 | 0 | 0 | 14 | 56 |
| 17 | Scene 17 | 0 | 0 | 12 | 61 |
| 18 | Scene 18 | 0 | 0 | 14 | 56 |
| 19 | Scene 19 | 0 | 0 | 12 | 51 |
| 1A | Scene 1A | 0 | 4 | 28 | 52 |
| 1B | Scene 1B | 0 | 0 | 6 | 47 |
| 1C | Scene 1C | 0 | 0 | 10 | 55 |
| 1D | Scene 1D | 0 | 0 | 10 | 55 |
| 1E | Scene 1E | 0 | 0 | 16 | 50 |
| 1F | Scene 1F | 0 | 0 | 12 | 51 |
| 20 | Scene 20 | 0 | 0 | 12 | 55 |
| 21 | Scene 21 | 0 | 0 | 12 | 55 |
| 22 | Scene 22 | 0 | 0 | 12 | 52 |
| 23 | Scene 23 | 0 | 2 | 10 | 50 |
| 24 | Scene 24 | 0 | 0 | 14 | 55 |
| 25 | Scene 25 | 0 | 0 | 14 | 55 |
| 26 | Scene 26 | 0 | 0 | 16 | 58 |
| 27 | Scene 27 | 0 | 0 | 14 | 45 |
| 28 | Scene 28 | 0 | 0 | 12 | 47 |
| 29 | Scene 29 | 0 | 0 | 12 | 59 |
| 2A | Scene 2A | 0 | 0 | 10 | 62 |
| 2B | Scene 2B | 0 | 0 | 10 | 60 |
| 2C | Scene 2C | 0 | 0 | 4 | 33 |
| 2D | Scene 2D | 0 | 0 | 15 | 34 |
| 2E | Scene 2E | 0 | 0 | 3 | 32 |
| 2F | Scene 2F | 0 | 0 | 4 | 46 |
| 30 | Scene 30 | 0 | 0 | 6 | 33 |
| 31 | Scene 31 | 0 | 0 | 3 | 32 |
| 32 | Scene 32 | 0 | 0 | 4 | 33 |
| 33 | Scene 33 | 0 | 0 | 8 | 43 |
| 34 | Scene 34 | 0 | 0 | 6 | 43 |
| 35 | Scene 35 | 0 | 0 | 2 | 43 |
| 36 | Scene 36 | 0 | 0 | 0 | 48 |
| 37 | Scene 37 | 0 | 0 | 0 | 44 |

## 25 highest-priority WRONG or MISSING rows

| Row | Scene | Category | Status | Observed | Diff |
|---|---:|---|---|---|---|
| `edge-s00-r0d57fa-camXlt0x1400` | 0 | edge | MISSING | dest=0, xy=0xFF,0x2780, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d5796` | 0 | door | MISSING | dest=0, xy=0x1F80,0x6FF, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57aa` | 0 | door | MISSING | dest=0, xy=0x6980,0x6FF, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57b4` | 0 | door | MISSING | dest=0, xy=0x10FF,0x2780, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57be` | 0 | door | MISSING | dest=0, xy=0x3F80,0x28FF, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57c8` | 0 | door | MISSING | dest=0, xy=0x4FF,0x3580, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57d2` | 0 | door | MISSING | dest=0, xy=0x24FF,0x3580, mode=0 | controller input did not produce the ROM event |
| `door-s00-r0d57dc` | 0 | door | MISSING | dest=0, xy=0x4580,0x36FF, mode=0 | controller input did not produce the ROM event |
| `door-s02-r1730fc` | 2 | door | WRONG | event=0x4, dest=2, xy=0x1CFF,0xA57, mode=0 | dest expected=13 observed=2;mode expected=1 observed=0; |
| `door-s02-r173110` | 2 | door | WRONG | event=0x6, dest=2, xy=0x52FF,0x657, mode=0 | dest expected=26 observed=2;mode expected=2 observed=0; |
| `door-s02-r17311a` | 2 | door | MISSING | dest=2, xy=0x6F80,0x8FF, mode=0 | controller input did not produce the ROM event |
| `door-s02-r173156` | 2 | door | MISSING | dest=2, xy=0x6780,0x48FF, mode=0 | controller input did not produce the ROM event |
| `door-s02-r173160` | 2 | door | MISSING | dest=2, xy=0x44FF,0x4980, mode=0 | controller input did not produce the ROM event |
| `door-s02-r17316a` | 2 | door | MISSING | dest=2, xy=0x2EFF,0x4980, mode=0 | controller input did not produce the ROM event |
| `door-s02-r173174` | 2 | door | MISSING | dest=2, xy=0x1C7F,0x44FF, mode=0 | controller input did not produce the ROM event |
| `travel-death-s02-002b24` | 2 | travel | MISSING | event=0x2, dest=2, xy=0x3CE0,0x288C, mode=0 | party-down prerequisite not reached by map input |
| `cave-room01-east-r1b45a6` | 7 | cave | MISSING | dest=7, xy=0xFF,0x5380, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room01-north-r1b459c` | 7 | cave | MISSING | event=0x8, dest=7, xy=0xAFF,0x66F, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room01-south-r1b45b0` | 7 | cave | MISSING | event=0xA, dest=7, xy=0x7580,0x4AD7, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room02-east-r1b4dfa` | 8 | cave | MISSING | dest=8, xy=0x6380,0x8FF, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room02-north-r1b4df0` | 8 | cave | MISSING | event=0x8, dest=8, xy=0xEFF,0x66F, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room02-south-r1b4e04` | 8 | cave | MISSING | dest=8, xy=0xFF,0x7180, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room02-west-r1b4e0e` | 8 | cave | MISSING | event=0xB, dest=8, xy=0x7580,0x70D7, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room03-east-r1b57a2` | 9 | cave | MISSING | dest=9, xy=0x6180,0x8FF, mode=3 | cave room gate not established by a natural incoming route |
| `cave-room03-north-r1b5798` | 9 | cave | MISSING | dest=9, xy=0xFF,0x6580, mode=3 | cave room gate not established by a natural incoming route |
