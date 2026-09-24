// Ghidra headless export
// Entry: 0x0005966E
// Name: refresh_runner_record_population
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000598B0 project_compact_runner_record


uint refresh_runner_record_population(void)

{
  uint uVar1;
  
  project_compact_runner_record();
  if (-1 < DAT_00fffc0c) {
    project_compact_runner_record();
  }
  uVar1 = (uint)DAT_00fffc0d;
  if (-1 < (char)DAT_00fffc0d) {
    uVar1 = project_compact_runner_record();
  }
  return uVar1;
}


