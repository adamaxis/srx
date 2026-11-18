# walkthrough_3: segment 2 Salish: FFFC1E b2, b3, b4
Worktree <WT>, build <BUILD>, tip <TIP>. Do NOT fetch (.git is read-only). Headless only.
Design: /workspace/review/walkthrough_DESIGN.md (read sections 3-4 first). Spine table is in section 3.
Start from seg1_end.json (or New Game if W2 isn't merged). ROM gates (verify):
   a. Owlfeather, trigger 2 0x53B78: b1 && !b2, loc 6/0, root 0xF4B68. It clears FFFC30 b8/b9. Cmd 0x17 at 0x218C6 gives item 0x59C34 d0=1 and sets b2 via 0x218D6 -> 0x2196C.
   b. Aragorn, trigger 3 0x53BCC: b2 && !b3, loc 3/3, needs FFFC30 b12 or FFFC38 b4/b5 (find what sets those; that's the Mitsuhama rescue). 0x53C90 sets b3, clears FFFC30 b10-12, sets b13, clears FFFC38 b4/b5, page 0xF4BBC, jsr 0x1DED8.
   c. Back to Owlfeather, trigger 4 0x53CEA: b3 && !b4, loc 6/0. Item d0=2 via 0x59C34. FFFC30 b8,9,10,13 cleared, b14 set. Page 0xF4BA0. Tail 0x53D4E sets b4.
Travel must use the real route to Salish (cab/travel). If the travel rows are MISSING, stop there and record it.
Rebuild (`cmake --build <BUILD> -j8`), then run the full suite: `. /workspace/tcenv.sh; cd <BUILD>; QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy ALSOFT_DRIVERS=null ctest -j8 --timeout 900`, and save the output to out/ctest_full.log. The baseline is 5 failures: gameplay_lua_suites (equiv_units), rom_coverage, rom_coverage_baseline_semantics, rom_byte_reachability, b3_outside_range_triage_check.
Rules: headless only. Real pad input through the runtime; never poke the flag a step is proving. Named enums/constants (E.StoryFact etc.), raw hex only in comments. A step whose output shows "warning: dialogue dead end" fails; port the missing page from the ROM instead (separate small commit) or stop the segment there and record it. Never weaken tests.
Don't commit. Write out/STATUS.md (what's green, reach line, first broken step with ROM evidence) and end it with "DONE".
Budget: about 3 hours. Ship the smallest ROM-correct slice; leave the rest in out/DIAG.md.
