# Grok Bot progress notes (port-fixes)

This file is kept in the repo so the notes survive if the box is wiped. The box's /workspace/shadowrun/out was lost on 2026-10-31.
Tags: [C] = confirmed from the ROM or emulator, [H] = hypothesis.

## Current state (2026-11-02 01:05 CST)
- Remote port-fixes: see latest Fixes entry. QA_GREEN (ctest all pass, Lua suites pass). Working playtest_2026-11-01 items one Luna run at a time.
- In progress: Luna (codex exec, gpt-6-luna, max effort, fast tier off) is fixing aa3b2cb7 forward. Brief is /workspace/briefs/aa3b.md, the earlier partial work is /workspace/briefs/aa3b_wip.diff and stash 'aa3b-wip'.
- Box toolchain: `. /workspace/tcenv.sh` (conda env /workspace/mamba/envs/srx). Build dirs are /workspace/b2 (Qt) and /workspace/b2n (non-Qt). Full QA is /workspace/qa.sh, which prints QA_GREEN.
- 7 gitignored files in decompilation/assets were dangling links into /tmp/srx_qa_asset_stage. They were replaced with real copies, not re-extracted.

## Review of fe9f16db..c6aecee3 (334 Luna commits, Oct 26-30)
- Verdict: the research and cleanup are heading the right way, but full QA stopped around Oct 29 and 91 commits were stacked on top of a regression.
- Commit themes (from subjects; they overlap): pedestrians/A800 58, hotel/doors 26, deck/Matrix 26, semantic-ID refactors 23, Karma 15, combat 14, dialogue choices 6, VDP 5, Options 4, blink/sound 3.
- Wins: ROM-traced work with addresses (for example 737beadd, A800 pedestrian flags from ROM 0x44BC). Net raw hex in Lua dropped by about 2,600 literals. kDebugMenuAlwaysUnlockedForTesting stays true. The outward exit facing (world_links.exit_bearing) is kept. Asset changes came from deliberate extraction commits [H].
- Breakages:
  - aa3b2cb7 "Use semantic IDs for item and weapon runtime data" causes 72 field ctest failures (bisected on shop, scene20 passkey and pedestrian scenes) [C].
  - E.MenuPage was stale in submenu_tests.lua (fixed in 3e11026e).
  - The movement_metric test had no asset-root define in the non-Qt build (fixed in c35bbc42).
- Concerns:
  - b478f30b moved effect spawn from Lua to C++ and deleted its Lua tests, against the Lua-core rule. Restore it or justify the move.
  - Commit bodies are empty. From now on each commit needs a one-line evidence note.
- Hotel door / #7: Luna reads the e8_karma_spend capture as the Scene 2/event 2 Stoker dialogue, not a motel load [H]. Needs the user's confirmation.

## Fixes since the review
- c35bbc42: defines SHADOWRUN_GHIDRA_ASSET_ROOT for movement_metric (ghidra_conversion/CMakeLists.txt).
- 3e11026e: the Lua suites are green. Mostly identifier/API renames in tests, plus two changes with evidence:
  - world_links_tests expects scene 0x0D = street, per ROM 0x167AE field-scene dispatch and profile 0x1B3C06 [C].
  - movement.lua resets animation_finished when a projectile restarts or stops, per ROM 0x14E32/0x14F8E.
