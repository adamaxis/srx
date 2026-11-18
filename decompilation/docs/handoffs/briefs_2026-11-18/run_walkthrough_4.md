# walkthrough_4: segment 3 Spirit Eyes: FFFC1E b5-b9 (expect to stop at the cave)
Worktree <WT>, build <BUILD>, tip <TIP>. Do NOT fetch (.git is read-only). Headless only.
Design: /workspace/review/walkthrough_DESIGN.md (read sections 3-4 first). Spine table is in section 3.
ROM 0x53D5A..0x53F8A (verify):
   b5 at 0x53D8E (FFDD64=5, page 0xF4BD8; FFFC30 b14 clr, b15/b16 set; FFFC40 b0).
   b6 at 0x53DCA needs FFFC22 b4 (Gargoyle Horn; find its pickup/drop source) and consumes it.
   b7 at 0x53E32 needs FFFC22 b3 (Hell Hound pelt) and consumes it.
   b8 at 0x53EFA..0x53F60 needs b7 && !b8 && FFFC0E != 0 && FFFC10 == 0x13 && FFFBF9 == 8. Page 0xF4C44, 0x2196C, sets FFFC22 b2. This is the Feathered Serpent cave.
   b9 at 0x53EA2 needs FFFC22 b2.
Item drops come from seeded real combat (design section 3). If the cave route is MISSING (all 48 cave map rows are), make the segment gate cover b5-b7 only, report reach at b8, and list exactly which cave exits/rows block it in DIAG.md.
Rebuild (`cmake --build <BUILD> -j8`), then run the full suite: `. /workspace/tcenv.sh; cd <BUILD>; QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy ALSOFT_DRIVERS=null ctest -j8 --timeout 900`, and save the output to out/ctest_full.log. The baseline is 5 failures: gameplay_lua_suites (equiv_units), rom_coverage, rom_coverage_baseline_semantics, rom_byte_reachability, b3_outside_range_triage_check.
Rules: headless only. Real pad input through the runtime; never poke the flag a step is proving. Named enums/constants (E.StoryFact etc.), raw hex only in comments. A step whose output shows "warning: dialogue dead end" fails; port the missing page from the ROM instead (separate small commit) or stop the segment there and record it. Never weaken tests.
Don't commit. Write out/STATUS.md (what's green, reach line, first broken step with ROM evidence) and end it with "DONE".
Budget: about 3 hours. Ship the smallest ROM-correct slice; leave the rest in out/DIAG.md.
