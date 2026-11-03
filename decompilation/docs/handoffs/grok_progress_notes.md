# Grok Bot progress notes (port-fixes)

This file is kept in the repo so the notes survive if the box is wiped. The box's /workspace/shadowrun/out was lost on 2026-10-31.
Tags: [C] = confirmed from the ROM or emulator, [H] = hypothesis.

## Current state (2026-11-03 08:51 CST)
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
- playtest #1: Coverage: map pre-title phase 0x5D996 and title setup 0x1D7A (#1 reveal measured). 0x5D996 phases 1/2/3 at updates 94/98/102 every 4; 0x1D7A/0x1E4E/0x1C6A 38 columns updates 91-165; longplay f337-f374
- playtest #4: Extractor: generate tail-rotor phase PNGs from ROM, with temp-dir ctest. map 0x1E87BA, palette 0x5B6B2, rotation 0x5B08E; pixel-equal to committed PNGs; extraction only into temp dir
- playtest #B2: Coverage: triage 66 unmapped routines (48 port, 16 replaced, 2 missing). camera 0x3B1E/0x3BAC, template 0x44BC, HUD 0x1728 mapped with headless samples; missing purchase handlers 0x24A1E/0x24A56
- playtest #8: AI: downed actors reach the ROM off-screen retirement check. 0x13904/0x13962 retirement before think dispatch, 0x1E514; corpses skip at 0xA91C after 0xAB02; 129/129 incl. pose/position/action guards
- playtest #PURCH: Shops: implement ROM purchase handlers 0xEB Heavy Combat Armor and 0xEC Power Focus. 0x12D060->0x24A1E 30000 item 0x32; 0x12D064->0x24A56 80000 item 0x18 phase 4 Mage/Shaman; Agira text extractor-generated; measured headless
- playtest #B3: ROM byte reachability from vectors + jump tables. Evidence [C]: rom_reachability.py over rom.bin: 9 vector roots, 4 dispatch tables (38 entries), 85,668 reachable bytes; 5,986 bytes in 39 ranges outside known functions (largest 0x0121E4-0x01253D, 858 B); lower bound, 8 unresolved indirect calls + 1 indirect jmp 0x2510A
- playtest #B3b: Reachability: dialogue table, shared dispatch, callbacks. Evidence [C]: tables 0x12D060/0x12D064, 0x24E3A, 0x214EC; 0x2510A candidates (160) unbounded, not traversed; 33,576 outside bytes in 171 ranges
- playtest #B3c: B3c: triage 171 outside-function ranges. Evidence [C]: 0x166514 ASCII via false candidate at 0x054960; dialogue bank 0x01F58A/table 0x12CCB4; open: 0x004FE2..0x005129, 0x025D0E/0x02611C
- playtest #A3: A3: emulator-captured fights asserted against Lua resolver. Evidence [C]: 64-seed emulator capture, 0x55640/0x556C6/0x55740/0x55948..0x559BC/0xA8EA; Decker/Weenr2 30/16/18, Weenr2/Yakuza 0/64/0
- playtest #22: #22: Stoker arrival ambush spawns ROM actor list. Evidence [C]: 0x249C4 roll<100, >=0x50 -> list 0x1D6E2A (0x1D66A4,0x1D665A x2) via 0x1E1FC/0x1E40C edge placement; A800 scenes 0x15-0x2B 46/46 matched
- playtest #E1: E1: effect callback timing/impact corrections per 0x46F6 table. Evidence [C]: dispatch 0x46F6/0x490A -> 0x4FE2/0x508A/0x5152/0x521E/0x52BE/0x5322/0x53C4; e1 Lua test 6/9 fail pre-fix, 9/9 after
- playtest #22: #22b: Stoker ambush uses ROM map-derived placement, emulator-captured. Evidence [C]: 0x249C4 loads D0=1 -> 0x1E216 -> 0x1E40C (port used 0x1E302, x=0x2B00); GPGX capture jh3.state 3/3 positions, 60/60 0xA8EA services
- playtest #28: #28: Jackal's Lantern booth return no longer restores stale doorway probe. Evidence [C]: Scene19 record 12 0x1D5F50 -> 0xB7B8/0x20990/0x20C78 same-map no-choice prompt; port reset player to (256,840) and froze
- playtest #28: #28c: generic same-scene prompt return keeps player in place. Evidence [C]: 0xB34/0xB52 callback return skips 0x2F32/0x2FA4; census 32 prompt routes (31 preserve, Stoker 0x173138->0x1F65A/0xBC36 restores door); also commits measure_bump_contact_rom.py (0xECCC/0x21174)
- playtest #28: #28b-2: generic bit-4 NPC contact dialogue per 0xECCC/0x21174. Evidence [C]: 0xEB92->0xECCC bit4 -> 0x1E2E0/FFE11C gate -> 0x21174 row 0xFE640; Ilene 0x17034/0x17236/0xFE680->0xFF12C->0x102424; GPGX forced contact frame1 lock59 = port delta0 lock59; [H] natural contact uncaptured
- playtest #29: docs: #29 startup pacing measured ROM-correct headless. Evidence [C]: 0x07F0->0x59C5E->0x5AF62->0x59D7C->0x5AA96->0x5AD1A->0x59D12/0x134D2; first field port 3627 vs ROM 3628, arrival text 3735 vs 3737
- playtest #28: #28b-3: ROM-generated NPC contact table; Ricky/Marrs/Ilene extraction talk. Evidence [C]: tools/enumerate_field_contacts.py --check vs ROM; 0xECCC/0x21174/0xFE640 generic, 0x54402 bit-3 extraction, 0x211E8 recruited info; GPGX forced state lock 59 = port
- playtest #29: #29b: drop frame-pacing backlog after long stalls. Evidence [C]: ROM 0x0BD2/0x075E one field pass per VBlank; injected stalls 1000/2000/3000 ms: before 154/207/254 ticks/s, after 59-62
- playtest #37: #37: common typed Lua dialogue framework; all open sites migrated. Evidence [C]: 0xB7B8 building, 0x1651E entry, 0xEB92->0xECCC contact, 0x1F65A source-door return, 0xE4C8 vid phone; census tests unchanged and green
- playtest #28: #28b-4: ROM text delay for contacts; classify 253 unresolved records. Evidence [C]: 0xEB92->0xECCC->0x21174/0xFE640, 0x1F146, 0x53C90, 0x54402, gates 0xA84C/0xA86E/0xA878; GPGX forced-state first text 22/23/24 = port; enumerator --check
- playtest #37b: #37b: dialogue framework executor drives all exits. [C] 0xB834->0xBC36/0xBD34 heading 0x09B8; Haggard 0x20FA2->0x23BB4->0x23CF2 forced GPGX r95.state; Renraku 0x2CE0->0x53EE2->0x1F246 lock 0x1E2EC; Stoker 0x173138->0x1F65A
- playtest #38: #38: generic per-actor post-attack movement lockout. [C] 15-row forced-RAM GPGX fixture (jh0.state) pistol/SMG/shotgun/spell, skill 0/8, Q/I, WR bits 12-14; 0xA8EA, 0xACAE/0xAD1A, 0xABFC, 0xD7EA/0xD81C, 0xB39A/0xB3EE->0xB574, 0xAF7C/0xAFAE/0xAFE2
- playtest #30: #30: B-button ROM audit (partial) and #38b follow-up row. [C] 0x195C/FFDD22 B=0x1000; Command 0x10EC8->0x10E4E; spell 0xFD72->0xFF5A; PocketSec 0xC2A6 no B; Notebook 0xF236; Johnsons 0xF5C2->0xF592; headless port 1-update measurements
- playtest #33: #33: posture bar selection outline via shared meter path. [C] 0x10722/0x107CE/C8600 and 0x119CE/0x11ABE/0x11AE4/CF334; headless GPGX US crops committed as QA fixtures
- playtest #30b: #30b: optional B always-cancel policy (E.MenuBPolicy) on shared Lua route. [C] audit table extended in menu_b_cancel_audit.md; menu_b_policy_tests 125/125 (43/125 pre-change); ROM-mode expectations unchanged
- playtest #31: #31: shops on shared Lua menu route; ROM shop input. [C] 0x56AC0 loop; A 0x56CE8->0x56DC4 gated FFF0D1; B 0x56D6C->0x56CF4; Start 0x56D9C; C 0x56D7C->0xD4C8; panels 0x58CDE; shop_route_tests 55/55 (34/45 pre)
- playtest #30c: #30c: remaining menus onto shared Lua menu route. [C] startup 0x59154..0x5916E; Hotel 0xCF98 no B, Check Out 0xDAF0; hospital 0xC928 no B, C 0xC9EA->0x391A, Medical 0xCA2E, Exit 0xCE7C; choices 0x1F3C4..0x1F58C; Vid-Phone 0xF90C/0xF972/0xF980
- playtest #32: #32: generic building entry/exit fade (34/34 VBlank). [C] fade-out 0x131C (17 steps x 2 0x0BD2 waits), fade-in 0x12F6 via 0x092E; GPGX jh1.state Jump House 34/34; QA callers updated to advance the measured fade
- playtest #34: #34: pedestrian gate audit, no code change. [C] 0xA800..0xA8AC gate, 0x43B6 alloc; GPGX scene 0x1A frames 1-60 no walker; port headless frame sprite 405/405 px when gated
- playtest #35: #35: SEGA logo top-down shimmer reveal. [C] 0x5D746, 0x5D938->0x5D996, 0x5DAC4->0x0D44->0x5DAFC, 0x5D8BA; GPGX/port crop px 318/317 1017/1016 1888/1983 2545/2529 2755/2755
- playtest #34b: #34b: pedestrian visibility audit, no code change. [C] 0xD5538/0x172D1C/0x67A00 default lists, 0xA800..0xA8AC gate; longplay 30fps 20/20; headless frames 18/18 visible
- playtest #35b: #35b: SEGA reveal exact row counts at 14 checkpoints. [C] 0x5D746, 0x5D938->0x5D996, 0x5DAC4->0x0D44->0x5DAFC, 0x5D8BA; GPGX phase-aligned crops 318..2755 all exact
- playtest #E1b: #E1b: GPGX-measured E1 callback branches fixture + test. [C] 0x5152/0x51E4/0x55B78, 0x52BE/0x5322/0x144CA, 0x53C4/0x5494; GPGX capture_e1_effect_callbacks.py 17-row fixture
- playtest #39: #39: inventory hover Atk/Def preview audit; add #39-#43 rows, rom_deviations.md. [C] 0x12CEE->0x12148, skill 0x12C74, Atk 0xCF3D4, Def 0xCF610; GPGX same-state + user ref, headless port frame
- playtest #40: #40: firearm main-panel max ammo vs portrait current rounds. [C] 0x12A8C draws D4540 +0x24 capacity (0xD45C4), portrait 0x36FC live +0x4E; GPGX same-state L36 11/04 fixtures + user refs 2,3

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