- shop stock uses: selected_shop_item_uses reads the selected offer byte 2 instead of removed static item.detail, per ROM 0x1AAC86 [C]. Fixes field_shop_qa (72 -> 71 fails, no new).
- armor/weapon equip: Lua keeps per-cell armor markers and an explicit empty weapon state (vs unresolved semantic ID), per ROM 0x1219C/0x121B0 [C]. Fixes field_action_qa and profile_combat_bar_permutation_qa (71 -> 69, no new).
- field contact: typed field-player B7B8 mode-transition service runs before the BB86 fallback (contact 0xFFC0 regression from c073dda6, found by bisect on scene20_passkey_qa); dynamic actor package ids per visual profile. 69 -> 11 fails, no new (2 load-flaky tests pass rerun).
- A800 conditional actors get stable field_scene_<scene>_activity_<candidate> IDs before Lua publication, so evaluate_field_alarm accepts them. 9 -> 3 fails (A800 contact x3, pedestrian scenes, scene26 terminal), no new.
- D-pad diagonals: input 0x05 = north-west, 0x06 = south-west per ROM 0x19D2[5]=0xE000/[6]=0xA000 (playtest #23). Expectation change with evidence: motel collision QA idle coherence now checks the current facing (+0x2E), since ROM 0xB530 turns in place before 0xB5BA/0xB5C0 select idle from +0x2E. 3 -> 2 fails (field_movement_qa), no new.
- run offers keep the ROM location code beside the semantic location, and active_route_ids expose the selected codes from ROM 0xF3E80..0xF3EBB. Fixes field_dialogue_runtime (2 -> 1), no new.
- Mitsuhama after-hours QA route takes one more south sample: ROM 0xEBB0..0xEC44 proximity is half-open [p-0x40, p+0x40), so y=0x40C0 vs actor 0x4100 is outside. 1 -> 0 fails.
- playtest #26: Down the runner when physical or mental reaches zero. ROM 0xA912 tests +0x9C/+0x4C, 0xAA4C->0xB330 down pose, 0xABB6/0x1DDC6 party-wipe timer -> 0xC64C
- playtest #7: Decode +0x90 capability flags in player weapon delay. Evidence: ROM 0xABFC/0xD7EA/0xAD1A/0xB39A..0xB3E4, measured headless pistol gate 51 / cast 102
- playtest #25: Render ROM H-flip for left-facing enemy walk frames. Evidence: ROM 0xB6BA..0xB6DE/C062 lists 0x922C attr 0x0800; headless dir7 flip=1 matches
- playtest #14: Draw ordinary enemies behind the sidebar window. Evidence: ROM 0xD4A0 VDP window x=272, sprite priority clear at 0x481E/0x622C; headless sidebar pixel check
- playtest #9: Restore source return anchor and heading on Boris greenhouse exit. Evidence: ROM 0xBC36/0xBD4C offset (0,+0x140), 0x09B8 heading $8000; headless (0x5300,0x0740)/$8000 after fix
- playtest #9: Restore ROM return anchor and outward heading on all building exits. Evidence: ROM 0xBC36/0xBD34 offset banks, 0x09B8 heading; headless exit sweep matches
- playtest #13: Stop treating the world-activity timer as a save-blocking combat lock. Evidence: ROM 0x59092..0x59360 save gate ignores FFDF84; headless save allowed with off-screen Weenrs
- playtest #9: Route every building exit through one generic return placement. Evidence: ROM 0xBC36/0xBD34 banks + 0x09B8 heading; headless exit sweep QA 30/30, exception 0x15BDB0 documented
- playtest #20: Resume the area music after building and service exits. Evidence: ROM fade 0x53126..0x53156 then stop-same/start 0x2900..0x2916/0x530EA; headless backend sweep: track playing, atten 0 after all 30 exits
- playtest #18: Match ROM staging for unarmed and Hand Razor attacks. Evidence: ROM 0x55C76..0x55E60, 0x15C58..0x15C7E; headless 200-attack runs match ROM formula row-for-row
- playtest #17: Play the weapon action pose through instead of latching on held input. Evidence: ROM 0x784C/0x5FDC/0x600E/0xB39A, gate 0xABFC/0xD7EA; headless tap/hold: pose 0-31, neutral 32, repeat 52
- playtest #11: Non-Qt tests: ROM D-pad diagonals and StoryFact names; #11 measured ROM-correct. 0x19D2[5]=0xE000 NW,[6]=0xA000 SW (matches 0ad99223); E.StoryFlag->E.StoryFact per 7e6d767a; #11 headless 24/24 screens; qa.sh now runs non-Qt ctest 295/295
- playtest #1: Intro: ROM post-circuit title shimmer carry and cadence. 0x1D7A/0x5D8BA/0x2634, trace frames 788-789,1043-1060; title-shimmer QA 421/421; also #2 table: fades measured ROM-correct
- playtest #3: Intro: ROM opening orc palette/hit/fade timeline. 0x5A0B2/0x5A146/0x5A16E/0x5A1BE/0x5A2D0; orc timeline QA 2133/2133 px, Lua 15/15
- playtest #4: Intro: ROM chopper tail-rotor palette cycle. 0x5AD1A->0x5B528, 0x5B08E rotates pal 17-19 every 8 VBlanks; rotor QA 104px/step; Lua 27/27
- playtest #5: Intro: rooftop rotor keeps ROM palette cycle after landing. 0x5B08E/0x5B1B6/0x5B100; rooftop rotor QA 15px per 8-VBlank step; also .gitignore line for #4 fan PNGs
- playtest #6: Taxi: ROM white-on-black map caption in intro and gameplay. 0x134D2/0x13726/0xC0DA/0xEA10; caption 898 black/254 white px both paths; taxi presentation Lua 2/2
- playtest #A: Tests: ROM combat engine suite (rules, seeded fights, progression). 0x55948/0x0DBA RNG 0x0D44/0x0D3A, 0xD4540/0x166AD4, 0xD4D50; L36 sample attack 3/def 1/dmg 3 across 7 armor tiers; combat_engine 77/77, unarmed 23/23, skill policy 4270/4270, karma 22/22; no port bug found
- playtest #B: Tools: ROM coverage map, report and baseline ctest. 871/911 Ghidra functions, 1008/1074 entries mapped, 66 unmapped; gaps 0x0236/0x3BAC/0x44BC/0x1728/0x0C2C
- playtest #A2: Tests: combat suite party profile, meters, downed state, turn order. party Decker 0x72A1A vs Weenr2 0x1D66A4 seed 0x0001 dmg 1 measured; meter branch 0x55A1E; order 0xA8EA; distributions [H]

## Open work, in order
1. Get ctest green again by fixing aa3b2cb7 forward.
2. Pedestrians on almost every map: the user disputes the 7-scene default list. The ROM default lists cover scenes 0x00-0x05 and 0x14 (scene 1 at 0x17D660, scene 2 at 0x172D1C, scene 3 at 0x18A73A, scene 0x14 at 0x1D91DA, template 0x1D6988). The 0xA800 single-walker branch for scenes 0x15-0x2B is gated on FFFBED==5, FFDF40==2, FFE12C==FFF0C8 and FFFC3C==0. Look for a runtime spawn path.
3. Karma compare (au/e8_karma_spend), deck screens, the main Options screen (no recording yet).
4. Blink and sound audit, combat spacing and impact sprites, VDP lag. None of these has started.
5. Playtest backlog from 2026-10-25 (the file was lost). Done: #3, #5, #9/10, #13/14, #18, #19, #24, #26, #32, #35. Still waiting on the user's repros: #7, #17 (Windows retest), #21, #23, #25, #27, #28, #31. Most of the rest were judged ROM-correct with addresses.

## Useful ROM facts [C]
- Entry placement: FFE114 is selected at 0x169E6 and applied at 0x2B78.
- Camera: steps apply at 0x3C36; the redraw rounds down to 0x80 at 0x2BE2/0x2BEA.
- Culling at 0x5A62: window sx 0x70..0x1AD, sy 0x63..0x179.
- Give-screen picture hold at 0x12A54. Safes at 0x540D6 and 0x541B2.

## playtest_2026-11-01 status (2026-11-01 10:15 CST)
- #23 diagonals: fixed in 0ad99223 (ROM 0x19D2) [C].
- #10 vid phones, #21 Gunderson booth: likely fixed by dd41821a (field contact/B7B8 transition regression from c073dda6); vid_phone QA passes, needs an in-game retest [H].
- #9 building exit placement: not addressed.
