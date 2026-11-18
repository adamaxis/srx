# ROM-to-runtime map coverage

Results come from `baseline_results.tsv`. Categories 7–9 are static presence checks only. MISSING rows may still reflect harness gaps (approach side, elevator selector, cave room seeding, taxi region mapping, exit-offset expectation); confirmed WRONG runtime rows have ROM-versus-port evidence in [out/REAL_FAILURES.md](../../../../../out/REAL_FAILURES.md).

## Totals by category and status

| Category | PASS | WRONG | MISSING | PRESENT-UNTESTED | Total |
|---|---:|---:|---:|---:|---:|
| edge | 14 | 0 | 0 | 0 | 14 |
| door | 38 | 0 | 0 | 1 | 39 |
| bexit | 66 | 0 | 20 | 12 | 98 |
| elevator | 176 | 0 | 0 | 0 | 176 |
| cave | 3 | 1 | 44 | 0 | 48 |
| travel | 134 | 0 | 54 | 0 | 188 |
| spawn | 0 | 0 | 0 | 1112 | 1112 |
| revent | 0 | 0 | 0 | 504 | 504 |
| trigger | 0 | 0 | 0 | 868 | 868 |
| **Total** | **431** | **1** | **118** | **2497** | **3047** |

## Per-scene status

| Scene | Name | PASS | WRONG | MISSING | PRESENT-UNTESTED |
|---:|---|---:|---:|---:|---:|
| 00 | Scene 00 | 23 | 0 | 8 | 23 |
| 01 | Scene 01 | 16 | 0 | 7 | 36 |
| 02 | Scene 02 | 14 | 0 | 8 | 43 |
| 03 | Scene 03 | 22 | 0 | 7 | 26 |
| 04 | Scene 04 | 4 | 0 | 7 | 30 |
| 05 | Scene 05 | 12 | 0 | 7 | 33 |
| 06 | Scene 06 | 6 | 0 | 7 | 28 |
| 07 | Scene 07 | 0 | 0 | 3 | 27 |
| 08 | Scene 08 | 0 | 0 | 4 | 27 |
| 09 | Scene 09 | 0 | 0 | 3 | 27 |
| 0A | Scene 0A | 4 | 0 | 0 | 44 |
| 0B | Scene 0B | 2 | 0 | 0 | 44 |
| 0C | Scene 0C | 2 | 0 | 0 | 42 |
| 0D | Scene 0D | 8 | 0 | 0 | 52 |
| 0E | Scene 0E | 0 | 0 | 0 | 43 |
| 0F | Scene 0F | 2 | 0 | 0 | 43 |
| 10 | Scene 10 | 2 | 0 | 0 | 43 |
| 11 | Scene 11 | 0 | 0 | 0 | 43 |
| 12 | Scene 12 | 2 | 0 | 0 | 43 |
| 13 | Scene 13 | 2 | 0 | 0 | 43 |
| 14 | Scene 14 | 0 | 0 | 0 | 50 |
| 15 | Scene 15 | 26 | 0 | 0 | 52 |
| 16 | Scene 16 | 14 | 0 | 0 | 56 |
| 17 | Scene 17 | 12 | 0 | 0 | 61 |
| 18 | Scene 18 | 14 | 0 | 0 | 56 |
| 19 | Scene 19 | 12 | 0 | 0 | 51 |
| 1A | Scene 1A | 28 | 0 | 0 | 56 |
| 1B | Scene 1B | 6 | 0 | 0 | 47 |
| 1C | Scene 1C | 10 | 0 | 0 | 55 |
| 1D | Scene 1D | 10 | 0 | 0 | 55 |
| 1E | Scene 1E | 16 | 0 | 0 | 50 |
| 1F | Scene 1F | 12 | 0 | 0 | 51 |
| 20 | Scene 20 | 12 | 0 | 0 | 55 |
| 21 | Scene 21 | 12 | 0 | 0 | 55 |
| 22 | Scene 22 | 12 | 0 | 0 | 52 |
| 23 | Scene 23 | 12 | 0 | 0 | 50 |
| 24 | Scene 24 | 14 | 0 | 0 | 55 |
| 25 | Scene 25 | 14 | 0 | 0 | 55 |
| 26 | Scene 26 | 16 | 0 | 0 | 58 |
| 27 | Scene 27 | 14 | 0 | 0 | 45 |
| 28 | Scene 28 | 12 | 0 | 0 | 47 |
| 29 | Scene 29 | 12 | 0 | 0 | 59 |
| 2A | Scene 2A | 10 | 0 | 0 | 62 |
| 2B | Scene 2B | 10 | 0 | 0 | 60 |
| 2C | Scene 2C | 0 | 0 | 4 | 33 |
| 2D | Scene 2D | 0 | 1 | 14 | 34 |
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
| `travel-death-s00-002aea` | 0 | travel | MISSING | dest=0, xy=0x1310,0x278, mode=0 | party-down prerequisite not reached by map input |
| `travel-taxi-r0-cursor0` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor1` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor2` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor3` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor4` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor5` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r0-cursor6` | 0 | travel | MISSING | dest=0, xy=0x2D7,0x387F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-death-s02-002b24` | 2 | travel | MISSING | event=0x2, dest=2, xy=0x3CE0,0x288C, mode=0 | party-down prerequisite not reached by map input |
| `travel-taxi-r2-cursor0` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor1` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor2` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor3` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor4` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor5` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `travel-taxi-r2-cursor6` | 2 | travel | MISSING | dest=2, xy=0x74D7,0xA7F, mode=0 | no map transition within 600 input frames;approach_sides_tried=south;approach_side=south |
| `cave-room01-east-r1b45a6` | 7 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room07-south-r1d4e5e;event=0xA;transition_fired=0;scene=45;expected_scene=7;room=7;expected_room=1;xy=0x2CFF,0x3958;expected_xy=0xB00,0x780;mode=0;expected_mode=0;probe= |
| `cave-room01-north-r1b459c` | 7 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room07-south-r1d4e5e;event=0xA;transition_fired=0;scene=45;expected_scene=7;room=7;expected_room=1;xy=0x2CFF,0x3958;expected_xy=0xB00,0x780;mode=0;expected_mode=0;probe= |
| `cave-room01-south-r1b45b0` | 7 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room07-south-r1d4e5e;event=0xA;transition_fired=0;scene=45;expected_scene=7;room=7;expected_room=1;xy=0x2CFF,0x3958;expected_xy=0xB00,0x780;mode=0;expected_mode=0;probe= |
| `cave-room02-east-r1b4dfa` | 8 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room05-south-r1d4e5e;event=0x8;transition_fired=0;scene=45;expected_scene=8;room=5;expected_room=2;xy=0x3CBA,0x3A2D;expected_xy=0x6480,0xB80;mode=0;expected_mode=0;probe=controller input did not produce the ROM event;position=0x3CBA,0x3A2D;mode=0;mode_event=0x8;collision=0x0;movement_event=0x8;movement_kind=13;route_flags=0x0;entry_route_flags=0x0;interaction_transition=0;field_pending=0;activity_timeout=0;negative_probes=2;probe_signs=-88,0,-88,0 |
| `cave-room02-north-r1b4df0` | 8 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room05-south-r1d4e5e;event=0x8;transition_fired=0;scene=45;expected_scene=8;room=5;expected_room=2;xy=0x3CBA,0x3A2D;expected_xy=0x6480,0xB80;mode=0;expected_mode=0;probe=controller input did not produce the ROM event;position=0x3CBA,0x3A2D;mode=0;mode_event=0x8;collision=0x0;movement_event=0x8;movement_kind=13;route_flags=0x0;entry_route_flags=0x0;interaction_transition=0;field_pending=0;activity_timeout=0;negative_probes=2;probe_signs=-88,0,-88,0 |
| `cave-room02-south-r1b4e04` | 8 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room05-south-r1d4e5e;event=0x8;transition_fired=0;scene=45;expected_scene=8;room=5;expected_room=2;xy=0x3CBA,0x3A2D;expected_xy=0x6480,0xB80;mode=0;expected_mode=0;probe=controller input did not produce the ROM event;position=0x3CBA,0x3A2D;mode=0;mode_event=0x8;collision=0x0;movement_event=0x8;movement_kind=13;route_flags=0x0;entry_route_flags=0x0;interaction_transition=0;field_pending=0;activity_timeout=0;negative_probes=2;probe_signs=-88,0,-88,0 |
| `cave-room02-west-r1b4e0e` | 8 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room05-south-r1d4e5e;event=0x8;transition_fired=0;scene=45;expected_scene=8;room=5;expected_room=2;xy=0x3CBA,0x3A2D;expected_xy=0x6480,0xB80;mode=0;expected_mode=0;probe=controller input did not produce the ROM event;position=0x3CBA,0x3A2D;mode=0;mode_event=0x8;collision=0x0;movement_event=0x8;movement_kind=13;route_flags=0x0;entry_route_flags=0x0;interaction_transition=0;field_pending=0;activity_timeout=0;negative_probes=2;probe_signs=-88,0,-88,0 |
| `cave-room03-east-r1b57a2` | 9 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room07-south-r1d4e5e;event=0xA;transition_fired=0;scene=45;expected_scene=7;room=7;expected_room=1;xy=0x2CFF,0x3958;expected_xy=0xB00,0x780;mode=0;expected_mode=0;probe= |
| `cave-room03-north-r1b5798` | 9 | cave | MISSING | dest=0, xy=0x0,0x0, mode=0 | route step failed at cave-room07-south-r1d4e5e;event=0xA;transition_fired=0;scene=45;expected_scene=7;room=7;expected_room=1;xy=0x2CFF,0x3958;expected_xy=0xB00,0x780;mode=0;expected_mode=0;probe= |
