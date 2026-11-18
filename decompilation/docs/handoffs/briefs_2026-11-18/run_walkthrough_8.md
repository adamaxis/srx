# walkthrough_8: segment 7 Vigore + ending: b21, b22, tomb gate, credits
Worktree <WT>, build <BUILD>, tip <TIP>. Do NOT fetch (.git is read-only). Headless only.
Design: /workspace/review/walkthrough_DESIGN.md (read sections 3-4 first). Spine table is in section 3.
1. Derive the tomb (scene 55) entry gate from the ROM. Find who loads scene 55, what it tests, and where the Licourtrix prompt 0x5BAC4 and credits 0x5C3B4 are reached (see docs/ghidra/tarislar-tomb-prelude-source-audit.md). Add the derived gate to the design spine (out/DIAG.md) with quoted bytes.
2. Trigger 21 0x5447E (loc 1/7): first entry 0xF4DF4, preset 0x54578 (FFDD64=0x15, b21). Repeat branch tests b30/b21/b16/b28/b26/b27. Preset 0x54524 sets b28. Trigger 22 0x545B6 (loc 2/7) tail 0x5463C sets b22 (Landmark Map).
3. Drive into the tomb to the credits presenter and assert the cutscene sequence starts. The tomb pre-credits page is currently an unported step (dead end): port it from the ROM if small, else stop and record.
Rebuild (`cmake --build <BUILD> -j8`), then run the full suite: `. /workspace/tcenv.sh; cd <BUILD>; QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy ALSOFT_DRIVERS=null ctest -j8 --timeout 900`, and save the output to out/ctest_full.log. The baseline is 5 failures: gameplay_lua_suites (equiv_units), rom_coverage, rom_coverage_baseline_semantics, rom_byte_reachability, b3_outside_range_triage_check.
Rules: headless only. Real pad input through the runtime; never poke the flag a step is proving. Named enums/constants (E.StoryFact etc.), raw hex only in comments. A step whose output shows "warning: dialogue dead end" fails; port the missing page from the ROM instead (separate small commit) or stop the segment there and record it. Never weaken tests.
Don't commit. Write out/STATUS.md (what's green, reach line, first broken step with ROM evidence) and end it with "DONE".
Budget: about 3 hours. Ship the smallest ROM-correct slice; leave the rest in out/DIAG.md.
